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
#include <unistd.h>
#include <time.h>

#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
  exit( EXIT_FAILURE );\
}
#define PKTSIZ 512
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function to log information, complete with timestamp
void logTime ( FILE *fp, char *msg );
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
  char *src;          // My Name
  char *pch;         // A charager to tokenize the input
  char line[PKTSIZ]; // Input from the init file
  char msg[PKTSIZ];  // Message for the log
  int count = 0;     // A counter to count my neighbors
  FILE *init;        // The init file
  FILE *log;         // My log file
  Nodes LS[PKTSIZ];  // A struct to hold my connections to my neighbors

  // Check command line args
  if( argc < 4 ) {
    printf("Usage: %s <RouterID> <LogFileName> <InitializationFile>\n", argv[0]);
    exit( EXIT_FAILURE );
  }

  src = argv[1];
  log  = fopen( argv[2], "w" ); ERROR( log < 0 );
  init = fopen( argv[3], "r" ); ERROR( init < 0 );

  sprintf( msg, "Initialized log for router %s\n", src );
  logTime( log, msg );

  while ( fgets( line, PKTSIZ, init ) != NULL ) {
   	sprintf( msg, "%s", line );
	logTime( log, msg );
  }

  sprintf( msg, "Finished running for router %s\n", src );
  logTime( log, msg );
  fclose( log );
  fclose( init );

  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to log information, complete with timestamp
void logTime ( FILE *fp, char *msg ) {
  time_t rawtime;
  struct tm *timeinfo;
  char timebuff[PKTSIZ];
   
  // Get time
  time( &rawtime );
  timeinfo = localtime( &rawtime );
  strftime( timebuff, PKTSIZ, "%r", timeinfo );
  
  // Update log
  fprintf( fp, "[%s] %s", timebuff, msg );
}
////////////////////////////////////////////////////////////////////////////////
