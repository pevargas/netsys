////////////////////////////////////////////////////////////////////////////////
// File: GBNclient.c                    Fall 2013
// Students: 
//   Brittney Harsha                    Patrick Vargas
//   b.grace.harsha@gmail.com           patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements:
//   This is a sample UDP client/sender using "sendto_.h" to simulate dropped packets.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>   /* memset() */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h> /* select() */
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "sendto_.h"

#define PACKETSIZE 1024
#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno) );\
  exit( EXIT_FAILURE );\
}

// Sliding Window Protocol Metadata modeled after pp. 111-112, L. Peterson & 
//   B. Davie. (2012). Computer Networks, 5 ed.

#define SWS 6 // Send Window Size
#define RWS 6 // Recieve Window Size

typedef u_char SwpSeqno; // sizeof() = 1
typedef struct {
  SwpSeqno SeqNum; // Sequence number of this frame
  SwpSeqno AckNum; // Acknowledgement of recieved frame
  u_char Flags;  // Flags
} SwpHdr;

typedef struct {
  int LAR; // Sequence number of last ACK recieved
  int LFS; // Last frame sent
  SwpHdr hdr; // Pre-Initialized Header
  struct sendQ_slot {
	char msg[ PACKETSIZE ];
  } sendQ[SWS];
} SwpState;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  // Variables
  int sd;                      // Socket
  int nbytes;                  // Number of bytes sent/received
  int index;                   // Index of file
  int i;
  char c;                      // Current character
  struct sockaddr_in remote;   // Server address
  struct sockaddr_in fromAddr; // response address
  unsigned int fromLen;        // Response length
  char msg[PACKETSIZE];        // Packet to send
  char recvmsg[PACKETSIZE];    // Response 
  FILE *in, *log;              // Pointer to file
  SwpState client;  
  SwpHdr current;
  time_t rawtime;
  struct tm *timeinfo;
  char timebuff[PACKETSIZE];

  // Set state
  client.hdr.SeqNum = 0;
  client.hdr.AckNum = 0;
  client.hdr.Flags  = 0;
  
  // Check command line args
  if( argc < 7 ) {
    printf("Usage: %s <server_ip> <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
    exit( EXIT_FAILURE );
  }

  // Note: you must initialize the network library first before calling sendto_().
  //   The arguments are the <errorrate> and <random seed>
  init_net_lib( atof( argv[ 3 ] ), atoi( argv[4] ) );
  printf( "Error rate: %f\n", atof( argv[3] ) );

  // Socket creation
  sd = socket( AF_INET, SOCK_DGRAM, 0 );
  ERROR( sd < 0 );

  // Get server IP address (input must be IP address, not DNS name)
  bzero( &remote, sizeof( remote ) );          //zero the struct
  remote.sin_family      = AF_INET;                 //address family
  remote.sin_port        = htons( atoi( argv[2] ) );      //sets port to network byte order
  remote.sin_addr.s_addr = inet_addr( argv[1] ); //sets remote IP address
  printf( "%s: sending data to '%s:%s' \n", argv[0], argv[1], argv[2] );

  // Open file
  in = fopen( argv[5], "r" ); ERROR( in == NULL );
  log = fopen( argv[6], "w" ); ERROR( log == NULL );  

  //  for ( i = 0; i < SWS; ++i ) {
  do {
	
	// Start index counting at 3 since header takes 3 bytes
	index = 3;
	
	// Set data
	do {
	  c = fgetc( in );
	  client.sendQ[0].msg[index++] = c;
	}
	while ( ( c != EOF ) && ( index < PACKETSIZE - 1 ) );
	if ( c == EOF ) client.hdr.Flags = 1;
	
	// Set header
	client.sendQ[0].msg[0] = client.hdr.SeqNum++;
	client.sendQ[0].msg[1] = client.hdr.AckNum;
	client.sendQ[0].msg[2] = client.hdr.Flags;

	// End the content with null terminator
	client.sendQ[0].msg[index] = '\0';
	
	// Call sendto_ in order to simulate dropped packets
	nbytes = sendto_( sd, client.sendQ[0].msg, PACKETSIZE, 0, (struct sockaddr *) &remote, sizeof( remote ) );
	ERROR( nbytes < 0 );
	
	// Get time
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime( timebuff, PACKETSIZE, "%r", timeinfo );
	
	// Update log
	fprintf( log, "SEND %i %s\n", client.hdr.SeqNum, timebuff );
	
	// Receive message from server
	bzero( recvmsg, sizeof( recvmsg ) );
	fromLen = sizeof( fromAddr );
	nbytes = recvfrom( sd, &recvmsg, PACKETSIZE, 0, (struct sockaddr *) &fromAddr, &fromLen );
	ERROR( nbytes < 0 );
	
	//printf( "Server(%s:%d): %s\n", inet_ntoa( fromAddr.sin_addr ), ntohs( fromAddr.sin_port), recvmsg );
	
	// Get time
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime( timebuff, PACKETSIZE, "%r", timeinfo );
	
	// Update log
	fprintf( log, "RECEIVE %i %s\n", recvmsg[1], timebuff );
  } while ( client.hdr.Flags );

  // Close files
  ERROR( fclose( in ) ); ERROR( fclose( log ) );
 
  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

