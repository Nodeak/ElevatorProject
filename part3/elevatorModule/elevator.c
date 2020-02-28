/*
* Group 13: Kaedon Hamm & Kiara Boone
*
* COP4610 Project 2 - Due March 8, 2020
* Elevator Module that is to be implemented into Linux Kernel v4.19
* 
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/kthread.h>  // Threading for kernel
#include <linux/proc_fs.h>  // File system calls
#include <linux/uaccess.h>  // Memory copy from kernel <-> userspace
#include <linux/time.h>     // For timespec
#include <linux/slab.h>     // Memory allocation functions
#include <linux/printk.h>   // printk() - prints to Kernel
#include <linux/list.h>     // Linked lists
#include <linux/string.h> 
#include <linux/delay.h> // Used for the ssleep() function.


MODULE_LICENSE("GPL");

/* Define different states as integers - eases code readability */
#define IDLE 0
#define UP 1
#define DOWN 2
#define LOADING 3
#define OFFLINE 4

/* Define animal types as integers - eases code readability */
#define CAT 0
#define DOG 1
#define NONE 2

/* Proc Creation */
#define MAX_STRING 1024
#define BUF_LEN 1024 //max length of read/write message

static struct proc_dir_entry* proc_entry; //pointer to proc entry
static int procfs_buf_len; //variable to hold length of message

static char*  msg;

/* Thread creation */
struct task_struct * elev_thread;

/* Struct for Queue and ElevatorFloor */
struct list_head floors[10];        // Array of 10 linked lists representing each floor
struct list_head elev_passengers;   // Linked list representing elevator passengers

struct Person{                      // Hold important information about Passengers
    struct list_head list;          // Give the struct a specific address. Needed to be used in a Linked List
    int floor_dest;
    int pet_type;
    int group_size;
    int weight;
};


/* Defining global variables */
static int elev_state;
static int temp_state;
static int current_floor;       // Keeps track where elevator is at
static int elev_weight;         // Weight includes both Person and pet
static int num_passengers;      // Passenger count includes both Person and pets
static int animal_type;         // Animal types can only be on elevator w/ same type
static int num_waiting;
static int num_serviced;


/* Sampe Proc Entry
Elevator state: UP
Elevator animals: dog
Current floor: 4
Number of passengers: 6
Current weight: 14
Number of passengers waiting: 34
Number passengers serviced: 61
[ ] Floor 10: 11 | o o o | x x x | o o
[ ] Floor 9: 0
[ ] Floor 8: 5 | o | x x
[ ] Floor 7: 0
[ ] Floor 6: 0
[ ] Floor 5: 0
[*] Floor 4: 8 | x x x | x x x
[ ] Floor 3: 10 | x | x x x | x x x
[ ] Floor 2: 0
[ ] Floor 1: 0
*/

static ssize_t proc_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
    int i;
    int j;
    char * state;
    char * ani_type;
    char * floor_string;
    struct list_head *temp;
    struct Person* passenger;

    printk(KERN_INFO "proc_read called\n");

    msg = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    state = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    ani_type = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    floor_string = kmalloc(sizeof(char) * MAX_STRING, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
   
    if (elev_state == IDLE){
        state = "IDLE";
    } else if (elev_state == OFFLINE){
        state = "OFFLINE";
    } else if (elev_state == UP){
        state = "UP";
    } else if (elev_state == DOWN){
        state = "DOWN";        
    } else if (elev_state == LOADING){
        state = "LOADING";
    }

    if (animal_type == DOG){
        ani_type = "dog";
    } else if (animal_type == CAT){
        ani_type = "cat";
    } else if (animal_type == NONE){
        ani_type = "none";
    }

    sprintf(msg, "Elevator State: %s\nElevator Animals: %s\nCurrent Floor: %d\nNumber of Passengers: %d\nCurrent Weight: %d\nNumber of Passengers Waiting: %d\nNumber of Passengers Serviced: %d\n",
        state, ani_type, current_floor, num_passengers, elev_weight, num_waiting, num_serviced);
    
    // | - person
    // x - dog
    // o - cat
    for (i = 9; i >= 0; i--) {
        if (current_floor == (i+1)){
            strcat(msg, "[*]");
        } else {    
            strcat(msg, "[ ]");
        }
        // strcat(msg, "Floor %d: %d", (i+1) );
        sprintf(floor_string, "Floor %d:", (i+1));
        strcat(msg, floor_string);

        list_for_each(temp, &floors[i]){
            passenger = list_entry(temp, struct Person, list);
            strcat(msg, "| ");
            if (passenger->pet_type == DOG){
                for(j = 0; j < (passenger->group_size - 1);j++){
                    strcat(msg, "x ");
                }
            } else if (animal_type == CAT){
                for(j = 0; j < (passenger->group_size - 1);j++){
                    strcat(msg, "o ");
                }
            }
        }
        strcat(msg, "\n");

    }


    procfs_buf_len = strlen(msg);
    if (*ppos > 0 || count < procfs_buf_len)    // Check if data already read and if space in user buffer
        return 0;

    if (copy_to_user(ubuf, msg, procfs_buf_len))    // Send data to user buffer
        return -EFAULT;
    
    *ppos = procfs_buf_len;     // Update position
    printk(KERN_INFO "Sent to user %s\n", msg);
    
    return procfs_buf_len;     //return number of characters read
}



