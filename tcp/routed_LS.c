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
#include <arpa/inet.h> // sockaddr_in
#include <ctype.h>     // toupper()
#include <errno.h>     // strerror( errno )
#include <netdb.h>
//#include <netinet/in.h>
//#include <signal.h>
#include <string.h>    // memset(), strerror()
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
//#include <sys/time.h> // select()
//#include <sys/types.h>
#include <unistd.h>
#include <time.h>       // For time stamping

#define ERROR( boolean ) if ( boolean ) {\
  fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__, strerror( errno ) );\
  exit( EXIT_FAILURE );\
}
#define PKTSIZ      512
#define MAX_PENDING 10
#define MAXLINE     256 
#define MAXROUTE    10
#define MAXPATH     MAXROUTE*3
#define MAXTTL      120
#define MAXHOP      2
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
typedef struct {
  char name[MAXLINE];      // The name of the node
  int  port;               // The port on me
  int  sock;               // My initial socket
  int  newsock;            // My socket to use when connection is made
  socklen_t len;           // Length of socket
  struct sockaddr_in addr; // My socket address
} Wire;

typedef struct {
  int  cost;               // The cost to use this connection
  Wire src;                // Me
  Wire dst;                // My neighbor
} Nodes;

typedef struct {       // Link State Packet (pg. 253)
  int ttl;             // Time to live for this packet
  float seqnum;        // Sequence number of packet
  char src[MAXLINE];   // Name of the original creator of this packet
  struct {               
	int cst;          // The total cost of the path
	char dst[MAXLINE]; // The destination node in the routing table
	char nxt[MAXLINE]; // The name of the node to send to to get there
  } route;             // The route we're advertising
} LSP;                 // Link State Packet

typedef struct {
  int total;           // The total number of nodes
  struct {
	char dst[MAXLINE]; // Address of Destination
	char nxt[MAXLINE]; // Address of next hop
	int  cst;          // Distance Metric
	int  ttl;          // Time to Live
  } route[MAXROUTE];   // List of al lthe routes
} RoutingTable;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to log information, complete with timestamp
void logTime ( char *fp, char *msg );

// Read in the initialization Text file and only store information for my node
int getNeighbors( char *file, Nodes *net, char *src );

// Print Link State Packet
void printLSP( char *file, LSP *packet );

// Print the routing table
void printRoutingTable( char *file, RoutingTable *rt );

