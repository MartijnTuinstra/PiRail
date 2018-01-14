/****************************************************************************
 * 	Copyright (C) 2009..2013 Alex Shepherd
 *	Copyright (C) 2013 Damian Philipp
 * 
 * 	Portions Copyright (C) Digitrax Inc.
 *	Portions Copyright (C) Uhlenbrock Elektronik GmbH
 * 
 * 	This library is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or (at your option) any later version.
 * 
 * 	This library is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * 	Lesser General Public License for more details.
 * 
 * 	You should have received a copy of the GNU Lesser General Public
 * 	License along with this library; if not, write to the Free Software
 * 	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *****************************************************************************
 * 
 * 	IMPORTANT:
 * 
 * 	Some of the message formats used in this code are Copyright Digitrax, Inc.
 * 	and are used with permission as part of the MRRwA (previously EmbeddedLocoNet) project.
 *  That permission does not extend to uses in other software products. If you wish
 * 	to use this code, algorithm or these message formats outside of
 * 	MRRwA, please contact Digitrax Inc, for specific permission.
 * 
 * 	Note: The sale any LocoNet device hardware (including bare PCB's) that
 * 	uses this or any other LocoNet software, requires testing and certification
 * 	by Digitrax Inc. and will be subject to a licensing agreement.
 * 
 * 	Please contact Digitrax Inc. for details.
 * 
 *****************************************************************************
 * 
 * 	IMPORTANT:
 * 
 * 	Some of the message formats used in this code are Copyright Uhlenbrock Elektronik GmbH
 * 	and are used with permission as part of the MRRwA (previously EmbeddedLocoNet) project.
 *  That permission does not extend to uses in other software products. If you wish
 * 	to use this code, algorithm or these message formats outside of
 * 	MRRwA, please contact Copyright Uhlenbrock Elektronik GmbH, for specific permission.
 * 
 *****************************************************************************
 * 	DESCRIPTION
 * 	This module provides functions that manage the sending and receiving of LocoNet packets.
 * 	
 * 	As bytes are received from the LocoNet, they are stored in a circular
 * 	buffer and after a valid packet has been received it can be read out.
 * 	
 * 	When packets are sent successfully, they are also appended to the Receive
 * 	circular buffer so they can be handled like they had been received from
 * 	another device.
 * 
 * 	Statistics are maintained for both the send and receiving of packets.
 * 
 * 	Any invalid packets that are received are discarded and the stats are
 * 	updated approproately.
 * 
 *****************************************************************************/

// Uncomment to enable SV Processing Debug Print statements
//#define DEBUG_SV

#include "RNet.h"
#include "utility/_sw_uart.h"
#include "utility/rn_config.h"
//#include "utility/utils.h"

#include <avr/eeprom.h>
#include <avr/wdt.h>

const char * LoconetStatusStrings[] = {
	"CD Backoff",
	"Prio Backoff",
	"Network Busy",
	"Done",
	"Collision",
	"Unknown Error",
	"Retry Error"
};

RailNetClass::RailNetClass()
{
}

void RailNetClass::init(void)
{
  init(6,5); // By default use pin 6 as the Tx pin and pin 5 as the duplex control to be compatible with the previous library default 
}

const char* RailNetClass::getStatusStr(RN_STATUS Status)
{
  if((Status >= RN_CD_BACKOFF) && (Status <= RN_RETRY_ERROR))
    return LoconetStatusStrings[Status];
	
  return "Invalid Status";
}


void RailNetClass::init(uint8_t txPin, uint8_t rs485Pin)
{
  initLnBuf(&LnBuffer) ;
  setTxPin(txPin);
  setDuplexPin(rs485Pin);
  initLocoNetHardware(&LnBuffer);
}

void RailNetClass::setTxPin(uint8_t txPin)
{
  pinMode(txPin, OUTPUT);
  
  // Not figure out which Port bit is the Tx Bit from the Arduino pin number
  uint8_t bitMask = digitalPinToBitMask(txPin);
  uint8_t bitMaskTest = 0x01;
  uint8_t bitNum = 0;
  
  uint8_t port = digitalPinToPort(txPin);
  volatile uint8_t *out = portOutputRegister(port);
  
  while(bitMask != bitMaskTest)
  bitMaskTest = 1 << ++bitNum;
  
  setTxPortAndPin(out, bitNum);
}

