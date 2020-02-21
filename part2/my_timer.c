#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h> //file system calls
#include <linux/uaccess.h> //memory copy from kernel <-> userspace
#include <linux/time.h> //for timespec
#include <linux/slab.h> //memory allocation functions
#include <linux/uaccess.h>
#include <linux/string.h> 

MODULE_LICENSE("Dual BSD/GPL");

#define BUF_LEN 100 //max length of read/write message
#define MAX_STRING 256

static struct proc_dir_entry* proc_entry; //pointer to proc entry
static int procfs_buf_len; //variable to hold length of message


struct timespec currentTime;
struct timespec lastTime;
struct timespec elapsedTime;
static char* msg;
static char* elapMsg;

static int first = 1;

 
static ssize_t proc_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk(KERN_INFO "proc_read called\n");

    msg = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    elapMsg = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | __GFP_IO | __GFP_FS);

    currentTime = current_kernel_time();

    sprintf(msg, "Current time: %ld.%09ld\n", currentTime.tv_sec, currentTime.tv_nsec);

    if (first){
        first = 0;
    } else {
        elapsedTime = timespec_sub(currentTime, lastTime);
        sprintf(elapMsg, "Elapsed time: %ld.%09ld\n", elapsedTime.tv_sec, elapsedTime.tv_nsec);
        strcat(msg, elapMsg);
    }
    
    lastTime = currentTime;

    procfs_buf_len = strlen(msg);
    if (*ppos > 0 || count < procfs_buf_len)    //check if data already read and if space in user buffer
        return 0;

    if (copy_to_user(ubuf, msg, procfs_buf_len))    //send data to user buffer
        return -EFAULT;
    
    *ppos = procfs_buf_len; //update position
    printk(KERN_INFO "Sent to user %s\n", msg);
    
    return procfs_buf_len;     //return number of characters read
}

static struct file_operations fileOps = {
    .owner = THIS_MODULE,
    .read = proc_read,
};

static int timer_init(void)
{
    printk(KERN_ALERT "my_timer initialized!\n");

    //proc_create(filename, permissions, parent, pointer to file ops)
    proc_entry = proc_create("timed", 0666, NULL, &fileOps);

    //if error creating proc entry
    if (proc_entry == NULL)
        return -ENOMEM;
    
    return 0;
}

static void timer_exit(void)
{
    printk(KERN_ALERT "Exiting my_timer!\n");

    //deallocate memory
    kfree(msg);
    kfree(elapMsg);

    //remove proc entry
    proc_remove(proc_entry);

    return;
}

module_init(timer_init);
module_exit(timer_exit);