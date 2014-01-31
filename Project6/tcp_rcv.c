/* tcp_rcv.c

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

	int s1, s2;
	if ((s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{perror("Socket!!"); exit(-1);}
	
	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(portNum);
	sin.sin_addr.s_addr=INADDR_ANY;
	
	int len = sizeof sin;
	if (bind(s1, (struct sockaddr *) &sin, len) < 0)
		{perror("Bind!!"); exit(-1);}
	if (listen(s1, 128) < 0)
		{perror("Listen!!"); exit(-1);}

	if ((s2 = accept(s1, (struct sockaddr *) &sin, &len)) < 0)
		{perror("Accept!!"); exit(-1);}

	int n; char *buf = malloc(4096);
	if (buf == NULL)
		{fprintf(stderr, "Unable to malloc!!\n"); exit(-1);}
	time_t preDataTransferTime = time(NULL);
	int totalBytes = 0;
	while((n = read(s2, buf, 4096)) != 0){
		if (n == -1)
			{perror("Read!!"); exit(-1);}
		totalBytes += n;
		buf[n]='\0';
		printf("%s", buf);
	}

	if (close(s1) == -1 || close(s2) == -1) //Close also performs shutdown
		{perror("Close!!"); exit(-1);}
	
	time_t postDataTransferTime = time(NULL);
	double totalTime = difftime(postDataTransferTime, preDataTransferTime);
	double MB_per_sec = totalBytes/(totalTime * 1000000);
	char *hostIP = inet_ntoa(sin.sin_addr);
	int hostPort = ntohs(sin.sin_port);
	struct hostent *DNSinfo = gethostbyaddr((struct sockaddr *) &sin, sizeof sin, AF_INET);
	if (DNSinfo == NULL) fprintf(stderr, "Unresolved Host Name: %s\n", hstrerror(h_errno));

	fprintf(stderr, "%d Bytes with Data transfer at %f MB/sec\n", totalBytes, MB_per_sec);
	fprintf(stderr, "Connection from %s:%i  - %s\n", hostIP, hostPort, DNSinfo->h_name);

	return 0;
}