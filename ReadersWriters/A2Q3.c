#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#define  BILLION 1E9

/*I will not comment on "old" functions, 
only on the new changes so they are noticable*/

sem_t ordermutex; //order of arrival for both readers and writers
sem_t accessmutex; // for writers: exclusive access to resource
sem_t readersmutex; //for readers: protects reader_count from conflicts
int read_count = 0 ;  
int target = 0;

double* rtimes;
int rtimer = 0;
double* wtimes;
int wtimer = 0;

static void *reader(void *arg) {

	int c = 0;
	int attempts = *(int*)arg;
	struct timespec start, stop;
    double accum;
	
	while(c< attempts){

			clock_gettime (CLOCK_REALTIME, &start);
		sem_wait(&ordermutex); //order of arrival
		sem_wait(&readersmutex); //if no readers, 
		if (read_count == 0){
			sem_wait(&accessmutex); //exlusive access for readers
		}
		read_count++;

		sem_post(&ordermutex); //this reader has been served
		sem_post(&readersmutex); //done w/ accessing number of readers for now
			clock_gettime( CLOCK_REALTIME, &stop);
			accum= (stop.tv_sec- start.tv_sec)
				+ ( stop.tv_nsec - start.tv_nsec )
            	/BILLION;
			rtimes[rtimer] = accum;
			rtimer++;

		// read is performed
		int t = rand() % 101;
		usleep(t);
		 printf("\n reader is reading value as %d \n ", target);  

		sem_wait(&readersmutex); //one less reader
		read_count--;
		if (read_count == 0){
			sem_post(&accessmutex); //release exclusive access
		}
		sem_post(&readersmutex); //done w/ accessing number of readers for now
		c++;
	}
}

static void *writer(void *arg) {

	int c = 0;
	int attempts = *(int*)arg;
	struct timespec start, stop;
    double accum;
	
	while(c< attempts){

			if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
 			   perror( "clock gettime" );
     			exit( EXIT_FAILURE );
   			}

		sem_wait(&ordermutex); //order of arrival
		sem_wait(&accessmutex); //request exclusive access
		sem_post(&ordermutex); //release order (were served)

			if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
		      perror( "clock gettime" );
		      exit( EXIT_FAILURE );
		    }
			accum= (stop.tv_sec- start.tv_sec)
				+ ( stop.tv_nsec - start.tv_nsec )
            	/BILLION;
			wtimes[wtimer] = accum;
			wtimer++;

		

		// write is performed
		int t = rand() % 101;
		usleep(t);
		target+= 10;
		//printf("\n target is now is %d \n ", target); //uncomment if needed 

		sem_post(&accessmutex); //release exclusive access
		c++;
	}
	
}

void times(int a, int b){
	double rtotal =0.0;
	double wtotal =0.0;
	double raverage, waverage;
	double rmax = 0.0;
	double wmax = 0.0;
	double rmin = 0.0;
	double wmin = 0.0;

	for (int q=0;q< 500*a; q++){
		rtotal+= rtimes[q];   //for average
		if (rtimes[q]< rmin){
			rmin = rtimes[q];
		}
		if (rtimes[q]> rmax){
			rmax = rtimes[q];
		}
	}

	raverage = rtotal / (500*a);
	printf("\nReader average time is %lf secs \n" , raverage);
	printf("Reader max time is %lf secs \n" , rmax);
	printf("Reader min time is %lf secs \n" , rmin);

	for (int i=0; i< 10*b;i++){
		wtotal+= wtimes[i];   //for average
		if (wtimes[i]< wmin){
			wmin = wtimes[i];
		}
		if (wtimes[i]> wmax){
			wmax = wtimes[i];
		}
	}
	waverage = wtotal / (500*a);
	printf("\nWriter average time is %lf secs \n" , waverage);
	printf("Writer max time is %lf secs \n" , wmax);
	printf("Writer min time is %lf secs \n" , wmin);

}

int main(int argc, char *argv[]) {
	pthread_t r[500], w[10];
	int s;
	int a= atoi(argv[1]);
	int b = atoi(argv[2]);
	rtimes = (double*) malloc ((500*a)*sizeof(double));
	wtimes = (double*) malloc ((10*b)*sizeof(double));
    
	//initialize one more sem
    if (sem_init(&accessmutex, 0, 1) == -1 || sem_init(&ordermutex, 0, 1) == -1 || sem_init(&readersmutex, 0, 1) == -1) {
    printf("Error, init semaphore\n");
    exit(1);
  	}

	for(int i=0; i<500; i++){
        s=pthread_create(&r[i],NULL,reader, &a);
        if (s != 0) {
    		printf("Error, creating reader %d \n", i);
   		 	exit(1);
   		 }
	}  
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

  
    times(a, b);

    free(rtimes);
    free(wtimes);
     exit(0);
}