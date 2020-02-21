#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.v> //file system calls
#include <linux/uaccess.h> //memory copy from kernel <-> userspace

MODULE_LICENSE("Dual BSD/GPL");

#define BUF_LEN 100 //max length of read/write message
static struct proc_dir_entry* proc_entry; //pointer to proc entry

static struct file_operations fileOps = {
    .owner = THIS_MODULE;
    .read = proc_read;
    .write = proc_write;
};

static char msg[BUF_LEN]; //buffer to store read/write message
static int procfs_buf_len; //variable to hold length of message

static struct Time {
    long seconds;
    long nanoseconds;
};

static Time time;


static ssize_t proc_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk(KERN_INFO "proc_write\n");
    //write min(user message size, buffer length) characters
    if (count > BUF_LEN)
        procfs_buf_len = BUF_LEN;
    else
        procfs_buf_len = count;
    
    copy_from_user(msg, ubuf, procfs_buf_len);
    printk(KERN_INFO "got from user: %s\n", msg);
    
    return procfs_buf_len;
}
 
static ssize_t proc_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk(KERN_INFO "proc_read\n");
    procfs_buf_len = strlen(msg);
    
    if (*ppos > 0 || count < procfs_buf_len)    //check if data already read and if space in user buffer
        return 0;
    
    if (copy_to_user(ubuf, msg, procfs_buf_len))    //send data to user buffer
        return -EFAULT;
    
    *ppos = procfs_buf_len;//update position
    
    printk(KERN_INFO "gave to user %s\n", msg);
    
    return procfs_buf_len;     //return number of characters read
}

static int timer_init(void)
{
    printk(KERN_ALERT "Hello, timer!\n");

    //proc_create(filename, permissions, parent, pointer to file ops)
    proc_entry = proc_create("timed", 0666, NULL, &fileOps);

    if (proc_entry == NULL)
        return -ENOMEM;
    
    return 0;
}

static void timer_exit(void)
{
    printk(KERN_ALERT "Goodbye, timer!\n");
    proc_remove(proc_entry);
    return;
}

module_init(timer_init);
module_exit(timer_exit);