void RailNetClass::setDuplexPin(uint8_t duplexPin)
{
  pinMode(duplexPin, OUTPUT);
  
  // Not figure out which Port bit is the Tx Bit from the Arduino pin number
  uint8_t bitMask = digitalPinToBitMask(duplexPin);
  uint8_t bitMaskTest = 0x01;
  uint8_t bitNum = 0;
  
  uint8_t port = digitalPinToPort(duplexPin);
  volatile uint8_t *out = portOutputRegister(port);
  
  while(bitMask != bitMaskTest)
  bitMaskTest = 1 << ++bitNum;
  
  setDuplexPortAndPin(duplexPin,out, bitNum);
}

// Check to see if any messages is ready to receive()?
boolean RailNetClass::available(void)
{
  return lnPacketReady(&LnBuffer);
}

// Check the size in bytes of waiting message
uint8_t RailNetClass::length(void)
{
  if (lnPacketReady(&LnBuffer))
  {
		lnMsg* m = (lnMsg *)&(LnBuffer.Buf[ LnBuffer.ReadIndex ]);
		return getRnMsgSize(m);
  } 
  else
		return 0;
}

lnMsg* RailNetClass::receive(void)
{
  return recvLnMsg(&LnBuffer);
}

/*
  Send a LocoNet message, using the default priority delay.

  When an attempt to send fails, this method will continue to try to re-send
  the message until it is successfully sent or until the maximum retry
  limit is reached.

  Return value is one of:
    RN_DONE -         Indicates successful send of the message
    RN_RETRY_ERROR -  Could not successfully send the message within 
                        RN_TX_RETRIES_MAX attempts
    RN_UNKNOWN -      Indicates an abnormal exit condition for the send attempt.
                        In this case, it is recommended to make another send 
                        attempt.
*/
RN_STATUS RailNetClass::send(lnMsg *pPacket)
{
  return send(pPacket, LN_BACKOFF_INITIAL);
}

/*
  Send a LocoNet message, using an argument for the priority delay.

  When an attempt to send fails, this method will continue to try to re-send
  the message until it is successfully sent or until the maximum retry
  limit is reached.

  Return value is one of:
    RN_DONE -         Indicates successful send of the message
    RN_RETRY_ERROR -  Could not successfully send the message within 
                        RN_TX_RETRIES_MAX attempts
    RN_UNKNOWN -      Indicates an abnormal exit condition for the send attempt.
                        In this case, it is recommended to make another send 
                        attempt.
*/
RN_STATUS RailNetClass::send(lnMsg *pPacket, uint8_t ucPrioDelay)
{
  unsigned char ucTry;
  RN_STATUS enReturn;
  unsigned char ucWaitForEnterBackoff;

  for (ucTry = 0; ucTry < LN_TX_RETRIES_MAX; ucTry++)
  {

    // wait previous traffic and than prio delay and than try tx
    ucWaitForEnterBackoff = 1;  // don't want to abort do/while loop before
    do                          // we did not see the backoff state once
    {
      enReturn = sendLocoNetPacketTry(pPacket, ucPrioDelay);

      if (enReturn == RN_DONE)  // success?
        return RN_DONE;

      if (enReturn == RN_PRIO_BACKOFF)
        ucWaitForEnterBackoff = 0; // now entered backoff -> next state != RN_BACKOFF is worth incrementing the try counter
    }
    while ((enReturn == RN_CD_BACKOFF) ||                             // waiting CD backoff
    (enReturn == RN_PRIO_BACKOFF) ||                           // waiting master+prio backoff
    ((enReturn == RN_NETWORK_BUSY) && ucWaitForEnterBackoff)); // or within any traffic unfinished
    // failed -> next try going to higher prio = smaller prio delay
    if (ucPrioDelay > LN_BACKOFF_MIN)
      ucPrioDelay--;
    Serial.println("RNet Retry");
    delay(10);
  }
  LnBuffer.Stats.TxErrors++ ;
  return RN_RETRY_ERROR;
}

