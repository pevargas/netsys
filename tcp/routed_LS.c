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
//#include <string.h>   /* memset() */
#include <stdio.h>
#include <stdlib.h>
//#include <sys/socket.h>
//#include <sys/time.h> /* select() */
//#include <sys/types.h>
//#include <unistd.h>
//#include <time.h>

#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno) );\
  exit( EXIT_FAILURE );\
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {

  // Check command line args
  if( argc < 4 ) {
    printf("Usage: %s <RouterID> <LogFileName> <InitializationFile>\n", argv[0]);
    exit( EXIT_FAILURE );
  }

  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