/* start_elevator is called when STUB_start_elevator is requested, 
        Sets up a new elevator with its default settings */
extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
    printk(KERN_NOTICE "start_elevator called\n");
    if (elev_state != OFFLINE){
        return 1;
    }
    elev_state = IDLE;
    return 0;
}


/* stop_elevator is called when STUB_stop_elevator is requested,
        Turns the elevator state to OFFLINE, but not exit the program 
        
        NEEDS:
            To drop rest of passengers still on elevator off before changing state */
extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {
    printk(KERN_NOTICE "stop_elevator called\n");
    elev_state = OFFLINE;
    // DROP REST OF PASSENGERS OFF
    return 0;
}

/* issue_request is called when STUB_issue_request is requested,
        Creates a completed Person struct and adds them to that floor's queue */
extern long (*STUB_issue_request)(int, int, int, int);
long issue_request(int num_pets, int pet_type, int start_floor, int destination_floor) {
    int tot_weight;
    struct Person * passenger;
    printk(KERN_NOTICE "issue_request called\nnum pets: %d\npet type: %d\nstart floor: %d\ndestination floor %d", 
        num_pets, pet_type, start_floor, destination_floor);
    passenger = kmalloc(sizeof(struct Person), __GFP_RECLAIM);

    /* Validate passengers */
    if ((start_floor < 1) | (start_floor > 10)){
        return 1;
    } 
    if ((destination_floor < 1) | (destination_floor > 10)){
        return 1;
    }
    if ((num_pets < 0) | (num_pets > 3)){
        return 1;
    }
    if ((pet_type != 1) && (pet_type != 2)){
        return 1;
    }


    /* Create Person */
    tot_weight = 3; // Add weight of one person
    if (pet_type == CAT){
        // 1 weight unit
        tot_weight += num_pets;
    } else if (pet_type == DOG){
        // 2 weight units
        tot_weight += num_pets * 2;
    }

    // Inserting info about Person into the struct
    passenger->weight = tot_weight;
    passenger->floor_dest = destination_floor;
    passenger->pet_type = pet_type;
    passenger->group_size = num_pets + 1;

    num_waiting += passenger->group_size;

    // Put passengers on start floor
    list_add_tail(&passenger->list, &floors[start_floor-1]);

    return 0;
}


/* In-house functions */
void checkLoad(int floor){

    struct Person * curr_passenger;
    struct Person * person_del;
    bool loading = true;
    printk(KERN_ALERT "entered checkLoad\n");

    while(loading){
        // Check first person
        printk(KERN_ALERT "Retrieving first person\n");
        curr_passenger = list_first_entry(&floors[floor-1], struct Person, list);
        if (curr_passenger != NULL){
            printk(KERN_ALERT "Checking first person (if any)\n");
            if (curr_passenger->weight + elev_weight <= 15 && ((curr_passenger->pet_type == animal_type) | (curr_passenger->pet_type == NONE))){
                //If can load,
                    // Remove from floors
                    printk(KERN_ALERT "Removing passengers from floor\n");
                    person_del = list_entry(&floors[floor-1], struct Person, list);
                    list_del(&floors[floor-1]);
                    // Add Person to elev_passengers
                    printk(KERN_ALERT "Adding passengers to elevator\n");
                    list_add_tail(&curr_passenger->list, &elev_passengers);
            } else {
                // If cant load, stop loading
                printk(KERN_ALERT "Setting loading to false\n");
                loading = false;
            }
        } else {
            // If no passengers,
            loading = false;
        }
    }

    return;
}


