#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

//mini_controller and vehicle structs
//you may make changes if necessary

typedef struct mini_controller_object
{
    int id;
    int time_green;
    int min_interval;
} mini_cntrl_t;

typedef struct vehicle_object
{
    int id;
    char direction[4];
} vehicle_t;

void *mini_controller_routine(void *);
void *vehicle_routine(void *);

//declare global mutex and condition variables
pthread_mutex_t intersection_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ns_cond, ew_cond, right_cond;
// you need to add something here

int main(int argc, char **argv)
{
    int n_vehicles;   //total number of vehicles
    int vehicle_rate; //vehicle arrival rate to the intersection
    int min_interval; // min time interval between two consecutive vehicles to pass the intersection

    //more variable declarations
    pthread_t *mini_controller_thrds_id;
    pthread_t *vehicle_thrds_id;
    mini_cntrl_t *mini_controller;
    vehicle_t *vehicle;
    int rc, g_t;
    // you need to add something here

    // Initialize mutex and condition variables
    rc = pthread_cond_init(&ns_cond, NULL);
    if (rc)
    {
        printf("Error in creation of ns cond, %d\n", rc);
        exit(1);
    }
    rc = pthread_cond_init(&ew_cond, NULL);
    if (rc)
    {
        printf("Error in creation of ew cond, %d\n", rc);
        exit(1);
    }
    rc = pthread_cond_init(&right_cond, NULL);
    if (rc)
    {
        printf("Error in creation of right cond, %d\n", rc);
        exit(1);
    }

    // you need to add something here if necessary

    //allocate memory for mini_controllers
    mini_controller_thrds_id = malloc(3 * sizeof(pthread_t)); //system thread ids
    if (mini_controller_thrds_id == NULL)
    {
        fprintf(stderr, "mini_controlle_thrds_id out of memory\n");
        exit(1);
    }
    mini_controller = malloc(3 * sizeof(mini_cntrl_t)); //mini_controller objects
    if (mini_controller == NULL)
    {
        fprintf(stderr, "mini_controller out of memory\n");
        exit(1);
    }

    // input parameters
    // ask for the total number of vehicles.
    printf("Enter the total number of vehicles (int): ");
    scanf("%d", &n_vehicles);

    //ask for vehicles' arrival rate to the intersection
    printf("Enter vehicles arrival rate (int): \n");
    scanf("%d", &vehicle_rate);

    //ask for the minimum time interval t (in seconds) between any two
    //consecutive vehicles in one direction to pass through the intersection
    printf("Enter minimum interval between two consecutive vehicles (int): \n");
    scanf("%d", &min_interval);

    //ask for green time for each mini_controller
    printf("Enter green time for forward-moving vehicles on trunk road (int): ");
    scanf("%d", &g_t);
    mini_controller[0].time_green = g_t;
    printf("Enter green time for vehicles on minor road (int): ");
    scanf("%d", &g_t);
    mini_controller[1].time_green = g_t;
    printf("Enter green time for right-turning vehicles on trunk road (int): ");
    scanf("%d", &g_t);
    mini_controller[2].time_green = g_t;

    //allocate memory for vehicles
    vehicle = malloc(n_vehicles * sizeof(vehicle_t));
    if (vehicle == NULL)
    {
        fprintf(stderr, "memory error in vehicles");
        exit(1);
    }
    vehicle_thrds_id = malloc(n_vehicles * sizeof(pthread_t));
    if (vehicle_thrds_id == NULL)
    {
        fprintf(stderr, "mem error\n");
        exit(1);
    }
    // you need to add something here

    //create mini_controller threads
    for (int k = 0; k < 3; k++)
    {
        mini_controller[k].id = k;
        rc = pthread_create(&mini_controller_thrds_id[k], NULL, mini_controller_routine, (void *)&mini_controller[k]);
        if (rc)
        {
            fprintf(stderr, "rc from mc thread: %d\n", rc);
            exit(1);
        }
    }
    // you need to add something here

    //create vehicles threads
    srand(time(0));
    for (int k = 0; k < n_vehicles; k++)
    {
        sleep((int)rand() % vehicle_rate);
        vehicle[k].id = k;
        int direction = (int)rand() % 6;
        if (direction == 0)
        {
            strcpy(vehicle[k].direction, "n2s");
        }
        else if (direction == 1)
        {
            strcpy(vehicle[k].direction, "s2n");
        }
        else if (direction == 2)
        {
            strcpy(vehicle[k].direction, "w2e");
        }
        else if (direction == 3)
        {
            strcpy(vehicle[k].direction, "e2w");
        }
        else if (direction == 4)
        {
            strcpy(vehicle[k].direction, "s2w");
        }
        else if (direction == 5)
        {
            strcpy(vehicle[k].direction, "w2n");
        }
        rc = pthread_create(&vehicle_thrds_id[k], NULL, vehicle_routine, (void *)&vehicle[k]);
        if (rc)
        {
            fprintf(stderr, "rc from vh thread: %d\n", rc);
            exit(1);
        }
    }
    // you need to add something here

    //join and terminating threads.
    for (int k = 0; k < n_vehicles; k++)
    {
        pthread_join(vehicle_thrds_id[k], NULL);
    }
    for (int k = 0; k < 3; k++)
    {
        pthread_cancel(mini_controller_thrds_id[k]);
    }
    // you need to add something here

    //deallocate allocated memory
    free(vehicle_thrds_id);
    free(mini_controller_thrds_id);
    free(vehicle);
    free(mini_controller);

    // you need to add something here

    //destroy mutex and condition variable objects
    pthread_mutex_destroy(&intersection_mutex);
    pthread_cond_destroy(&ns_cond);
    pthread_cond_destroy(&ew_cond);

    pthread_cond_destroy(&right_cond);

    // you need to add something here

    printf("Main thread: There are no more vehicles to serve. The simulation will end now.\n");

    exit(0);
}

