////////////////////////////////////////////////////////////////////////////////
// File: routed_LS.c                    Fall 2013
// Students: 
//   Brittney Harsha                    Patrick Vargas
//   b.grace.harsha@gmail.com           patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements:
//   
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//#include <arpa/inet.h>
#include <errno.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <signal.h>
#include <string.h>   /* memset(), strerror() */
#include <stdio.h>
#include <stdlib.h>
//#include <sys/socket.h>
//#include <sys/time.h> /* select() */
//#include <sys/types.h>
//#include <unistd.h>
//#include <time.h>

#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
  exit( EXIT_FAILURE );\
}
#define PKTSIZ 512
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
typedef struct {
  char src;    // The name of the source router
  char dst;    // The name of the destination router
  int srcPort; // The port on source router (ie: Me)
  int dstPort; // The port on the destination router
  int cost;    // The cost to get there
} Nodes;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {
  // Variables
  char src;          // My Name
  char *pch;         // A charager to tokenize the input
  char line[PKTSIZ]; // Input from the init file
  int count = 0;     // A counter to count my neighbors
  FILE *init;        // The init file
  FILE *log;         // My log file
  Nodes LS[PKTSIZ];  // A struct to hold my connections to my neighbors

  // Check command line args
  if( argc < 4 ) {
    printf("Usage: %s <RouterID> <LogFileName> <InitializationFile>\n", argv[0]);
    exit( EXIT_FAILURE );
  }

  log  = fopen( argv[2], "w" ); ERROR( log < 0 );
  init = fopen( argv[3], "r" ); ERROR( init < 0 );

  while ( fgets( line, PKTSIZ, init ) != NULL ) {
	fprintf( log, "%s", line );	
  }

  fclose( log );
  fclose( init );

  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
