 /*
 * Code to start Socket Programming in ICEN/ICSI 416
 * Author: Jingyuan Yi
 *
 * udp_client.c - A simple UDP client, transmits a test message
 * and waits for 5 seconds for response.
 *
 * Compile in itsunix: gcc -lsocket -lnsl udp_client_timeout.c -o udp_client
 *
 * usage: udp_client <host> <port>
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#define BUFSIZE 1024
#define DATASIZE 895
#define WAIT_TIME 5 // In seconds

/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}


static void sig_alrm(int);

static void sig_alrm(int signo)
{
	return;
}


typedef struct{
    int src_port; // source port
    int dest_port; // destination port
    int seq_no;//sequence number for sender and expected sequence number for receiver
    int ack_no;
    int check;
    int urg_ptr;
}TCP_hearder;

long long unsigned int dec2bin(int dec) //convert dec to bin ,using llu to avoid overflow
{
    if (dec == 0)
    {
        return 0;
    }
    else
    {
        return (dec % 2 + 10 * dec2bin(dec / 2));
    }
}

void tobinstr(long long unsigned int value, int bitsCount, char* output) //bin integer to string formation
{
    int i;
    output[bitsCount] = '\0';
    for (i = bitsCount - 1; i >= 0; --i)
    {
        output[i] = (value % 10) + '0';
        value /= 10;
    }
}

void get_tcp_header_string(TCP_hearder *header,char *head_string)  //transform TCP_hearder to bin string
{
    char *temp=(char *)malloc(sizeof(char)*32+1);

    tobinstr(dec2bin(header->src_port),16,temp);
    strcat(head_string,temp);

    tobinstr(dec2bin(header->dest_port),16,temp);
    strcat(head_string,temp);

    tobinstr(dec2bin(header->seq_no),32,temp);
    strcat(head_string,temp);

    tobinstr(dec2bin(header->ack_no),32,temp);
    strcat(head_string,temp);

    tobinstr(dec2bin(header->check),16,temp);
    strcat(head_string,temp);

    tobinstr(dec2bin(header->urg_ptr),16,temp);
    strcat(head_string,temp);

    free(temp);
}

void print_header(TCP_hearder * header)  //print the TCP header information (using the struct)
{
  printf("--------------TCP HEADER--------------\n");
	printf( "seq_no = %d \n", header->seq_no);
	printf( "src_port = %d \n", header->src_port);
  printf( "dest_port = %d \n", header->dest_port);
  printf( "ack_no = %d \n", header->ack_no);
  printf( "check = %d \n", header->check);
  printf( "urg_ptr = %d \n", header->urg_ptr);
  printf("-----------------END-----------------\n");
}



int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char data[DATASIZE]= "test data...\n";
    char buf[BUFSIZE];
    char *headstr=(char *)malloc(sizeof(char)*128+1);

	signal(SIGALRM, sig_alrm);

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket\n");


    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);



    serverlen = sizeof(serveraddr);

    /* initialize the header */
    TCP_hearder *myhdr = (TCP_hearder *)malloc(sizeof(myhdr));
    myhdr->ack_no=2;
    myhdr->check=3;
    myhdr->dest_port=2333;
    myhdr->seq_no=1000;
    myhdr->src_port=2334;
    myhdr->urg_ptr=2;

    /* print the header information (at client side)*/
    print_header(myhdr);


    /* transform header into binary string */
    get_tcp_header_string(myhdr, headstr);

    /* attach data to string */
    printf("header length = %d\n", strlen(headstr));
    printf("data length = %d\n", strlen(data));
    memcpy(buf,headstr,strlen(headstr));
    memcpy(buf+strlen(headstr),data,strlen(data));

    /* send the message to the server */
    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, serverlen);
    if (n < 0)
      error("ERROR in sendto");

    /*Set the timer for recvfrom*/
    alarm(WAIT_TIME);

    /* print the server's reply */
    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, &serverlen);
    if (n < 0)
    {
    	if (errno == EINTR)
        	fprintf(stderr, "Socket timeout\n");
    	else
        	error("recvfrom() error\n");
    }
    else {
    	alarm(0);
	    printf("Echo from server: %s", buf);
	}

    return 0;
}
