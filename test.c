#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct child_object
{
    int id;
    int play_time;
	int next_child;
} child_obj;

void * child_routine(void *);

//declare global mutex and condition variables
pthread_mutex_t toy_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c1_cond, c2_cond;

// You need to add something here.

int main(int argc, char ** argv)
{
	pthread_t *child_thrd_id; //system thread id
	child_obj *child; //user-defined thread id
	int total_time; //total amount of time childs can play
 	int k, b_t, rc;

	child_thrd_id = malloc(2 * sizeof(pthread_t)); //system thread ids
	if(child_thrd_id == NULL){
		fprintf(stderr, "threads out of memory\n");
		exit(1);
	}	
	
	child = malloc(2 * sizeof(child_obj)); //child objects
	if(child == NULL){
		fprintf(stderr, "t out of memory\n");
		exit(1);
	}	

	//Initialize condition variable objects 
	rc = pthread_cond_init(&c1_cond, NULL);
	if (rc) {
		printf("ERROR");
	}
	rc = pthread_cond_init(&c2_cond, NULL);
	if (rc) {
		printf("ERROR");
	}
	// you need to add something here.
	
	// ask for the total time.
	printf("Enter the total amount children can play (int): \n");
	scanf("%d", &total_time);
	
	//ask for children playing time 
	printf("Enter child 0 playing time (int): \n");
	scanf("%d", &b_t);
	child[0].play_time = b_t;
	child[0].next_child = 1;
	printf("Enter child 1 playing time (int): \n");
	scanf("%d", &b_t);
	child[1].play_time = b_t;	
	child[1].next_child = 0;

	//create child threads 
    for (k = 0; k<2; k++)
    {
		child[k].id = k;
		rc = pthread_create(&child_thrd_id[k], NULL, child_routine, (void *)&child[k]);
		if (rc) {
			printf("ERROR; return code from pthread_create() (child) is %d\n", rc);
			exit(-1);
		}
    }
    
	//sleep total_time seconds
	sleep(total_time);
	
	//Time is up and "the children's parent" calls the children to stop playing, i.e., terminate child threads.
    for (k = 0; k<2; k++) 
    {
		pthread_cancel(child_thrd_id[k]); 
    }

	//deallocate allocated memory
	free(child_thrd_id);
	free(child);

	//destroy mutex and condition variable objects
 	pthread_mutex_destroy(&toy_mutex);
	pthread_cond_destroy(&c1_cond);
	pthread_cond_destroy(&c2_cond);
	// you need to add something here.
	
    exit(0);
}

void * child_routine(void * arg)
{
	// you need to write a child routine.
	child_obj* child;
	child = (child_obj*) arg;
	int i = 0;
	while(1) {
		if (child->id == 1) {
			pthread_mutex_lock(&toy_mutex);
			pthread_cond_wait(&c2_cond, &toy_mutex);
		}
		
		printf("child %d: I get to play with the toy for %d units of time.\n", child->id, child->play_time);
		
		pthread_mutex_unlock(&toy_mutex);
		
		sleep(child->play_time);
		
		printf("child %d: I now give the toy to child %d.\n", child->id, child->next_child);

		// lock here?
		pthread_mutex_lock(&toy_mutex);
		if (child->id == 0) {
			pthread_cond_signal(&c2_cond);
			// unlock?
			pthread_mutex_unlock(&toy_mutex);
			// lock?
			pthread_mutex_lock(&toy_mutex);
			pthread_cond_wait(&c1_cond, &toy_mutex);
		} else {
			pthread_cond_signal(&c1_cond);
			// unlock?
			pthread_mutex_unlock(&toy_mutex);
		}
		i++;
	}
	
	
	

}

