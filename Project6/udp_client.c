/* udp_client.c

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
	if (argc != 4)	
		{fprintf(stderr, "NOPE! Expecting \"udp_client hostname port request\"\n"); exit(-1);}
	int portNum = atoi(argv[2]);
	if (portNum < 1025)
		{fprintf(stderr, "Please choose a port number >1024\n"); exit(-1);}
	char *hostName = argv[1];
	char *request = argv[3];
	
	int i, isDottedDec = 1;
	for (i=0; i<strlen(hostName); i++){
		if ( (hostName[i] >= '0' && hostName[i] <= '9') || hostName[i] == '.') ;
		else {fprintf(stderr, "NOT Dotted Decimal.\n"); isDottedDec = 0; break;}
	}

	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(portNum);
	if (isDottedDec)
		sin.sin_addr.s_addr = inet_addr(hostName);
	else{
		struct hostent *DNSinfo = gethostbyname(hostName);
		memcpy(&sin.sin_addr.s_addr, DNSinfo->h_addr_list[0], sizeof sin.sin_addr.s_addr);
	}

	int s;
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{perror("Socket!!"); exit(-1);}

	int len = sizeof sin;
	if (sendto(s, request, strlen(request), 0, (struct sockaddr *) &sin, len) == -1)
		{perror("Sendto!!"); exit(-1);}
	char *returnedStr = malloc(4096);
	if (recvfrom(s, returnedStr, 4096, 0, (struct sockaddr *) &sin, &len) == -1)
		{perror("recvfrom!!"); exit(-1);}
	printf("%s\n", returnedStr);

	return 0;
}