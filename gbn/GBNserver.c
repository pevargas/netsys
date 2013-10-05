////////////////////////////////////////////////////////////////////////////////
// File: GBNserver.c                    Fall 2013
// Students: 
//   Brittney Harsha                    Patrick Vargas
//   b.grace.harsha@gmail.com           patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements:
//   This is a sample UDP server/receiver program.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h> /* close() */
#include "sendto_.h"

#define PACKETSIZE 1024
#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
  exit( EXIT_FAILURE );\
}

// Sliding Window Protocol Metadata modeled after pp. 111-112, L. Peterson & 
//   B. Davie. (2012). Computer Networks, 5 ed.

#define RWS 6 // Recieve Window Size

enum SEGMENT { SEQNUM, FLAGS };

typedef u_char SwpSeqno; // sizeof() = 1
typedef struct {
  SwpSeqno SeqNum; // Sequence of Ack number of this frame
  u_char Flags;    // Flags
} SwpHdr;

typedef struct {
  SwpSeqno NFE;    // Sequence number of next frame expected
  SwpSeqno LFRead; // Last Frame Read
  SwpSeqno LFRcvd; // Last Frame Recieved
  SwpSeqno LAF;    // Largest Acceptable Frame
  SwpHdr hdr;   // Pre-Initialized Header
  struct recvQ_slot {
	int recieved; // Is msg valid?
	char msg[PACKETSIZE];
  } recvQ[RWS];
} SwpState;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Server Side Log
void logTime ( FILE * fp, char *msg, SwpState *ss );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {
  // Variables
  char c;                                  // Character to write to our program
  int sock;                                // Our Socket
  int index;                               // Index to get data from frame
  struct sockaddr_in servAddr;             // Server Address
  struct sockaddr_in cliAddr;              // Client Address
  unsigned int cliLen = sizeof( cliAddr ); // Length of Client struct
  int nbytes;                              // Number of bytes read
  char ack[2];                             // Acknowledgment packet
  SwpState server;                         // Window state
  FILE *log, *out;                         // File pointers to log and output file
   
  // Initalize Variables
  server.hdr.SeqNum = 0;
  server.hdr.Flags  = 0;
  server.LFRead = 0;
  server.LFRcvd = -1;
  server.LAF = RWS;
  server.NFE = 0;

  // Check command line args.
  if( argc < 6 ) {
    printf( "Usage: %s <server_port> <error rate> <random seed> <output_file> <recieve_log> \n", argv[0] );
    exit( EXIT_FAILURE );
  }
  
  // Note: you must initialize the network library first before calling sendto_().
  //   The arguments are the <errorrate> and <random seed>
  init_net_lib( atof(argv[2]), atoi( argv[3] ) );
  printf( "Error rate: %f\n", atof( argv[2] ) );

  // Open log and output file
  out = fopen( argv[4], "w" ); ERROR( out == NULL );
  log = fopen( argv[5], "w" ); ERROR( log == NULL );

  // Socket creation
  sock = socket( AF_INET, SOCK_DGRAM, 0 );
  ERROR( sock < 0 );

  // Bind server port to "well-known" port whose value is known by the client
  bzero( &servAddr, sizeof( servAddr ) );              // Zero the struct
  servAddr.sin_family      = AF_INET;                  // Address family
  servAddr.sin_port        = htons( atoi( argv[1] ) ); // htons() sets the port # to network byte order
  servAddr.sin_addr.s_addr = INADDR_ANY;               // Supplies the IP address of the local machine
  ERROR( bind( sock, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) < 0 );

  char buffer[PACKETSIZE];
  bzero( &buffer, PACKETSIZE );
  
  do {
 	// Receive message from client
	nbytes = recvfrom( sock, &buffer, PACKETSIZE, 0, (struct sockaddr *) &cliAddr, &cliLen );
	ERROR( nbytes < 0 );

	// Make sure to cap string
	buffer[nbytes] = '\0';
	
	// Check to see if packet is what we need
	if ( ( buffer[SEQNUM] >= server.LAF - RWS ) && 
		 ( buffer[SEQNUM] < server.LAF ) ) {
	  // Set Last Frame Recieved to this buffer number
	  server.LFRcvd = buffer[SEQNUM];
	  memcpy( server.recvQ[ server.LFRcvd % RWS ].msg, buffer, PACKETSIZE);
	  
	  // Update log
	  logTime( log, "RECEIVE", &server );
	}
	
	// Check to see if the one we recieved is the next frame
	if ( server.LFRcvd == server.NFE ) {
	  
	  // Copy and write recieved data
	  index = 2;
	  do {
		c = server.recvQ[server.NFE % RWS].msg[index];
		if ( c == '\0' ) break;
		fputc( c, out ); index++;
	  }
	  while ( index < PACKETSIZE - 1 );

	  // Then we'll set a flag for when there's a timeout to resend
	  //   the current ack sometime around here. 
	  
	  // Set Ack Header Data
	  ack[SEQNUM] = server.LFRcvd;
	  ack[FLAGS] = 0;
	  
	  // Respond using sendto_ in order to simulate dropped packets
	  nbytes = sendto_( sock, ack, PACKETSIZE, 0, (struct sockaddr *) &cliAddr, sizeof( cliAddr ) );
	  ERROR( nbytes < 0 );
	  
	  // Update log
	  logTime( log, "SEND", &server );
	  // if ( we timed out and need to resend )
	  // logTime( log, "RESEND", &server );
	  
	  // Increment Next Frame Expected
	  server.NFE++;
	  // Increment Largest Acceptable Frame
	  server.LAF++;
	}
  } while ( buffer[FLAGS] != 1 ); 
  
  // Close files
  fclose( log ); fclose( out );
  
  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Server Side Log
// <Send | Resend | Receive> <Seq #> [Free Slots] <LFRead> <LFRcvd> <LAF> <Time>
void logTime ( FILE * fp, char *msg, SwpState *ss ) {
  time_t rawtime;
  struct tm *timeinfo;
  char timebuff[32];
   
  // Get time
  time( &rawtime );
  timeinfo = localtime( &rawtime );
  strftime( timebuff, PACKETSIZE, "%r", timeinfo );
  
  // Update log
  fprintf( fp, "%8s | SEQ %3i | LFRead %3i | LFRcvd %3i | LAF %3i | %s\n", 
		   msg, ss->hdr.SeqNum, ss->LFRead, ss->LFRcvd, ss->LAF, timebuff );
  printf( "Server: %8s | SEQ %3i | LFRead %3i | LFRcvd %3i | LAF %3i | %s\n", 
		   msg, ss->hdr.SeqNum, ss->LFRead, ss->LFRcvd, ss->LAF, timebuff );
}
////////////////////////////////////////////////////////////////////////////////
