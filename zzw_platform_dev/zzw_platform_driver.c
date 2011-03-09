#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/device.h>

#define ZZW_MAX_MEM 100
#define ZZW_MAJOR	250
#define DRIVER_NAME "zzw_platform"

static int zzw_major = ZZW_MAJOR;

struct zzw_dev {
	struct cdev cdev;
	unsigned char mem[ZZW_MAX_MEM];
};

struct zzw_dev *zzw_devp;

int zzw_dev_open(struct inode *inode, struct file *filp)
{
	filp->private_data = zzw_devp;
	return 0;
}

int zzw_dev_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t zzw_dev_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;

	struct zzw_dev *dev = filp->private_data;

	if (p >= ZZW_MAX_MEM)
		return - EFAULT;
	if (count > ZZW_MAX_MEM - p)
		count = ZZW_MAX_MEM - p;

	if (copy_to_user(buf, (void *)dev->mem + p, count))
		return - EFAULT;
	else {
		*ppos += count;
		ret = count;
	}

	return ret;
}

static ssize_t zzw_dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;

	struct zzw_dev *dev = filp->private_data;

	if (p >= ZZW_MAX_MEM)
		return - EFAULT;
	if (count > ZZW_MAX_MEM - p)
		count = ZZW_MAX_MEM - p;

	if (copy_from_user(dev->mem + p, buf, count))
		return - EFAULT;
	else {
		*ppos += count;
		ret = count;
	}

	return ret;
}

static const struct file_operations zzw_dev_fops = {
	.owner = THIS_MODULE,
	.write = zzw_dev_write,
	.read = zzw_dev_read,
//	.llseek = NULL,
//	.ioctl = NULL,
	.open = zzw_dev_open,
	.release = zzw_dev_release,
};

static void zzw_dev_cdev_setup(struct zzw_dev *dev, int index)
{
	int err;
	dev_t cdev_no = MKDEV(zzw_major, 0);

	cdev_init(&dev->cdev, &zzw_dev_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, cdev_no, 1);
	if(err)
		 printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

static int zzw_platform_driver_probe(struct platform_device *dev)
{
	int result;
	dev_t cdev_no = MKDEV(zzw_major, 0);

	if (zzw_major)
		result = register_chrdev_region(cdev_no, 1, "zzw_dev");
	else {
		result = alloc_chrdev_region(&cdev_no, 0, 1, "zzw_dev");
		zzw_major = MAJOR(cdev_no);
	}

	if (result < 0)
		return result;

	zzw_devp = kmalloc(sizeof(struct zzw_dev), GFP_KERNEL);
	if (!zzw_devp) {
		result = - ENOMEM;
		goto fail_malloc;
	}

	memset(zzw_devp, 0, sizeof(struct zzw_dev));

	zzw_dev_cdev_setup(zzw_devp, 0);

	printk(DRIVER_NAME "zzw platform driver & device matched!\n");
	return result;

fail_malloc:
	unregister_chrdev_region(cdev_no, 1);
	return result;
}

static int zzw_platform_driver_remove(struct platform_device *dev)
{
	cdev_del(&zzw_devp->cdev);
	kfree(zzw_devp);
	unregister_chrdev_region(MKDEV(zzw_major, 0), 1);

	printk(KERN_INFO "Hello zzw removed!\n");
	return 0;
}


struct platform_driver zzw_platform_driver = {
	.probe = zzw_platform_driver_probe,
	.remove = zzw_platform_driver_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static ssize_t zzw_platform_store(struct device_driver *drv, const char *buf, size_t count)
{
	memset(zzw_devp->mem,0,ZZW_MAX_MEM);
	strncpy(zzw_devp->mem, buf, count-1);
	return count;
}

static ssize_t zzw_platform_show(struct device_driver *drv, char *buf)
{
	return sprintf(buf, "zzw_platform device have data: \"%s\" !\n", zzw_devp->mem);
}

static DRIVER_ATTR(data, S_IRUGO|S_IWUSR, zzw_platform_show, zzw_platform_store);

static int __init zzw_platform_driver_init(void)
{
	int ret = 0;

	ret =  platform_driver_register(&zzw_platform_driver);
	if (ret)
		return ret;

	driver_create_file(&(zzw_platform_driver.driver), &driver_attr_data);

	return ret;
}

static void __exit zzw_platform_driver_exit(void)
{
	platform_driver_unregister(&zzw_platform_driver);
}

module_init(zzw_platform_driver_init);
module_exit(zzw_platform_driver_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Joewen Chow <joewen.chow@gmail.com>");
