#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct passenger_object
{
    int id;
    char direction[4];
} passenger_obj;

void * boat_routine(void *);
void * passenger_routine(void *);

//declare global mutex and condition variables
pthread_mutex_t boating_mutex = PTHREAD_MUTEX_INITIALIZER; //static initialization
pthread_cond_t n2s_cond, s2n_cond;

int main(int argc, char ** argv)
{
	int no_of_passengers; //total number of passengers to be created
	pthread_t *c_thrd_ids, b_thrd_id; //system thread id
	passenger_obj *passenger; //user-defined thread id
	int boat_pace, passenger_rate;
 	int k, rc;
	int drct, n_c, s_c;
	
	// ask for the total number of passengers.
	printf("Enter the total number of passengers (int): \n");
	scanf("%d", &no_of_passengers);
	
	//ask for boat's working pace
	printf("Enter boat's working pace (int): \n");
	scanf("%d", &boat_pace);
	
	//ask for passengers' arrival rate
	printf("Enter passengers arrival rate (int): \n");
	scanf("%d", &passenger_rate);
		
	c_thrd_ids = malloc((no_of_passengers) * sizeof(pthread_t)); //passenger thread ids
	if(c_thrd_ids == NULL){
		fprintf(stderr, "c_thrd_ids out of memory\n");
		exit(1);
	}	
	
	passenger = malloc((no_of_passengers) * sizeof(passenger_obj)); //no_Of_passengers 
	if(passenger == NULL){
		fprintf(stderr, "passenger out of memory\n");
		exit(1);
	}	

	//Initialize condition variable objects 
	rc = pthread_cond_init(&n2s_cond, NULL);
	if (rc) {
		printf("ERROR; return code from pthread_cond_init() (n2s) is %d\n", rc);
		exit(-1);
	}
	rc = pthread_cond_init(&s2n_cond, NULL);
	if (rc) {
		printf("ERROR; return code from pthread_cond_init() (s2n) is %d\n", rc);
		exit(-1);
	}
	
	//create the boat thread.
	if (pthread_create(&b_thrd_id, NULL, boat_routine, (void *) &boat_pace)){
		printf("ERROR; return code from pthread_create() (boat) is %d\n", rc);
		exit(-1);
	}

	//create passengers according to the arrival rate
	n_c = s_c = 0;
	srand(time(0));
    for (k = 0; k<no_of_passengers; k++)
    {
		sleep((int)rand() % passenger_rate); 
		drct = (int)rand()% 2;
		if (drct == 0){//n2s passenger
			strcpy(passenger[k].direction, "n2s");
			passenger[k].id = n_c;
			n_c++;
		}
		else if (drct == 1){//s2n passenger
			strcpy(passenger[k].direction, "s2n");
			passenger[k].id = s_c;
			s_c++;
		}
		rc = pthread_create(&c_thrd_ids[k], NULL, passenger_routine, (void *)&passenger[k]);
		if (rc) {
			printf("ERROR; return code from pthread_create() (consumer) is %d\n", rc);
			exit(-1);
		}
    }
    
	//join passenger threads.
    for (k = 0; k<no_of_passengers; k++) 
    {
		pthread_join(c_thrd_ids[k], NULL);
    }
	
	//terminate the boat thread using pthread_cancel().
    pthread_cancel(b_thrd_id); 
			
	//deallocate allocated memory
	free(c_thrd_ids);
	free(passenger);

	//destroy mutex and condition variable objects
	pthread_mutex_destroy(&boating_mutex);
	pthread_cond_destroy(&n2s_cond);
	pthread_cond_destroy(&s2n_cond);	

    exit(0);
}

void * boat_routine(void * arg)
{
	int *work_pace;
	work_pace = (int *) arg;
	int direction = 0; //initially boat moving n2s
	printf("boat starts working in pace %d.\n", *work_pace);
    while (1)
    {
        pthread_mutex_lock(&boating_mutex);
		if (direction == 0){//moving n2s
			printf("boat to move n2s.\n");
			pthread_cond_signal(&n2s_cond); //take one passenger from n2s if any
			direction = 1;			
		}
		else{//moving s2n
			printf("boat to move s2n.\n");		
			pthread_cond_signal(&s2n_cond); //take one passenger from s2n if any
			direction = 0;
		}
        pthread_mutex_unlock(&boating_mutex);
		sleep (*work_pace); 
    }
}

void * passenger_routine(void * arg)
{
	passenger_obj *myid;
	
	myid = (passenger_obj *)arg;
    pthread_mutex_lock(&boating_mutex);
    printf("Passenger %s %d arrives and waits to cross the river.\n", myid->direction, myid->id);
	if (!strcmp(myid->direction, "n2s")){
		pthread_cond_wait(&n2s_cond, &boating_mutex);
		printf("passenger %s %d is now on the boat.\n", myid->direction, myid->id);	
	}		
	else if (!strcmp(myid->direction, "s2n")){
		pthread_cond_wait(&s2n_cond, &boating_mutex);
		printf("passenger %s %d: on the boat.\n", myid->direction, myid->id);
	}		
    pthread_mutex_unlock(&boating_mutex);
    pthread_exit(EXIT_SUCCESS);

}
