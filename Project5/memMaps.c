/* memMaps.c

Rappaport, Elliot D
ECE357: Operating Systems
November 6, 2013
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <signal.h>
#include <setjmp.h>
jmp_buf int_jb;

void sigHandler(int sig){
	fprintf(stderr, "Signal \"%s\" received on command!\n", strsignal(sig));
	longjmp(int_jb, 1);
}

int creatFile(){
	if (system("dd if=/dev/urandom of=random_text.txt bs=8195 count=1") == -1)
		{fprintf(stderr, "Unable to create random text file.\n"); exit(-1);}
	int fd = open("random_text.txt", O_RDWR);
	if (fd == -1) {fprintf(stderr, "Unable to open random text file.\n"); exit(-1);}
	return fd;
}

int main(int argc, char const *argv[]){
	char qNum = (char) *argv[1];
	char *addr; size_t length = 8195; int fd;
	int fl = MAP_PRIVATE;
	int possible_sig, it;
	char *buf;
	switch(qNum){
		case 'A':
			fprintf(stderr, "PART A:\nMapping READ ONLY file.\n");
			if ((addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, creatFile(), 0)) == MAP_FAILED)
				{fprintf(stderr, "Mapping failed\n%s\n", strerror(errno)); exit(-1);}
			for (possible_sig = 1; possible_sig < 32; possible_sig++)
				if (possible_sig != 9 && possible_sig != 19)
				if (signal(possible_sig, sigHandler) == SIG_ERR)
					{fprintf(stderr, "Unable to set signal handler %i %s.\n", possible_sig, strsignal(possible_sig));}
			if (sigsetjmp(int_jb, 1)==0) strcpy(addr, "C");
			break;

		case 'B':
			fprintf(stderr, "PART B:\nMapping READ/WRITE file of type MAP_SHARED.\n");
			fl = MAP_SHARED;
		case 'C':
			if (qNum == 'C') fprintf(stderr, "PART C:\nMapping READ/WRITE file of type MAP_PRIVATE.\n");
			fd = creatFile();
			if ((addr = mmap(NULL, length, PROT_READ|PROT_WRITE, fl, fd, 0)) == MAP_FAILED)
				{fprintf(stderr, "Mapping failed\n%s\n", strerror(errno)); exit(-1);}
			fprintf(stderr, "Changing data stored in memory map.\n");
			strcpy(addr, "C");
			if (lseek(fd, 0, SEEK_SET) == -1)
				{fprintf(stderr, "lseek failed.\n%s\n", strerror(errno)); exit(-1);}
			if ((buf = malloc(4)) == NULL)
				{fprintf(stderr, "Failed to malloc.\n"); exit(-1);}
			if (read(fd, buf, 4) == -1)
				{fprintf(stderr, "Error Reading!\n%s\n", strerror(errno)); exit(-1);}
			if (!strcmp(buf, "C"))
				fprintf(stderr, "IMMEDIATE MATCH from original file.\n");
			else
				fprintf(stderr, "DID NOT MATCH from original file.\n");
			break;

		case 'D':
		case 'E':
			fprintf(stderr, "PART D/E:\nMapping READ/WRITE file of type MAP_PRIVATE.\nWriting at offset %d in memory map", (int) length);
			fd = creatFile(); struct stat sb;
			if (fstat(fd, &sb) == -1)
				{fprintf(stderr, "Error Stat-ing!\n%s\n", strerror(errno)); exit(-1);}
			int orig_size = sb.st_size;
			if ((addr = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
				{fprintf(stderr, "Mapping failed\n%s\n", strerror(errno)); exit(-1);}
			*(addr+length) = 'D'; *(addr+length+1) = 'E'; *(addr+length+2) = 'F';
		
			fprintf(stderr, "Memory dump starting at offset 8195:\n");
			for (it=0; it<3; it++) fprintf(stderr, "<%02x> ", *(addr+length+it));
			fprintf(stderr, "\n");

			if (fstat(fd, &sb) == -1)
				{fprintf(stderr, "Error Stat-ing!\n%s\n", strerror(errno)); exit(-1);}
			int new_size = sb.st_size;
			if (orig_size != new_size)
				fprintf(stderr, "Size changed from %i to %i bytes.\n", orig_size, new_size);
			else { //PART E:
				fprintf(stderr, "Size Unchanged (%i bytes)\n\n", new_size);
				lseek(fd, 10, SEEK_END);
				char *buf = "Stuff";

				int total_written = 0; int goal = strlen(buf); int lim = goal; int n;
				while (total_written < goal){
					if ((n = write(fd, buf, lim)) < 0)
						{fprintf(stderr, "Error writing.\n%s\n", strerror(errno)); exit(-1);}
					lim -= n; total_written += n;
				}
				if (lseek(fd, length, SEEK_SET) == -1)
					{fprintf(stderr, "lseek failed.\n%s\n", strerror(errno)); exit(-1);}
				if ((buf = malloc(64)) == NULL)
					{fprintf(stderr, "Failed to malloc.\n"); exit(-1);}
				if (read(fd, buf, 15) == -1)
					{fprintf(stderr, "Error Reading!\n%s\n", strerror(errno)); exit(-1);}

				fprintf(stderr, "File dump starting at offset 8195\n");
				for (it=0; it<15; it++) fprintf(stderr, "<%02x> ", buf[it]);
				fprintf(stderr, "\n");

				if (buf[0] == 'D' && buf[1] == 'E' && buf[2] == 'F')
					fprintf(stderr, "YUP, the Data previously written to the hole remain.\n");
				else fprintf(stderr, "NOPE, the Data previously written to the hole do NOT remain.\n");
			}
			break;

		case 'F':
			fprintf(stderr, "PART F:\nMapping small file to 2 pages.\n");
			if (system("dd if=/dev/urandom of=random_text.txt bs=10 count=1") == -1)
				{fprintf(stderr, "Unable to create random text file.\n"); exit(-1);}
			if ((fd = open("random_text.txt", O_RDWR)) == -1)
				{fprintf(stderr, "Unable to open random text file.\n"); exit(-1);}
			if ((addr = mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
				{fprintf(stderr, "Mapping failed\n%s\n", strerror(errno)); exit(-1);}
			for (possible_sig = 1; possible_sig < 32; possible_sig++)
				if (possible_sig != 9 && possible_sig != 19)
				if (signal(possible_sig, sigHandler) == SIG_ERR)
					{fprintf(stderr, "Unable to set signal handler %i %s.\n", possible_sig, strsignal(possible_sig));}
			
			fprintf(stderr, "Attempting to access memory from second page.\n");
			if (sigsetjmp(int_jb, 1)==0) *(addr+(8192/2)) = 'A';
			
			fprintf(stderr, "Attempting to access memory from first page.\n");
			if (sigsetjmp(int_jb, 1)==0) *(addr) = 'A';
			break;

		default:
			fprintf(stderr, "Choose option A-F only.\n"); exit(-1);
			break;
		}
	free(buf);
	if (close(fd) == -1) {fprintf(stderr, "Error closing.\n%s\n", strerror(errno)); exit(-1);}
	return 0;
}
