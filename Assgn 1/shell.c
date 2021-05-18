#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


int MAX = 128; //128 chars allowed
char **historyCmd;

void printPrompt() {
	char s[MAX];
	fflush(stdout);
	printf("MTL458:%s$ ", getcwd(s, MAX));
}


void echo(char ** args){
// implement echo
	int status;
	pid_t id = fork();

	if(id == -1){
		printf("\necho: Error in forking..\n");
	}
	else if(id == 0){
		fflush(stdout);
		execvp(args[0], args);
		fflush(stdout);
	}
	else{
		wait(NULL);
	}
}

void ls(char **args){
	int status;
	pid_t id = fork();

	if(id == -1){
		printf("\nls: Error in forking..\n");
	}
	else if(id == 0){
		fflush(stdout);
		execvp(args[0], args);
		fflush(stdout);
	}
	else{
		wait(NULL);
	}
}

void cat(char ** args){
// implement cat
	int status;
	pid_t id = fork();

	if(id == -1){
		printf("\ncat: Error in forking..\n");
	}
	else if(id == 0){
		fflush(stdout);
		execvp(args[0], args);
		fflush(stdout);
	}
	else{
		wait(NULL);
	}
}


void history(char cmd[]){
	// implement history

	int j =0;
	for(;j<5;j++){
		if(historyCmd[j] == NULL){
			historyCmd[j] = malloc(MAX*sizeof(char));
			strncpy(historyCmd[j], cmd, MAX);
			break;
		}
	}
	if(j==5){
		//all 5 commands present
		free(historyCmd[0]);
		historyCmd[0] = malloc(MAX*sizeof(char));
		strncpy(historyCmd[0], historyCmd[1], MAX);
		strncpy(historyCmd[1], historyCmd[2], MAX);
		strncpy(historyCmd[2], historyCmd[3], MAX);
		strncpy(historyCmd[3], historyCmd[4], MAX);
		strncpy(historyCmd[4], cmd, MAX);
	}
}

void printHistory(){
	fflush(stdout);
	for(int j=0;j<5;j++){
		if(historyCmd[j]!= NULL){
			printf("%d- %s",(j+1), historyCmd[j]);
		}
		else break;
	}
	fflush(stdout);
}

void exec_sleep(char ** args){
	int status;
	pid_t id = fork();

	if(id == -1){
		printf("\nsleep: Error in forking..\n");
	}
	else if(id == 0){
		fflush(stdout);
		execvp(args[0], args);
		fflush(stdout);
	}
	else{
		wait(NULL);
	}
}


void signalHandler(int sig_num){
	fprintf(stdout, "\n");
	fflush(stdout);
	free(historyCmd);
	exit(0);
}

int main(int argc, char **argv) {
	signal(SIGINT, signalHandler);

	//list of built-in commands..
	char builtInCmd[3][128] = {
		"cd",
		"exit",
		"help"
	};

	char *HOME = malloc(MAX*sizeof(char));
	getcwd(HOME, MAX);
	historyCmd = (char**) malloc(5 * sizeof(char*));

	while(1){

		printPrompt();
		char input[MAX];
		char * temp_ = (char *) malloc(MAX*sizeof(char));
		size_t len = 128;
		int line = getline(&temp_, &len, stdin);
		if(line == -1){
			fprintf(stderr, "ERROR!");
		}
		else{
			if(strlen(temp_) > MAX){
				printf("\nChar limit(MAX=128) exceeded.\n");
				continue;
			}
			else{
				strcpy(input, temp_);
			}
		}
		free(temp_);
		// fgets(input, MAX, stdin);

		if(strcmp(input, "\n") != 0){
			history(input);
		}

		char ** args = malloc(MAX*sizeof(char*));
		args[0] = malloc(MAX*sizeof(char));
		args[1] = malloc(MAX*sizeof(char));

		sscanf(input, "%[^ ^\n] %[^\n]", args[0], args[1]);

		char buf[128] = "";
		if(strcmp(args[1], "") !=0){

			char *ptr = malloc(MAX*sizeof(char));
			ptr = strtok(args[1], " ");
			int arg_idx = 1;

			int quote = 0;
			while(ptr != NULL){

				char *stripQuotes = malloc(MAX*sizeof(char));
				int i,j;
				if(ptr[0] != '"'){
					stripQuotes[0] = ptr[0];
					j=1;
				}
				else{
					j=0;
					if(quote == 1){
						quote=0;
					}
					else if(quote==0){
						quote=1;
					}
				}

				for(i=1; i<strlen(ptr);i++){
					if(ptr[i]=='"' && ptr[i-1] != '\\'){
						if(quote == 1){
							quote = 0;
						}
						else if(quote==0){
							quote=1;
						}
						continue;
					}
					stripQuotes[j++] = ptr[i];
				}

				if(quote==1){
					strcat(buf, stripQuotes);
					strcat(buf, " ");
				}
				else if(quote==0){
					strcat(buf, stripQuotes);
					args[arg_idx] = strdup(buf);
					memset(buf, '\0', sizeof buf);      //clear the buffer
					arg_idx++;
				}

				ptr = strtok(NULL, " ");
				free(stripQuotes);
			}
			args[arg_idx] = NULL;
			free(ptr);

		}

		else args[1] = NULL;

		// printf("\n{args0: %s, args1: %s}\n", args[0], args[1]);


		if(strcmp(args[0], "cd") ==0){
			if(args[1] == NULL){
				int home = chdir(HOME);
			}
			else if(strcmp(args[1], "~") == 0){
				int home_ = chdir(HOME);
			}
			else{
				int res = chdir(args[1]); //returns 0 on success
			}
		}
		else if(strcmp(args[0], "exit") ==0){
			exit(0);
		}
		else if(strcmp(args[0], "") ==0){

		}
		else if(strcmp(args[0], "ls") ==0){
			args[0] = strdup("ls");
			ls(args);
		}
		else if(strcmp(args[0], "cat") ==0){
			args[0] = strdup("cat");
			cat(args);
		}
		else if(strcmp(args[0], "echo") ==0){
			args[0] = strdup("echo");
			echo(args);
		}
		else if(strcmp(args[0], "sleep") ==0){
			args[0] = strdup("sleep");
			exec_sleep(args);
		}
		else if(strcmp(args[0], "history") ==0){
			printHistory();
		}
		else{
			args[0] = strdup(args[0]);
			pid_t id = fork();
				if(id == -1){
					printf("\nmain: Error in forking..\n");
				}
				else if(id == 0){
					fflush(stdout);
					if(execvp(args[0], args) < 0){
						printf("%s: command not found..\n", args[0]);
					}
					fflush(stdout);
					exit(0);
				}
				else{
					wait(NULL);
					fflush(stdout);
			}
		}

		free(args);

	}

	free(HOME);
	return 0;
}
