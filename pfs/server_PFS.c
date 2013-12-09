////////////////////////////////////////////////////////////////////////////////
// File: server_PFS.c                   Fall 2013
// Student: Patrick Vargas              patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements:
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
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXBUFSIZE 512
#define MAXLINE    256
#define MAXPENDING 5
#define MAXFILES   512
#define MAXCLIENTS 5

#define ERROR( boolean ) if ( boolean ) {\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
    exit ( EXIT_FAILURE );\
  }

enum COMMAND { SUCCESS, FAILURE, CONNECT, LS, EXIT, INVALID };

typedef struct {  
  int size;              // The size of the file (in B)
  char name[ MAXLINE ];  // The name of the file
} FM;                    // FileMeta

typedef struct {
  int total;
  struct {
	  char name[MAXLINE];      // File owner
	  struct in_addr sin_addr; // Address of owner
	  unsigned short sin_port; // Port of owner 
	  int entries;             // Number of Files
	  FM list[ MAXFILES ];
  } cxn[ MAXCLIENTS ];
  struct {
	int cname; // Maxlength of Conneciton name
	int addr;  // Max address length
	int port;  // Max port length
	int fname; // Max file name length
	int fsize; // Max Size length
  } max; // Used in printing table, Don't judge my OCD
} Repository;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Accepts a TCP Conection along the socket - From ***REFERENCE***
int acceptConnection( int sock );

// Creates a socket for the port - From ***REFERENCE***
int createSocket( unsigned short port );

// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );

// Function to get the list from a certain connection
void getList ( Repository *r, int sock );

// Function to aid in printing process
void maxPrint( Repository *r );

// Take the input and see what needs to be done.
int parseCmd ( char cmd[ ] );

// Recieve put data from the client
void put ( char msg [], char buffer [], int sock, struct sockaddr_in remote );

// Pretty print the catalog
void printRepo ( Repository *r );

// Register the client
int registerClient( Repository *r, int sock );

