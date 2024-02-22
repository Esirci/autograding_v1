#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>



#define SIZE 1024
#define TRUE 1

volatile sig_atomic_t sig_int = 0;
volatile sig_atomic_t sig_term = 0;
volatile sig_atomic_t sig_tstp = 0;




void forkError();
void openFileError();
void writeError();
void pipeError();
void closePipeError();
void dupError();
void execlError();
void closeFileError();
void waitError();
void pipe_no_redirection_no(int fd, char buffer[SIZE]);
void sigint_handler(){
	sig_int = 1;
}
void sigterm_handler(){
	sig_term = 1;
}
void sigtstp_handler(){
	sig_tstp = 1;
}

int main(int argc, char const *argv[])
{	
	// SIGINT
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;

	// SIGTERM
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigterm_handler;
	sigaction(SIGTERM, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;

	// SIGTSTP
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigtstp_handler;
	sigaction(SIGTSTP, &sa, NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;

	int bytes_read;
	char buffer[SIZE];
	char writeLogFileBuffer[SIZE];

	char enter_shell[2] = {"$ "};

	printf("Hello World!\n");

	while(TRUE){

		time_t rawtime;
	    struct tm *timeinfo;
	    char filename[SIZE];

	    time(&rawtime);
	    timeinfo = localtime(&rawtime);
	    strftime(filename, SIZE, "%Y-%m-%d_%H-%M-%S.log", timeinfo);

		// open log file
		int fd = open(filename, O_WRONLY | O_CREAT, 0666);
		if(fd == -1){
			openFileError();
		}

		int pipe_have = 0;
		int redirection_forward = 0;
		int redirection_backward = 0;

		// shell begins to $
		if(write(STDOUT_FILENO, enter_shell, 2) == -1){
			writeError();
		}

		bzero(buffer, SIZE);
		bzero(writeLogFileBuffer, SIZE);

		// user input
		bytes_read = read(STDIN_FILENO, buffer, SIZE);

		if(bytes_read < 0){
			fprintf(stderr, "Error reading input\n");
		}

		buffer[bytes_read] = '\0';

		if(strncmp(buffer, "q", 2) == 0){
			break;
		}
		// Check pipe and redirections have or not
		if(strchr(buffer, '|') != NULL){
			pipe_have = 1;
		}	
		if(strchr(buffer, '>') != NULL){
			redirection_forward = 1;
		}
		if(strchr(buffer, '<') != NULL){
			redirection_backward = 1;
		}

		char *token;
		char commandList[20][512];

		int numberOfProcess = 0;

		// pipe var redirection yok.
		if(pipe_have == 1 && redirection_forward == 0 && redirection_backward == 0){
			// parsing command with pipe
			token = strtok(buffer, "|");

			while(token != NULL){
				strcpy(commandList[numberOfProcess], token);
				token = strtok(NULL, "|");
				numberOfProcess++;
			}

			int pipefd[numberOfProcess-1][2];
			// create number of process - 1 pipe
			for(int i=0;i<numberOfProcess-1;i++){
				if(pipe(pipefd[i]) == -1){
					pipeError();
				}
			}

			pid_t pid[numberOfProcess];
			int status1;

			for(int i=0;i<numberOfProcess;i++){
				// create new process
				pid[i] = fork();

				
				if(pid[i] < 0){ // error
					forkError();
				}
				else if(pid[i] == 0){ // child process
					if(i == 0){ // first command
						
						if(close(pipefd[i][0]) == -1){ // Read end is unused
							closePipeError();
						}
		 				if(pipefd[i][1] != STDOUT_FILENO){ // defensive check
		 					if(dup2(pipefd[i][1],STDOUT_FILENO) == -1){
		 						dupError();
		 					}
		 					if(close(pipefd[i][1]) == -1){
		 						closePipeError();
		 					}
		 				}
					}
					else if(i == (numberOfProcess-1)){ // last command
						// close previous unnecessary pipe 
						for(int j=0;j<(i-1);j++){			
							if(close(pipefd[j][0]) == -1){
								closePipeError();
							}
				 			if(close(pipefd[j][1]) == -1){
				 				closePipeError();
				 			}
				 		}

						if(close(pipefd[i-1][1]) == -1){ // Write end is unused
							closePipeError();
						}
						if(pipefd[i-1][0] != STDIN_FILENO){ // defensive check
		 					if(dup2(pipefd[i-1][0],STDIN_FILENO) == -1){
		 						dupError();
		 					}
		 					if(close(pipefd[i-1][0]) == -1){
		 						closePipeError();
		 					}
		 				}
					}
					else{ // middle command
						// close previous unnecessary pipe 
						for(int j=0;j<(i-1);j++){			
							if(close(pipefd[j][0]) == -1){
								closePipeError();
							}
				 			if(close(pipefd[j][1]) == -1){
				 				closePipeError();
				 			}
				 		}
						
						if(close(pipefd[i-1][1]) == -1){
							closePipeError();
						}
						if(close(pipefd[i][0]) == -1){
							closePipeError();
						}
						if(pipefd[i-1][0] != STDIN_FILENO){ // defensive check
							if(dup2(pipefd[i-1][0], 0) == -1){
								dupError();
							}
							if(close(pipefd[i-1][0]) == -1){
								closePipeError();
							}
						}
						if(pipefd[i][1] != STDOUT_FILENO){ // defensive check
							if(dup2(pipefd[i][1], 1) == -1){
								dupError();
							}
							if(close(pipefd[i][1]) == -1){
								closePipeError();
							}
						}
					}
					// write to log file
	 				sprintf(writeLogFileBuffer, "PID: %d PPID: %d Command: ", getpid(),getppid());
	 				strcat(writeLogFileBuffer, commandList[i]);
		 			strcat(writeLogFileBuffer, "\n");
	 				if(write(fd, writeLogFileBuffer, strlen(writeLogFileBuffer)) == -1){
	 					writeError();
	 				}
					if(execl("/bin/sh", "sh", "-c", commandList[i], (char *) NULL) == -1){
						execlError();
					}
				}
			}

			if(sig_int == 1 || sig_term == 1 || sig_tstp == 1){
				for(int k=0;k<numberOfProcess;k++){
					kill(pid[k],SIGKILL);
				}
				sig_int = 0;
				sig_term = 0;
				sig_tstp = 0;
			}

			for(int i=0;i<numberOfProcess-1;i++){
				if(close(pipefd[i][0]) == -1){ // Close read end
		        	closePipeError();
		        }
		        if(close(pipefd[i][1]) == -1){ // Close write end
					closePipeError();
				}
			}

			for(int i=0;i<numberOfProcess;i++){ // wait to childs
			  	if(waitpid(pid[i], &status1, 0) == -1){ // wait for child to finish
		        	waitError();
		        }
				if(WEXITSTATUS(status1) == 127){
					fprintf(stderr, "Probably could not invoke shell\n");
				}
			}
		}
		// pipe yok redirection yok
		else if(pipe_have == 0 && redirection_forward == 0 && redirection_backward == 0){
			pipe_no_redirection_no(fd, buffer);

		} 
		// pipe yok redirection var
		else if(pipe_have == 0 && (redirection_forward == 1 || redirection_backward == 1)){
			char output[SIZE];
			char command[SIZE];
			char temp[SIZE];

			if(redirection_forward == 1){
				strcpy(temp, buffer);
				// parse buffer with >
				token = strtok(buffer, ">");

			    strcpy(command, token);

			    token = strtok(NULL, " ");
			    if(token != NULL) {
			       	while(token[0] == ' '){
			        	token++;
			        }
			        while(token[strlen(token)-1] == ' ' || token[strlen(token)-1] == '\n'){
			            token[strlen(token)-1] = '\0';
			        }
			        strcpy(output, token);
			    }
			    
			    int status_2;

			    pid_t pid = fork();
		  		if(pid < 0){ // error
					forkError();
				}
				else if(pid == 0){ // child process
					int out_fd = open(output, O_WRONLY|O_CREAT|O_TRUNC, 0666);
					if(out_fd == -1){
						openFileError();
					}
					// redirect standard output to file
        			if(dup2(out_fd, STDOUT_FILENO) == -1){
        				dupError();
        			}

        			if(close(out_fd) == -1){
        				closeFileError();
        			}

				    // write to log file
	 				sprintf(writeLogFileBuffer, "PID: %d PPID: %d Command: ", getpid(),getppid());
	 				strcat(writeLogFileBuffer, temp);
	 				strcat(writeLogFileBuffer, "\n");
	 				if(write(fd, writeLogFileBuffer, strlen(writeLogFileBuffer)) == -1){
	 					writeError();
	 				}
				    if(execl("/bin/sh", "sh", "-c", command, (char *) NULL) == -1){
						execlError();
					}

				}
				else{ // parent
					if(sig_int == 1 || sig_term == 1 || sig_tstp == 1){
						kill(pid,SIGKILL);
						sig_int = 0;
						sig_term = 0;
						sig_tstp = 0;
					}
			        if(waitpid(pid, &status_2, 0) == -1){ // wait for child to finish
			        	waitError();
			        }
			       
				}
			}
			else if(redirection_backward == 1){
				strcpy(temp, buffer);
				// parse buffer with <
				token = strtok(buffer, "<");

			    strcpy(command, token);

			   	token = strtok(NULL, " ");
			    if(token != NULL) {
			       	while(token[0] == ' '){
			        	token++;
			        }
			        while(token[strlen(token)-1] == ' ' || token[strlen(token)-1] == '\n'){
			            token[strlen(token)-1] = '\0';
			        }
			        strcpy(output, token);
			    }

			    int status_2;

			    pid_t pid = fork();
		  		if(pid < 0){ // error
					forkError();
				}
				else if(pid == 0){ // child process
        			int in_fd = open(output, O_RDONLY, 0666);
        			if(in_fd == -1){
        				openFileError();
        			}
					// redirect standard input to file
        			if(dup2(in_fd, STDIN_FILENO) == -1){
        				dupError();
        			}
        			if(close(in_fd) == -1){
        				closeFileError();
        			}
        			
				
			        // write to log file
	 				sprintf(writeLogFileBuffer, "PID: %d PPID: %d Command:", getpid(),getppid());
	 				strcat(writeLogFileBuffer, temp);
	 				strcat(writeLogFileBuffer, "\n");
	 				if(write(fd, writeLogFileBuffer, strlen(writeLogFileBuffer)) == -1){
	 					writeError();
	 				}

				    if(execl("/bin/sh", "sh", "-c", command, (char *) NULL) == -1){
						execlError();
					}
				}
				else{ // parent
					if(sig_int == 1 || sig_term == 1 || sig_tstp == 1){
						kill(pid,SIGKILL);
						sig_int = 0;
						sig_term = 0;
						sig_tstp = 0;
					}
			        if(waitpid(pid, &status_2, 0) == -1){ // wait for child to finish
			        	waitError();
			        }
			        if(WEXITSTATUS(status_2) == 127){
						fprintf(stderr, "Probably could not invoke shell\n");
					}
				}
			}
		}
		// pipe var redirection var
		else if(pipe_have == 1 && (redirection_forward == 1 || redirection_backward == 1)){
			// parse bugger with | 
			token = strtok(buffer, "|");

			while(token != NULL){
				strcpy(commandList[numberOfProcess], token);
				token = strtok(NULL, "|");
				numberOfProcess++;
			}

			int pipefd[numberOfProcess-1][2];
			// create pipe 
			for(int i=0;i<numberOfProcess-1;i++){
				if(pipe(pipefd[i]) == -1){
					pipeError();
				}
			}

			pid_t pid[numberOfProcess];
			int status1;
			int i;
			for(i=0;i<numberOfProcess;i++){
				// create new process
				pid[i] = fork();

				// error
				if(pid[i] < 0){
					forkError();
				}
				else if(pid[i] == 0){ // child process
					if(i == 0){ // pipe first part
						if(close(pipefd[i][0]) == -1){ // Read end is unused
							closePipeError();
						}
		 				if(pipefd[i][1] != STDOUT_FILENO){ // defensive check
		 					if(dup2(pipefd[i][1],STDOUT_FILENO) == -1){
		 						dupError();
		 					}
		 					if(close(pipefd[i][1]) == -1){
		 						closePipeError();
		 					}
		 				}
					}
					else if(i == (numberOfProcess-1)){ // pipe last part
						// close previous unnecessary pipe 
						for(int j=0;j<(i-1);j++){			
							if(close(pipefd[j][0]) == -1){
								closePipeError();
							}
				 			if(close(pipefd[j][1]) == -1){
				 				closePipeError();
				 			}
				 		}

						if(close(pipefd[i-1][1]) == -1){ // Write end is unused
							closePipeError();
						}
						if(pipefd[i-1][0] != STDIN_FILENO){ // defensive check
		 					if(dup2(pipefd[i-1][0],STDIN_FILENO) == -1){
		 						dupError();
		 					}
		 					if(close(pipefd[i-1][0]) == -1){
		 						closePipeError();
		 					}
		 				}
					}
					else{ // middle child
						// close previous unnecessary pipe 
						for(int j=0;j<(i-1);j++){			
							if(close(pipefd[j][0]) == -1){
								closePipeError();
							}
				 			if(close(pipefd[j][1]) == -1){
				 				closePipeError();
				 			}
				 		}
						if(close(pipefd[i-1][1]) == -1){
							closePipeError();
						}
						if(close(pipefd[i][0]) == -1){
							closePipeError();
						}
						if(pipefd[i-1][0] != STDIN_FILENO){
							if(dup2(pipefd[i-1][0], 0) == -1){
								dupError();
							}
							if(close(pipefd[i-1][0]) == -1){
								closePipeError();
							}
						}
						if(pipefd[i][1] != STDOUT_FILENO){
							if(dup2(pipefd[i][1], 1) == -1){
								dupError();
							}
							if(close(pipefd[i][1]) == -1){
								closePipeError();
							}
						}
					}
					// If have redirection in pipe
					if(strchr(commandList[i], '>') != NULL){
						char *token;
						char command[SIZE];
						char output[SIZE];
						char temp[SIZE];
						strcpy(temp, commandList[i]);
						token = strtok(commandList[i], ">");

						strcpy(command, token);

						token = strtok(NULL, " ");
					    if(token != NULL) {
					       	while(token[0] == ' '){
					        	token++;
					        }
					        while(token[strlen(token)-1] == ' ' || token[strlen(token)-1] == '\n'){
					            token[strlen(token)-1] = '\0';
					        }
					        strcpy(output, token);
					    }

						int out_fd = open(output, O_WRONLY|O_CREAT|O_TRUNC, 0666);
						if(out_fd == -1){
							openFileError();
						}

						// redirect standard output to file
				        if(dup2(out_fd, STDOUT_FILENO) == -1){
				        	dupError();
				        }

				        if(close(out_fd) == -1){
				        	closeFileError();
				        }
				       
						sprintf(writeLogFileBuffer, "PID: %d PPID: %d Command: ", getpid(),getppid());
		 				strcat(writeLogFileBuffer, temp);
		 				strcat(writeLogFileBuffer, "\n");
		 				if(write(fd, writeLogFileBuffer, strlen(writeLogFileBuffer)) == -1){
		 					writeError();
		 				}
						if(execl("/bin/sh", "sh", "-c", command, (char *) NULL) == -1){
							execlError();
						}
					}
					else if(strchr(commandList[i], '<') != NULL){
						char *token;
						char command[SIZE];
						char output[SIZE];
						char temp[SIZE];
						strcpy(temp, commandList[i]);
						token = strtok(commandList[i], "<");

						strcpy(command, token);

						token = strtok(NULL, " ");
					    if(token != NULL) {
					       	while(token[0] == ' '){
					        	token++;
					        }
					        while(token[strlen(token)-1] == ' ' || token[strlen(token)-1] == '\n'){
					            token[strlen(token)-1] = '\0';
					        }
					        strcpy(output, token);
					    }

					    // open output file
        				int in_fd = open(output, O_RDONLY, 0666);
        				if(in_fd == -1){
        					openFileError();
        				}
        				// redirect standard input to file
        				if(dup2(in_fd, STDIN_FILENO) == -1){
        					dupError();
        				}
        				if(close(in_fd) == -1){
        					closeFileError();
        				}

				       	sprintf(writeLogFileBuffer, "PID: %d PPID: %d Command: ", getpid(),getppid());
		 				strcat(writeLogFileBuffer, temp);
		 				strcat(writeLogFileBuffer, "\n");
		 				if(write(fd, writeLogFileBuffer, strlen(writeLogFileBuffer)) == -1){
		 					writeError();
		 				}
						if(execl("/bin/sh", "sh", "-c", command, (char *) NULL) == -1){
							execlError();
						}
					}
					else{
						
		 				sprintf(writeLogFileBuffer, "PID: %d PPID: %d Command:", getpid(),getppid());
		 				strcat(writeLogFileBuffer, commandList[i]);
		 				strcat(writeLogFileBuffer, "\n");
		 				if(write(fd, writeLogFileBuffer, strlen(writeLogFileBuffer)) == -1){
		 					writeError();
		 				}
						if(execl("/bin/sh", "sh", "-c", commandList[i], (char *) NULL) == -1){
							execlError();
						}
					}
				}
			}

			if(sig_int == 1 || sig_term == 1 || sig_tstp == 1){
				for(int k=0;k<numberOfProcess;k++){
					kill(pid[k],SIGKILL);
				}
				sig_int = 0;
				sig_term = 0;
				sig_tstp = 0;
			}
				
			for(int i=0;i<numberOfProcess-1;i++){
				if(close(pipefd[i][0]) == -1){ // Close read end
		        	closePipeError();
		        }
		        if(close(pipefd[i][1]) == -1){ // Close write end
					closePipeError();
				}
			}

			for(int i=0;i<numberOfProcess;i++){ // parent process
			 	if(waitpid(pid[i], &status1, 0) == -1){ // wait for child to finish
			    	waitError();
			    }	
				if(WEXITSTATUS(status1) == 127){
					fprintf(stderr, "Probably could not invoke shell\n");
				}
			}
		}
		
		if(close(fd) == -1){
			closeFileError();
		}
	}

	return 0;
}


void forkError(){
	perror("fork");
}
void openFileError(){
	perror("open file");
}
void writeError(){
	perror("write");
}
void pipeError(){
	perror("pipe");
}
void closePipeError(){
	perror("close pipe");
}
void dupError(){
	perror("dup2");
}
void execlError(){
	perror("execl");
}
void closeFileError(){
	perror("close file");
}
void waitError(){
	perror("wait");
}

void pipe_no_redirection_no(int fd, char buffer[SIZE]){
	int status;
    char writeLogFileBuffer[SIZE];


	pid_t pid = fork();

	if(pid < 0){ // error
		forkError();
	}
	else if(pid == 0){ // child process
		sprintf(writeLogFileBuffer, "PID: %d PPID: %d Command: ", getpid(),getppid());

 		strcat(writeLogFileBuffer, buffer);
 		strcat(writeLogFileBuffer, "\n");
 		if(write(fd, writeLogFileBuffer, strlen(writeLogFileBuffer)) == -1){
 			writeError();
 		}
		if(execl("/bin/sh", "sh", "-c", buffer, (char *) NULL) == -1){
			execlError();
		}
	}
	else{ // parent process
		if(sig_int == 1 || sig_term == 1 || sig_tstp == 1){
			kill(pid,SIGKILL);
			sig_int = 0;
			sig_term = 0;
			sig_tstp = 0;
		}
		if(waitpid(pid, &status, 0) == -1){
			waitError();
		}
		if(WEXITSTATUS(status) == 127){
			fprintf(stderr, "Probably could not invoke shell\n");
		}
	}
}
