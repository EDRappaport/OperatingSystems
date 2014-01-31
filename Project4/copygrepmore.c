/* copygrepmore.c

Rappaport, Elliot D
ECE357: Operating Systems
October 30, 2013
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

int fileNum, bytesProc, pid_g, pid_m;

static void sigIntHandler(int sig, siginfo_t *sa, void *unused){
	if (kill(pid_g, 9) != 0) {perror("Cannot kill grep!\n"); exit(-1);}
	if (kill(pid_m, 9) != 0) {perror("Cannot kill grep!\n"); exit(-1);}
	fprintf(stderr, "EXITING\n%d files fully processed.\n%d bytes processed.\n", fileNum-2, bytesProc);
	exit(-1);
}
static void sigPipeHandler(int sig, siginfo_t *sa, void *unused){
	fprintf(stderr, "GOT SIGPIPE\n");
	if (kill(pid_g, 9) != 0) {perror("Cannot kill grep!\n"); exit(-1);}
}
int main(int argc, char const *argv[])
{
	char *pattern = argv[1];
	int stat1, stat2, fds_gm[2], fds_rg[2];

	struct sigaction saInt, saPipe;
	saInt.sa_flags = SA_SIGINFO; saPipe.sa_flags = SA_SIGINFO;
	sigemptyset(&saInt.sa_mask); sigemptyset(&saPipe.sa_mask);
	saInt.sa_sigaction = sigIntHandler;
	saPipe.sa_sigaction = sigPipeHandler;
	if(sigaction(SIGPIPE, &saPipe, NULL) == -1) {fprintf(stderr, "Failed to handle signal.\n"); exit(-1);}
	if(sigaction(SIGINT, &saInt, NULL) == -1) {fprintf(stderr, "Failed to handle signal.\n"); exit(-1);}

	for (fileNum = 2; fileNum < argc; fileNum++){
		if (pipe(fds_gm) < 0) {perror("Cannot create gr->mo pipe!"); return -1;}
		if (pipe(fds_rg) < 0) {perror("Cannot create ma->gr pipe!"); return -1;}	

		pid_g = fork();
		switch(pid_g){
			case -1:
				fprintf(stderr, "Unable to fork!!\n%s\n", strerror(errno)); exit(-1);
				break;
			case 0:
				fprintf(stderr, "In Child 1!!\nSetting up grep.\n");
				if (dup2(fds_gm[1], 1) == -1 || dup2(fds_rg[0], 0) == -1)
					{perror("Cannot dup2!\n"); exit(-1);}
				if (close(fds_gm[1]) == -1 || close(fds_gm[0]) == -1 ||
					close(fds_rg[1]) == -1 || close(fds_rg[0]) == -1)
					{perror("Failing to close files\n"); exit(-1);}
				if (execlp("grep", "grep", "-n", pattern, NULL) == -1)
					{perror("Failed to exec grep!\n"); exit(-1);}
				break;
			default:
				fprintf(stderr, "In Parent (1).\n");
				break;}
		pid_m = fork();
		switch(pid_m){
			case -1:
				fprintf(stderr, "Unable to fork!!\n%s\n", strerror(errno)); exit(-1);
				break;
			case 0:
				fprintf(stderr, "In Child 2!!\nSetting up more (pg)\n");
				if (dup2(fds_gm[0], 0) == -1) {perror("Cannot dup2!\n"); exit(-1);}
				if (close(fds_gm[1]) == -1 || close(fds_gm[0]) == -1 ||
					close(fds_rg[1]) == -1 || close(fds_rg[0]) == -1)
					{perror("Failing to close files\n"); exit(-1);}
				if (execlp("pg", "pg", NULL) == -1)
					{perror("Failed to exec pg!\n"); exit(-1);}
				break;
			default:
				fprintf(stderr, "In Parent (2).\n");
				break;}

		if (close(fds_gm[1]) == -1 || close(fds_gm[0]) == -1 ||
				close(fds_rg[0]) == -1)
				{perror("Failing to close files\n"); exit(-1);}

		int bufsize = 4096;
		char *buf = malloc(bufsize);
		if (buf == NULL) fprintf(stderr, "Failed to allocate memory for the buffer.\n");
	
		int j, n, fd_in, total_written, lim;
		char *temp_buf;
		if ((fd_in = open(argv[fileNum], O_RDONLY)) < 0){
			fprintf(stderr, "Sorry, unable to open file %s in mode 'r'.\n%s\n", argv[fileNum], strerror(errno));
			exit(-1);
		}

		while ((j = read(fd_in, buf, bufsize)) != 0){
			if (j < 0){
				fprintf(stderr, "Sorry, there has been an error reading your file: %s\n%s\n", argv[fileNum], strerror(errno));
				exit(-1);
			}
			total_written=0; lim = j; temp_buf=buf;
			while(total_written < lim){
				n = write(fds_rg[1], temp_buf, lim);
				if (n <= 0){
					if (errno == EPIPE){
						fprintf(stderr, "GOT EPIPE\n");
						j = 0; total_written = lim-n; 
						break;
					}
					fprintf(stderr, "Sorry, there has been an error writing to your file to pipe.\n%s\n", strerror(errno));
					exit(-1);
				}
				bytesProc += n;
				temp_buf += n; lim -= n; total_written += n;
			}
			if (j == 0) break;
		}
		if (close(fds_rg[1]) == -1 || close(fd_in) == -1)
			{perror("Failing to close!\n"); exit(-1);}

		if (waitpid(pid_m, &stat2, 0) == -1){perror("Waiting error!\n"); exit(-1);}
		if (waitpid(pid_g, &stat1, 0) == -1){perror("Waiting error!\n"); exit(-1);}
		free(buf);
	}
	return 0;
}