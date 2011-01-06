#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/string.h>

#define DEVICE_NAME "zzw_platform"

MODULE_AUTHOR("Joewen Chow <joewen.chow@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");

static struct platform_device zzw_pdev = {
	.name = "zzw_platform",
	.id = -1,
};

//static struct platform_device *zzw_pdev;

static int __init zzw_platform_dev_init(void)
{
	int ret = 0;

	//zzw_pdev = platform_device_alloc(DEVICE_NAME, -1);

	ret = platform_device_register(&zzw_pdev);

	/*if (ret)
		platform_device_put(zzw_pdev);*/

	return ret;
}

static void __exit zzw_platform_dev_exit(void)
{
	platform_device_unregister(&zzw_pdev);
}

module_init(zzw_platform_dev_init);
module_exit(zzw_platform_dev_exit);