void checkUnload(int floor){

    // Temporary pointers
    struct list_head *temp;
    struct Person * passenger;
    printk(KERN_ALERT "entered checkUnload\n");

    // Iterate through elev_passengers, storing ptr for each Person strcut in temp. Idk what dummy does.
    list_for_each(temp, &elev_passengers) {
        passenger = list_entry(temp, struct Person, list);
        // Unloads passengers from the elevator
        if(passenger->floor_dest == current_floor){
            elev_weight -= passenger->weight;              // Remove weight from elevator
            num_passengers -= passenger->group_size;    // Remove passengers from elevator
            num_serviced -= passenger->group_size;        // Includes pets also
            list_del(temp);                                             // Remove from linked list
            kfree(passenger);                                           // Deallocate passenger created from issue_request
        }
    }  
    // Allow any passenger w/ any pet type to get on next checkLoad
    if(num_passengers == 0){
        animal_type = NONE;
    }

    return;
}

/* Function to check if there are any waiting passengers on any floor
    Mainly used to kick elevator from IDLE state */
int isWaitingAll(void){
    int i;
    for(i = 0; i < 10; i++){
        if(list_empty(&floors[i]) == 0){
            return 1;
        }
    }
    return 0;
}

/* Function to check if there are any waiting passengers on a certain floor */
int isWaitingOne(int floor){
    if(list_empty(&floors[floor-1]) == 0){
        return 1;
    }
    return 0;
}

// Will run on own thread. Loops until elevator_exit()'s kthread_stop() is called
int runElevator(void *data){
    printk(KERN_ALERT "entered runElevator\n");
    while(!kthread_should_stop()){
       
        switch(elev_state){
            case OFFLINE:
                break;
            case IDLE:
                if (isWaitingOne(current_floor)){
                    temp_state = UP;
                    elev_state = LOADING;
                }
                if(isWaitingAll()){
                    elev_state = UP;
                }
                break;
            case UP:
                if (current_floor < 10){
                    current_floor++;
                } else if (current_floor == 10){
                    elev_state = DOWN;
                    current_floor--;
                }
                ssleep(2);

                if (isWaitingOne(current_floor)){
                    temp_state = elev_state;
                    elev_state = LOADING;
                }
                break;
            case DOWN:
                if (current_floor > 1){
                    current_floor--;
                } else if (current_floor == 1){
                    elev_state = UP;
                    current_floor++;
                }
                ssleep(2);

                if (isWaitingOne(current_floor)){
                    temp_state = elev_state;
                    elev_state = LOADING;
                }
                break;
            case LOADING:
                ssleep(1);
                checkLoad(current_floor);
                if (num_passengers > 0){
                    checkUnload(current_floor);
                }
                elev_state = temp_state;
                break;
        }
    }
    return 0;
}


static struct file_operations fileOps = {
    .owner = THIS_MODULE,
    .read = proc_read,
};

/* Init and Exit functions */
static int elevator_init(void){
    int i;

    printk(KERN_ALERT "elevator initializing\n");


    printk("Initializing system calls\n");
    STUB_start_elevator = start_elevator;
    STUB_stop_elevator = stop_elevator;
    STUB_issue_request = issue_request;


    printk("Initializing elevator variables\n");
    elev_state = OFFLINE;
    elev_weight = 0;
    num_passengers = 0;
    current_floor = 1;

    animal_type = NONE;
    num_waiting = 0;
    num_serviced = 0;

    printk("Initializing queues\n");
    // Initialize the queues
    for (i = 0; i < 10; i++){
        INIT_LIST_HEAD(&floors[i]);     // Initialize Linked List for each floor
    }
    INIT_LIST_HEAD(&elev_passengers);   // Initialize elevator passenger Linked List

    // Run thread
    printk("Running thread\n");
    elev_thread = kthread_run(runElevator, NULL, "Elevator Thread");
    if(IS_ERR(elev_thread)) {
      printk("Error: runElevator\n");
      return PTR_ERR(elev_thread);
    }

    printk("Creating proc entry\n");
    //proc_create(filename, permissions, parent, pointer to file ops)
    proc_entry = proc_create("elevator", 0666, NULL, &fileOps);
    //if error creating proc entry
    if (proc_entry == NULL)
        return -ENOMEM;

    return 0;
}


static void elevator_exit(void){
    int c;
    printk(KERN_ALERT "elevator exiting\n");

    printk("Removing system calls\n");
    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
    STUB_issue_request = NULL;

    
    // Stop thread
    printk("Stopping main thread\n");
    c = kthread_stop(elev_thread);
    if(c != -EINTR) {
      printk("Elevator thread stopped...\n");
    }

    // Clean up proc entry
    printk("Removing proc entry\n");
    kfree(msg);
    proc_remove(proc_entry);
    printk("Proc entry removed\n");

    return;
}

module_init(elevator_init);
module_exit(elevator_exit);
