////////////////////////////////////////////////////////////////////////////////
// File:    athena.h                    Fall 2013
// Student: Patrick Vargas              patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Definition
//   I thought it would be wise to make a shared header files that 
//   included shared functions and defintions.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#ifndef ATHENA
#define ATHENA

#define MAXBUFSIZE 512
#define MAXFILES   256
#define MAXLINE    256
#define MAXPENDING 5
#define MAXCLIENTS 5

#define ERROR( boolean ) if ( boolean ) {\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
    exit ( EXIT_FAILURE );\
  }
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
enum COMMAND { SUCCESS, FAILURE, CONNECT, GET, LS, EXIT, INVALID };

typedef struct {
  int size;              // The size of the file (in B)
  char name[ MAXLINE ];  // The name of the file
} FM;                    // FileMeta

typedef struct {
  char name[ MAXBUFSIZE ]; // Name of client
  int total;               // Total number of files
  FM list[ MAXFILES ];
} Folder;

typedef struct {
  int total;
  struct {
	  int entries;             // Number of Files
	  struct in_addr sin_addr; // Address of owner
	  unsigned short sin_port; // Port of owner 
	  char name[ MAXLINE ];      // File owner
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
// Function to aid in printing process
void maxPrint( Repository *r ) {
  int i, temp;

  if ( r->total < 1 ) {
	r->max.fname = strlen( "File Name"  );
	r->max.fsize = strlen( "File Size"  );
	r->max.cname = strlen( "File Owner" ); 
	r->max.addr  = strlen( "Owner IP "  );
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
  else if ( strcmp( "get",      command ) == 0 ) return GET;  
  else if ( strcmp( "ls",       command ) == 0 ) return LS;
  else if ( strcmp( "exit",     command ) == 0 ) return EXIT;
  else return INVALID;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Pretty print the catalog
void printRepo ( Repository *r ) {
  int i, j;
  int count = 0;
  char header[ MAXBUFSIZE ];
  if ( r->total < 1 ) return;

  maxPrint( r );

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
#endif
////////////////////////////////////////////////////////////////////////////////