/*
  Create a four-byte LocoNet message from the three method parameters plus a 
  computed checksum, and send that message to LocoNet using a default priority 
  delay.

  When an attempt to send fails, this method will continue to try to re-send
  the message until it is successfully sent or until the maximum retry
  limit is reached.

  Return value is one of:
    RN_DONE -         Indicates successful send of the message
    RN_RETRY_ERROR -  Could not successfully send the message within 
                        RN_TX_RETRIES_MAX attempts
    RN_UNKNOWN -      Indicates an abnormal exit condition for the send attempt.
                        In this case, it is recommended to make another send 
                        attempt.
*/
RN_STATUS RailNetClass::send( uint8_t OpCode, uint8_t Data1, uint8_t Data2 )
{
  lnMsg SendPacket ;

  SendPacket.data[ 0 ] = OpCode ;
  SendPacket.data[ 1 ] = Data1 ;
  SendPacket.data[ 2 ] = Data2 ;

  return send( &SendPacket ) ;
}

/*
  Create a four-byte LocoNet message from the three method parameters plus a 
  computed checksum, and send that message to LocoNet using a parameter for the
  priority delay.

  This method will make exactly one attempt to send the LocoNet message which
  may or may not succeed.  Code which uses this method must check the return
  status value to determine if the send should be re-tried.  Such code must also
  implement a retry limit counter.

  Return value is one of:
    RN_DONE -         Indicates successful send of the message.
    RN_CD_BACKOFF -   Indicates that the message cannot be sent at this time
                        because the LocoNet state machine is in the "Carrier Detect
                        Backoff" phase.  This should not count against the "retry"
                        count.
    RN_PRIO_BACKOFF - Indicates that the message cannot be sent at this time
                        because the LocoNet state machine is in the "Priority 
                        Backoff" phase.  This should not count against the "retry" 
                        count.
    RN_NETWORK_BUSY - Indicates that the message cannot be sent at this time
                        because some other LocoNet agent is currently sending
                        a message.  This should not count against the "retry"
                        count.
    RN_COLLISSION -   Indicates that an attempt was made to send the message 
                        but that the message was corrupted by some other LocoNet
                        agent's LocoNe traffic.  The retry counter should be 
                        decremented and another send should be attempted if the 
                        retry limit has not been reached.
    RN_UNKNOWN -      Indicates an abnormal exit condition for the send attempt.
                        In this case, it is recommended to decrement the retry 
                        counter and make another send attempt if the retry limit
                        has not been reached.
*/
RN_STATUS RailNetClass::send( uint8_t OpCode, uint8_t Data1, uint8_t Data2, uint8_t PrioDelay )
{
  lnMsg SendPacket ;

  SendPacket.data[ 0 ] = OpCode ;
  SendPacket.data[ 1 ] = Data1 ;
  SendPacket.data[ 2 ] = Data2 ;

  return sendLocoNetPacketTry( &SendPacket, PrioDelay ) ;
}

/* send a LONG_ACK (LACK) message to Loconet as a response to an OPC_PEER_XFER 
   message, using the method parameter as the error code in the LONG_ACK message.

  When an attempt to send fails, this method will continue to try to re-send
  the message until it is successfully sent or until the maximum retry
  limit is reached.

  Return value is one of:
    RN_DONE -         Indicates successful send of the message.
    RN_RETRY_ERROR -  Indicates that the method could not successfully send the
                        message within RN_TX_RETRIES_MAX attempts.
    RN_UNKNOWN -      Indicates an abnormal exit condition for the send attempt.
                        In this case, it is recommended to make another send 
                        attempt.
*/
RN_STATUS RailNetClass::sendAck(uint8_t DevCode)
{
  lnMsg SendPacket ;

  SendPacket.data[ 0 ] = RN_OPC_ACK ;
  SendPacket.data[ 1 ] = DevCode;

  return send( &SendPacket ) ;
}

LnBufStats* RailNetClass::getStats(void)
{
    return &LnBuffer.Stats;
}

RN_STATUS RailNetClass::reportPower(uint8_t State)
{
  lnMsg SendPacket ;

  if(State)
    SendPacket.data[ 0 ] = RN_OPC_POWER_ON ;
  else
    SendPacket.data[ 0 ] = RN_OPC_POWER_OFF ;

  return send( &SendPacket ) ;
}

RailNetClass RailNet = RailNetClass();


