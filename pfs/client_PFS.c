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
#include <math.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAXBUFSIZE 256
#define MAXLINE    256
#define MAXFILES   512
#define ERROR( boolean ) if ( boolean ) {								\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );	\
    exit ( EXIT_FAILURE );												\
  }

enum COMMAND { SUCCESS, FAILURE, PUT, GET, LS, EXIT, INVALID };

typedef struct {
  char name[ MAXBUFSIZE ]; // Name of client
  int total;               // Total number of files
  struct {
	char name[ MAXLINE ];  // The name of the file
	int size;              // The size of the file (in B)
  } list[ MAXFILES ];
} Folder;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates a socket for the ip address and port - From ***REFERENCE***
int createSocket( unsigned long ip, unsigned short port );

// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );

// List all the files this client has.
void ls ( Folder *dir ); 

// Take the input and see what needs to be done.
int parseCmd ( char cmd[ ] );

// Pretty print the catalog
void printCatalog ( Folder *dir );

// Function to handle put command
void put ( char cmd [ ], int sock, struct sockaddr_in remote );

// Send my list of files
void sendList( Folder *dir, int sock );

// Register myself with the server
int registerClient( Folder *dir, int sock );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  //
  // Variables
  // 
  int nbytes;                   // Number of bytes
  int sock;                     // This will be our socket
  char buffer[ MAXBUFSIZE ];    // Recieve data from Server
  //  char cmd[ MAXBUFSIZE ];       // Command to be sent to Server
  //char temp[ MAXBUFSIZE ];      // Temporary string holder
  //char *newline = NULL;         // Get newline
  //struct sockaddr_in remote;    // "Internet socket address structure"
  //struct sockaddr_in from_addr; // Socket for server
  //unsigned int addr_length = sizeof( struct sockaddr );
  Folder dir;

  // Make sure name, ip and port are given
  if ( argc < 4 ) {
	printf("USAGE:  <client_name> <server_ip> <server_port>\n");
	exit ( EXIT_FAILURE );
  }

  strcpy( dir.name, argv[1] );
  
  // 
  // Set up the socket 
  //
  sock = createSocket( inet_addr( argv[2] ), atoi( argv[3] ) );
  if ( registerClient( &dir, sock ) == FAILURE ) {
	close( sock );
	return EXIT_SUCCESS;
  }

  bzero( buffer, MAXBUFSIZE );
  nbytes = recv( sock, buffer, MAXBUFSIZE, MSG_WAITALL );
  ERROR( nbytes < 0 );
  printf( "%s\n", buffer );

  /*
  //
  // Enter command loop
  //
  do {
	*cmd = '\0';
	//bzero( cmd, sizeof( cmd ) );
	printf( "> " );
	
	//
	// Grab Command Line
	//
	if ( fgets( cmd, MAXBUFSIZE, stdin ) != NULL ) {
	  newline = strchr( cmd, '\n'); // Find the newline
	  if ( newline != NULL ) *newline = '\0'; // Overwrite
	  
	  //
	  // sendto() sends immediately.  
	  //    it will report an error if the message fails to leave the 
      //    computer.Hhowever, with UDP, there is no error if the message 
      //    is lost in the network once it leaves the computer.
	  nbytes = sendto( sock, cmd, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  
	  //
	  // Put Command
	  //
	  switch ( parseCmd ( cmd ) ) {
	  case PUT:	put ( cmd, sock, remote ); break;
	  case GET: get ( cmd, sock, remote ); break;
	  }
		
	  // Blocks till bytes are received
	  bzero( buffer, sizeof( buffer ) );
	  nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &from_addr, &addr_length);  
	  ERROR ( nbytes < 0 );
	  
	  printf( "%s\n", buffer );
	}
	else if ( strcmp( "get", temp ) == 0 ) {

	}
  } while ( !isQuit( cmd ) );  
  */
  close( sock );
  
  return EXIT_SUCCESS;
} // main( )
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
  printf( "Connected to remote\n" );

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
  else if ( strcmp( "put",      command ) == 0 ) return PUT;
  else if ( strcmp( "get",      command ) == 0 ) return GET;
  else if ( strcmp( "ls",       command ) == 0 ) return LS;
  else if ( strcmp( "exit",     command ) == 0 ) return EXIT;
  else return INVALID;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Pretty print the catalog
