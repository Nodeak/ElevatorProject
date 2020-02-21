#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h> //file system calls
#include <linux/uaccess.h> //memory copy from kernel <-> userspace
#include <linux/time.h> //for timespec

MODULE_LICENSE("Dual BSD/GPL");

#define BUF_LEN 100 //max length of read/write message
#define MAX_STRING 256
static struct proc_dir_entry* proc_entry; //pointer to proc entry



// static char msg[BUF_LEN]; //buffer to store read/write message
static int procfs_buf_len; //variable to hold length of message


//timespec data type

/*
timespec t1;
t1 = current_kernel_time();
timespec elapsed = timespec_sub (t1, t2);

sprintf() - format to string

kfree - anything allocated w kmalloc

in read:
check if it is first time, if not, also print elapsed time

*/

struct timespec currentTime;
struct timespec lastTime;
struct timespec elapsedTime;
static char* msg;
static char* elapMsg;

static int first = 1;

 
static ssize_t proc_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	printk(KERN_INFO "proc_read called\n");

    msg = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | _GFP_IO | _GFP_FS);
    elapMsg = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | _GFP_IO | _GFP_FS);

    currentTime = current_kernel_time();

    sprintf(msg, "Current time: %ld.%09ld\n", currentTime.tv_sec, currentTime.tv_nsec);

    if (!first){
        elapsedTime = timespec_sub(last, current);
        sprintf(elapMsg, "Elapsed time: %%ld.%09ld\n", elapsedTime.tv_sec, elapsedTime.tv_nsec);
        strcat(msg, elapMsg);
    }
    
    lastTime = currentTime;

    procfs_buf_len = strlen(msg);
    if (*ppos > 0 || count < procfs_buf_len)    //check if data already read and if space in user buffer
        return 0;

    if (copy_to_user(ubuf, msg, procfs_buf_len))    //send data to user buffer
        return -EFAULT;
    
    *ppos = procfs_buf_len; //update position
    
    printk(KERN_INFO "gave to user %s\n", msg);
    
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
    printk(KERN_ALERT "exiting my_timer!\n");

    //deallocate memory
    kfree(msg);
    kfree(elapMsg);

    //remove proc entry
    proc_remove(proc_entry);

    return;
}



module_init(timer_init);
module_exit(timer_exit);


/*

part 3:
set up kernel module
    kernel set up - download source code
    sys calls - modify files and add new ones

hour to compile
proc entry

spawn thread in init
up and down all

linked list needs to be deallocated w kfree

*/