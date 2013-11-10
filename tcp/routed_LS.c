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
#include <ctype.h>  // toupper()
#include <errno.h>  // strerror( errno )
//#include <netdb.h>
//#include <netinet/in.h>
//#include <signal.h>
#include <string.h> // memset(), strerror()
#include <stdio.h>
#include <stdlib.h>
//#include <sys/socket.h>
//#include <sys/time.h> // select()
//#include <sys/types.h>
#include <unistd.h>
#include <time.h>   // For timestamping

#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
  exit( EXIT_FAILURE );\
}
#define PKTSIZ 512
#define MAX_PENDING 5 
#define MAX_LINE 256 
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
  char  *src;         // My Name
  char  *pch;         // A character to tokenize the input
  char  line[PKTSIZ]; // Input from the init file
  char  msg[PKTSIZ];  // Message for the log
  int   count = 0;    // A counter to count my neighbors
  FILE  *init;        // The init file
  FILE  *log;         // My log file
  Nodes LS[PKTSIZ];   // A struct to hold my connections to my neighbors

  struct sockaddr_in sin; 
      char buf[MAX_LINE]; 
      int len; 
      int s, new_s;

  // Check command line args
  if( argc < 4 ) {
    fprintf( stderr, 
			 "Usage: %s <RouterID> <LogFileName> <InitializationFile>\n", 
			 argv[0] );
    exit( EXIT_FAILURE );
  }

  src  = argv[1];
  log  = fopen( argv[2], "w" ); ERROR( log < 0 );
  init = fopen( argv[3], "r" ); ERROR( init < 0 );

  sprintf( msg, "Initialized log for router %s\n", src );
  logTime( log, msg );

  while ( fgets( line, PKTSIZ, init ) != NULL ) {
	pch = strtok( line, " <,>" );
	fprintf( log, "%s vs %s is %i\n", pch, src, pch == src );
	if ( pch == src ) {
	  sprintf( msg, "%s", line );
	  logTime( log, msg );
	}
  }

  if ( count == 0 ) {
	sprintf( msg, "Unable to find router %s in %s\n", src, argv[3] );
	logTime( log, msg );
  }
  // neighbor tables
  // while < ports (in A's case 3)
  int i = 0;
  while(i < count)
    {
      /* build address data structure */ 
      bzero((char *)&sin, sizeof(sin)); 
      sin.sin_family = AF_INET; 
      sin.sin_addr.s_addr = INADDR_ANY; 
      // sin.sin_port = htons(nodes.dst);  NEED TO KNOW WHAT THIS VARIABLE IS FROM NEIGHBOR TABLES, DON'T THINK ITS NODES.DST
      /* setup passive open */ 
      if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) { 
	perror("simplex-talk: socket"); 
	exit(1); 
      } 
      if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) { 
	perror("simplex-talk: bind"); 
	exit(1); 
      } 
      listen(s, MAX_PENDING);
      i++;
    }
  sleep(rand() % 50);
  // pull each line from neighbor table for port numbers
  /*
    

    struct hostent *hp; 
    struct sockaddr_in sin; 
    char *host; 
    char buf[MAX_LINE]; 
    int s; 
    int len;


    bzero((char *)&sin, sizeof(sin)); 
    sin.sin_family = AF_INET; 
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length); 
    sin.sin_port = htons(nodes.dst); 
 
    // active open
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) { 
    perror("simplex-talk: socket"); 
    exit(1); 
    } 
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) 
    { 
    perror("simplex-talk: connect"); 
    close(s); 
    exit(1); 
    } 
   */

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
