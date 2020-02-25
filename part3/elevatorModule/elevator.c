#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
MODULE_LICENSE("GPL");

extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
    printk(KERN_NOTICE "start_elevator called\n");
    return 0;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {
    printk(KERN_NOTICE "stop_elevator called\n");
    return 0;
}

extern long (*STUB_issue_request)(int, int, int, int);
long issue_request(int num_pets, int pet_type, int start_floor, int destination_floor) {
    printk(KERN_NOTICE "issue_request called\nnum pets: %ld\npet type: %ld\nstart floor: %ld\ndestination floor %ld", 
        num_pets, pet_type, start_floor, destination_floor);
    return 0;
}


static int elevator_init(void){
    STUB_start_elevator = start_elevator;
    STUB_stop_elevator = stop_elevator;
    STUB_issue_request = issue_request;
    return 0;
}
module_init(elevator_init);

static void elevator_exit(void){
    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
    STUB_issue_request = NULL;
}
module_exit(elevator_exit);