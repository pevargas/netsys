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
#include "sendto_.h"
//#include "swp.h" // Contains funcs and strcuts for SWP

#define PACKETSIZE 1024
#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno) );\
  exit( EXIT_FAILURE );\
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  //
  // Variables
  //
  int sd;                      // Socket
  int nbytes;                  // Number of bytes sent/received
  struct sockaddr_in remote;   // Server address
  struct sockaddr_in fromAddr; // response address
  unsigned int fromLen;        // Response length
  char msg[ PACKETSIZE ];      // Packet to send
  char recvmsg[ PACKETSIZE ];  // Response 
  FILE *fp;                    // Pointer to file

  //
  // Check command line args
  //
  if( argc < 7 ) {
    printf("Usage: %s <server_ip> <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
    exit( EXIT_FAILURE );
  }

  // Note: you must initialize the network library first before calling sendto_().
  //   The arguments are the <errorrate> and <random seed>
  init_net_lib( atof( argv[ 3 ] ), atoi( argv[4] ) );
  printf( "Error rate: %f\n", atof( argv[3] ) );

  //
  // Socket creation
  //
  sd = socket( AF_INET, SOCK_DGRAM, 0 );
  ERROR( sd < 0 );

  // Get server IP address (input must be IP address, not DNS name)
  bzero( &remote, sizeof( remote ) );          //zero the struct
  remote.sin_family      = AF_INET;                 //address family
  remote.sin_port        = htons( atoi( argv[2] ) );      //sets port to network byte order
  remote.sin_addr.s_addr = inet_addr( argv[1] ); //sets remote IP address
  printf( "%s: sending data to '%s:%s' \n", argv[0], argv[1], argv[2] );

  //
  // Open file
  //
  fp = fopen( argv[5], "r" );
  ERROR( fp == NULL );

  fgets( msg, PACKETSIZE, fp );

  //
  // Call sendto_ in order to simulate dropped packets
  //
  nbytes = sendto_( sd, msg, PACKETSIZE, 0, (struct sockaddr *) &remote, sizeof( remote ) );
  ERROR( nbytes < 0 );

  //
  // Receive message from server
  //
  bzero( recvmsg, sizeof( recvmsg ) );
  fromLen = sizeof( fromAddr );
  nbytes = recvfrom( sd, &recvmsg, PACKETSIZE, 0, (struct sockaddr *) &fromAddr, &fromLen );
  ERROR( nbytes < 0 );

  printf( "Server(%s:%d): %s\n", inet_ntoa( fromAddr.sin_addr ), ntohs( fromAddr.sin_port), recvmsg );

  ERROR( fclose( fp ) );

  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

