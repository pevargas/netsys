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

#define SWS 6 // Send Window Size
#define RWS 6 // Recieve Window Size

typedef u_char SwpSeqno; // sizeof() = 1
typedef struct {
  SwpSeqno SeqNum; // Sequence number of this frame
  SwpSeqno AckNum; // Acknowledgement of recieved frame
  u_char Flags;  // Flags
} SwpHdr;

typedef struct {
  SwpSeqno NFE;  // Sequence number of next frame expected
  struct recvQ_slot {
	int recieved; // Is msg valid?
	char msg[PACKETSIZE];
  } recvQ[RWS];
} SwpState;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {
  // Variables
  int sd;                      // Our Socket
  int i;
  char c;
  int index = 3;
  struct sockaddr_in servAddr; // Server Address
  struct sockaddr_in cliAddr;
  unsigned int cliLen;
  int nbytes;
  char ack[ 3 ];
  SwpState server;

  FILE *log, *out;
  time_t rawtime;
  struct tm *timeinfo;
  char timebuff[ PACKETSIZE ];
   
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
  sd = socket( AF_INET, SOCK_DGRAM, 0 );
  ERROR( sd < 0 );

  // Bind server port to "well-known" port whose value is known by the client
  bzero( &servAddr, sizeof( servAddr ) );              // Zero the struct
  servAddr.sin_family      = AF_INET;                  // Address family
  servAddr.sin_port        = htons( atoi( argv[1] ) ); // htons() sets the port # to network byte order
  servAddr.sin_addr.s_addr = INADDR_ANY;               // Supplies the IP address of the local machine
  ERROR( bind( sd, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) < 0 );

  cliLen = sizeof( cliAddr );

  //  for ( i = 0; i < SWS; ++i ) {
  do {
	// Receive message from client
	nbytes = recvfrom( sd, &server.recvQ[0].msg, sizeof( server.recvQ[0].msg ), 0, (struct sockaddr *) &cliAddr, &cliLen );
	ERROR( nbytes < 0 );
	
	// Make sure to cap string to not over shoot
	server.recvQ[0].msg[nbytes] = '\0';

	//printf( "Client(%s:%d): %s\n", inet_ntoa( cliAddr.sin_addr ), ntohs( cliAddr.sin_port), recvmsg );
	
	// Get Header Data
	ack[1] = server.recvQ[0].msg[0];
	ack[2] = server.recvQ[0].msg[2];
	
	// Get time
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime( timebuff, PACKETSIZE, "%r", timeinfo );
	
	// Copy and write recieved data
	index = 3;
	do {
	  c = fputc( server.recvQ[0].msg[index++], out );
	} while ( c != '\0' && index < PACKETSIZE - 3);
	
	// Update log
	fprintf( log, "RECIEVE %i %s\n", server.recvQ[0].msg[1], timebuff );
	
	// Respond using sendto_ in order to simulate dropped packets
	nbytes = sendto_( sd, ack, PACKETSIZE, 0, (struct sockaddr *) &cliAddr, sizeof( cliAddr ) );
	ERROR( nbytes < 0 );
	
	// Get time
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime( timebuff, PACKETSIZE, "%r", timeinfo );
	
	// Update log
	fprintf( log, "SEND %i %s\n", ack[1], timebuff );
  } while ( ack[2] ); 
	
  // Close files
  fclose( log ); fclose( out );

  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
