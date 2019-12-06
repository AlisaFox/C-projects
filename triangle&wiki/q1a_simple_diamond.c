	
#include <stdio.h>
#include <stdlib.h>

int makediamond(int x){
int midpoint = x/2+1;
int stars =-1;

for (int i=0; i <midpoint; i++){
	int space= midpoint-i-1;
	stars += 2;
	for(int j=0; j<space; j++){
	printf(" ");
	}
	for(int j=0; j<stars; j++){
	printf("*");
	}
	printf("\n");	
}
for (int i=midpoint; i > 1; i--){
        int space= midpoint-i+1;
        stars -= 2;
        for(int j=0; j<space; j++){
        printf(" ");
        }
        for(int j=0; j<stars; j++){
        printf("*");
        }
        printf("\n");
}
return 0;
}

int main (int argc, char *argv[]){
//check if there is an input, if there isn't exit the program
if (argv[1] == NULL){
	printf("ERROR: Wrong number of arguments. One required. \n");
	exit(-1);
}
//I want a variable instead of using the pointer every single time
	int x = atoi(argv[1]);
//check that there aren't any extra arguments
if (argv[2] != NULL){
	printf("ERROR: Wrong number of arguments. One required. \n");
	}else if(x %2 ==0 || x < 1){
		//this checks the number if its odd and positive
		printf("ERROR: Bad argument. Height must be positive odd integer. \n");
		}
		else{ //if the input is fine, go  to the makediamond method
			makediamond(x);
		}
return 0;
}




