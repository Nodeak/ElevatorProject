#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/printk.h>   // printk() - prints to Kernel
#include <linux/list.h>     // Linked lists
MODULE_LICENSE("GPL");


/* #define different states as integers */
#define IDLE 0
#define UP 1
#define DOWN 2
#define LOADING 3
#define OFFLINE 4

#define CAT 0
#define DOG 1
#define NONE 2

/* Thread creation */

struct task_struct * thread;

/* Struct for Queue and ElevatorFloor */
struct list_head floors[10];    // Array of 10 linked lists

struct Person{
    struct list_head list;
    int floor_dest;
    int pet_type;
    int group_size;
    int weight;
};

struct list_head elev_passengers;



/* Defining global variables 
    Elevator Linked List,
    Elevator State,
    Floor Linked List,
    Current Floor,
    Weight,
    Number of Passengers,
    Number of Animals,
    Animal Type,
    Number of Passengers Waiting,
    Number of Passengers Serviced
*/

static int elev_state;
static int current_floor;
static int elev_weight;
static int num_passengers;
static int animal_type;
static int num_waiting;
static int num_serviced;



/* Functions called by STUBS  */
extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
    printk(KERN_NOTICE "start_elevator called\n");

    elev_state = IDLE;
    elev_weight = 0;
    num_passengers = 0;
    current_floor = 1;

    animal_type = NONE;
    num_waiting = 0;
    num_serviced = 0;
    
    return 0;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {
    printk(KERN_NOTICE "stop_elevator called\n");
    elev_state = OFFLINE;
    // DROP REST OF PASSENGERS OFF

    return 0;
}

extern long (*STUB_issue_request)(int, int, int, int);
long issue_request(int num_pets, int pet_type, int start_floor, int destination_floor) {
    printk(KERN_NOTICE "issue_request called\nnum pets: %d\npet type: %d\nstart floor: %d\ndestination floor %d", 
        num_pets, pet_type, start_floor, destination_floor);




    return 0;
}

void checkLoad(int floor){
    struct list_head * temp;
    struct Person * curr_passenger;
    bool loading = true;

    while(loading){
        // Check first person
        curr_passenger = list_first_entry(floors[floor-1], Person, list);
        if (curr_passenger.weight + elev_weight <= 15 && (curr_passenger.pet_type == animal_type | curr_passenger.pet_type == NONE)){
            //If can load,
                // Add Person to elev_passengers
                list_add_tail(&curr_passenger->list, &elev_passengers);
                // Remove from floors
                struct Person * person = list_entry(floors[floor-1], Person, list);
                list_del(person);

        } else {
            // If cant load, stop loading
            loading = false;
        }
    }
}


void checkUnload(int floor){

    //temporary pointers
    struct list_head *temp;
    struct list_head *dummy;
    struct Person* passenger;

    // Iterate through elev_passengers, storing ptr for each Person strcut in temp. Idk what dummy does.
    list_for_each(temp, dummy, &elev_passengers) {
        passenger = list_entry(temp, Person, list);
        // Unloads passengers from the elevator
        if(passenger->floor_dest == current_floor){
            elev_weight = elev_weight - passenger->weight;              // Remove weight from elevator
            num_passengers = num_passengers - passenger->group_size;    // Remove passengers from elevator
            num_serviced = num_serviced + passenger->group_size;        // Includes pets also
            list_del(temp);                                             // Remove from linked list
            kfree(passenger);                                           // Deallocate passenger
        }
    }  
    // Allow any passenger w/ any pet type to get on next checkLoad
    if(num_passengers == 0){
        animal_type = NONE;
    }

}

int checkFloors(void){
    int i;
    for(i = 0; i < 10; i++){
        if(list_empty(floors[i] != 0){
            return i;
        }
    }
    return -1;
}

/* In house functions */

int runElevator(void){
    while(!kthread_should_stop(thread)){
        int check_floors = checkFloors();
        
        // Check if waiting passengers
        if (elev_state == IDLE && check_floors != -1){
            elev_state = UP;
        }

        // Load and/or unload passengers
        checkLoad(current_floor);

        // Check if waiting passengers after load/unload
        check_floors = checkFloors();

        if(elev_state == UP && current_floor < 10){
            //elevator goes up
            current_floor++;
        } else if (elev_state == UP && current_floor == 10){
            //elevator now goes down
            elev_state = DOWN;
            current_floor--;
        } else if (elev_state == DOWN && current_floor > 1){
            //elevator goes down
            current_floor--;
        } else if (elev_state == DOWN && current_floor == 1){
            //elevator goes down
            elev_state = UP;
            current_floor++;
        } else if (num_passengers == 0 && check_floors == -1){
            // If no passengers and no people waiting
            elev_state = IDLE;
        }
    }
    return 0;
}




/* Init and Exit functions */
static int elevator_init(void){
    STUB_start_elevator = start_elevator;
    STUB_stop_elevator = stop_elevator;
    STUB_issue_request = issue_request;

    // Initialize the queues
    int i;
    for (i = 0; i < 10; i++){
        INIT_LIST_HEAD(&floors[i]);
    }
    INIT_LIST_HEAD(&elev_passengers);

    // Run thread
    thread = kthread_run(runElevator, NULL, "elevator");

    return 0;
}
module_init(elevator_init);

static void elevator_exit(void){
    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
    STUB_issue_request = NULL;

    // Stop thread
    kthread_stop(thread);
}
module_exit(elevator_exit);