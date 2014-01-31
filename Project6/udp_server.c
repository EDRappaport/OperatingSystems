/* udp_server.c

Rappaport, Elliot D
ECE357: Operating Systems
November 13, 2013
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char const *argv[])
{
	if (argc != 2)
		{fprintf(stderr, "NOPE!\nExpecting argument of Port #.\n"); exit(-1);}
	int portNum = atoi(argv[1]);
	if (portNum < 1025)
		{fprintf(stderr, "Please choose a port number >1024\n"); exit(-1);}

	int s;
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{perror("Socket!!"); exit(-1);}
	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(portNum);
	sin.sin_addr.s_addr=INADDR_ANY;
	
	int len = sizeof sin;
	char *buf = malloc(4096);
	if (buf == NULL)
		{fprintf(stderr, "Unable to malloc!!\n"); exit(-1);}
	if (bind(s, (struct sockaddr *) &sin, len) < 0)
		{perror("Bind!!"); exit(-1);}
	while(1){
		if (recvfrom(s, buf, 4096, 0, (struct sockaddr *) &sin, &len) == -1)
			{perror("recvfrom!!"); exit(-1);}
		int bufLen = strlen(buf);
		buf[bufLen] = '\0';
		char *cmdStr = malloc(4096); FILE *cmdFS;
		if (!strncmp(buf, "UPTIME", 6)){
			cmdFS = popen("uptime", "r");
			fread(cmdStr, 4096, 1, cmdFS);}
		else if (!strncmp(buf, "DATE", 4)){
			cmdFS = popen("date", "r");
			fread(cmdStr, 4096, 1, cmdFS);}
		else
			sprintf(cmdStr, "ERROR: Command must be either \"DATE\" or \"UPTIME\"");

		if (sendto(s, cmdStr, 4096, 0, (struct sockaddr *) &sin, len) == -1)
			{perror("Sento!!"); exit(-1);}
	}

	return 0;
}