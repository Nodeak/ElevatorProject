#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linuz/syscalls.h>

/* start elevator stub */
// system call stub
long(*STUB_start_elevator)(void) == NULL;
EXPORT_SYMBOL(STUB_start_elevator);

// system call wrapper
SYSCALL_DEFINE0(start_elevator, void) {
    printk(KERN_NOTICE "Inside SYSCALL DEFINE1 block, start_elevator\n");
    if(STUB_start_elevator != NULL)
        return STUB_start_elevator(void);
    else
        return -ENOSYS;
}

/* stop elevator stub */
long(*STUB_stop_elevator)(void) == NULL;
EXPORT_SYMBOL(STUB_stop_elevator);

SYSCALL_DEFINE0(stop_elevator, void) {
    printk(KERN_NOTICE "Inside SYSCALL DEFINE1 block, stop_elevator\n");
    if(STUB_stop_elevator != NULL)
        return STUB_stop_elevator(void);
    else
        return -ENOSYS;
}

/* issue request stub*/
long(*STUB_issue_request)(int, int, int, int) == NULL;
EXPORT_SYMBOL(STUB_issue_request);

SYSCALL_DEFINE4(issue_request, int, num_pets, int, pet_type, int, start_floor, int, destination_floor) {
    printk(KERN_NOTICE "Inside SYSCALL DEFINE1 block, issue_request\n");
    if(STUB_issue_request != NULL)
        return STUB_issue_request(num_pets, pet_type, start_floor, destination_floor);
    else
        return -ENOSYS;
}