// Print connection table
void printTable( char *file, Nodes *net, int count );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {
  // Variables
  char  *src;           // My Name
  char  msg[MAXLINE];   // Message for the log
  int   count = 0;      // My number of neighbors
  int   i,j,k;            // Iterator
  int   code;           // Used to check error codes
  char  *log;           // My log file
  Nodes net[MAXROUTE];  // A struct to hold my connections to my neighbors
  RoutingTable rt;      // Routing Table
  int stabilized;       // Used to determine when routing table is done converging on lowest cost pathes
  int update = 0;       // Route Table Update Value
  //char buf[MAXLINE];    // test message for connection  
  char buf[sizeof(LSP)];    // test message for connection  
  
  float seqnum = 0;     // Our sequence number
  LSP send, recv;       // Link state packets we send and received

  //--------------------------------------
  // Initialization File Set-Up
  //--------------------------------------

  // Check command line args
  if( argc < 4 ) {
    fprintf( stderr, 
			 "Usage: %s <RouterID> <LogFileName> <InitializationFile>\n", 
			 argv[0] );
    exit( EXIT_FAILURE );
  }

  // Get which node this router is
  src  = (char *) toupper( (int) argv[1] );

  // Initialize the log
  log  = argv[2];
  sprintf( msg, "Initialized log for router %s\n", src );
  logTime( log, msg );  

  // Get connections from Init file
  count = getNeighbors( argv[3], net, src );
  sprintf( msg, "Router %s has %i neighbors\n", src, count );
  logTime( log, msg );  

  // If count is zero, then there was an error on the command line or
  //   in the initialization file
  if ( count == 0 ) {
	sprintf( msg, "Unable to find router %s in %s\n", src, argv[3] );
	logTime( log, msg );
	exit( EXIT_FAILURE );
  }

  printTable( log, net, count );

  //--------------------------------------
  // Update Routing Table
  //--------------------------------------
  for ( i = 0; i < count; ++i ) {
	strcpy( rt.route[i].dst, net[i].dst.name );
	strcpy( rt.route[i].nxt, net[i].dst.name );
	rt.route[i].cst = net[i].cost;
	rt.route[i].ttl = MAXTTL;
  }
  rt.total = count;

  printRoutingTable( log, &rt );

  //--------------------------------------
  // Opening of the Sockets
  //--------------------------------------
  for ( i = 0; i < count; ++i ) {
	// Create the sockets
	net[i].src.sock = socket( PF_INET, SOCK_STREAM, 0 ); ERROR( net[i].src.sock < 0 );
	net[i].dst.sock = socket( PF_INET, SOCK_STREAM, 0 ); ERROR( net[i].dst.sock < 0 );

	sprintf( msg, "(%s <==> %s) Creating sockets\n", net[i].src.name, net[i].dst.name );
	logTime( log, msg );
	
	// Build destination address data structure
	net[i].dst.len = sizeof( net[i].dst.addr );
	bzero( (char *) &net[i].dst.addr, net[i].dst.len );
	net[i].dst.addr.sin_family      = AF_INET;
	net[i].dst.addr.sin_addr.s_addr = INADDR_ANY;
 	net[i].dst.addr.sin_port        = htons( net[i].dst.port );
		
	sprintf( msg, "(%s <==> %s) Built destination address\n", net[i].src.name, net[i].dst.name );
	logTime( log, msg );

	// Bind the socket to the address (Passive Open)
	code = bind( net[i].dst.sock, (struct sockaddr *) &net[i].dst.addr, net[i].dst.len);
	ERROR( code < 0 );
	
	sprintf( msg, "(%s <==> %s) Bound the source socket\n", net[i].src.name, net[i].dst.name );
	logTime( log, msg );
	sleep( rand() % 10 );

	// Build source address data structure
	net[i].src.len = sizeof( net[i].src.addr );
	bzero( (char *) &net[i].src.addr, net[i].src.len );
	net[i].src.addr.sin_family      = AF_INET;
	//    net[i].src.addr.sin_addr.s_addr = INADDR_ANY;
	net[i].src.addr.sin_port        = htons( net[i].src.port );

	sprintf( msg, "(%s <==> %s) Built source address\n", net[i].src.name, net[i].dst.name );
	logTime( log, msg );

	// Active open
	listen( net[i].dst.sock, MAX_PENDING );
	code = connect( net[i].src.sock, (struct sockaddr *) &net[i].dst.addr, net[i].dst.len );
	ERROR( code < 0 );
	sprintf( msg, "(%s <==> %s) Made active connection\n", net[i].src.name, net[i].dst.name );
	logTime( log, msg );

	net[i].src.newsock = accept( net[i].dst.sock, (struct sockaddr *) &net[i].src.addr, &net[i].src.len );
	ERROR( net[i].src.newsock < 0 );
	sprintf( msg, "(%s <==> %s) Accepted active connection\n", net[i].src.name, net[i].dst.name );
	logTime( log, msg );
	
	printTable( log, net, count );
  }

  //--------------------------------------
  // The Sending of the Link State Packets
  //--------------------------------------
  while ( stabilized == 0 ) {
	// loop through each neighbor
	for ( i = 0; i < count; ++i ) {
	  // Set up Link State Packet
	  send.ttl = MAXHOP;
	  send.seqnum = seqnum++;
	  strcpy( send.src, src );
	  send.route.cst = net[i].cost;
	  strcpy( send.route.dst, net[i].dst.name );
	  strcpy( send.route.nxt, net[i].src.name );	
	  
	  // Send LSP
	  memcpy( &buf, &send, sizeof( LSP ) );
	  code = write( net[i].src.sock, buf, MAXLINE); ERROR( code < 0 );
	  sprintf( msg, "Sent Link State Packet to router %s\n", net[i].dst.name );
	  logTime( log, msg );
	  printLSP( log, &send );	  

	  // listen for LSP's
	  bzero( buf, MAXLINE );
	  code = read( net[i].src.newsock, buf, MAXLINE ); ERROR( code < 0 );
	  memcpy( &recv, &buf, sizeof( LSP ) );
	  sprintf( msg, "Recieved Link State Packet from router %s\n", recv.src );
	  logTime( log, msg );
	  printLSP( log, &recv );	  

	  //     If LSP recieved, update routing table as necessary, set routing table update to 
	  //     0 (routing table update is global value, or global to the while loop... not 
	  //     specific for each neighbor in the node)
	  for ( j = 0; j < rt.total; ++j ) {
		if ( strcmp( rt.route[j].dst, recv.route.dst ) == 0 ) {
		  if ( rt.route[j].cst > recv.route.cst ) {
			strcpy( rt.route[j].dst, recv.route.dst);
			strcpy( rt.route[j].nxt, recv.route.nxt );
			rt.route[j].cst = recv.route.cst;
			rt.route[j].ttl = MAXTTL;			
		  }
		}
		else {
		  strcpy( rt.route[rt.total++].dst, recv.route.dst);
		  strcpy( rt.route[rt.total].nxt, recv.route.nxt );
		  rt.route[rt.total].cst = recv.route.cst;
		  rt.route[rt.total].ttl = MAXTTL;
		}
		update++;
	  }

	  //     If no LSP recieved, add 1 to Routing table update value
	  // Wait random amount of time
	  sleep( rand() % 10 );	  
	  printRoutingTable( log, &rt );
	} // end loop through neighbors
	
	//if router table update value  > 5 set stabilized to 1
	if( update > 5 ) stabilized = 1;

	// Update Routing Table
	for ( j = 0; j < rt.total; ++j ) {
	  if ( --rt.route[i].ttl == 0 ) {
		for ( k = j; k < rt.total-1; ++k ) {
		  strcpy( rt.route[k].dst, rt.route[k + 1].dst ); 
		  strcpy( rt.route[k].nxt, rt.route[k + 1].nxt ); 
		  rt.route[k].cst = rt.route[k + 1].cst; 
		  rt.route[k].ttl = rt.route[k + 1].ttl; 
		}
		rt.total--;
	  }
	}
	printRoutingTable( log, &rt );
  }

  // Close Sockets
  for ( i = 0; i < count; ++i ) {
	close( net[i].src.sock );
	close( net[i].dst.sock ); 
	close( net[i].src.newsock ); 
	sprintf( msg, "(%s <==> %s) Closed sockets\n", net[i].src.name, net[i].dst.name );
	logTime( log, msg );	
  }
  sprintf( msg, "Finished running for router %s\n", src );
  logTime( log, msg );

  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Read in the initialization Text file and only store information for my node
int getNeighbors( char *file, Nodes *net, char *src ) {
  char  *pch;          // A character pointer to tokenize the input
  char  line[MAXLINE]; // Input from the init file
  int   count = 0;     // A counter to count my neighbors
  FILE  *init;         // The init file

  init = fopen( file, "r" ); ERROR( init < 0 );
  while ( fgets( line, PKTSIZ, init ) != NULL ) {
	pch = strtok( line, " <,>" );
	if ( strcmp( src, pch ) == 0 ) {
	  // Get Source Name
	  strcpy( net[count].src.name, pch );

	  // Get Source Port
	  pch = strtok( NULL, " <,>" ); ERROR ( pch == NULL );
	  net[count].src.port = (int) atoi( pch );

	  // Get Destination Name
	  pch = strtok( NULL, " <,>" ); ERROR ( pch == NULL );
	  strcpy( net[count].dst.name, pch );

	  // Get Destination Port
	  pch = strtok( NULL, " <,>" ); ERROR ( pch == NULL );
	  net[count].dst.port = (int) atoi( pch );

	  // Get Cost of Link
	  pch = strtok( NULL, " <,>" ); ERROR ( pch == NULL );
	  net[count].cost     = (int) atoi( pch );

	  // Init Basics
	  net[count].src.sock    = -1;
	  net[count].dst.sock    = -1;
	  net[count].src.newsock = -1;
	  net[count].dst.newsock = -1;

	  // Increment
	  count++;
	}
  }
  fclose( init );

  return count;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to log information, complete with timestamp
void logTime ( char *fp, char *msg ) {
  time_t rawtime;
  struct tm *timeinfo;
  char timebuff[PKTSIZ];
  FILE *log;
   
  // Get time
  time( &rawtime );
  timeinfo = localtime( &rawtime );
  strftime( timebuff, PKTSIZ, "%r", timeinfo );

  // Update log
  log = fopen( fp, "a" ); ERROR( log < 0 );
  fprintf( log, "[%s] %s", timebuff, msg );
  fclose( log );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Print Link State Packet
void printLSP( char *file, LSP *packet ) {
  FILE *log;
  char msg[MAXLINE];
  
  sprintf( msg, "Printing %s's Link State Packet No. %f\n", 
		   packet->src, packet->seqnum );
  logTime( file, msg );
    
  log = fopen( file, "a" ); ERROR( log < 0 );
  fprintf( log, "DEST\tNEXT\tCOST\tTTL\n" );
  fprintf( log, "%s\t%s\t%i\t%i\n", 
		   packet->route.dst, packet->route.nxt, packet->route.cst, packet->ttl );
  fclose( log );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Print the LS table
void printRoutingTable( char *file, RoutingTable *rt ) {
  int i;
  FILE *log;
  char msg[MAXLINE];
  
  sprintf( msg, "Printing routing table\n" );
  logTime( file, msg );
    
  log = fopen( file, "a" ); ERROR( log < 0 );
  fprintf( log, "DEST\tNEXT\tCOST\tTTL\n" );
  for ( i = 0; i < rt->total; ++i ) {
	fprintf( log, "%s\t%s\t%i\t%i\n", 
			 rt->route[i].dst, rt->route[i].nxt, rt->route[i].cst, rt->route[i].ttl );
  }
  fclose( log );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Print connection table
void printTable( char *file, Nodes *net, int count ) {
  int i;
  FILE *log;
  char msg[MAXLINE];
  
  sprintf( msg, "Printing connection table\n" );
  logTime( file, msg );

  log = fopen( file, "a" ); ERROR( log < 0 );
  fprintf( log, "SRC\tSRCPRT\tDST\tDSTPRT\tCOST\tSRCSCK\tDSTSCK\n" );
  for ( i = 0; i < count; ++i ) {
	fprintf( log, "%s\t%i\t%s\t%i\t%i\t%i\t%i\n", 
			 net[i].src.name, net[i].src.port, net[i].dst.name, net[i].dst.port, 
			 net[i].cost, net[i].src.sock, net[i].dst.sock );
  }
  fclose( log );
}
////////////////////////////////////////////////////////////////////////////////
