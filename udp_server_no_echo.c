/*
 * Code to start Socket Programming in ICEN/ICSI 416
 * Author : Jingyuan Yi 
 *
 * udp_server.c - A simple UDP server, sending TCP header(a binary header in tcp formation in accuracy),does not send echo
 *
 * Compile in itsunix: gcc -lsocket -lm -lnsl udp_server_no_echo.c -o udp_server
 *
 * usage: udp_server <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>

#define BUFSIZE 1024
#define DATASIZE 895

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

typedef struct{
    int src_port; // source port
    int dest_port; // destination port
    int seq_no;//sequence number for sender and expected sequence number for receiver
    int ack_no;
    int check;
    int urg_ptr;
}TCP_hearder;


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

void decode_tcp_header(char *buf,TCP_hearder *header,char *data) // decode the tcp header as well as data attached.
{
  int i=0;
  int j=0;
  int var=0;
  for(j=0;i<16;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,16-j-1);
  }
  header->src_port = var;

  var = 0;
  for(j=0;i<16+16;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,16-j-1);
  }
  header->dest_port = var;

  var = 0;
  for(j=0;i<16+16+32;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,32-j-1);
  }
  header->seq_no = var;

  var = 0;
  for(j=0;i<16+16+32+32;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,32-j-1);
  }
  header->ack_no = var;

  var = 0;
  for(j=0;i<16+16+32+32+16;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,16-j-1);
  }
  header->check = var;

  var = 0;
  for(j=0;i<16+16+32+32+16+16;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,16-j-1);
  }
  header->urg_ptr = var;

  memcpy(data,buf+i,strlen(buf+i));
  //printf("%s\n",data);
}


int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  TCP_hearder *myhdr = (TCP_hearder *)malloc(sizeof(myhdr)); /* TCP_hearder struct variable */
  char data[DATASIZE]; /* store attached datastring */

  /*
   * check command line arguments
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /*
   * socket: create the parent socket
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr,
	   sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  /*
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /*
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n",
	   hostp->h_name, hostaddrp);
    printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);


    /* decode buf into header and data */
    decode_tcp_header(buf,myhdr,data);

    /* print header value */
    print_header(myhdr);

    /* print attached data */
    printf("data = %s\n",data);

    /*
     * sendto: echo the input back to the client
     */
     /*
    n = sendto(sockfd, buf, strlen(buf), 0,
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0)
      error("ERROR in sendto");
      */
  }
}
