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

/* Struct for Queue and ElevatorFloor */
struct list_head floors[10];    // Array of 10 linked lists

struct Person{
    struct list_head list;
    int floor_dest;
    int pet_type;
    int num_pets;
    int weight;
}

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
static int num_animals;
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
    num_animals = 0;
    num_waiting = 0;
    num_serviced = 0;
    
    // Call main thread
    return 0;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {
    printk(KERN_NOTICE "stop_elevator called\n");
    elev_state = OFFLINE;
    kthread_stop();
    // DROP REST OF PASSENGERS OFF

    return 0;
}

extern long (*STUB_issue_request)(int, int, int, int);
long issue_request(int num_pets, int pet_type, int start_floor, int destination_floor) {
    printk(KERN_NOTICE "issue_request called\nnum pets: %ld\npet type: %ld\nstart floor: %ld\ndestination floor %ld", 
        num_pets, pet_type, start_floor, destination_floor);




    return 0;
}


/* In house functions */

int runElevator(){
    while(!kthread_should_stop()){
        int check_floors = checkFloors();

        if (elev_state == IDLE && check_floors != -1){
            elev_state = UP;
        }

        

        //load and/or unload passengers

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
            elev_state = IDLE;
        }
    }
}

void checkLoad(int floor){
    struct list_head * temp;
    Person * curr_passenger;
    bool loading = true;

    while(loading){
        //check first person
        curr_passenger = list_first_entry(floors[floor-1], Person, list);
        if (curr_passenger.weight + elev_weight <= 15 && (curr_passenger.pet_type == animal_type | curr_passenger.pet_type == NONE)){
            //if can load,
                //remove pointer from floors[floor]
                list_add_tail(&curr_passenger.list, &elev_passengers);
                //add to elev_pass
                //deallocate from floors

        }
            //if cant load, set loading to false
            
    }

    // Person curr_passenger = list_first_entry(floors[floor-1]);
    // if (curr_passenger.weight + elev_weight <= 15 && curr_passenger.pet_type == animal_type){
    //     //load onto elevator (put into elev_passengers)
    //     //delete Person from floors[floor-1]
    // }
}

void checkUnload(int floor){

}

int checkFloors(){
    int i;
    for(i = 0; i < 10; i++){
        if(list_empty(floors[i] != 0){
            return i;
        }
    }
    return -1;
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

    return 0;
}
module_init(elevator_init);

static void elevator_exit(void){
    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
    STUB_issue_request = NULL;
}
module_exit(elevator_exit);