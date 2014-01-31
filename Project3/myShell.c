/* myShell.c

Rappaport, Elliot D
ECE357: Operating Systems
October 16, 2013
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

void redirect(int mode, int std, char *redir_file){
	int fd;
	if (mode == 1) fd = open(redir_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (mode == 2) fd = open(redir_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	else if (mode == 3) fd = open(redir_file, O_RDONLY);
	if (fd < 0){
		fprintf(stderr, "Unable to open %s for redirect.\n%s\n", redir_file, strerror(errno));
		exit(-1);
	}
	if (dup2(fd, std) < 0){
		fprintf(stderr, "Unable to dup2 %s.\n%s\n", redir_file, strerror(errno)); exit(-1);
	}
	if (close(fd) != 0){
		fprintf(stderr, "Unable to close redirect%s\n%s\n", redir_file, strerror(errno));
		exit(-1);
	}
}

int main(int argc, char const *argv[]){
	FILE *cmndPath; char **args;
	// Some help from: http://stackoverflow.com/questions/1585186/array-of-pointers-initialization
	int numArgs = 10; args = (char**) calloc(numArgs, sizeof(char*)*numArgs);
	if (args == NULL) {fprintf(stderr, "Unable to calloc for arguments.\n"); exit(-1);}
	if (argc == 1) cmndPath = stdin;
	else if (argc == 2) {
		cmndPath = fopen(argv[1], "r");
		if (cmndPath == NULL){
			fprintf(stderr, "Failed to open %s.\n%s\n", argv[1], strerror(errno)); exit(-1);
		}
	}
	else {fprintf(stderr, "Expecting only 0 or 1 input file.\n"); exit(-1);}

	char *line = NULL; size_t len = 0; ssize_t read;
	while ((read = getline(&line, &len, cmndPath)) != -1){
		if (!strncmp(line, "#", 1)) continue;
		int redir_o = 0, redir_e = 0, redir_i = 0;
		char *out, *err, *in, *cmnd;
		if (line[read-1] == *"\n") line[read-1] = *" ";
    	cmnd = strtok(line, " ");
    	args[0] = cmnd;
    	int i = 1;
    	while(cmnd != NULL){
    		if (i > numArgs-1) {
    			numArgs+=5; args = (char**) realloc (args, sizeof(char*)*numArgs);
    			if (args == NULL) {fprintf(stderr, "Unable to calloc for arguments.\n"); exit(-1);}
    		}
        	cmnd = strtok(NULL, " ");
        	if (cmnd != NULL){
        		if (!strncmp(cmnd, "#", 1)) cmnd=NULL; //midline comment ends cmnd
	        	else if (!strncmp(cmnd, ">>", 2)) {redir_o = 2; out = cmnd+2;}
	        	else if (!strncmp(cmnd, ">", 1)) {redir_o = 1; out = cmnd+1;}
	        	else if (!strncmp(cmnd, "<", 1)) {redir_i = 1; in = cmnd+1;}
				else if (!strncmp(cmnd, "2>>", 3)) {redir_e = 2; err = cmnd+3;}
	        	else if (!strncmp(cmnd, "2>", 2)) {redir_e = 1; err = cmnd+2;}
	        	else{ args[i] = cmnd; i++;}
	        }
	        else args[i] = cmnd;
    	}

    	struct timeval startTime; //get time right before forking
    	if (gettimeofday(&startTime, NULL) == -1) {
    		fprintf(stderr, "Unable to get current time at start.\n%s\n", strerror(errno));
    		exit(-1);
    	}
		int pid = fork();
		switch (pid){
			case -1:
				fprintf(stderr, "Unable to fork!!\n%s\n", strerror(errno)); exit(-1);
				break;
			case 0:
				fprintf(stderr, "In Child\n");

				if (argc == 2) if (fclose(cmndPath) != 0){ //close cmnd file if entered
					fprintf(stderr, "Unable to close script file.\n%s\n", strerror(errno));
					exit(-1);
				}

				if (redir_o != 0) redirect(redir_o, 1, out);
				if (redir_e != 0) redirect(redir_e, 2, err);
				if (redir_i != 0) redirect(3, 0, in);

				if (execvp(args[0], args) < 0){
					fprintf(stderr, "Unable to exec %s\n", args[0]);
					return -1;
				}
				break;
			default:
				fprintf(stderr, "In Parent, Child pid is %i\n", pid);
				pid_t cpid; int status; struct rusage childInfo;
				if ((cpid=wait4(pid, &status, 0, &childInfo)) == -1) perror("WAIT FAILED");
				else{
					if (WIFEXITED(status)) fprintf(stderr, "Exit status %d\n", WEXITSTATUS(status));
					else if (WIFSIGNALED(status)) fprintf(stderr, "Terminated by %d\n", WTERMSIG(status));
					else fprintf(stderr, "Program did not exit normally NOR terminate by signal.\n");
			    	struct timeval curTime;
			    	if (gettimeofday(&curTime, NULL) == -1) {
			    		fprintf(stderr, "Unable to get current time at exit.\n%s\n", strerror(errno));
			    		exit(-1);
			    	}
			    	double realSec = ((double)curTime.tv_sec - (double)startTime.tv_sec) + (((double)curTime.tv_usec - (double)startTime.tv_usec)/1000000);
					fprintf(stderr, "This command took %f user time.\n", (double) childInfo.ru_utime.tv_sec + ((double) childInfo.ru_utime.tv_usec/1000000));
					fprintf(stderr, "This command took %f system time.\n", (double) childInfo.ru_stime.tv_sec + ((double) childInfo.ru_stime.tv_usec/1000000));
					fprintf(stderr, "This command took %f real time.\n", realSec);
				}
				break;
		}
	}
	free(args);
	free(line);
	return 0;
}
