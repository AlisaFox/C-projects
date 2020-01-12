#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#define  BILLION 1E9

sem_t rw_mutex; 
sem_t mutex;
int read_count = 0;  
int target = 0; //what we are reading / writing to
//for timing:
double* rtimes; //doubles array for readers
int rtimer = 0; //pointer for rtimes
double* wtimes; //doubles array for writers
int wtimer = 0; //pointer for wtimes

static void *reader(void *arg) {

	int c = 0; //counter for iteration
	int attempts = *(int*)arg; //iterations allowed
	struct timespec start, stop; //for timing purposes
    double accum; //seconds passed
	
	while(c< attempts){ //c is iteration number, attempts is iterations allowed
		
		//start clock
		if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
 			   perror( "clock gettime" );
     			exit( EXIT_FAILURE );
   			} 

		sem_wait(&mutex); //reader wants to enter critical
		read_count++; //number of readers increased
		if (read_count == 1){ //if there is a reader, no writer is allowed to enter
			sem_wait(&rw_mutex);
		}
		//critical section
		sem_post(&mutex); //other readers can enter

		//timing calculations
			if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
		      perror( "clock gettime" );
		      exit( EXIT_FAILURE );
		    }
			accum= (stop.tv_sec- start.tv_sec)
				+ ( stop.tv_nsec - start.tv_nsec )
            	/BILLION; //get seconds
			rtimes[rtimer] = accum; //update array for this iteration
			rtimer++; //move pointer

		// read is performed
		int t = rand() % 101; //0 to 100 milliseconds
		usleep(t); //sleep for that time
		//read the value
		 printf("\n reader is reading value as %d \n ", target);  

		sem_wait(&mutex); //reader want to leace
		read_count--; //decrease accordingly
		//critical ends
		if (read_count == 0){ //if no readers
			sem_post(&rw_mutex); //writers can enter
		}
		sem_post(&mutex);  //reader leaves
		c++; //increase counter for iterations
	}
}

static void *writer(void *arg) {

	int c = 0; //counter for iteration
	int attempts = *(int*)arg; //iterations allowed
	struct timespec start, stop; //for timing purposes
    double accum; //seconds passed
	
	while(c< attempts){ //c is iteration number, attempts is iterations allowed
			//start clock
			if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
 			   perror( "clock gettime" );
     			exit( EXIT_FAILURE );
   			}

		sem_wait(&rw_mutex); //request critical

			//timing stuff
			if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
		      perror( "clock gettime" );
		      exit( EXIT_FAILURE );
		    }
			accum= (stop.tv_sec- start.tv_sec)
				+ ( stop.tv_nsec - start.tv_nsec )
            	/BILLION; //calculate seconds
			wtimes[wtimer] = accum; //write seconds
			wtimer++; //move in array


		// write is performed
		int t = rand() % 101;
		usleep(t); //sleep for random time (0-100ms)
		target+= 10; //increase int
		// printf("\n target is now is %d \n ", target); //to see what the new value is if needed

		//leave critical
		sem_post(&rw_mutex); 
		//increase iteration counter
		c++;
	}
	
}

//timing calculations function
void times(int a, int b){
	double rtotal =0.0;
	double wtotal =0.0;
	double raverage, waverage; //total /(iterations allowed*threads)
	double rmax = 0.0;
	double wmax = 0.0;
	double rmin = 0.0;
	double wmin = 0.0;

	for (int q=0;q< 500*a; q++){
		rtotal+= rtimes[q];   //for average
		if (rtimes[q]< rmin){
			rmin = rtimes[q]; //update min
		}
		if (rtimes[q]> rmax){
			rmax = rtimes[q]; //update max
		}
	}

	raverage = rtotal / (500*a); //get average
	printf("\nReader average time is %lf secs \n" , raverage);
	printf("Reader max time is %lf secs \n" , rmax);
	printf("Reader min time is %lf secs \n" , rmin);

	for (int i=0; i< 10*b;i++){
		wtotal+= wtimes[i];   //for average
		if (wtimes[i]< wmin){
			wmin = wtimes[i]; //update min
		}
		if (wtimes[i]> wmax){
			wmax = wtimes[i]; //update max
		}
	}
	waverage = wtotal / (500*a); //get average
	printf("\nWriter average time is %lf secs \n" , waverage);
	printf("Writer max time is %lf secs \n" , wmax);
	printf("Writer min time is %lf secs \n" , wmin);

}

int main(int argc, char *argv[]) {
	pthread_t r[500], w[10]; //500 readers, 10 writers
	int s; //for error managment
	int a= atoi(argv[1]); //reader iterations allowed
	int b = atoi(argv[2]); //writer iterations allowed

	//timing array initialization, where size is
	//number iterations allowed * threads
	rtimes = (double*) malloc ((500*a)*sizeof(double)); 
	wtimes = (double*) malloc ((10*b)*sizeof(double));
    
	//initialize semaphores
    if (sem_init(&mutex, 0, 1) == -1 || sem_init(&rw_mutex, 0, 1) == -1) {
    printf("Error, init semaphore\n");
    exit(1);
  	}

  	//create 500 readers
	for(int i=0; i<500; i++){
        s=pthread_create(&r[i],NULL,reader, &a);
        if (s != 0) {
    		printf("Error, creating reader %d \n", i);
   		 	exit(1);
   		 }
	}  
	//create 10 writers
    for(int i=0; i<10; i++){
        s= pthread_create(&w[i],NULL,writer, &b);
        if (s != 0) {
    		printf("Error, creating writer %d \n", i);
   		 	exit(1);
   		 }
    }

    for (int i = 0; i < 500; i++) { 
        s= pthread_join(r[i], NULL); 
         if (s != 0) {
    		printf("Error, join reader %d \n", i);
   		 	exit(1);
   		 }
    } 
    for (int i = 0; i < 10; i++) { 
        s= pthread_join(w[i], NULL); 
        if (s != 0) {
    		printf("Error, join writer %d \n", i);
   		 	exit(1);
   		 }
    }

  	//now that timing arrays are filled out, get times
    times(a, b);

    //release mem for the malloc arrays
    free(rtimes);
    free(wtimes);

    //we're done
     exit(0);
}