/*
void LocoNetSystemVariableClass::init(uint8_t newMfgId, uint8_t newDevId, uint16_t newProductId, uint8_t newSwVersion)
{
  DeferredProcessingRequired = 0;
  DeferredSrcAddr = 0;
    
  mfgId = newMfgId ;
  devId = newDevId ;
	productId = newProductId ;
  swVersion = newSwVersion ;
}

uint8_t LocoNetSystemVariableClass::readSVStorage(uint16_t Offset )
{
	uint8_t retValue;
	
  if( Offset == SV_ADDR_EEPROM_SIZE)
#if (E2END==0x0FF)	/* E2END is defined in processor include 
								return SV_EE_SZ_256;
#elif (E2END==0x1FF)
								return SV_EE_SZ_512;
#elif (E2END==0x3FF)
								return SV_EE_SZ_1024;
#elif (E2END==0x7FF)
								return SV_EE_SZ_2048;
#elif (E2END==0xFFF)
								return SV_EE_SZ_4096;
#else
								return 0xFF;
#endif
  if( Offset == SV_ADDR_SW_VERSION )
    retValue = swVersion ;
    
  else
  {
    Offset -= 2;    // Map SV Address to EEPROM Offset - Skip SV_ADDR_EEPROM_SIZE & SV_ADDR_SW_VERSION
    retValue = eeprom_read_byte((uint8_t*)Offset);
  }
	return retValue;
}

uint8_t LocoNetSystemVariableClass::writeSVStorage(uint16_t Offset, uint8_t Value)
{
  Offset -= 2;      // Map SV Address to EEPROM Offset - Skip SV_ADDR_EEPROM_SIZE & SV_ADDR_SW_VERSION
  if( eeprom_read_byte((uint8_t*)Offset) != Value )
  {
    eeprom_write_byte((uint8_t*)Offset, Value);
    
    if(notifySVChanged)
      notifySVChanged(Offset+2);
  }    
  return eeprom_read_byte((uint8_t*)Offset) ;
}

uint8_t LocoNetSystemVariableClass::isSVStorageValid(uint16_t Offset)
{
  return (Offset >= SV_ADDR_EEPROM_SIZE ) && (Offset <= E2END + 2) ; 
}

bool LocoNetSystemVariableClass::CheckAddressRange(uint16_t startAddress, uint8_t Count)
{
#ifdef DEBUG_SV
  Serial.print("LNSV CheckAddressRange: Start: ");
  Serial.print(startAddress);
  Serial.print(" Size: ");
  Serial.println(Count);
#endif    
  while (Count != 0)
  {
    if (!isSVStorageValid(startAddress))
    {
       RailNet.sendLongAck(42); // report invalid SV address error
       return 0;
    }
    startAddress++;
    Count--;
  }
  
  return 1; // all valid
}

uint16_t LocoNetSystemVariableClass::writeSVNodeId(uint16_t newNodeId)
{
#ifdef DEBUG_SV
    Serial.print("LNSV Write Node Addr: ");
    Serial.println(newNodeId);
#endif
    
    writeSVStorage(SV_ADDR_NODE_ID_H, newNodeId >> (byte) 8);
    writeSVStorage(SV_ADDR_NODE_ID_L, newNodeId & (byte) 0x00FF);
    
    return readSVNodeId();
}

uint16_t LocoNetSystemVariableClass::readSVNodeId(void)
{
    return (readSVStorage(SV_ADDR_NODE_ID_H) << 8 ) | readSVStorage(SV_ADDR_NODE_ID_L);
}

typedef union
{
word                   w;
struct { byte lo,hi; } b;
} U16_t;

typedef union
{
struct
{
  U16_t unDestinationId;
  U16_t unMfgIdDevIdOrSvAddress;
  U16_t unproductId;
  U16_t unSerialNumber;
}    stDecoded;
byte abPlain[8];
} SV_Addr_t;

SV_STATUS LocoNetSystemVariableClass::processMessage(lnMsg *LnPacket )
{
  SV_Addr_t unData ;
   
  if( ( LnPacket->sv.mesg_size != (byte) 0x10 ) ||
      ( LnPacket->sv.command != (byte) OPC_PEER_XFER ) ||
      ( LnPacket->sv.sv_type != (byte) 0x02 ) ||
      ( LnPacket->sv.sv_cmd & (byte) 0x40 ) ||
      ( ( LnPacket->sv.svx1 & (byte) 0xF0 ) != (byte) 0x10 ) ||
      ( ( LnPacket->sv.svx2 & (byte) 0xF0 ) != (byte) 0x10 ) )
    return SV_OK ;
 
  decodePeerData( &LnPacket->px, unData.abPlain ) ;

#ifdef DEBUG_SV
    Serial.print("LNSV Src: ");
    Serial.print(LnPacket->sv.src);
    Serial.print("  Dest: ");
    Serial.print(unData.stDecoded.unDestinationId.w);
    Serial.print("  CMD: ");
    Serial.println(LnPacket->sv.sv_cmd, HEX);
#endif
  if ((LnPacket->sv.sv_cmd != SV_DISCOVER) && 
      (LnPacket->sv.sv_cmd != SV_CHANGE_ADDRESS) && 
      (unData.stDecoded.unDestinationId.w != readSVNodeId()))
  {
#ifdef DEBUG_SV
    Serial.print("LNSV Dest Not Equal: ");
    Serial.println(readSVNodeId());
#endif
    return SV_OK;
  }

  switch( LnPacket->sv.sv_cmd )
  {
    case SV_WRITE_SINGLE:
        if (!CheckAddressRange(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, 1)) return SV_ERROR;
        writeSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, unData.abPlain[4]);
        // fall through intended!
    case SV_READ_SINGLE:
        if (!CheckAddressRange(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, 1)) return SV_ERROR;
        unData.abPlain[4] = readSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w);
        break;

    case SV_WRITE_MASKED:
        if (!CheckAddressRange(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, 1)) return SV_ERROR;
        // new scope for temporary local variables only
        {
         unsigned char ucOld = readSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w) & (~unData.abPlain[5]);
         unsigned char ucNew = unData.abPlain[4] & unData.abPlain[5];
         writeSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, ucOld | ucNew);
        }
        unData.abPlain[4] = readSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w);
        break;

    case SV_WRITE_QUAD:
        if (!CheckAddressRange(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, 4)) return SV_ERROR;
        writeSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+0, unData.abPlain[4]);
        writeSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+1, unData.abPlain[5]);
        writeSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+2, unData.abPlain[6]);
        writeSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+3, unData.abPlain[7]);
        // fall through intended!
    case SV_READ_QUAD:
        if (!CheckAddressRange(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, 4)) return SV_ERROR;
        unData.abPlain[4] = readSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+0);
        unData.abPlain[5] = readSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+1);
        unData.abPlain[6] = readSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+2);
        unData.abPlain[7] = readSVStorage(unData.stDecoded.unMfgIdDevIdOrSvAddress.w+3);
        break;

    case SV_DISCOVER:
        DeferredSrcAddr = LnPacket->sv.src ;
        DeferredProcessingRequired = 1 ;
        return SV_DEFERRED_PROCESSING_NEEDED ;
        break;
    
    case SV_IDENTIFY:
        unData.stDecoded.unDestinationId.w            = readSVNodeId();
        unData.stDecoded.unMfgIdDevIdOrSvAddress.b.hi = devId;
        unData.stDecoded.unMfgIdDevIdOrSvAddress.b.lo = mfgId ;
        unData.stDecoded.unproductId.w                = productId;
        unData.stDecoded.unSerialNumber.b.lo          = readSVStorage(SV_ADDR_SERIAL_NUMBER_L);
        unData.stDecoded.unSerialNumber.b.hi          = readSVStorage(SV_ADDR_SERIAL_NUMBER_H);
        break;

    case SV_CHANGE_ADDRESS:
        if((mfgId != unData.stDecoded.unMfgIdDevIdOrSvAddress.b.lo) || (devId != unData.stDecoded.unMfgIdDevIdOrSvAddress.b.hi))
          return SV_OK; // not addressed
        if(productId != unData.stDecoded.unproductId.w)
          return SV_OK; // not addressed
        if(readSVStorage(SV_ADDR_SERIAL_NUMBER_L) != unData.stDecoded.unSerialNumber.b.lo)
          return SV_OK; // not addressed
        if(readSVStorage(SV_ADDR_SERIAL_NUMBER_H) != unData.stDecoded.unSerialNumber.b.hi)
          return SV_OK; // not addressed
          
        if (writeSVNodeId(unData.stDecoded.unDestinationId.w) != unData.stDecoded.unDestinationId.w)
        {
          RailNet.sendLongAck(44);  // failed to change address in non-volatile memory (not implemented or failed to write)
          return SV_OK ; // the LN reception was ok, we processed the message
        }
        break;

    case SV_RECONFIGURE:
        break;  // actual handling is done after sending out the reply

    default:
        RailNet.sendLongAck(43); // not yet implemented
        return SV_ERROR;
  }
    
  encodePeerData( &LnPacket->px, unData.abPlain ); // recycling the received packet
    
  LnPacket->sv.sv_cmd |= 0x40;    // flag the message as reply
  
  RN_STATUS lnStatus = RailNet.send(LnPacket, LN_BACKOFF_INITIAL);
	
#ifdef DEBUG_SV
  Serial.print("LNSV Send Response - Status: ");
  Serial.println(lnStatus);   // report status value from send attempt
#endif

  if (lnStatus != RN_DONE) {
    // failed to send the SV reply message.  Send will NOT be re-tried.
    RailNet.sendLongAck(44);  // indicate failure to send the reply
  }
    
  if (LnPacket->sv.sv_cmd == (SV_RECONFIGURE | 0x40))
  {
    wdt_enable(WDTO_15MS);  // prepare for reset
    while (1) {}            // stop and wait for watchdog to knock us out
  }
   
  return SV_OK;
}

SV_STATUS LocoNetSystemVariableClass::doDeferredProcessing( void )
{
  if( DeferredProcessingRequired )
  {
    lnMsg msg ;
    SV_Addr_t unData ;
    
    msg.sv.command = (byte) OPC_PEER_XFER ;
    msg.sv.mesg_size = (byte) 0x10 ;
    msg.sv.src = DeferredSrcAddr ;
    msg.sv.sv_cmd = SV_DISCOVER | (byte) 0x40 ;
    msg.sv.sv_type = (byte) 0x02 ; 
    msg.sv.svx1 = (byte) 0x10 ;
    msg.sv.svx2 = (byte) 0x10 ;
    
    unData.stDecoded.unDestinationId.w            = readSVNodeId();
    unData.stDecoded.unMfgIdDevIdOrSvAddress.b.lo = mfgId;
    unData.stDecoded.unMfgIdDevIdOrSvAddress.b.hi = devId;
    unData.stDecoded.unproductId.w                = productId;
    unData.stDecoded.unSerialNumber.b.lo          = readSVStorage(SV_ADDR_SERIAL_NUMBER_L);
    unData.stDecoded.unSerialNumber.b.hi          = readSVStorage(SV_ADDR_SERIAL_NUMBER_H);
    
    encodePeerData( &msg.px, unData.abPlain );

    /* Note that this operation intentionally uses a "make one attempt to
       send to LocoNet" method here 
    if( sendLocoNetPacketTry( &msg, LN_BACKOFF_INITIAL + ( unData.stDecoded.unSerialNumber.b.lo % (byte) 10 ) ) != RN_DONE )
      return SV_DEFERRED_PROCESSING_NEEDED ;

    DeferredProcessingRequired = 0 ;
  }

  return SV_OK ;
}

 /*****************************************************************************
 *	DESCRIPTION
 *	This module provides functions that manage the LNCV-specifiv programming protocol
 * 
 *****************************************************************************/

