////////////////////////////////////////////////////////////////////////////////
// File:    client_PFS.c                Fall 2013
// Student: Patrick Vargas              patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "athena.h"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Funciton to connect and register and all that nonsense
void connectPlease( Folder *dir, int sock );

// Creates a socket for the ip address and port - From ***REFERENCE***
int createSocket( unsigned long ip, unsigned short port );

// Gets the master list from the server
void getMaster( Repository *r, int sock );

// List all the files this client has.
void ls( Folder *dir ); 

// Pretty print the catalog
void printCatalog( Folder *dir );

// Send my list of files
void sendList( Folder *dir, int sock );

// Register myself with the server
int registerClient( Folder *dir, int sock );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  int nbytes;                // Number of bytes
  int sock;                  // This will be our socket
  char buffer[ MAXBUFSIZE ]; // A buffer to buff things
  char cmd[ MAXBUFSIZE ];    // Command to be sent to Server
  char *newline = NULL;      // Get newline
  Folder dir;                // Local list of folder Files
  Repository repo;           // Master file list

  // Make sure name, ip and port are given
  if ( argc < 4 ) {
	printf("USAGE:  <client_name> <server_ip> <server_port>\n");
	exit ( EXIT_FAILURE );
  }

  sprintf( dir.name, "%s", argv[1] );
  ls( &dir );
  
  printf( "$ Welcome '%s'\n", dir.name );
  
  sock = createSocket( inet_addr( argv[2] ), atoi( argv[3] ) );
  connectPlease( &dir, sock );
  //  getMaster( &repo, sock );

  //
  // Enter command loop
  //
  do {
	bzero( cmd, sizeof( cmd ) );
	printf( "> " );
	
	//
	// Grab Command Line
	//
	if ( fgets( cmd, MAXBUFSIZE, stdin ) != NULL ) {
	  newline = strchr( cmd, '\n'); // Find the newline
	  if ( newline != NULL ) *newline = '\0'; // Overwrite
	  
	  switch ( parseCmd ( cmd ) ) 
		{
		case LS:
		  printCatalog ( &dir );
		  break;
		case GET:
		  getMaster( &repo, sock );
		  break;
		case EXIT: 
		  nbytes = write( sock, "EXIT", MAXBUFSIZE );
		  ERROR( nbytes < 0 );
		  break;
		default:
		  nbytes = write( sock, cmd, MAXBUFSIZE );
		  ERROR( nbytes < 0 );
		  bzero( buffer, MAXBUFSIZE );
		  nbytes = read( sock, buffer, MAXBUFSIZE );
		  printf( "$ %s\n", buffer);
		  break;
		}	
	}
  } while ( !isQuit( cmd ) );  

  close( sock );
  
  return EXIT_SUCCESS;
} // main( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Funciton to connect and register and all that nonsense
void connectPlease( Folder *dir, int sock ) {
  int nbytes;                // Number of bytes
  char buffer[ MAXBUFSIZE ]; // Buffer

  nbytes = write( sock, "CONNECT", MAXBUFSIZE );
  ERROR( nbytes < 0 );
  
  bzero( buffer, MAXBUFSIZE );
  nbytes = read( sock, buffer, MAXBUFSIZE );
  ERROR ( nbytes < 0 );
  switch ( parseCmd( buffer ) ) {
  case CONNECT:
	{
	  printf( "$ Connection Established\n" );
	  switch ( registerClient( dir, sock ) ) {
	  case FAILURE: 
		{
		  printf( "$ There is already a user with the name '%s' on the server.\n", 
				  dir->name );
		  close( sock );
		  exit( EXIT_SUCCESS );
		}
	  case SUCCESS:
		{
		  printf( "$ You have been connected as %s\n", dir->name );
		  sendList( dir, sock );
		  break;
		}
	  }	  
	  break;
	}
  default: 
	{
	  printf( "$ Error in registering\n" );
	}
  }
} // connectionPlease( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates a socket for the port - From ***REFERENCE***
int createSocket( unsigned long ip, unsigned short port ) {
  int sock;                 // The socket to create
  struct sockaddr_in remote; // Local address

  // Create socket for incoming connections
  sock = socket( PF_INET, SOCK_STREAM, 0 );
  ERROR( sock < 0 );

  // Construct local address structure
  bzero( &remote, sizeof( remote ) );     // Zero out structure
  remote.sin_family      = PF_INET;       // Internet address family
  remote.sin_addr.s_addr = ip;            // The address for the server
  remote.sin_port        = htons( port ); // Local port

  ERROR( connect( sock, (struct sockaddr *) &remote, sizeof( remote ) ) < 0 );
  printf( "$ Connected to remote\n" );

  return sock; 
} // createSocket( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Gets the master list from the server
void getMaster( Repository *r, int sock ) {
  int nbytes;                // Number of Bytes
  char buffer[ MAXBUFSIZE ]; // Buffer

  nbytes = write( sock, "GET", MAXBUFSIZE );
  ERROR( nbytes < 0 );

  bzero( buffer, MAXBUFSIZE );
  nbytes = read( sock, buffer, MAXBUFSIZE );
  ERROR( nbytes < 0 );

  memcpy( r, &buffer, sizeof( Repository ) );
  printRepo( r );
} // getMaster( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// List all the files this client has. Returns the number of files
void ls ( Folder *dir ) {
  int count;                 // The token number
  FILE *fp;                  // The file pointer to the pipe
  char *pch;                 // Pointer to tokens
  char buffer[ MAXBUFSIZE ]; // Buffer to hold the line from the pipe
   
  dir->total = 0;

  // Open the pipe
  fp = popen( "ls -o", "r" );
  ERROR( fp == NULL );

  // Read until the pipe is empty
  while ( fgets( buffer, MAXBUFSIZE, fp ) != NULL ) {
	count = 0;

	// Tokenize the input
	pch = strtok( buffer, " " );

	// First line of ls is how many total file size
	if ( strcmp( pch, "total" ) == 0 ) continue;

	// Read each token
	while ( pch != NULL ) {
	  switch ( count ) {
	  case 3: // Size of File
		dir->list[dir->total].size = atoi( pch );
		break;
	  case 7: // File Name
		pch[strlen( pch ) - 1] = '\0';
		strcpy( dir->list[dir->total++].name, pch );
		break;
	  }
	  pch = strtok( NULL, " " );
	  count++;
	}
  }
  ERROR( pclose( fp ) < 0 );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Pretty print the catalog
void printCatalog ( Folder *dir ) {
  int i, temp, maxName, maxSize;
  char header[ MAXLINE ];
  if ( dir->total < 1 ) return;

  maxName = strlen( dir->list[0].name );
  maxSize = 8;

  for ( i = 1; i < dir->total; ++i ) {
	if ( ( temp = strlen( dir->list[i].name ) ) > maxName )
	  maxName = temp;

	/*if ( dir->list[i].size > 0 )
	  if ( ( temp = (int) log10( (double) dir->list[i].size ) + 1 ) > maxSize )
	  maxSize = temp;*/
  }

  temp = sprintf( header, "| %*s | %*s |", maxName, "File Name", maxSize, "Size" );
  printf( "+" );
  for ( i = 1; i < temp-1; ++i ) {
	if ( i == ( maxName + 3 ) ) printf( "+" );
	else                        printf( "-" );
  }
  printf( "+\n%s\n+", header );
  for ( i = 1; i < temp - 1; ++i ) {
	if ( i == ( maxName + 3 ) ) printf( "+" );
	else                        printf( "-" );
  }
  printf( "+\n" );

  for ( i = 0; i < dir->total; ++i )
	printf( "| %*s | %*i |\n", maxName, dir->list[i].name, maxSize, dir->list[i].size );

  printf( "+" );
  for ( i = 1; i < temp-1; ++i ) {
	if ( i == ( maxName + 3 ) ) printf( "+" );
	else                        printf( "-" );
  }
  printf( "+\nTotal:\t%i\n", dir->total );
} // printCatalog( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Send my list of files
void sendList( Folder *dir, int sock ) {
  int i;                  // Iterator
  int nbytes;             // Number of bytes
  char buffer[ MAXBUFSIZE ]; // Message to send
  //  FM temp; 

  ls( dir );

  bzero( buffer, MAXBUFSIZE );
  sprintf( buffer, "%i", dir->total );
  printf( "$ Will forward %s files\n", buffer );
  nbytes = write( sock, buffer, MAXBUFSIZE );
  ERROR( nbytes < 0 );

  for ( i = 0; i < dir->total; ++i ){
	bzero( buffer, MAXBUFSIZE );
	memcpy( &buffer, &dir->list[i], sizeof( FM ) );
	nbytes = write( sock, buffer, MAXBUFSIZE );
	ERROR( nbytes < 0 );
  }

  printCatalog( dir );
  
} // sendList( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Register myself with the server
int registerClient( Folder *dir, int sock ) {
  int nbytes;                // Number of bytes
  char buffer[ MAXBUFSIZE ]; // Buffer

  printf( "$ Sending name '%s'\n", dir->name );
  sprintf( buffer, "%s", dir->name );

  nbytes = write( sock, dir->name, MAXBUFSIZE );
  ERROR( nbytes < 0 );

  bzero( buffer, MAXBUFSIZE );
  nbytes = read( sock, buffer, MAXBUFSIZE );
  ERROR( nbytes < 0 );

  return parseCmd( buffer );
} // registerClient( )
////////////////////////////////////////////////////////////////////////////////
