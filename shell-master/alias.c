#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include "alias.h"
#include "history.h"

//group 2

static char *file;
static alias alias_map[max_alias_size];
static int alias_len = 10;

void add_alias(char **tokens, int args) {

	alias_len = max_alias_size;

	char *str = tokens[1];

	//if alias command entered is empty then print error
	if (tokens[2] == NULL) {
		printf("Alias: Not enough arguments given, try: alias <name> <command>\n");
		return;
	}
	//allocates space to tokens array in order to allow mapping to stored properly
	char *str1 = malloc(sizeof(char) * (strlen(*tokens) + 1));
	//clears strings since malloc does not
	strcpy(str1, "");

	//this for loop concatenates everything after str1 as one string for storing
	for(int i = 2; i < args; i++) {
		strcat(str1, tokens[i]);
		//if there is nothing after it then dont add in space
		if(i == args -1) {
			break;
		}
		strcat(str1, " ");
	}


	alias_len = number_of_aliases();

	//for loop for checking if the alias already exists
	for (int i = 0; i < alias_len; i++) {
		if(strcmp(alias_map[i].aliasName, str) == 0) {
			printf("alias %s %s, replaced with: %s %s\n", str, alias_map[i].aliasCommand, str, str1);
			strcpy(alias_map[i].aliasCommand, "");
			strcpy(alias_map[i].aliasCommand, str1);
			return;
		}

	}
	
	//if alias is full then print message and return
	if(alias_len >= max_alias_size - 1) {
		printf("Alias: Maximum amount of alias has been set.\n");
		return;
	}

	//if alias does not exist, then add them to next available spot in array, i.e count
	strcpy(alias_map[alias_len].aliasName, str);
	strcpy(alias_map[alias_len].aliasCommand, str1);

	free(str1);

}


void remove_alias(char **tokens, int args) {
	
	char *command = tokens[1];
	int count = max_alias_size;
	//if the index is not changed then alias does not exist to remove
	int index = -1;

	if(tokens[1] == NULL) {
		printf("Alias: Not enough arguments given, try: unalias <command>\n");
		return;
	}
	
	count = number_of_aliases();

	if(count == 0) {
		printf("Alias: No aliases have been set\n");
		return;
	}

	//checks if the alias exists if so then remove it.
	for(int i = 0; i < count; i++) {
		if(strcmp(command, alias_map[i].aliasName) == 0) {
			printf("Unaliased: %s\n", alias_map[i].aliasName);
			strcpy(alias_map[i].aliasName, "");
			strcpy(alias_map[i].aliasCommand, "");
			//set index to position that was replaced
			index = i;
		}
	}

	//if the index is not changed then alias does not exist to remove
	if (index == -1 || count == 0) {
		printf("Alias: %s is not an alias.\n", command);
		return;
	}

	for(int i = index; i < max_alias_size - 1; i++) {
		//if the current spot is empty and the one infront is not them move it back
		if(strcmp(alias_map[i].aliasName, "") == 0 && strcmp(alias_map[i+1].aliasName, "") != 0) {
			//move statement infront to empty spot
			strcpy(alias_map[i].aliasName, alias_map[i+1].aliasName);
			strcpy(alias_map[i].aliasCommand, alias_map[i+1].aliasCommand);
			//make statement that has been moved empty
			strcpy(alias_map[i+1].aliasCommand, "");
			strcpy(alias_map[i+1].aliasName, "");
		}
	}

}

char *invoke_alias(char *fullinp, char *alias_command, int invoke, int display) {

	//used to check if there is any alias invoked, if check is 0 then we know that input is not alias
	int swaps = 0;
	//to ensure there is a set maximum amount of swaps that can be carried out
	int max_swapps = 4;
	int allowed_swaps = 3;
	//to store aliases for concatenation
	char *storeInvoke[max_array_size];
	//input to return and parse
	int count = 0;
	//gets the size of the alias_map
	int size = number_of_aliases();
	char inp[max_buffer_size];
	strcpy(inp, fullinp);
	strtok(inp, "\n");


	char *tok = strtok(inp, " ");

	//loop to store all the values from the input in one array, split by spaces
	while(tok != NULL) {
		storeInvoke[count] = tok;
		count++;
		tok = strtok(NULL, " ");
	}

	/*the first loop is used to go through all the elements of the array
	 * the second loop is used to carry out the swaps for the aliases, and ensure that this do not exceed 3
	 * the third loop is used to replace the alias with the actual command
	 * */
	for(int i = 0; i < count; i++) {
		for(int j = 0; j < max_swapps; j++) {
			for(int k = 0; k < size; k++) {
				if(strcmp(storeInvoke[i], alias_map[k].aliasName) == 0) {
					storeInvoke[i] = alias_map[k].aliasCommand;
					swaps++;
					break;
				}
			}
		}
	}

	if(swaps > (allowed_swaps * count) && display == 1) {
		printf("Alias: Maximum amount of aliases swaps.\n");
		//return enter to not pass on the invalid alias
		return "\n";
	}
	//if nothing is an alias then return NULL
	else {
		//copy over the first index to the alias
		strcpy(alias_command, storeInvoke[0]);
		// add a space for the concatenation
		strcat(alias_command, " ");
		//loop from the first index til count and concatenate the remaining tokens
		for(int i = 1; i < count; i++) {
			strcat(alias_command, storeInvoke[i]);
			strcat(alias_command, " ");
		}
	}
	 
	//if the invoke is 0, then we add the command to the history, else the command should not be added
	if(invoke == 0) {
		add_History(strtok(fullinp, "\n"));
	} 
	
	return alias_command;
}


