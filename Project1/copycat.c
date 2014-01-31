/* copycat.c

This function copies and concatenates files.  User can choose buffer size to
be used.  User may copy to and from a selected file or from standard I/O.

USAGE:
	copycat [-b ####] [-o outfile] [infile1...]

Rappaport, Elliot D
ECE357: Operating Systems
September 24, 2013
*/


#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

int inputParse(int argc, char const *argv[], int *b_pntr, char **outfile_pntr){

	int number_in = argc - 1; //number input files is all but command, unless -b or -o used

	//Define function to check bufsize if -b is used
	void confirm_b(int i){
		if (argc > i+1){ //there must be an arg following -b that defines bufsize
			*b_pntr = atoi(argv[i+1]);
			if (*b_pntr < 1){
				fprintf(stderr, "You entered %i for buffer size.\nBuffer size, set with -b, must be a positive number.\nIf -b unused, buffer size will default to 4096.\n", *b_pntr);
				exit(-1);
			}
		}
		else{
			fprintf(stderr, "You did not enter a buffer size.\nBuffer size, set with -b, must be a positive number.\nIf -b unused, buffer size will default to 4096.\n");
			exit(-1);
		}
	}

	//Define function to check output file if -o is used
	void confirm_out(int i){
		if (argc > i+1){
			*outfile_pntr = argv[i+1];
		}
		else{
			fprintf(stderr, "You did not enter an output file.\nIf -o unused, output file will default to std out.\n");
			exit(-1);
		}
	}

	*b_pntr = 4096; //default, otherwise may be set below
	*outfile_pntr="-";
	
	for (int i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-b")){
			//if -b is used, use confirm_b to check bufsize is set and valid
			confirm_b(i);
			number_in-=2; //subtract these two args from total for input number
			i++; //skip the next command-line arg; it should define bufsize
		}
		else if (!strcmp(argv[i], "-o")){
			//if -o is used, use confirm_out to check output file is set
			confirm_out(i);
			number_in-=2;
			i++;
		}
		else break; //if not -o or -b, exit for-loop

	}
	return number_in;
}

int openFile(char* file, char *mode){
	int fd;

	if(strcmp(file, "-")){
		if(!strcmp(mode, "r")) fd = open(file, O_RDONLY);
		else if(!strcmp(mode, "w")) fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		else if(!strcmp(mode, "a")) fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0777);
	}
	else if(!strcmp(mode, "r")) fd = 0;
	else if(!strcmp(mode, "w") | !strcmp(mode, "a")) fd = 1;

	if (fd < 0){
		fprintf(stderr, "Sorry, unable to open file %s in mode %s.\n%s\n", file, mode, strerror(errno));
		exit(-1);
	}

	return fd;
}

void rwFile(int fd_out, char *outfile, char *infile, char *buf, int bufsize){
	int j, n, fd_in, total_written, lim;
	char *temp_buf;
	fd_in = openFile(infile, "r");

	j=1;
	while ( j != 0){
		j = read(fd_in, buf, bufsize);
		if (j < 0){
			fprintf(stderr, "Sorry, there has been an error reading your file: %s\n%s\n", infile, strerror(errno));
			exit(-1);
		}

		total_written=0; lim = j; temp_buf=buf;
		while(total_written < lim){
			n = write(fd_out, temp_buf, lim);
				if (n <= 0){
					fprintf(stderr, "Sorry, there has been an error writing to your file: %s\n%s\n", outfile, strerror(errno));
					exit(-1);
				}	
			temp_buf += n; lim -= n; total_written += n;
		}	
	}

	if(fd_in > 2) close(fd_in);
}


int main(int argc, char const *argv[]){
	int bufsize;
	int *b_pntr = &bufsize;

	char *outfile="\0";
	char **outfile_pntr = &outfile;

	//parse inputs & options, returns number of input files
	int number_in, input_start_index;
	number_in = inputParse(argc, argv, b_pntr, outfile_pntr);
	input_start_index = argc - number_in;

	int fd_out = openFile(outfile, "w"); //open outfile as "w" to clear it

	char *buf = malloc(bufsize);
	if (buf == NULL) fprintf(stderr, "Failed to allocate memory for the buffer.\n");

	for (int i = 0; i < number_in; i++)
	{
		printf("Copying over %s to %s\n", argv[input_start_index+i], outfile);
		rwFile(fd_out, outfile, argv[input_start_index+i], buf, bufsize);
	}
	if(number_in == 0) rwFile(fd_out, outfile, "-", buf, bufsize);
	if(fd_out > 2) close (fd_out);	

	free(buf);

	return 0;
}