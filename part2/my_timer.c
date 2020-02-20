#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int timer_init(void)
{
printk(KERN_ALERT "Hello, timer!\n");
return 0;
}

static void timer_exit(void)
{
printk(KERN_ALERT "Goodbye, timer!\n");
return;
}
module_init(timer_init);
module_exit(timer_exit);