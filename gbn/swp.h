0////////////////////////////////////////////////////////////////////////////////
// File: swp.h                          Fall 2013
// Students: 
//   Brittney Harsha                    Patrick Vargas
//   b.grace.harsha@gmail.com           patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Descriptoin:
//   This file contains a few functions and data structures needed for a sliding
//   window protocol program.
//
// JK GUYS,
//   to run the code from the book we need the x-kernel installed, which we can't
//   on the csel machines.
//
//   More info: www.cs.arizona.edu/projects/xkernel/
//
////////////////////////////////////////////////////////////////////////////////

window = 0
  while(c!=EOF)
    while( window%sws < sws)
	  do ... while  //build sws distinct packets with headers
  }
     i = window - sws
     while(i<window)
	   sendto_()    //for each distinct packet, can use i
          i++
		 }
then we get into the ack stuff, which I am still working on so I will probably send you one or several more emails tonight if I can figure out the semaphore/select() business further.

////////////////////////////////////////////////////////////////////////////////
#ifndef SWP_H
#define SWP_H

//#include <semaphore.h>

#define PACKETSIZE 1024
typedef char[PACKETSIZE] Msg;
typedef int Semaphore;

typedef int bool;
#define TRUE 1
#define FALSE 0

// Sliding Window Protocol Metadata
//   (pp. 111-112, L. Peterson & B. Davie. (2012). Computer Networks, 5 ed.)
#define SWS 6
#define RWS 6
#define HLEN 8

typedef u_char SwpSeqno;
typedef struct {
  SwpSeqno SeqNum; // Sequence number of this frame
  SwpSeqno AckNum; //Ack of recieved frame
  u_char   Flags;  // Up to 8 bits worth of flags
} SwpHdr;

typedef struct {
  //
  // Sender Side State
  //
  SwpSeqno LAR;    // Sequence number of last ACK recieved
  SwpSeqno LFT;    // Last frame sent
  Semaphore sendWindowNotFull;
  SwpHdr hdr;      // Pre-initialized header
  struct sendQ_slot {
	Event timeout; // Event associated with send-timeout
	Msg   msg;
  } sendQ[SWS];

  //
  // Receiver Side State
  //
  SwpSeqno NFE;    // Sequence number of next frame expected
  struct recvQ_slot {
	int received; // Is msg valid?
	Msg msg;
  } recvQ[RWS];
} SwpState;

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to recieve
//   (pp. 115-116, L. Peterson & B. Davie. (2012). Computer Networks, 5 ed.)
static int deliverSWP( SwpState state, Msg *frame ) {
  SwpHdr hdr;
  char   *hbuf;

  hbuf = msgStripHdr( frame, HLEN );
  load_swp_hdr( &hdr, hbuf );
  if ( hdr->Flags & FLAG_ACK_VALID ) {
	//
	// Received an acknowledgment -- do SENDER side
	//
	if ( swpInWindow( hdr.AckNum, state->LAR + 1, state->LFS ) ) {
	  do {
		struct sendQ_slot *slot;
		
		slot = &state->sendQ[++state->LAR % SWS];
		evCancel( slot->timeout );
		msgDestroy( &slot->msg );
		semSignal( &state->sendWindowNotFull );
	  } while ( state->LAR != hdr.AckNum );
	}
  }
  
  if ( hdr.Flags & FLAG_HAS_DATA ) {
	struct recv!_slot *slot;
	//
	// Recieved data packed -- do RECEIVER side
	//
	slot = &state->recvQ[hdr.SeqNum % RWS];
	if ( !swpInWindow( hdr.SeqNum, state->NFE, state->NFE + RWS - 1 ) ) {
	  // Drop the message
	  return SUCCESS;
	}

	msgSaveCopy( &slot->msg, frame );
	slot->received = TRUE;
	if ( hdr.SeqNum == state->NFE ) {
	  Msg m;
	  
	  while ( slot->recieved ) {
		deliver( HLP, &slot->msg );
		msgDestroy( &slot->msg );
		slot->received = FALSE;
		slot = &state->recvQ[++state->NFE % RWS];
	  }

	  // Send ACK
	  prepare_ack( &m, state->NFE - 1 );
	  send( LINK, &m );
	  msgDestroy( &m );
	}
  }

  return SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Sending side of Sliding Window Protocol
//   (pp. 113, L. Peterson & B. Davie. (2012). Computer Networks, 5 ed.)
static int sendSWP( SwpState *state, Msg *frame ) {
  struct sendQ_slot *slot;
  hbuf[HLEN];

  // Wait for send window to open
  semWait( &state->sendWindowNotFull );
  stat->hedr.SeqNum = ++stte->LFS;
  slot = &state->sendQ[state->hdr.SeqNum % SWS];
  store_swp_hdr( state->hdr, hbuf );
  msgAddHdr( frame, hbuf, HLEN );
  msgSaveCopy( &slot->msg, frame );
  slot->timeout = evSchedule( swpTimeout, slot, SWP_SEND_TIMEOUT );
  return send( LINK, frame );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Make sure sequence number falls within window
//   (pp. 116-117, L. Peterson & B. Davie. (2012). Computer Networks, 5 ed.)
static bool swpInWindow( SwpSeqno seqno, SwpSeqno min, SwpSeqno max ) {
  SwpSeqno pos, maxpos;
  pos    = seqno - min;   // should be in rance [0..Max)
  maxpos = max - min + 1; // is in range [0..Max]
  return pos < maxpos;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#endif // SWP_H
////////////////////////////////////////////////////////////////////////////////