// Adresses for the 'SRC' part of an UhlenbrockMsg
#define LNCV_SRC_MASTER 0x00
#define LNCV_SRC_KPU 0x01
// KPU is, e.g., an IntelliBox
// 0x02 has no associated meaning
#define LNCV_SRC_TWINBOX_FRED 0x03
#define LNCV_SRC_IBSWITCH 0x04
#define LNCV_SRC_MODULE 0x05

// Adresses for the 'DSTL'/'DSTH' part of an UhlenbrockMsg
#define LNCV_BROADCAST_DSTL 0x00
#define LNCV_BROADCAST_DSTH 0x00
#define LNCV_INTELLIBOX_SPU_DSTL 'I'
#define LNCV_INTELLIBOX_SPU_DSTH 'B'
#define LNCV_INTELLIBOX_KPU_DSTL 'I'
#define LNCV_INTELLIBOX_KPU_DSTH 'K'
#define LNCV_TWINBOX_DSTH 'T'
// For TwinBox, DSTL can be anything from 0 to 15
#define LNCV_IBSWITCH_KPU_DSTL 'I'
#define LNCV_IBSWITCH_KPU_DSTH 'S'
#define LNCV_MODULE_DSTL 0x05
#define LNCV_MODULE_DSTH 0x00

// Request IDs
#define LNCV_REQID_CFGREAD 31
#define LNCV_REQID_CFGWRITE 32
#define LNCV_REQID_CFGREQUEST 33

