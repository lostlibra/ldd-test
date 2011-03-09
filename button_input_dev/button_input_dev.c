#include <linux/input.h> 
#include <linux/module.h> 
#include <linux/init.h>
#include <asm/irq.h> 
#include <asm/io.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/gpio.h>


#define KEY_TIMER_DELAY1    (HZ/50)             //按键按下去抖延时20毫秒        
#define KEY_TIMER_DELAY2    (HZ/10)             //按键抬起去抖延时100毫秒
#define BUTTON_DOWN            1                   //按键按下                    
#define BUTTON_UP              0                   //按键抬起  
#define BUTTON_UNCERTAIN          2                   //按键未确定     
#define BUTTON_COUNT           6                   //6个按键    


/*定义中断所用的结构体*/
struct button_irq_desc {
	int irq; //按键对应的中断号
	int pin; //按键所对应的GPIO 端口
	volatile int value; //按键当前的值
	int keycode; //定义键码
	char *name; //每个按键的名称
	volatile int keystatus; //按键状态
	int num;    //按键号
};

/*结构体实体定义*/
static struct button_irq_desc button_irqs [] = {
	{IRQ_EINT8 , S3C2410_GPG(0), 0, KEY_0, "KEY0", BUTTON_UP, 0},
	{IRQ_EINT11, S3C2410_GPG(3), 0, KEY_1, "KEY1", BUTTON_UP, 1},
	{IRQ_EINT13, S3C2410_GPG(5), 0, KEY_2, "KEY2", BUTTON_UP, 2},
	{IRQ_EINT14, S3C2410_GPG(6), 0, KEY_3, "KEY3", BUTTON_UP, 3},
	{IRQ_EINT15, S3C2410_GPG(7), 0, KEY_4, "KEY4", BUTTON_UP, 4},
	{IRQ_EINT19, S3C2410_GPG(11), 0, KEY_5, "KEY5", BUTTON_UP, 5},
};

static struct input_dev *button_dev;
static struct timer_list key_timers[BUTTON_COUNT]; //定义6个按键去抖动定时器


/*本按键驱动的中断服务程序*/
static irqreturn_t buttons_interrupt(int irq, void *dev_id)
{
	struct button_irq_desc *button_irqs = (struct button_irq_desc *)dev_id;

	if(button_irqs->keystatus == BUTTON_UP) {
		
		button_irqs->keystatus = BUTTON_UNCERTAIN;

		key_timers[button_irqs->num].expires = jiffies + KEY_TIMER_DELAY1;
        add_timer(&key_timers[button_irqs->num]);
		
	}

	return IRQ_RETVAL(IRQ_HANDLED);
}


static void buttons_timer(unsigned long arg)
{
	//获取当前按键资源的索引
	int button = arg;
	//struct button_irq_desc *button_irqs = （struct button_irq_desc *）arg；

	//获取当前按键引脚上的电平值来判断按键是按下还是抬起
	int volatile down = !s3c2410_gpio_getpin(button_irqs[button].pin);

	if(down)//低电平，按键按下
	{
			//标识当前按键状态为按下
			if(button_irqs[button].keystatus == BUTTON_UNCERTAIN) {
				
				button_irqs[button].keystatus = BUTTON_DOWN;

				//标识当前按键已按下并唤醒等待队列
				button_irqs[button].value = down;
		
				input_report_key(button_dev, button_irqs[button].keycode, button_irqs[button].value);
 				input_sync(button_dev);
			}

		//设置当前按键抬起去抖定时器的延时并启动定时器
		key_timers[button].expires = jiffies + KEY_TIMER_DELAY2;
		add_timer(&key_timers[button]);
	}
	else//高电平，按键抬起
	{
		button_irqs[button].value = down;
		input_report_key(button_dev, button_irqs[button].keycode, button_irqs[button].value);
 		input_sync(button_dev);
		//标识当前按键状态为抬起
		button_irqs[button].keystatus = BUTTON_UP;
	}
}



/*设备初始化，主要是注册设备*/
static int __init dev_init(void)
{
	int error;
	int key;
	int i;

	for (key = 0; key < BUTTON_COUNT; key++) {
		/*注册中断函数*/
		error = request_irq(button_irqs[key].irq, buttons_interrupt, IRQ_TYPE_EDGE_BOTH,
		button_irqs[key].name, (void *)&button_irqs[key]);
		
		setup_timer(&key_timers[key], buttons_timer, key);

		if (error)
			goto err_free_irq;
	}

	button_dev = input_allocate_device();
	if (!button_dev) {
		printk(KERN_ERR "button.c: Not enough memory\n");
		goto err_free_dev;
	}

	//button_dev->evbit[0] = BIT_MASK(EV_KEY);
	set_bit(EV_KEY,button_dev->evbit);
	for(i = 0; i < BUTTON_COUNT; i++) {
		//button_dev->keybit[BIT_WORD(button_irqs[i].keycode)] = BIT_MASK(BTN_0);	
		set_bit(button_irqs[i].keycode,button_dev->keybit);
	}

	button_dev->name = "micro2440 button";

	error = input_register_device(button_dev);
	if (error) {
		printk(KERN_ERR "button.c: Failed to register device\n");
		goto err_unregister_dev;
	}

	return 0;

err_unregister_dev:
	input_unregister_device(button_dev);
err_free_dev:
	input_free_device(button_dev);
err_free_irq:
	for(key--; key >=0; key--) {
		free_irq(button_irqs[key].irq, (void *)&button_irqs[key]);
	}
	return error;
}

/*注销设备*/
static void __exit dev_exit(void)
{
	int i;

	for (i = 0; i < BUTTON_COUNT; i++) {
		free_irq(button_irqs[i].irq, (void *)&button_irqs[i]);
	}

	input_unregister_device(button_dev);
	input_free_device(button_dev);	

}

module_init(dev_init); //模块初始化，仅当使用insmod/podprobe 命令加载时有用，如果设备不是通过模块方式加载，此处将不会被调用
module_exit(dev_exit); //卸载模块，当该设备通过模块方式加载后，可以通过rmmod 命令卸载，将调用此函数

MODULE_LICENSE("GPL");//版权信息
MODULE_AUTHOR("ZZW");//作者名字
