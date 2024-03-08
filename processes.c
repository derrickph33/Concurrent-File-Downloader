#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PROCESSES 50				//Value for the Max Number of Processes
int main(int argc, char *argv[]){			//Main
	if (argc != 3){					//Check if There are Exactly 3 Commands when Ran
		printf("Invalid Number of Commands\n");
		return 1;
	}
	
	char *endptr;						  //endptr Variable to be Used for strtol
	long max = strtol(argv[2], &endptr, 10);		  //Convert the Value Provided to a String Using strtol
	if (*endptr != '\0' || max <= 0 || max > MAX_PROCESSES){  //Verify that the endptr is not Pointing to Null Char, max is a Positve Value, and that max is not Greater than the MAX_PROCESSES Value
		printf("Second Argument is not a Positive Integer or Value Exceeds Max Number of Processes\n");
		return 1;	
	}	
	FILE *fp = fopen(argv[1], "r");		  //Open the File in Read Mode
	if (fp == NULL){			  //Check if File was Successfully Opened
		printf("Unable to Open File\n");
		return 1;
	}
	
	char *line = NULL;	//Pointer to Store the Lines that are Being Read During getline 
	size_t size = 0;	//Variable that Stores Size of the buffer
	ssize_t characters;	//Variable to Store the Characters Read by getline
	int processes = 0;	//Variable to Keep Count of the Number of Child Processes Currently Running
	int lineNumber = 0;	//Variable to Keep Count of the Current Line Number that is Being Read
	while((characters = getline(&line, &size, fp)) != -1){	//Read the Lines from the File Using getline
		lineNumber++;					//Increment the Line Number Every Line
		if (processes >= max){				//Check if the Number of Processes Running is Equal or More than the max Amount
			wait(NULL);				//If so, then Wait for Child Processes to Finish and then Decrement the processes Variable Before Continuing
			processes--;
		}
		pid_t pid = fork();				//Create a New Child Process Using fork()
		if (pid == -1){					//Check if fork() was Successful
			perror("fork");
			break;
		} else if (pid == 0) {				//Child Process
			char fileName[256];			//Variable(s) to Store the File Name as well as the URL
			char url[256];
			int seconds = 0;			//Variable to Keep Track of the Optional Seconds Value from the File
			if (sscanf(line, "%255s %255s %d", fileName, url, &seconds) < 2){   //Read the Lines, Check if the Format is Incorrect
				printf("Incorrect Line Format\n");
				free(line);
				fclose(fp);
				return 1;
			}
			char seconds_array[32];						//Array to store the Max Seconds
			snprintf(seconds_array, sizeof(seconds_array), "%d", seconds);	//Store the Value of Seconds Provided by the File
			char* curlargs[] = {"curl", "-m", seconds_array, "-o", fileName, "-s", url, NULL};  //Arguments for curl to be Passed into exec
			execvp("curl", curlargs);	//execvp for curl Call
			perror("execvp");		//perror for if execvp Fails
			exit(1);
		} else {					//Parent Process
			printf("From Parent Process: Child Process %d Started Processing Line #%d\n", (int)pid, lineNumber); //Print Statement(s) Regarding Processing
			processes++;   											     //Increment the Number of Processes
			waitpid(pid, NULL, 0);										     //Wait for the Child Process to Complete
			printf("Child Process %d Processing Line #%d: Process Complete\n", getpid(), lineNumber);  
		}
	} 
	while (wait(NULL) > 0){   //Loop that Waits for the Child Processes to Terminate
		processes--;	  //Decrement the Number of Processes
	}
	free(line);   //Free the Pointer Used in getline
	fclose(fp);   //Close the File and Complete the Process
	printf("Process Complete\n");
	return 0;
}