void printCatalog ( Folder *dir ) {
  int i, temp, maxName, maxSize;
  char header[ MAXLINE ];
  if ( dir->total < 1 ) return;

  maxName = strlen( dir->list[0].name );
  if ( dir->list[0].size > 0 ) maxSize = (int) log10( (double) dir->list[0].size ) + 1;
  else                         maxSize = 1;

  for ( i = 1; i < dir->total; ++i ) {
	if ( ( temp = strlen( dir->list[i].name ) ) > maxName )
	  maxName = temp;

	if ( dir->list[i].size > 0 )
	  if ( ( temp = (int) log10( (double) dir->list[i].size ) + 1 ) > maxSize )
		maxSize = temp;
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
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to handle put command
void put ( char cmd [ ], int sock, struct sockaddr_in remote ) { 
  int nbytes;                   // Number of bytes send by sendto()
  char buffer[ MAXBUFSIZE ];    // Recieve data from Server
  char filename[ MAXBUFSIZE ];  // Name of file
  FILE *fp;                     // Pointer to file
  
  // Check for filename
  memcpy( filename, cmd + 4, strlen( cmd ) + 1 );
  // Make sure filename isn't null
  if ( strcmp( filename, "" ) != 0 ) {
	fp = fopen( filename, "r" );
	// See if file exists
	if ( fp == NULL ) {
	  // If file is MIA, print message to buffer
	  if ( errno == ENOENT ) {
		sprintf( buffer, "File does not exist" );
		// Send buffer
		nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
		ERROR ( nbytes < 0 );
	  }
	  else ERROR ( fp == NULL );
	} // fp == NULL
	else {
	  *buffer = '\0';
	  // Else read contents of file into buffer
	  while ( fgets( buffer, MAXBUFSIZE, fp ) != NULL ) {
		printf( "%s", buffer );
		// Send one line from file
		nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
		ERROR ( nbytes < 0 );
	  }
	  // Tell server we're done
	  sprintf( buffer, "Finished putting file" );
	  // Send buffer
	  nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  ERROR ( fclose( fp ) );
	}
  } // filename != ""
} // put( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Send my list of files
void sendList( Folder *dir, int sock ) {
  int nbytes;             // Number of bytes
  char msg[ MAXBUFSIZE ]; // Message to send

  ls( dir );
  printCatalog( dir );
  
  sprintf( msg, "%i", dir->total );
  printf( "Will forward %s files\n", msg );
  nbytes = send( sock, msg, MAXBUFSIZE, 0 );
  ERROR( nbytes < 0 );
  
} // sendList( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Register myself with the server
int registerClient( Folder *dir, int sock ) {
  int cmd;                   // The command
  int nbytes;                // Number of bytes
  char buffer[ MAXBUFSIZE ]; // Buffer

  nbytes = send( sock, dir->name, MAXBUFSIZE, 0 );
  ERROR( nbytes < 0 );

  bzero( buffer, MAXBUFSIZE );
  nbytes = recv( sock, buffer, MAXBUFSIZE, MSG_WAITALL );
  printf( "%s\n", buffer );

  if ( ( cmd = parseCmd( buffer ) ) == FAILURE ) {
	printf( "There is already a user with the name '%s' on the server.\n", 
			dir->name );
  }
  else if ( cmd == GET ) {
	sendList( dir, sock );
  }

  return cmd;
} // registerClient( )
////////////////////////////////////////////////////////////////////////////////
