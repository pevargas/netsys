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
  int LAR; // Sequence number of last ACK recieved
  int LFS; // Last frame sent
  SwpHdr hdr; // Pre-Initialized Header
} SwpState;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  //
  // Variables
  //
  int sd;                      // Our Socket
  struct sockaddr_in servAddr; // Server Address
  struct sockaddr_in cliAddr;
  unsigned int cliLen;
  int nbytes;
  char recvmsg[ PACKETSIZE ];
  char buffer[ PACKETSIZE ];
  char response[ PACKETSIZE ] = "respond this";
  char *log, *out;
  SwpHdr current;
  FILE *logfp, *outfp;
  time_t rawtime;
  struct tm *timeinfo;
  char timebuff[ PACKETSIZE ];

  //
  // Check command line args.
  //
  if( argc < 6 ) {
    printf( "Usage: %s <server_port> <error rate> <random seed> <output_file> <recieve_log> \n", argv[0] );
    exit( EXIT_FAILURE );
  }
  out = argv[4];
  log = argv[5];

  //  printf( "Error rate : %f\n", atof( argv[2] ) );

  // Note: you must initialize the network library first before calling sendto_().
  //   The arguments are the <errorrate> and <random seed>
  init_net_lib( atof(argv[2]), atoi( argv[3] ) );
  printf( "Error rate: %f\n", atof( argv[2] ) );

  //
  // Socket creation
  // 
  sd = socket( AF_INET, SOCK_DGRAM, 0 );
  ERROR( sd < 0 );

  // bind server port to "well-known" port whose value is known by the client
  bzero( &servAddr, sizeof( servAddr ) );              //zero the struct
  servAddr.sin_family      = AF_INET;                  //address family
  servAddr.sin_port        = htons( atoi( argv[1] ) ); //htons() sets the port # to network byte order
  servAddr.sin_addr.s_addr = INADDR_ANY;               //supplies the IP address of the local machine
  ERROR( bind( sd, (struct sockaddr *) &servAddr, sizeof( servAddr ) ) < 0 );
  
  //
  // Receive message from client
  //
  bzero( recvmsg, sizeof( recvmsg ) );
  cliLen = sizeof( cliAddr );
  nbytes = recvfrom( sd, &recvmsg, sizeof( recvmsg ), 0, (struct sockaddr *) &cliAddr, &cliLen );
  ERROR( nbytes < 0 );

  printf( "Client(%s:%d): %s\n", inet_ntoa( cliAddr.sin_addr ), ntohs( cliAddr.sin_port), recvmsg );


  // Get Header Data
  current.SeqNum = recvmsg[0];
  current.AckNum = recvmsg[1];
  current.Flags  = recvmsg[2];
  
  // Open log and output file
  logfp = fopen( log, "w" ); ERROR( logfp < 0 );
  outfp = fopen( out, "w" ); ERROR( outfp < 0 );

  // Get time
  time(&rawtime);
  timeinfo = localtime( &rawtime );
  strftime( timebuff, PACKETSIZE, "%I:%M%p", timeinfo );

  // Copy and write  recieved data
  memcpy( buffer, recvmsg + 3, strlen(recvmsg) - 3 );
  fprintf( outfp, "%s", buffer );
  fprintf( logfp, "Recieve %i %s\n", current.SeqNum, timebuff );

  // Close files
  fclose( logfp ); fclose( outfp );
  
  //
  // Respond using sendto_ in order to simulate dropped packets
  //
  nbytes = sendto_( sd, response, PACKETSIZE, 0, (struct sockaddr *) &cliAddr, sizeof( cliAddr ) );
  ERROR( nbytes < 0 );

  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
