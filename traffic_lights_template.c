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
    char direction[11];
} mini_cntrl_t;

typedef struct vehicle_object
{
    int id;
    char direction[4];
    int min_interval;
} vehicle_t;

void *mini_controller_routine(void *);
void *vehicle_routine(void *);

//declare global mutex and condition variables
pthread_mutex_t intersection_mutex, n2s_mutex, s2n_mutex, e2w_mutex, w2e_mutex, n2w_mutex, s2e_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ns_cntrl_cond, ew_cntrl_cond, right_cntrl_cond;
pthread_cond_t n2s_vhcl_cond, e2w_vhcl_cond, n2w_vhcl_cond;
pthread_cond_t s2n_vhcl_cond, w2e_vhcl_cond, s2e_vhcl_cond;
bool n2s, s2n, e2w, w2e, n2w, s2e = false;

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
    rc = pthread_cond_init(&ns_cntrl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of ns mini controller cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&ew_cntrl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of ew mini controller cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&right_cntrl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of right mini controller cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&e2w_vhcl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of e2w vehicle cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&n2w_vhcl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of n2w vehicle cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&n2s_vhcl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of n2s vehicle cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&s2n_vhcl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of s2n vehicle cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&w2e_vhcl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of w2e vehicle cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = pthread_cond_init(&s2e_vhcl_cond, NULL);
    if (rc)
    {
        printf("Error in creation of s2e vehicle cond, %d\n", rc);
        exit(EXIT_FAILURE);
    }
    // you need to add something here if necessary

    //allocate memory for mini_controllers
    mini_controller_thrds_id = malloc(3 * sizeof(pthread_t)); //system thread ids
    if (mini_controller_thrds_id == NULL)
    {
        fprintf(stderr, "mini_controlle_thrds_id out of memory\n");
        exit(EXIT_FAILURE);
    }
    mini_controller = malloc(3 * sizeof(mini_cntrl_t)); //mini_controller objects
    if (mini_controller == NULL)
    {
        fprintf(stderr, "mini_controller out of memory\n");
        exit(EXIT_FAILURE);
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
        fprintf(stderr, "memory error in vehicle array");
        exit(EXIT_FAILURE);
    }
    vehicle_thrds_id = malloc(n_vehicles * sizeof(pthread_t));
    if (vehicle_thrds_id == NULL)
    {
        fprintf(stderr, "mem error in vhcl thrd id array\n");
        exit(EXIT_FAILURE);
    }
    // you need to add something here

    //create mini_controller threads
    for (int k = 0; k < 3; k++)
    {
        mini_controller[k].id = k;
        switch (k)
        {
        case 0:
            strcpy(mini_controller[k].direction, "(n2s, s2n)");
            break;
        case 1:
            strcpy(mini_controller[k].direction, "(e2w, w2e)");
            break;
        case 2:
            strcpy(mini_controller[k].direction, "(n2w, s2e)");
            break;
        }

        rc = pthread_create(&mini_controller_thrds_id[k], NULL, mini_controller_routine, (void *)&mini_controller[k]);
        if (rc)
        {
            fprintf(stderr, "rc from mc thread: %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }
    // you need to add something here

    //create vehicles threads
    srand(time(0));
    int prevDir = -1;
    int n2s_id = 0;
    int s2n_id = 0;
    int e2w_id = 0;
    int w2e_id = 0;
    int n2w_id = 0;
    int s2e_id = 0;
    for (int k = 0; k < n_vehicles; k++)
    {
        int direction = (int)rand() % 6;

        if (direction == prevDir)
        {
            vehicle[k].min_interval = min_interval + 1;
            sleep((int)rand() % vehicle_rate + 1);
        }
        else
        {
            vehicle[k].min_interval = min_interval;
            sleep((int)rand() % vehicle_rate);
        }
        prevDir = direction;

        switch (direction)
        {
        case 0:
            strcpy(vehicle[k].direction, "n2s");
            vehicle[k].id = n2s_id;
            n2s_id++;
            break;
        case 1:
            strcpy(vehicle[k].direction, "s2n");
            vehicle[k].id = s2n_id;
            s2n_id++;
            break;
        case 2:
            strcpy(vehicle[k].direction, "w2e");
            vehicle[k].id = w2e_id;
            w2e_id++;
            break;
        case 3:
            strcpy(vehicle[k].direction, "e2w");
            vehicle[k].id = e2w_id;
            e2w_id++;
            break;
        case 4:
            strcpy(vehicle[k].direction, "s2e");
            vehicle[k].id = s2e_id;
            s2e_id++;
            break;
        case 5:
            strcpy(vehicle[k].direction, "n2w");
            vehicle[k].id = n2w_id;
            n2w_id++;
            break;
        }

        rc = pthread_create(&vehicle_thrds_id[k], NULL, vehicle_routine, (void *)&vehicle[k]);
        if (rc)
        {
            fprintf(stderr, "rc from vh thread: %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    //join and terminating threads.
    for (int k = 0; k < n_vehicles; k++)
    {
        pthread_join(vehicle_thrds_id[k], NULL);
    }
    for (int k = 0; k < 3; k++)
    {
        pthread_cancel(mini_controller_thrds_id[k]);
    }

    //deallocate allocated memory
    free(vehicle_thrds_id);
    free(mini_controller_thrds_id);
    free(vehicle);
    free(mini_controller);

    //destroy mutex and condition variable objects
    pthread_mutex_destroy(&intersection_mutex);
    pthread_mutex_destroy(&n2s_mutex);
    pthread_mutex_destroy(&s2n_mutex);
    pthread_mutex_destroy(&e2w_mutex);
    pthread_mutex_destroy(&w2e_mutex);
    pthread_mutex_destroy(&n2w_mutex);
    pthread_mutex_destroy(&s2e_mutex);
    pthread_cond_destroy(&ns_cntrl_cond);
    pthread_cond_destroy(&ew_cntrl_cond);
    pthread_cond_destroy(&n2s_vhcl_cond);
    pthread_cond_destroy(&s2n_vhcl_cond);
    pthread_cond_destroy(&e2w_vhcl_cond);
    pthread_cond_destroy(&w2e_vhcl_cond);
    pthread_cond_destroy(&n2w_vhcl_cond);
    pthread_cond_destroy(&s2e_vhcl_cond);

    printf("Main thread: There are no more vehicles to serve. The simulation will end now.\n");

    exit(EXIT_SUCCESS);
}

void *mini_controller_routine(void *arg)
{
    mini_cntrl_t *mini_controller;
    mini_controller = (mini_cntrl_t *)arg;
    printf("Traffic light mini controller %s: Initialization complete. I am ready.\n", mini_controller->direction);
    while (1)
    {
        switch (mini_controller->id)
        {
        case 1:
            pthread_mutex_lock(&intersection_mutex);
            pthread_cond_wait(&ew_cntrl_cond, &intersection_mutex);
            break;
        case 2:
            pthread_mutex_lock(&intersection_mutex);
            pthread_cond_wait(&right_cntrl_cond, &intersection_mutex);
            break;
        }

        printf("The traffic lights %s will change to green now.\n", mini_controller->direction);

        time_t startTime = time(NULL);
        while (time(NULL) - startTime < mini_controller->time_green)
        {
            switch (mini_controller->id)
            {
            case 0:
                pthread_mutex_lock(&n2s_mutex);
                n2s = true;
                pthread_cond_signal(&n2s_vhcl_cond);
                pthread_mutex_unlock(&n2s_mutex);

                pthread_mutex_lock(&s2n_mutex);
                s2n = true;
                pthread_cond_signal(&s2n_vhcl_cond);
                pthread_mutex_unlock(&s2n_mutex);
                break;
            case 1:
                pthread_mutex_lock(&e2w_mutex);
                e2w = true;
                pthread_cond_signal(&e2w_vhcl_cond);
                pthread_mutex_unlock(&e2w_mutex);

                pthread_mutex_lock(&w2e_mutex);
                w2e = true;
                pthread_cond_signal(&w2e_vhcl_cond);
                pthread_mutex_unlock(&w2e_mutex);
                break;
            case 2:
                pthread_mutex_lock(&n2w_mutex);
                n2w = true;
                pthread_cond_signal(&n2w_vhcl_cond);
                pthread_mutex_unlock(&n2w_mutex);

                pthread_mutex_lock(&s2e_mutex);
                s2e = true;
                pthread_cond_signal(&s2e_vhcl_cond);
                pthread_mutex_unlock(&s2e_mutex);
                break;
            }
        }
        n2s = false;
        s2n = false;
        e2w = false;
        w2e = false;
        n2w = false;
        s2e = false;
        pthread_mutex_unlock(&intersection_mutex);
        printf("The traffic lights %s will change to red now.\n", mini_controller->direction);
        sleep(2);
        pthread_mutex_lock(&intersection_mutex);

        switch (mini_controller->id)
        {
        case 0:
            pthread_cond_signal(&ew_cntrl_cond);
            pthread_mutex_unlock(&intersection_mutex);
            pthread_mutex_lock(&intersection_mutex);
            pthread_cond_wait(&ns_cntrl_cond, &intersection_mutex);
            break;
        case 1:
            pthread_cond_signal(&right_cntrl_cond);
            pthread_mutex_unlock(&intersection_mutex);
            break;
        case 2:
            pthread_cond_signal(&ns_cntrl_cond);
            pthread_mutex_unlock(&intersection_mutex);
            break;
        }
    }
}

void *vehicle_routine(void *arg)
{
    vehicle_t *vehicle;
    vehicle = (vehicle_t *)arg;
    printf("Vehicle %d %s has arrived at the intersection.\n", vehicle->id, vehicle->direction);
    if (!strcmp(vehicle->direction, "n2s"))
    {
        pthread_mutex_lock(&n2s_mutex);
        while (!n2s)
        {
            pthread_cond_wait(&n2s_vhcl_cond, &n2s_mutex);
        }

        n2s = false;
        printf("Vehicle %d %s is proceeding through the intersection.\n", vehicle->id, vehicle->direction);
        sleep(vehicle->min_interval);
        pthread_mutex_unlock(&n2s_mutex);
    }
    if (!strcmp(vehicle->direction, "s2n"))
    {
        pthread_mutex_lock(&s2n_mutex);
        while (!s2n)
        {
            pthread_cond_wait(&s2n_vhcl_cond, &s2n_mutex);
        }

        s2n = false;
        printf("Vehicle %d %s is proceeding through the intersection.\n", vehicle->id, vehicle->direction);
        sleep(vehicle->min_interval);
        pthread_mutex_unlock(&s2n_mutex);
    }
    if (!strcmp(vehicle->direction, "e2w"))
    {
        pthread_mutex_lock(&e2w_mutex);
        while (!e2w)
        {
            pthread_cond_wait(&e2w_vhcl_cond, &e2w_mutex);
        }

        e2w = false;
        printf("Vehicle %d %s is proceeding through the intersection.\n", vehicle->id, vehicle->direction);
        sleep(vehicle->min_interval);
        pthread_mutex_unlock(&e2w_mutex);
    }
    if (!strcmp(vehicle->direction, "w2e"))
    {
        pthread_mutex_lock(&w2e_mutex);
        while (!w2e)
        {
            pthread_cond_wait(&w2e_vhcl_cond, &w2e_mutex);
        }

        w2e = false;
        printf("Vehicle %d %s is proceeding through the intersection.\n", vehicle->id, vehicle->direction);
        sleep(vehicle->min_interval);
        pthread_mutex_unlock(&w2e_mutex);
    }
    if (!strcmp(vehicle->direction, "n2w"))
    {
        pthread_mutex_lock(&n2w_mutex);
        while (!n2w)
        {
            pthread_cond_wait(&n2w_vhcl_cond, &n2w_mutex);
        }

        n2w = false;
        printf("Vehicle %d %s is proceeding through the intersection.\n", vehicle->id, vehicle->direction);
        sleep(vehicle->min_interval);
        pthread_mutex_unlock(&n2w_mutex);
    }
    if (!strcmp(vehicle->direction, "s2e"))
    {
        pthread_mutex_lock(&s2e_mutex);
        while (!s2e)
        {
            pthread_cond_wait(&s2e_vhcl_cond, &s2e_mutex);
        }

        s2e = false;
        printf("Vehicle %d %s is proceeding through the intersection.\n", vehicle->id, vehicle->direction);
        sleep(vehicle->min_interval);
        pthread_mutex_unlock(&s2e_mutex);
    }

    pthread_exit(EXIT_SUCCESS);
}
