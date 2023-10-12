#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

/*

External Influences

Redirecting a stdin in C: https://stackoverflow.com/questions/26132393/redirecting-stdin-c

*/

int PIPE_CALLED = 0;

int my_system(char **parsed_tokens, int pos, pid_t pid) {
	/*
      	int i = 0;
       	for (i =  0; i <= pos; i++) {

       	    printf("my_system token %s\n", parsed_tokens[i]);
    	}
     	*/
	
 //  	printf("this is pos in my_system %d\n", pos);
//
	if (pid < 0) {
		fprintf(stderr, "Error: Forking failed");
	}
	
	if (pid == 0) {
		int arrow_found = 0;
        // only if more than 1 argument, check.
		if (pos > 1) {
			
			int i = 0;
			//int arrow_found = 0; // 0 - false, 1 - true
			int cat_found = 0;
			char ** current_command;
			current_command = malloc(sizeof(char*) * pos + 1);

			for (i = 0; i < pos-1; i++) {
				
				if (strcmp(parsed_tokens[i], ">") == 0){
					//create/overwrite
					arrow_found = 1;

					int fd = open(parsed_tokens[i+1], O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
					
					dup2(fd,1); // fd is now the stdout
					close(fd);

					execvp(current_command[0], current_command); 
					fprintf(stderr, "Error: invalid command");

					char * buffer; //the actual user input string
					size_t bufsize = 1000;
		
					size_t user_input; // how many characters.
					buffer = malloc(bufsize * sizeof(char));

					user_input = getline(&buffer, &bufsize, stdin);


					if (user_input > 1){
						printf("%s",buffer); 
					}
					else if (user_input == 1) {
						printf("\n");
					}

					//getline() returns -1 if EOF (sent by Ctrl+D)
					//terminate if EOF
					//
					while (user_input != -1) {
						buffer = malloc(user_input * sizeof(char));
						user_input = getline(&buffer, &bufsize, stdin);

						dup2(fd,1); // the txt file is stdout.
						close(fd);
						if (user_input > 1) {
						
							printf("%s",buffer);
						} 								
						else if (user_input == 1) {
							printf("\n");
						}
					}
					
					
					//execvp(current_command[0], current_command); 
					continue;
					
			
				}
				if (strcmp(parsed_tokens[i], ">>") == 0) {
					arrow_found = 1;
					int fd = open(parsed_tokens[i+1], O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
					char * buffer; //the actual user input string
					size_t bufsize = 1000;


					size_t user_input; // how many characters.
					buffer = malloc(bufsize * sizeof(char));

					user_input = getline(&buffer, &bufsize, stdin);
				
					dup2(fd,1);
					close(fd);

					

					if (user_input > 1) {
						
						printf("%s",buffer); 
					}
					else if (user_input == 1) {
						printf("\n");
					}

				//getline() returns -1 if EOF (sent by Ctrl+D)
				//terminate if EOF
					while (user_input != -1) {
						buffer = malloc(user_input * sizeof(char));
						user_input = getline(&buffer, &bufsize, stdin);

						dup2(fd,1);
						close(fd);
						if (user_input > 1) {

							printf("%s",buffer); 
						}
						else if (user_input == 1) {
							printf("\n");
						}
					}

					execvp(current_command[0], current_command); 
					fprintf(stderr, "Error: invalid command");
					//execvp(current_command[0], current_command); 
					continue;
				} 

					// input redirection
				if(strcmp(parsed_tokens[i], "<") == 0){
					arrow_found = 1;
		
					int fd = open(parsed_tokens[i+1], O_RDONLY | O_CREAT);
					if (fd < 0) {
						fprintf(stderr, "Error: invalid file\n");
						exit(-1);
					}

					if (pos >= 5){
						
						if (strstr(parsed_tokens[i+3], ".txt") != NULL) {
							//printf("help\n");
							int fd2 = open(parsed_tokens[i+3], O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);

							dup2(fd, 0); // 0 points to same as fd, which is stdin
							dup2(fd2, 1); // 1 points to same as fd2, which is stdout
							close(fd);	
							close(fd2);
							//printf("hey\n");
						}
					}
					else {
						dup2(fd, 0); // fd is now stdin
						close(fd);
					}
					execvp(current_command[0], current_command); 
					fprintf(stderr, "Error: invalid command");

					// but in the case of "wc < input.txt", don't read from stdin, 
					// https://stackoverflow.com/questions/26132393/redirecting-stdin-c
					char line[256];

					while (fgets(line, sizeof(line), stdin) != NULL) {
							
						printf("%s", line);
					}
					//execvp(current_command[0], current_command); 
					continue;
		
				}	



				if (arrow_found == 1) {
					int l = 0;
					for (l = 0; l < i; l++) {
						current_command[l] = NULL;
					}
				}	
				else {
					current_command[i] = malloc(sizeof(char) * strlen(parsed_tokens[i]));
					current_command[i] = parsed_tokens[i];
					
				}
				
				
			}// end of for loop	


		}// end of if pos > 1
		// if pos < 1, for situations like "cat" and "ls"
        
		//if ((pos <= 1) || (arrow_found == 0)) {	
		/*
		if (pos <= 1) {	
			execvp(parsed_tokens[0], parsed_tokens); 
			// step 5: Handle output redirection, such as cat > output.txt
			fprintf(stderr, "Error: invaid command");
			exit(-1);
		}
		*/
		execvp(parsed_tokens[0], parsed_tokens); 
		fprintf(stderr, "Error: invalid command");
		exit(-1);
	}// end of if pid == 0
    wait(NULL);
    return 0;

}

/* handles pipes. pipe is a special kind of redirection.*/
int pipe_redirect(char ** parsed_tokens, int pos){
	// WHAT IS A PIPE?????
	// A connection between two processes, such that the standard output from one process becomes the standard
	// input of the other process. (INTER PROCESS COMMUNICATION)
	// THE PROCESS:
	// pipe() finds the first two available positions in the process's open file table and allocates them for
	// the read and write ends of the pipe.
	// write p[1] ----> p[0] read
	//
	//
	// WHAT IS A FILE DESCRIPTOR
	// Array of opened files -- fds[0]: standard input, fds[1]:stdout, fds[2]: stderr
	// create pipe
	PIPE_CALLED = 1;

	/*
	int fds[2];

	if (pipe(fds) == -1) {
		fprintf(stderr, "Error: Pipe failed");
		return 1;
	}
	
	
	*/
	

	
	char ** current_command;
	current_command = malloc(sizeof(char*) * pos + 1);

	int pipe_found = 0;
	int c; 
	for (c = 0; c < pos; c++){
		pipe_found = 0;

		if (strcmp(parsed_tokens[c], "|") == 0) {
			int fds[2];

			if (pipe(fds) == -1) {
				fprintf(stderr, "Error: Pipe failed");
				return 1;
			}

			pipe_found = 1;

			pid_t child_1, child_2;
			child_1 = fork();
			// both children run concurrently
			//printf("idkwhatswriong\n");
				
			if (child_1 < 0) {
				fprintf(stderr, "Error: Fork failed in child_1");
				return 1;
			}
			if (child_1 == 0) { 
				// child_1 redirects its output to the write end of the pipe				 
				dup2(fds[1], 1); //which is stdout
				// 2 file descriptors that point to same one
				close(fds[0]);
				close(fds[1]);
				//execvp(current_command[0], current_command);
				
				my_system(current_command, c, child_1);

			}

			int l = 0;
			for (l = 0; l < c; l++) {
				current_command[l] = NULL;
			}

			int j;
			int k;
			k = 0;	
			// index c is the pipe.
			for (j = c + 1; j < pos; j++) {
				
				if (strcmp(parsed_tokens[j], "|") == 0){
					//printf("%s\n", current_command[k]);
					break;
					
					
					
				}
				current_command[k] = malloc (sizeof(char)  * strlen(parsed_tokens[j]));
				current_command[k] = parsed_tokens[j];
				
				

				
				

				k++;

			}

			child_2 = fork();
			if (child_1 < 0) {
				fprintf(stderr, "Error: Fork failed in child_2");
				return 1;
			}
			if (child_2 == 0) {
				/*
				dup2(fds[0], 0);
				close(fds[0]);
				close(fds[1]);
				*/
			//	execvp(current_command[0], current_command);
				// child_1 redirects its output to the write end of the pipe
						
				dup2(fds[0], 0); //which is stdin
				// 2 file descriptors that point to same one
				close(fds[0]);
				close(fds[1]);
				//execvp(current_command[0], current_command);
				
				// k is length of current command
				printf("current %s\n", current_command[0]);
				my_system(current_command, k, child_2);

			}
			close(fds[0]);
			close(fds[1]);
			
			waitpid(child_1, NULL, 0);
			waitpid(child_2, NULL, 0);

		}
		


		if (pipe_found == 1) {
			int l = 0;
			for (l = 0; l < c; l++) {
				current_command[l] = NULL;
			}
		}	
		else {
			current_command[c] = malloc(sizeof(char) * strlen(parsed_tokens[c]));
			current_command[c] = parsed_tokens[c];
			
		}

	} //end of for loop

	/*
	while (strcmp(parsed_tokens[i], "|") != 0) {			
		
		current_command[i] = malloc(sizeof(char) * strlen(parsed_tokens[i]));
		current_command[i] = parsed_tokens[i];
		//printf("%s\n", current_command[i]);
		i++;
	}

	current_command[i+1] = NULL;
		
	// parent (shell) process creates pipe and forks two children.
	pid_t child_1, child_2;
	child_1 = fork();
	// both children run concurrently
	//printf("idkwhatswriong\n");
		
	if (child_1 < 0) {
		fprintf(stderr, "Error: Fork failed in child_1");
		return 1;
	}
	if (child_1 == 0) { 
		// child_1 redirects its output to the write end of the pipe				 
		dup2(fds[1], 1); //which is stdout
		// 2 file descriptors that point to same one
		close(fds[0]);
		close(fds[1]);
		//execvp(current_command[0], current_command);
		
		my_system(current_command, i, child_1);

	}
	
	

	int l = 0;
	for (l = 0; l < i; l++) {
		current_command[l] = NULL;
	}
	
	int j;
	int k;
	k = 0;	
	// index i is the pipe.
	for (j = i + 1; j < pos; j++) {
		current_command[k] = malloc (sizeof(char)  * strlen(parsed_tokens[j]));
		current_command[k] = parsed_tokens[j];
		//printf("%s\n", current_command[k]);
		k++;
	}
	current_command[k+1] = NULL;
	
	child_2 = fork();
	if (child_1 < 0) {
		fprintf(stderr, "Error: Fork failed in child_2");
		return 1;
	}
	if (child_2 == 0) {
		dup2(fds[0], 0);
		close(fds[0]);
		close(fds[1]);
	//	execvp(current_command[0], current_command);
		// child_1 redirects its output to the write end of the pipe
				 
		dup2(fds[0], 0); //which is stdout
		// 2 file descriptors that point to same one
		close(fds[0]);
		close(fds[1]);
		//execvp(current_command[0], current_command);
		
		// k is length of current command
		my_system(current_command, k, child_2);

	}
	close(fds[0]);
	close(fds[1]);
	
	waitpid(child_1, NULL, 0);
	waitpid(child_2, NULL, 0);
	*/
	
	// call my_system
	return 0;
}

int main() {

    while (1) {
		PIPE_CALLED = 0;        
        char path[100];
        getcwd(path, 100);

        char * bn = malloc((sizeof(path)) * sizeof(char*));
        bn = basename(path);


        /* 1. Write a simple command parser */
        char * buffer; //the actual user input string
        size_t bufsize = 1000;

        size_t user_input; // how many characters.
        buffer = malloc(bufsize * user_input * sizeof(char));

        printf("[nyush %s]$ ", bn);
        fflush(stdout);
        user_input = getline(&buffer, &bufsize, stdin);
        
        //if there is at least 1 input
        if (user_input > 1) {
            char * token = strtok(buffer, "\t\n ");
            char ** parsed_tokens;
            parsed_tokens = malloc(sizeof(char*) * (user_input + 1));

            int pos = 0;
            while (token != NULL) {
                parsed_tokens[pos] = malloc(strlen(token) * sizeof(char) + 1);
                parsed_tokens[pos] = token;
                token = strtok(NULL, "\t\n ");
                pos++;
            }   
            
            // terminated by NULL pointer.
            parsed_tokens[pos] = NULL;

	    // check if there is a pipe
	    
			int i;
			int pipe_count = 0;
			for (i = 0; i<pos; i++) {
				if (strcmp(parsed_tokens[i], "|") == 0) {
					pipe_count++;
					
				}

				/*
				if (strcmp(parsed_tokens[i], "|") == 0) {
					
					pipe_redirect(parsed_tokens, pos);	
					break;
					PIPE_CALLED = 1;
				}
				*/
			}

			if (pipe_count > 0){
				PIPE_CALLED = 1;

			}

			if (PIPE_CALLED == 1){
				pipe_redirect(parsed_tokens, pos);	
			}
			/*
			for (i = 0; i < pipe_count; i++) {
				pipe_redirect(parsed_tokens, pos);	
				PIPE_CALLED = 1;
			}
			*/
            // 4 built in commands: cd, exit, jobs, fg
            //printf("%d\n", strcmp("cd", "cd"));

            if (strcmp(parsed_tokens[0], "cd") == 0){
                chdir(parsed_tokens[1]);
                
            }

			else if (strcmp(parsed_tokens[0], "jobs") == 0) {
				printf("");
			}
            else if (strcmp(parsed_tokens[0], "exit") == 0){
                exit(0);
            }

            else if (PIPE_CALLED == 0){
                pid_t pid;
				pid = fork(); 
                my_system(parsed_tokens, pos, pid);
            }
	
            
        }
        

        
        
    }
    

    return 0;
}