void *mini_controller_routine(void *arg)
{

    // you need to implement mini_controller routine
    mini_cntrl_t *mini_controller;
    mini_controller = (mini_cntrl_t *)arg;
    printf("MC %d: Start.\n", mini_controller->id);
    while (1)
    {
        if (mini_controller->id == 1)
        {
            pthread_mutex_lock(&intersection_mutex);
            pthread_cond_wait(&ew_cond, &intersection_mutex);
        }
        else if (mini_controller->id == 2)
        {
            pthread_mutex_lock(&intersection_mutex);
            pthread_cond_wait(&right_cond, &intersection_mutex);
        }

        printf("MC %d: Will change to green now.\n", mini_controller->id);
        pthread_mutex_unlock(&intersection_mutex);
        sleep(mini_controller->time_green);
        printf("MC %d: Will change to red now.\n", mini_controller->id);
        sleep(2);

        pthread_mutex_lock(&intersection_mutex);
        if (mini_controller->id == 0)
        {
            pthread_cond_signal(&ew_cond);
            pthread_mutex_unlock(&intersection_mutex);
            pthread_mutex_lock(&intersection_mutex);
            pthread_cond_wait(&ns_cond, &intersection_mutex);
        }
        else if (mini_controller->id == 1)
        {
            pthread_cond_signal(&right_cond);
            pthread_mutex_unlock(&intersection_mutex);
        }
        else
        {
            pthread_cond_signal(&ns_cond);
            pthread_mutex_unlock(&intersection_mutex);
        }
    }
}

void *vehicle_routine(void *arg)
{
    // you need to implement vehicle routine
    vehicle_t *vehicle;
    vehicle = (vehicle_t *)arg;
    printf("Vehicle %d %s has arrived at the intersection.\n", vehicle->id, vehicle->direction);
    if (!strcmp(vehicle->direction, "n2s"))
    {
    }
    printf("Vehicle %d %s is proceeding through the intersection.\n", vehicle->id, vehicle->direction);
}
