#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include "history.h"
#include <sys/wait.h>
#include "alias.h"

int parse_input(char *inp, char *path, int invoke);
int process(char *tokens[max_array_size], int args);
void getPath(int args);
void changedir(char **tokens, int args);
void setPath(char **tokens, int args);
void commands(char **tokens, char *path, int args);
void checkAlias(char *str, char *line, char *path, int display);
int quit(char *path);

/*
 * Group 2
 * Erin Allen
 * Tim Enos
 * Sudeep Dhakal
 * Adrian Pihlgren
 * Karmen Tsang
*/


int main(void) {

	//gets the home directory
	char *path = getenv("PATH");
	char *homedir = getenv("HOME");
	char *username = getenv("USER");
	char inp[max_buffer_size];
	char *invoked_hist;
	char *store;
	int display; //used to prevent multiple error messages begin displayed
	int is_invoked;
	chdir(homedir); //sets starting directory as home
	system("clear");
	load_history();
	load_alias();
	
	while(1) {
		printf("%s$ ", username);
		char *line = fgets(inp, max_buffer_size, stdin);
		display = 0;
		is_invoked = 0;
		if(feof(stdin)) { //CTRL+D == EXIT
            return quit(path);
        } 
		else if(is_alias(line) == 1) {
			is_invoked = 1;
			char *command = malloc(sizeof(char) * max_buffer_size); //allocate the appropriate memory for the concatenation of alias command
			store = invoke_alias(line, command, is_invoked, display);

			if(store[0] == '!') {
				strcpy(line, store);
				free(command); //free memory allocated for concat
				invoked_hist = invoke_History(line);
				checkAlias(invoked_hist, line, path, display);
			} else {
				is_invoked = 0;
				//set display to 1 as we want to display any error messages
				display = 1;
				store = invoke_alias(line, command, is_invoked, display);
				strcpy(line, store);
				free(command);
				parse_input(line, path, 1);
			}
		} 
		else if(line[0] == '!') {
				invoked_hist = invoke_History(line);
				//pass to function to see if it is alias
				checkAlias(invoked_hist, line, path, display);
		} 
		else {
			parse_input(line, path, is_invoked);
		}
	}

	return 0;
}

int parse_input(char *inp, char *path, int invoke){
	
	char fullinp[max_buffer_size];
	char *token;
    char *tokens[max_array_size];                  //array of tokens (t)    

	strcpy(fullinp, inp); // get full command line input
    char delim[] = " \t|<>&;\n";  
    
	memset(tokens, 0, sizeof(tokens));
	int i = 0;
	token = strtok(inp, delim);   // delimiter to tokenize an array of strings

	while (token){
		tokens[i++] = token;
		token = strtok(NULL, delim);
	}
	
	// prevent null values being added to history
	if(*tokens == NULL) { 
		return 0;
	}
	
	//if not invoked then add to history
	if(invoke == 0) {
		add_History(strtok(fullinp, "\n"));
	} 

	commands(tokens, path, i); // execute command

	return 1;
}


void commands(char **tokens, char *path, int args) {

	if(strcmp(tokens[0], "exit") == 0) {
		quit(path);
	} 
	else if (strcmp(tokens[0], "history") == 0) { // if user enters history
		if(args > 1)  {                        //error message if the user inputs history + something
            printf("Invalid number of arguments. Usage: history\n");
		} else {
			print_History(); 
        }
	} 
	else if (strcmp(tokens[0], "cd") == 0)  { 
		if(args > 2) {                          // check for valid number of arguments
			printf("Invalid number of arguments. Usage: cd [path]\n");
		} else {
			changedir(tokens, args);
		}
	} 
	else if(strcmp(tokens[0], "alias") == 0) {
		 if(args == 1) {
			print_alias();
		} else {
			add_alias(tokens, args);
		}
	} 
	else if(strcmp(tokens[0], "unalias") == 0) {
		remove_alias(tokens, args);
	} 
	else if(strcmp(tokens[0],"setpath") == 0) {
		setPath(tokens, args);
	} 
	else if(strcmp(tokens[0],"getpath") == 0) {
		getPath(args);
	} 
	else {
		process(tokens, args);
	}
}

// creating a separate process for fork() system call
int process(char *tokens[max_array_size], int args) {

	pid_t pid;
	pid = fork();                          // fork a child process
	
	if (pid < 0) {                         // error occurred
		fprintf(stderr, "Fork Failed!");   // prints error if fork failed
	} 
	else if (pid == 0) {                 // if child process successful

		if (execvp(tokens[0], tokens) == -1) { 
			printf("shell: command not found %s\n", *tokens);
		}
		exit(0);
	} 
	else {                               // parent process
		pid = wait(NULL);                  // wait for child process
	}
	   return 1;
}

void setPath(char **tokens, int args) {
    
    if (tokens[1] == NULL) {                    // print " if nothing is entered after setpath
        printf("No arguments found. Usage: setpath [dir]\n");
        return;
	} else if (args > 2) {               // if more than one input print
        printf("Invalid number of arguments. Usage: setpath [dir]\n");
		return;
	}
    

	if (setenv("PATH", tokens[1], 1) == 0) {
        printf("PATH set successfully to %s\n", tokens[1]);
    } 
	else {
        printf("Error setting path variable\n");
    }
}


void getPath(int args) {
	
	if (args == 1) { // if no arguments
		printf("PATH:%s\n", getenv("PATH"));
	} 
	else { // if more than one argument, print appropriate error message
		printf("Invalid number of arguments. Usage: getpath \n");
	}
}

//method for exit of program, saves alias and history and restores path
int quit(char *path) {

  if (setenv("PATH", path, 1) == 0) {
        printf("PATH restored to %s\n", getenv("PATH"));
  } 
  else {
	  printf("Error restoring path variable\n");
  }

    printf("Bye...\n");
	save_history();
	save_alias();
    exit(0);
}

void changedir(char **tokens, int args) {
	//only needs to redirect to home
	if(tokens[1] == NULL) {
		chdir(getenv("HOME"));

	} 
	else if(chdir(tokens[1]) == -1) {
		perror(tokens[1]);
	}
}

void checkAlias(char *invoked_hist, char *line, char *path, int display) {
	//this method is used to check if an invoked history is an alias, if so then we replace the output
	
	int is_invoked = 1;

	if(invoked_hist != NULL) {
		//if invoked_hist contains value then copy over to line
		strcpy(line, invoked_hist);

		if(is_alias(line) == 1) {
			//set display to 1 as we want to display error messages
			display = 1;
			char *command = malloc(sizeof(char) * max_buffer_size);
			char *store = invoke_alias(line, command, is_invoked, display);
			//copy store over to line, to avoid segfault
			strcpy(line, store);
			//free memory that was allocated for concat 
			free(command);
			//parse aliased input
			parse_input(line, path, is_invoked);
		} 
		else {
			strcpy(line, invoked_hist);
			parse_input(line, path, is_invoked);
		}
	}
}