// Remove client from list
int removeClient( Repository *r, int sock ); 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  //
  // Variables
  //
  int sock, nuSock;                  // This will be our sockets
  int nbytes;                        // number of bytes we receive in our message
  //  struct sockaddr_in client, remote; // "Internet socket address structure"
  //unsigned int remote_length;        // length of the sockaddr_in structure
  char buffer[ MAXBUFSIZE ];         // a buffer to store our received message
  char msg[ MAXBUFSIZE ];            // Message to return
  Repository repo; // The registration list

  //
  // Make sure port is given on command line
  // 
  if ( argc != 2 ) {
	printf( "USAGE:  <port>\n" );
	exit( EXIT_FAILURE );
  }

  //
  // Initialize all the things!
  //
  repo.total = 0;
  maxPrint( &repo );
  sock = createSocket( atoi( argv[1] ) );

  //
  // Enter command loop
  //
  for ( ; ; ) {
	printf( "$ Listening\n" );
	// Loop through all the conenctions Available
	while ( ( nuSock = acceptConnection( sock ) )  < 0 );

	bzero( buffer, MAXBUFSIZE );
	nbytes = read( nuSock, buffer, MAXBUFSIZE );
	ERROR( nbytes < 0 );
	printf( "> %s\n", buffer );
	
	switch ( parseCmd ( buffer ) ) 
	  {
	  case CONNECT:
		{
		  nbytes = write( nuSock, "SUCCESS", MAXBUFSIZE );
		  ERROR( nbytes < 0 );
		  switch( registerClient( &repo, nuSock ) ) 
			{
			case SUCCESS:
			  getList ( &repo, nuSock );
			  printf( "> Success in registering '%s'\n", repo.cxn[repo.total-1].name );
			  break;
			case FAILURE:
			  printf( "> Error in registration\n" );
			  break;
			}
		  break;
		}
	  case EXIT:
		{
		  switch( removeClient( &repo, nuSock ) )
			{
			case SUCCESS: 
			  {
			    printf( "> Client has been removed.\n" );
				break;
			  }
			case FAILURE:
			  {
				printf( "> Error removing client.\n" );
				break;
			  }
			}
		  break;
		}
	  default: sprintf( msg, "> Invalid command: %s", buffer); break;
	  }
  }
	//  } while ( !isQuit( buffer ) );  

  close( nuSock );
  close( sock );
  
  return EXIT_SUCCESS;
} // main( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Accepts a TCP Conection along the socket - From ***REFERENCE***
int acceptConnection( int sock ) {
  int nuSock;                 // The new socket to be used
  struct sockaddr_in remote;  // The address of the remote connection
  unsigned int remote_length; // The length of the address

  // Set the size
  remote_length = sizeof( remote );
  // Wait for the client to connect
  nuSock = accept( sock, (struct sockaddr *) &remote, &remote_length );
  if ( nuSock < 0 ) {
	ERROR( (errno != ECHILD) && (errno != ERESTART) && (errno != EINTR) );
  }
  // We are connected to the client!
  printf( "> Client at %s:%i wants to talk\n", 
		  inet_ntoa( remote.sin_addr ), ntohs( remote.sin_port ) );
  
  return nuSock;
} // acceptConnection( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates a socket for the port - From ***REFERENCE***
int createSocket( unsigned short port ) {
  int sock;                 // The socket to create
  struct sockaddr_in local; // Local address

  // Create socket for incoming connections
  sock = socket( AF_INET, SOCK_STREAM, 0 );
  ERROR( sock < 0 );

  // Construct local address structure
  bzero( &local, sizeof( local ) );            // Zero out structure
  local.sin_family      = AF_INET;             // Internet address family
  local.sin_addr.s_addr = htonl( INADDR_ANY ); // Any incoming interface
  local.sin_port        = htons( port );       // Local port

  // Bind to the local address
  ERROR( bind( sock, (struct sockaddr *) &local, sizeof( local ) ) < 0 );

  // Mark the socket so it will listen for incoming connections
  ERROR( listen( sock, MAXPENDING ) < 0 );

  printf( "$ Waiting for clients\n" );

  return sock; 
} // createSocket( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] ) {
  // Convert string to lowercase
  int i;
  for (i = 0; cmd[i]; ++i ) cmd[i] = tolower( cmd[i] );

  if ( strcmp( "exit", cmd ) == 0 ) return 1;
  return 0;
} // isQuit( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to get the list from a certain connection
void getList ( Repository *r, int sock ) {
  int i;                     // Iterator
  int nbytes;                // Number of Bytes
  int n;                     // Number of files to add
  char buffer[ MAXBUFSIZE ]; // Buffer
  char msg[ MAXBUFSIZE ];    // Message to send
  
  do {
	bzero( buffer, MAXBUFSIZE );
	nbytes = read( sock, buffer, MAXBUFSIZE );
	ERROR( nbytes < 0 );
	printf( "$ '%s'\n", buffer );
  }	while( !strcmp( buffer, "" ) );

  n = atoi( buffer );
  printf( "$ Will add %i files\n",  n );

  r->cxn[r->total - 1].entries = n;

  for ( i = 0; i < n; ++i ){
	nbytes = read( sock, buffer, MAXBUFSIZE );
	ERROR( nbytes < 0 );
	memcpy( &r->cxn[r->total -1].list[i], &buffer, sizeof( FM ) );
  }

			  
} // getList( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to aid in printing process
void maxPrint( Repository *r ) {
  int i, temp;

  if ( r->total < 1 ) {
	r->max.cname = strlen( "File Owner" ); 
	r->max.addr  = strlen( "Owner IP " );
	r->max.port  = strlen( "Owner Port" );
  }
  else {
	for ( i = 0; i < r->total; ++i ) {
	  if ( ( temp = strlen( r->cxn[r->total].name ) ) > r->max.cname )
		r->max.cname = temp;
	  
	  if ( ( temp = strlen( inet_ntoa( r->cxn[r->total].sin_addr ) ) ) > r->max.addr )
		r->max.addr = temp;
	  
	  /*if ( r->cxn[r->total].sin_port > 0 ) {
		if ( ( temp = (int) log10( (double) r->cxn[r->total].sin_port ) + 1) > r->max.port )
		  r->max.port = temp;
		  }*/
	}
  }
} // maxPrint( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Take the input and see what needs to be done.
int parseCmd ( char cmd[ ] ) {
  int i;
  char command[ MAXBUFSIZE ];

  // Convert string to lowercase
  for (i = 0; cmd[i]; ++i ) cmd[i] = tolower( cmd[i] );

  // Get the command
  sscanf( cmd, "%s", command );

  if      ( strcmp( "success",  command ) == 0 ) return SUCCESS;
  else if ( strcmp( "failure",  command ) == 0 ) return FAILURE;
  else if ( strcmp( "connect",  command ) == 0 ) return CONNECT;
  else if ( strcmp( "ls",       command ) == 0 ) return LS;
  else if ( strcmp( "exit",     command ) == 0 ) return EXIT;
  else return INVALID;
} // parseCmd( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Recieve put data from the client
void put ( char msg [], char buffer [], int sock, struct sockaddr_in remote ) { 
  int nbytes;                        // number of bytes we receive in our message
  int eof;                           // Flag for end of file stream
  unsigned int remote_length;        // length of the sockaddr_in structure
  char filename[ MAXBUFSIZE ];       // Name of file
  FILE *fp;                          // Pointer to file

  remote_length = sizeof( remote );

  // Check for filename
  memcpy( filename, buffer + 4, strlen( buffer ) + 1 );
  if ( strcmp( filename, "" ) != 0 ) {
	strcat( filename, "_server" );
	sprintf( msg, "Filename: %s", filename );
	
	// Wait for incoming message
	bzero( buffer, sizeof( buffer ) );
	nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
	ERROR( nbytes < 0 );
	
	if ( strcmp( buffer, "File does not exist" ) == 0 ) sprintf( msg, "%s", buffer );
	else {
	  fp = fopen( filename, "w" );
	  ERROR( fp == NULL );
	  eof = 0;
	  while ( !eof ) {
		if ( strcmp( buffer, "Finished putting file" ) == 0 ) eof = 1;
		else {
		  fputs( buffer, fp );
		  nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
		  ERROR( nbytes < 0 );
		}
	  }
	  ERROR( fclose( fp ) );
	}
  }
  else sprintf( msg, "USAGE: put <file_name>");
} // put()
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Pretty print the catalog
void printRepo ( Repository *r ) {
  int i, j;
  int count = 0;
  char header[ MAXLINE ];
  if ( r->total < 1 ) return;

  sprintf( header, "| %*s | %*s | %*s | %*s | %*s |", 
		   r->max.fname, "File Name",
		   r->max.fsize, "File Size",
		   r->max.cname, "File Owner", 
		   r->max.addr,  "Owner IP",
		   r->max.port,  "Owner Port" );
  { ////////////////////////////////////////
	printf( "+" );

	for ( i = 1; i < r->max.fname + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.fsize + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.cname + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.addr + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.port + 3; ++i )
	  printf( "-" );

	printf( "+\n" );
  } ////////////////////////////////////////
  
  printf( "%s\n", header );
  
  { ////////////////////////////////////////
	printf( "+" );

	for ( i = 1; i < r->max.fname + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.fsize + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.cname + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.addr + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.port + 3; ++i )
	  printf( "-" );

	printf( "+\n" );
  } ////////////////////////////////////////
  
  for ( i = 0; i < r->total; ++i ) {
	if ( r->cxn[i].entries == 0 ) {
	  printf( "| %*s | %*s | %*s | %*s | %*i |\n",
			  r->max.fname, "X",
			  r->max.fsize, "X",
			  r->max.cname, r->cxn[i].name, 
			  r->max.addr,  inet_ntoa( r->cxn[i].sin_addr ), 
			  r->max.port,  r->cxn[i].sin_port );
	}
	else {
	  for ( j = 0; j < r->cxn[i].entries; ++j ) {
		printf( "| %*s | %*i | %*s | %*s | %*i |\n",
				r->max.fname, r->cxn[i].list[j].name,
				r->max.fsize, r->cxn[i].list[j].size,
				r->max.cname, r->cxn[i].name, 
				r->max.addr,  inet_ntoa( r->cxn[i].sin_addr ), 
				r->max.port,  r->cxn[i].sin_port );
		count++;
	  }
	}
  }
  
  { ////////////////////////////////////////
	printf( "+" );

	for ( i = 1; i < r->max.fname + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.fsize + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.cname + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.addr + 3; ++i )
	  printf( "-" );
	printf( "+" );

	for ( i = 1; i < r->max.port + 3; ++i )
	  printf( "-" );

	printf( "+\n" );
  } ////////////////////////////////////////
  
  printf( "Clients:\t%i\tFiles:\t%i\n", r->total, count );
} // printRepo( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Register the client
int registerClient( Repository *r, int sock ) {
  int i;                      // Iterator
  int nbytes;                 // Number of Bytes
  char buffer[ MAXBUFSIZE ];  // Buffer
  struct sockaddr_in remote;  // Local address
  unsigned int remote_length; // Length of address
  remote_length = sizeof( remote );

  bzero( buffer, MAXBUFSIZE );
  nbytes = read( sock, buffer, MAXBUFSIZE );
  //  nbytes = read( sock, buffer, MAXBUFSIZE );
  //nbytes = read( sock, buffer, MAXBUFSIZE );
  ERROR( nbytes < 0 );
  printf( "$ Checking if '%s' is already registered\n", buffer );

  for ( i = 0; i < r->total; ++i ) {
	if ( strcmp( buffer, r->cxn[i].name ) == 0 ) {
	  printf( "'%s' already exists\n", buffer );
	  nbytes = write( sock, "FAILURE", MAXBUFSIZE );
	  ERROR( nbytes < 0 );	  
	  return FAILURE;
	}
  } 
 
  ERROR( getsockname( sock, (struct sockaddr *) &remote, &remote_length ) < 0 );
  strcpy( r->cxn[r->total].name, buffer );
  r->cxn[r->total].entries         = 0;
  r->cxn[r->total].sin_addr.s_addr = remote.sin_addr.s_addr;
  r->cxn[r->total++].sin_port      = remote.sin_port;

  maxPrint( r );
  printRepo( r );
  //  getList( r, sock );
  
  nbytes = write( sock, "SUCCESS", MAXBUFSIZE );
  ERROR( nbytes < 0 );	    

  return SUCCESS;
} // registerClient( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Remove client from list
int removeClient( Repository *r, int sock ) {
  int i;                      // Iterator
  struct sockaddr_in remote;  // Local address
  unsigned int remote_length; // Length of address
  remote_length = sizeof( remote );
  
  ERROR( getsockname( sock, (struct sockaddr *) &remote, &remote_length ) < 0 );

  if ( r->total > 0 ) {
	for ( i = 0; i < r->total; ++i )
	  if ( ( r->cxn[i].sin_addr.s_addr == remote.sin_addr.s_addr ) &&
		   ( r->cxn[i].sin_port        == remote.sin_port ) )
		break;
	
	if ( i == r->total )
	return FAILURE;
	else 
	  for ( ; i < r->total - 1; ++i )
		memcpy( &r->cxn[i], &r->cxn[i+1], sizeof( r->cxn[i] ) );
	
	r->total--;
  }
  else {
	return FAILURE;
  }

  return SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////
