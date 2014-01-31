/* tcp_send.c

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
	if (argc != 3)
		{fprintf(stderr, "NOPE!\nExpecting argument of Hostname:Port.\n"); exit(-1);}
	int portNum = atoi(argv[2]);
	if (portNum < 1025)
		{fprintf(stderr, "Please choose a port number >1024\n"); exit(-1);}
	char *hostName = argv[1];
	
	int i, isDottedDec = 1;
	for (i=0; i<strlen(hostName); i++){
		if ( (hostName[i] >= '0' && hostName[i] <= '9') || hostName[i] == '.') ;
		else {isDottedDec = 0; break;}
	}

	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(portNum);
	if (isDottedDec){
		//sin.sin_addr.s_addr = inet_addr(hostName);
		struct in_addr addr;
		inet_aton(hostName, &addr);
		sin.sin_addr = addr;
	}
	else{
		struct hostent *DNSinfo = gethostbyname(hostName);
		memcpy(&sin.sin_addr.s_addr, DNSinfo->h_addr_list[0], sizeof sin.sin_addr.s_addr);
	}

	int s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{perror("Socket!!"); exit(-1);}
	struct linger sOpts;
	sOpts.l_onoff = 1; sOpts.l_linger = -1;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &sOpts, sizeof sOpts) == -1)
		{perror("Setsockopt!!"); exit(-1);}
	if (connect(s, (struct sockaddr *) &sin, sizeof sin) == -1)
		{perror("Connect!!"); exit(-1);}

	int j, n, totalWritten, lim; char *temp_buf;
	char *buf = malloc(4096);
	if (buf == NULL)
		{fprintf(stderr, "Unable to malloc!!\n"); exit(-1);}
	time_t preDataTransferTime = time(NULL);
	int totalBytes = 0;
	while((n = read(0, buf, 4096)) != 0){
		if (n == -1)
			{perror("Read!!"); exit(-1);}
		totalBytes += n;
		totalWritten=0; lim = n; temp_buf=buf;
		while(totalWritten < lim){
			j = write(s, temp_buf, lim);
			if (j <= 0)
				{perror("Write!!"); exit(-1);}
			temp_buf += j; lim -= j; totalWritten += j;
		}
	}

	if (close(s) == -1) {perror("Close!!"); exit(-1);}

	time_t postDataTransferTime = time(NULL);
	double totalTime = difftime(postDataTransferTime, preDataTransferTime);
	double MB_per_sec = totalBytes/(totalTime * 1000000);
	fprintf(stderr, "%d Bytes with Data transfer at %f MB/sec\n", totalBytes, MB_per_sec);
	return 0;
}