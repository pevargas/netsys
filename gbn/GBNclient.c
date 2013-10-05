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
#define SMALLSIZE  32
#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno) );\
  exit( EXIT_FAILURE );\
}

// Sliding Window Protocol Metadata modeled after pp. 111-112, L. Peterson & 
//   B. Davie. (2012). Computer Networks, 5 ed.

#define SWS 6 // Send Window Size

enum SEGMENT { SEQNUM, FLAGS };

typedef u_char SwpSeqno; // sizeof() = 1
typedef struct {
  SwpSeqno SeqNum; // Sequence or Ack number of this frame.
  u_char Flags;  // Flags
} SwpHdr;

typedef struct {
  SwpSeqno LAR; // Last Ack recieved
  SwpSeqno LFS; // Last Frame Sent
  SwpHdr hdr;   // Pre-Initialized Header
  struct sendQ_slot {
	char msg[ PACKETSIZE ];
  } sendQ[SWS];
} SwpState;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Client Side Log
void logTime ( FILE * fp, char *msg, SwpState *ss );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  // Variables
  int sd;                      // Socket
  int nbytes;                  // Number of bytes sent/received
  int index;                   // Index of file
  char c;                      // Current character
  struct sockaddr_in remote;   // Server address
  struct sockaddr_in fromAddr; // response address
  unsigned int fromLen;        // Response length
  char recvmsg[PACKETSIZE];    // Response 
  FILE *in, *log;              // Pointer to file
  SwpState client;
 
  // Set state
  client.LAR = 0;
  client.LFS = 0;
  client.hdr.SeqNum = 0;
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

  // Initialize sendQ Semaphore
  int sendQ = 0;

  do {	
	// Build Loop
	if ( sendQ < SWS ) {
	  // Start index counting at 2 since header takes 2 bytes
	  index = 2;
	  // Set data
	  do {
		c = fgetc( in );
		client.sendQ[client.hdr.SeqNum % SWS].msg[index++] = c;
	  }
	  while ( ( c != EOF ) && ( index < PACKETSIZE - 1 ) );
	  if ( c == EOF ) client.hdr.Flags = 1;
	  
	  // Set header
	  client.sendQ[client.hdr.SeqNum % SWS].msg[SEQNUM] = client.hdr.SeqNum;
	  client.sendQ[client.hdr.SeqNum % SWS].msg[FLAGS] = client.hdr.Flags;
	  
	  // End the content with null terminator
	  client.sendQ[client.hdr.SeqNum % SWS].msg[index] = '\0';
	  	
	  // Update log
	  logTime ( log, "BUILD", &client );	  
	  //	}

	// We'll set a flag for when we don't recieve acks
	//   and resend our current window sometime around here

	// Send Packet Loop
	//	if ( client.LAR == client.LFS ) {
	  // Call sendto_ in order to simulate dropped packets
	  nbytes = sendto_( sd, client.sendQ[client.hdr.SeqNum % SWS].msg, PACKETSIZE, 0, (struct sockaddr *) &remote, sizeof( remote ) );
	  ERROR( nbytes < 0 );
	  
	  // Set Last Frame Sent to current Sequence Number
	  client.LFS = client.hdr.SeqNum;
	  
	  // Update log
	  logTime ( log, "SEND", &client );
	  
	  // Increment Sequence Counter	
	  client.hdr.SeqNum++;
	  // Increment sendQ Semaphore
	  sendQ++;
	}

	// Receive Acks Loop
	if ( sendQ >= 0 ) {
	  // Receive message from server
	  bzero( recvmsg, sizeof( recvmsg ) );
	  fromLen = sizeof( fromAddr );
	  nbytes = recvfrom( sd, &recvmsg, PACKETSIZE, 0, (struct sockaddr *) &fromAddr, &fromLen );
	  ERROR( nbytes < 0 );
	  
	  // Set Last Ack Recieved
	  client.LAR = recvmsg[SEQNUM];
	  logTime ( log, "RECEIVE", &client );
	  // Decrement sendQ semaphore
	  sendQ--;
	}	
  } while ( client.hdr.Flags != 1 );
  
  // Close files
  ERROR( fclose( in ) ); ERROR( fclose( log ) );
 
  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Client Side Send
// <Send | Resend | Receive> <Seq #> [Free Slots] <LAR> <LFS> <Time>
void logTime ( FILE * fp, char *msg, SwpState *ss ) {
  time_t rawtime;
  struct tm *timeinfo;
  char timebuff[ PACKETSIZE ];
   
  // Get time
  time( &rawtime );
  timeinfo = localtime( &rawtime );
  strftime( timebuff, PACKETSIZE, "%r", timeinfo );
  
  // Update log
  fprintf( fp, "%8s | SEQ %3i | LAR %3i | LFS %3i | %s\n", 
		   msg, ss->hdr.SeqNum, ss->LAR, ss->LFS, timebuff );
  printf( "Client: %8s | SEQ %3i | LAR %3i | LFS %3i | %s\n", 
		   msg, ss->hdr.SeqNum, ss->LAR, ss->LFS, timebuff );
}
////////////////////////////////////////////////////////////////////////////////