void print_alias() {

	for(int i = 0; i < max_alias_size; i++) {

		if(strcmp(alias_map[i].aliasName, "") == 0) {
			if(i == 0) {
				printf("Alias: No aliases have been added\n");
			}
			break;
		} 

		printf("%s = '%s'\n", alias_map[i].aliasName, alias_map[i].aliasCommand);

	}
}

void load_alias() {

	char buffer[max_buffer_size];
	//index for loop
	int index;
	FILE *fp;
	
	//allocates memory to concat the home and history_file
	file = malloc(sizeof(char) * strlen(getenv("HOME")) + strlen(alias_file) + 1);
	//copys path to file
	strcpy(file, getenv("HOME"));
	//concatenates the file and history_file 
	strcat(file, alias_file);
	

	fp = fopen(file, "r");

	if(fp == NULL) {
		fp = fopen(file, "a");
	}


	index = 0;

	while(fgets(buffer, max_buffer_size, fp) != NULL) {

		//to check that file does not contain empty inputs
		if(strcmp(buffer,"\n") == 0) {
			printf("Alias: could not read .aliases\n");
			empty_alias();
			break;
		}

		//splits the string at the first space
		char *store = strtok(buffer, " ");
		strcpy(alias_map[index].aliasName, store);

		//takes the rest of the string after the first space to store as aliasCommand
		store = strtok(NULL, "\n");
		if(store == NULL) {
			printf("Alias: could not read .aliases\n");
			empty_alias();
			break;
		} 
		else {
			strcpy(alias_map[index].aliasCommand, store);
		}

		index++;
	}

	fclose(fp);
	free(file);

}

int number_of_aliases() {
	
	//checks the amount of elements in the list
	for (int i = 0; i < max_alias_size; i++) {
		if(strcmp(alias_map[i].aliasName, "") == 0) {
			alias_len = i;
			return alias_len;
		}
	}
	return max_alias_size;
}

void empty_alias() {

	//gets the size of the alias_map
	int size = number_of_aliases();

	//clears the alias_map
	for(int i = 0; i < size; i++) {
		strcpy(alias_map[i].aliasName, "");
		strcpy(alias_map[i].aliasCommand, "");
	}
}

void save_alias() {

	//allocates memory to concat the home and alias_file 
	file = malloc(sizeof(char) *strlen(getenv("HOME")) + strlen(alias_file) + 1);
	//copys path to file
	strcpy(file, getenv("HOME"));
	//concatenates the file and .aliases 
	strcat(file, alias_file);
	
	alias_len = number_of_aliases();

	FILE *fp = fopen(file,  "w");
	for (int i = 0; i < alias_len; i++) {
		fprintf(fp, "%s %s\n", alias_map[i].aliasName, alias_map[i].aliasCommand);
	}

	fclose(fp);
	free(file);
}

//method for check if a command is an alias, if true then return 1, else -1
int is_alias(char *fullinp) {
	
	char *storeInvoke[max_array_size];
	//input to return and parse
	int count = 0;
	//gets the size of the alias_map
	int size = number_of_aliases();
	char inp[max_buffer_size];
	strcpy(inp, fullinp);
	strtok(inp, "\n");

	char *tok = strtok(inp, " ");
	//if alias is alias or unalias then return 0
	if(strcmp(tok, "alias") == 0 || strcmp(tok, "unalias") == 0) {
		return -1;
	}


	//loop to store all the values from the input in one array, split by spaces
	while(tok != NULL) {
		storeInvoke[count] = tok;
		count++;
		tok = strtok(NULL, " ");
	}


	//loop through each token and check if it exists as an alias, if so then return 1
		for(int i = 0; i < count; i++) {
			for(int j = 0; j < size; j++) {
				if(strcmp(storeInvoke[i], alias_map[j].aliasName) == 0) {
					return 1;
				}
			}
		}

		return -1;
}