// Flags for the 7th data Byte
#define LNCV_FLAG_PRON 0x80
#define LNCV_FLAG_PROFF 0x40
#define LNCV_FLAG_RO 0x01
// other flags are currently unused

//#define DEBUG_OUTPUT
#undef DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
//#define DEBUG(x) Serial.print(F(x))
#define DEBUG(x) Serial.print(x)
#else
#define DEBUG(x)
#endif

#ifdef DEBUG_OUTPUT
void printPacket(lnMsg* LnPacket) {
  Serial.print("LoconetPacket ");
  Serial.print(LnPacket->ub.command, HEX);
  Serial.print(" ");
  Serial.print(LnPacket->ub.mesg_size, HEX);
  Serial.print(" ");
  Serial.print(LnPacket->ub.SRC, HEX);
  Serial.print(" ");
  Serial.print(LnPacket->ub.DSTL, HEX);
  Serial.print(" ");
  Serial.print(LnPacket->ub.DSTH, HEX);
  Serial.print(" ");
  Serial.print(LnPacket->ub.ReqId, HEX);
  Serial.print(" ");
  Serial.print(LnPacket->ub.PXCT1, HEX);
  for (int i(0); i < 7; ++i) {
    Serial.print(" ");
    Serial.print(LnPacket->ub.payload.D[i], HEX);
  }
  Serial.print("\n");
}
#endif
/*
uint8_t LocoNetCVClass::processLNCVMessage(lnMsg * LnPacket) {
	uint8_t ConsumedFlag(0);

	switch (LnPacket->sr.command) {
	case OPC_IMM_PACKET:
	case OPC_PEER_XFER:
		Serial.println("Possibly a LNCV message.");
		// Either of these message types may be a LNCV message
		// Sanity check: Message length, Verify addresses
		if (LnPacket->ub.mesg_size == 15 && LnPacket->ub.DSTL == LNCV_MODULE_DSTL && LnPacket->ub.DSTH == LNCV_MODULE_DSTH) {
			// It is a LNCV programming message
			computeBytesFromPXCT(LnPacket->ub);
			#ifdef DEBUG_OUTPUT
			Serial.print("Message bytes: ");
			Serial.print(LnPacket->ub.ReqId);
			Serial.write(" ");
			Serial.print(LnPacket->ub.payload.data.deviceClass, HEX);
			Serial.write(" ");
			Serial.print(LnPacket->ub.payload.data.lncvNumber, HEX);
			Serial.write(" ");
			Serial.print(LnPacket->ub.payload.data.lncvValue, HEX);
			Serial.write("\n");
			#endif

			lnMsg response;

			switch (LnPacket->ub.ReqId) {
			case LNCV_REQID_CFGREQUEST:
				if (LnPacket->ub.payload.data.deviceClass == 0xFFFF && LnPacket->ub.payload.data.lncvNumber == 0x0000 && LnPacket->ub.payload.data.lncvValue == 0xFFFF) {
					// This is a discover message
					DEBUG("LNCV discover: ");
					if (notifyLNCVdiscover) {
						DEBUG(" executing...");
						if (notifyLNCVdiscover(LnPacket->ub.payload.data.deviceClass, LnPacket->ub.payload.data.lncvValue) == LNCV_LACK_OK) {
							makeLNCVresponse(response.ub, LnPacket->ub.SRC, LnPacket->ub.payload.data.deviceClass, 0x00, LnPacket->ub.payload.data.lncvValue, 0x00);
							RailNet.send(&response);
						}
					}
					#ifdef DEBUG_OUTPUT
					else {DEBUG(" NOT EXECUTING!");}
					#endif
				} else if (LnPacket->ub.payload.data.flags == 0x00) {
					// This can only be a read message
					DEBUG("LNCV read: ");
					if (notifyLNCVread) {
						DEBUG(" executing...");
						int8_t returnCode(notifyLNCVread(LnPacket->ub.payload.data.deviceClass, LnPacket->ub.payload.data.lncvNumber, LnPacket->ub.payload.data.lncvValue, LnPacket->ub.payload.data.lncvValue));
						if (returnCode == LNCV_LACK_OK) {
							// return the read value
							makeLNCVresponse(response.ub, LnPacket->ub.SRC, LnPacket->ub.payload.data.deviceClass, LnPacket->ub.payload.data.lncvNumber, LnPacket->ub.payload.data.lncvValue, 0x00); // TODO: D7 was 0x80 here, but spec says that it is unused.
							RailNet.send(&response);	
							ConsumedFlag = 1;
						} else if (returnCode >= 0) {
							uint8_t old_opcode(0x7F & LnPacket->ub.command);
							RailNet.send(OPC_LONG_ACK, old_opcode, returnCode);
							// return a nack
							ConsumedFlag = 1;
						}
					}
					#ifdef DEBUG_OUTPUT
					else {DEBUG(" NOT EXECUTING!");}
					#endif
				} else {
					// Its a "control" message
					DEBUG("LNCV control: ");
					if ((LnPacket->ub.payload.data.flags & LNCV_FLAG_PRON) != 0x00 && ((LnPacket->ub.payload.data.flags & LNCV_FLAG_PROFF) != 0x00)) {
						DEBUG("Illegal, ignoring.");
						// Illegal message, no action.
					} else if ((LnPacket->ub.payload.data.flags & LNCV_FLAG_PRON) != 0x00) {
						DEBUG("Programming Start, ");
						// LNCV PROGAMMING START
						// We'll skip the check whether D[2]/D[3] are 0x0000.
						if (notifyLNCVprogrammingStart) {
							DEBUG(" executing...");
							if (notifyLNCVprogrammingStart(LnPacket->ub.payload.data.deviceClass, LnPacket->ub.payload.data.lncvValue) == LNCV_LACK_OK) {
								DEBUG("LNCV_LACK_OK ");
								DEBUG(LnPacket->ub.payload.data.deviceClass);
								DEBUG(" ");
								DEBUG(LnPacket->ub.payload.data.lncvValue);
								DEBUG("\n");
								makeLNCVresponse(response.ub, LnPacket->ub.SRC, LnPacket->ub.payload.data.deviceClass, 0x00, LnPacket->ub.payload.data.lncvValue, 0x80);
								delay(10); // for whatever reason, we need to delay, otherwise the message will not be sent.
								#ifdef DEBUG_OUTPUT
								printPacket((lnMsg*)&response);
								#endif
								RailNet.send((lnMsg*)&response);	
								#ifdef DEBUG_OUTPUT
								RN_STATUS status = RailNet.send((lnMsg*)&response);	
								Serial.print(F("Return Code from Send: "));
								Serial.print(status, HEX);
								Serial.print("\n");
								#endif
								ConsumedFlag = 1;
							} // not for us? then no reaction!
							#ifdef DEBUG_OUTPUT
							else {DEBUG("Ignoring.\n");}
							#endif
						} 
						#ifdef DEBUG_OUTPUT
						else {DEBUG(" NOT EXECUTING!");}
						#endif
							
					}
					if ((LnPacket->ub.payload.data.flags & LNCV_FLAG_PROFF) != 0x00) {
						// LNCV PROGRAMMING END
						if (notifyLNCVprogrammingStop) {
							notifyLNCVprogrammingStop(LnPacket->ub.payload.data.deviceClass, LnPacket->ub.payload.data.lncvValue);
							ConsumedFlag = 1;
						}
					}
					// Read-Only mode not implmeneted.
				}

			break;
			case LNCV_REQID_CFGWRITE:
				if (notifyLNCVwrite) {
					// Negative return code indicates that we are not interested in this message.
					int8_t returnCode(notifyLNCVwrite(LnPacket->ub.payload.data.deviceClass, LnPacket->ub.payload.data.lncvNumber, LnPacket->ub.payload.data.lncvValue));
					if (returnCode >= 0) {
						ConsumedFlag = 1;
						uint8_t old_opcode(0x7F & LnPacket->ub.command);
						RailNet.send(OPC_LONG_ACK, old_opcode, returnCode);
					}
				}
			break;

			}

		}
	break;
#ifdef DEBUG_OUTPUT
	default:
		Serial.println("Not a LNCV message."); 
#endif
	}

	return ConsumedFlag;
}

uint16_t LocoNetCVClass::getAddress(uint8_t lower, uint8_t higher) {
	uint16_t result(higher);
	result <<= 8;
	result |= lower;
	return result;
}
*/

