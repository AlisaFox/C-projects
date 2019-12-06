#include <stdio.h>
#include <stdlib.h>

//I did not get though this assignment, the only faultless part is the parameter checking in main
int makediamondUp(int x, int Bspace, int twotriangles){
int midpoint = x/2+1;
int stars =-1;

for (int i=0; i <midpoint; i++){
        int space= midpoint-i-1;
        stars += 2;
	//make top diamond first (w/ the spaces around)
		for ( int k=0; k < Bspace; k++){
                	printf(" ");
        	}
        	for(int j=0; j<space; j++){
        		printf(" ");
        	}
        	for(int j=0; j<stars; j++){
        		printf("*");
       		}
		//make the lower half next
		if(twotriangles == -1){
			printf(" ");
			for(int j=0; j<space; j++){
        			printf(" ");
        			}
			for(int j=0; j<space; j++){
        			printf(" ");
        			}
        		for(int j=0; j<stars; j++){
        			printf("*");
        			}
	space +=space;
	}
        printf("\n");
}
return 0;
}

int makeupper(int height, int L, int width, int space, int twotriangles, int repeat){
	//my try to make a recursive function
	if (L==1){
	makediamondUp(width, space, twotriangles);
	}else{
	makeupper(height/2, L-1, width/2, width/4 + 1 + space, 0, repeat );
	makeupper(height/2, L-1, width/2, space, - 1, repeat +1);
	}
	return 0;
}


   
int main (int argc, char *argv[]){
//check if there is an input, if there isn't exit the program
if (argv[1] == NULL || argv[2] == NULL){
        printf("ERROR: Wrong number of arguments. Two required. \n");
        exit(-1);
}
//I want a variable instead of using the pointer every single time
        int x = atoi(argv[1]);	
	int L = atoi(argv[2]);
	int tri_height=(x/2+1);
//check that there aren't any extra arguments
if (argv[3] != NULL){
        printf("ERROR: Wrong number of arguments. Two required. \n");
        }else if(x %2 ==0 || x < 1){
                //this checks the number if its odd and positive
                printf("ERROR: Bad argument. Height must be positive odd integer. \n");
                }else if(tri_height < (1<<L-1)){
		printf("ERROR: Height does not allow evenly dividing requested number of levels. \n");
			}
                	else{ //if the input is fine, go  to the makediamond method
                        	makeupper(tri_height, L, x, 0, 0, 0);
                		}
return 0;
}
