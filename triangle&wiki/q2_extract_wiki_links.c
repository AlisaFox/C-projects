# include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[]){

//try to open file
FILE *fp = fopen (argv[1], "r");
 if (fp == NULL) 
    { 
        printf("Cannot open file \n"); 
        exit(0); 
    } 
//check number of arguements
if (argc != 2){
	printf("Sorry, incorrect amount of arguments, try again\n");
}
//make file into one big string
char *buffer = 0;
long length;
  fseek (fp, 0, SEEK_END);
  length = ftell (fp);
  fseek (fp, 0, SEEK_SET);
  buffer = malloc (length);
  if (buffer)
  {
    fread (buffer, 1, length, fp);
  }

if (buffer){
int len = strlen(buffer);
	printf("meow %d\n", len);
//set up for while loop
int looping = -1;
int counter = 0;
char *prev = buffer;
while(looping == -1){
	//smaller cut-off of buffer should decrease with each cycle so that strstr does nto find the same link again
	char small [len- counter];
	for(int i=0; i<len-counter; i++){
                   small[i]=buffer[i+counter];
           }
	if(!small){
		break;
	}
	char *g=0;
	char *h=0;
	//check if any links exist
 	if (strstr(small, "<a href=\"/wiki/") != NULL){
		//find beginning
		g=strstr(small, "<a href=\"/wiki/");
		//find end
		h=strstr(small, "</a>");
		//with strlen, we can find the exact location of the string (where it starts)
		int z = strlen(g);
		int y = strlen(h);
		int length = strlen(small);
		//length of link
		int j = y-z;
		//counter for the small buffer conversion 
		counter+=length-y+4;
		//make link string
		char link[j];
		for(int i=0; i<j; i++){
			link[i]=g[i];
		}
		//only print it if it contains title=
		if(strstr(link, "title=") != NULL){
			int switchy = 0;
			int switchz=0;
			//to doublecheck against repeats
			for (int k=0; k<3; k++){
				if (link[15+k]!=prev[k]){
					switchz=0;
					break;
				}else{
					switchz=1;
				}
			}
			if (switchz == 1){
				continue;
			}
			*prev=0;
			//print the titlename of the link (should be until ")
			for (int k=0; k <j; k++){
				if(link[15+k]!='"' && link[15+k] != ' '){
				if (strcmp(prev, link) == 0 ){
					break;
				}	
				printf("%c", link [15+k]);
				prev[k]=link[15+k];
				switchy =1;
				}else{
				break;
				}
			}
			//if we did print smth, make a new line
			if (switchy ==1){
				printf("\n");
			}
			//reset everything
			switchy = 0;
		}
		*small= 0;
		*g=0;
		*h=0;
	}else{
		//if no more <a href's are found, close loop
		looping = 1;
		}
	}
}
fclose(fp);
return 0;
}
