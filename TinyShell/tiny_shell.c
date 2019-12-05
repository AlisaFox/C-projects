#include <unistd.h>
#include <stdio.h>
#include <string.h> //strlen
#include <signal.h>
#include <stdlib.h> //exit
#include <sys/types.h> //pid_t
#include <sys/wait.h> //wait
#include <sys/time.h> //limit
#include <sys/resource.h> //limit
#include <sys/stat.h> //mkfifo
#include <fcntl.h> //o-wronly, rdonly, open

#define inputl 255
void sig_handler(int);
char str[inputl];
int currenthistory = 0;
char* history[100];
struct rlimit old_lim, new_lim;
char* fifoname;
int fd;
int pipe_exists=0;
int pipe_location=0; 
char* buffer[256];



int my_system(char* line){

//update history of last 100 commands, 
history[currenthistory] = strdup(line);	//save line for history
currenthistory = (currenthistory + 1) % 100;  //counter, %100 to ensure loop

//PARSING
char* argv[100]; //all the commands broken up as tokens
int argc; //total # of tokens

//parsing input line into tokens
char *token;    
token = strtok(line," ");
int i=0;
while(token!=NULL){
	argv[i]=token;      
	if (strcmp(argv[i], "|")==0){
		pipe_exists=1;       //acts as a switch for the forking process below (1 is pipe, 0 is normal command)
		pipe_location=i;
	}
	token = strtok(NULL," ");
	i++;
}
argv[i]='\0'; //set last value to NULL for execvp, thats why i didn't use for loop
argc = i; //since we count from 0


//INTERNAL COMMANDS
//can be done in the parent
if (strcmp(argv[0], "history") == 0) {                  // if user types history 
	int h= currenthistory;
	do{
		if(history[h]){
			printf("%s\n", history[h]);
		}
	    h= (h+1)%100;     //to make sure we don't go over 100 and cycle back                
	} while (h != currenthistory);

} else if (strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "chdir")==0) {                     // change directory, two ways because we were told chdir but i think the shell command is cd 
    chdir(argv[1]);            
} else if (strcmp(argv[0], "limit") == 0){                  //limit memory
	getrlimit(RLIMIT_AS, &old_lim);
	new_lim.rlim_max= old_lim.rlim_max;
	int l;

	printf("Please enter a new limit as an integer");
	scanf("%d", &l);
	new_lim.rlim_cur = l;

	if (setrlimit( RLIMIT_AS, &new_lim) == 0){
		printf("limit updated succesfully\n");
	}else{
		printf("Something went wrong with limit\n");
	}
} else if(strcmp(argv[0], "exit")==0){  //exit
	    exit(0);
}

//FORKING
pid_t  pid;
//now onto actually executing via child processes
//w/o pipe (normal)
if (pipe_exists == 0){
	pid = fork();
     if (pid < 0) {     // fork a child process, and check for error           
     	printf("Forking child process failed\n");
     	exit(1);
     }
     else if (pid == 0) {          // for the child process:       
          if (execvp(*argv, argv) < 0 && strcmp(argv[0], "cd") != 0 && strcmp(argv[0], "history") != 0 && strcmp(argv[0], "limit") != 0 ) {     // execute the command,and check for errors  
          	printf("Exec failed, try a different command?\n");
          	exit(1);
          }
      }
     else {                                  // for the parent:      
          wait(NULL);    // wait until child finishes         
      }
  }else

     //FIFO FIFO FIFO
    //for when we have a pipe
  	//we identified the pipe during the tokenizing proccess
   {
  	pid_t pid2;
  	pipe_exists=0; // reset pipe identifier switch
  	
  	pid = fork();
  	if (pid < 0) {     // fork a child process, and check for error           
     	printf("Forking child process failed\n");
     	exit(1);
     	}
	 else if (pid == 0) {   	
		close(1);                              //close stdout
 	 	fd = open(fifoname, O_WRONLY);	
 	 	dup2(fd, fileno(stdout));
 	 	 
 	 	//split the argv into two parts, since we know the location of the |
 	 	for (int j=0; j< pipe_location; j++){ //this is the first part (ex. ls out of ls | wc)
 	 		buffer[j]= argv[j];
 	 	}
 	 	                                   
 	 	if (execvp(*buffer, buffer) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        }

        close(fd);
    } 
    	else{
    		//back in parent
    	pid2 = fork();  //make second child

        	if (pid2 < 0) { 
          	printf("Forking grandchild process failed\n");
     		exit(1);
	     	}
	     	if (pid2 == 0) { 	     		
				close(0);	     	   	
	           fd= open(fifoname, O_RDONLY);
	           dup2(fd, fileno(stdin));

	            for (int j=pipe_location+1; j< argc; j++){  //get second half of line, after '|'
	 	 			buffer[j-pipe_location-1] = argv[j];
	 	 		}
	            if (execvp(*buffer, buffer) < 0) { 
	                printf("\nCould not execute command 2.."); 
	                exit(0); 
	            } 
	           
	            close(fd); 
        } 
        else { 
            // wait for 2nd child
         wait(NULL); 
        } 
    	wait(NULL);  //waiting for 1st child
    }	
  }
      return 0;
  }


char* get_a_line(){

	printf("# ");  //user prompt

	//SIGNAL HANDLING Pt 1
	//signal handling, helper function sig_handler below
	//put it before the fgets, because if the signal the first cmd the shell receives, it will quit without asking for confirmation
	signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_handler);
   
   //GET LINE
	if( fgets(str, inputl, stdin) == NULL){ //watch for eof
		exit(0);
	} 
  // remove newline, if present 
	int i = strlen(str)-1;
	if( str[ i ] == '\n') 
		str[i] = '\0';

	return str;
}

//SIGNAL HANDLING Pt 2
void sig_handler(int signo)
{
  if (signo == SIGINT){  //ctrl c
    printf("\n Received SIGINT, enter 'y' if you want to exit \n");
     char c = getchar();
     if (c == 'y' || c == 'Y'){
          exit(0);
     }
	}
	if (signo == SIGTSTP){ //ctrl z
		printf ("\n Received SIGTSTP, ignored \n");
	}
}


int main (int argc, char* argv[]){
	if (argv[1]!= NULL){
	fifoname=strdup(argv[1]);   //get fifoname
}
	while (1) {
		char* line = get_a_line();
		if (strlen(line) > 1){
			my_system(line);
		}
	}
	return 0;
}