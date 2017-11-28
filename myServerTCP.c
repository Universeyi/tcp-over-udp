/*
 * Code to start Socket Programming in ICEN/ICSI 416
 * Author : Jingyuan Yi
 *
 * udp_server.c - A simple TCP-over-UDP server, sending handshake echo and store text into as disk files.
 *
 * Compile in itsunix: gcc -lsocket -lm -lnsl myServerTCP.c -o tcp_server
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
#define DATASIZE 877

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
    int SYN;
    int FIN;
    int ACK;
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
  printf( "SYN = %d \n", header->SYN);
  printf( "FIN = %d \n", header->FIN);
  printf( "ACK = %d \n", header->ACK);
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

  var = 0;
  for(j=0;i<16+16+32+32+16+16+6;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,6-j-1);
  }
  header->SYN = var;

  var = 0;
  for(j=0;i<16+16+32+32+16+16+6+6;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,6-j-1);
  }
  header->FIN = var;

  var = 0;
  for(j=0;i<16+16+32+32+16+16+6+6+6;i++,j++)
  {
    var+=(int)(buf[i]-'0')*pow(2,6-j-1);
  }
  header->ACK = var;


  memcpy(data,buf+i,strlen(buf+i));

}

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

    tobinstr(dec2bin(header->SYN),6,temp);
    strcat(head_string,temp);

    tobinstr(dec2bin(header->FIN),6,temp);
    strcat(head_string,temp);

    tobinstr(dec2bin(header->ACK),6,temp);
    strcat(head_string,temp);

    free(temp);
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
  char data[DATASIZE]=""; /* store attached datastring */
  char *headstr=(char *)malloc(sizeof(char)*128+1);
  char *filename;
  FILE *fp;

  /*
   * check command line arguments
   */
  if (argc != 3) {
    fprintf(stderr, "usage: %s <port> <filename>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);
  filename = argv[2];

  if((fp=fopen(filename,"a"))==NULL){
      error("ERROR opening file");
  }

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
    bzero(data, DATASIZE);
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



    /* decode buf into header and data */
    decode_tcp_header(buf,myhdr,data);

    /* print header value */
    printf("start a new connection, header below\n");
    print_header(myhdr);


    /* judge head type */
    if(myhdr->SYN && myhdr->FIN)
    error("SYN and FIN cannot be 1 at the same time!");

    if (myhdr->SYN) // 1st shake
    {
      TCP_hearder *ack_hdr = (TCP_hearder *)malloc(sizeof(ack_hdr));
      ack_hdr->dest_port = myhdr->src_port;
      ack_hdr->src_port = myhdr->dest_port;
      ack_hdr->check = 3;
      ack_hdr->ack_no = myhdr->seq_no + 1;
      ack_hdr->seq_no = 2000;
      ack_hdr->urg_ptr=2;
      ack_hdr->SYN = 1;
      ack_hdr->FIN=0;
      ack_hdr->ACK=1;
      printf("received 1st shake, sending 2nd shake.\n");

      /* sending 2nd handshake */
      printf("\n\nsending 2nd handshake, header below\n");
      print_header(ack_hdr);
      get_tcp_header_string(ack_hdr, headstr);
      memcpy(buf,headstr,strlen(headstr));

      memcpy(data,"2nd shake\n",strlen("2nd shake\n"));

      memcpy(buf+strlen(headstr),data,strlen(data));

      n = sendto(sockfd, buf, strlen(buf), 0,
  	       (struct sockaddr *) &clientaddr, clientlen);
      if (n < 0)
        error("ERROR in sendto");
      printf("2nd handshake sent\n");

      /* wait for the 3rd handshake from client*/
      bzero(buf, BUFSIZE);
      n = recvfrom(sockfd, buf, BUFSIZE, 0,
  		 (struct sockaddr *) &clientaddr, &clientlen);
      if (n < 0)
        error("ERROR in recvfrom");
      decode_tcp_header(buf,myhdr,data);
      printf("\nreceiving echo from 3rd handshake, header blow\n\n");
      print_header(myhdr);
      printf("Connection built on server side, start to receive files\n");

    }
    while(1) // data(file) transmition
    {

      bzero(buf,BUFSIZE);
      bzero(data,DATASIZE);

      n = recvfrom(sockfd, buf, BUFSIZE, 0,
       (struct sockaddr *) &clientaddr, &clientlen);
      if (n < 0)
        error("ERROR in recvfrom");

      decode_tcp_header(buf,myhdr,data);
      printf("\n\ntransmission header\n");
      print_header(myhdr);
      printf("decoded data = %s\n", data);
      if(strcmp(data,"$")==0)
              {
                n = sendto(sockfd, buf, strlen(buf), 0,
                     (struct sockaddr *) &clientaddr, clientlen);
                if (n < 0)
                  error("ERROR in sendto");

                  fclose(fp);
                  break; //jump out of the loop
              }
      /*
       * write message line into file:
       */
      if(!fputs(data,fp))
          {
              error("ERRO writing into files");
          }
    }
    printf("Finished\n");

    break;

  }
}
