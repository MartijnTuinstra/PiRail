#ifndef LOCONET_INCLUDED
#define LOCONET_INCLUDED

/****************************************************************************
 * 	Copyright (C) 2009 to 2013 Alex Shepherd
 * 	Copyright (C) 2013 Damian Philipp
 * 
 * 	Portions Copyright (C) Digitrax Inc.
 * 	Portions Copyright (C) Uhlenbrock Elektronik GmbH
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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "utility/_buf.h"
#include "utility/_opc.h"
//#include "utility/utils.h"

typedef enum
{
	RN_CD_BACKOFF = 0,
	RN_PRIO_BACKOFF,
	RN_NETWORK_BUSY,
	RN_DONE,
	RN_COLLISION,
	RN_UNKNOWN_ERROR,
	RN_RETRY_ERROR
} RN_STATUS ;

// CD Backoff starts after the Stop Bit (Bit 9) and has a minimum or 20 Bit Times
// but initially starts with an additional 20 Bit Times 
#define   LN_CARRIER_TICKS      20  // carrier detect backoff - all devices have to wait this
#define   LN_MASTER_DELAY        6  // non master devices have to wait this additionally
#define   LN_INITIAL_PRIO_DELAY 20  // initial attempt adds priority delay
#define   LN_BACKOFF_MIN      (LN_CARRIER_TICKS + LN_MASTER_DELAY)      // not going below this
#define   LN_BACKOFF_INITIAL  (LN_BACKOFF_MIN + LN_INITIAL_PRIO_DELAY)  // for the first normal tx attempt
#define   LN_BACKOFF_MAX      (LN_BACKOFF_INITIAL + 10)                 // lower priority is not supported

//
// LNCV error codes
// Used by the LNCV callbacks to signal what kind of error has occurred.
//

// Error-codes for write-requests
#define LNCV_LACK_ERROR_GENERIC (0)
// Unsupported/non-existing CV
#define LNCV_LACK_ERROR_UNSUPPORTED (1)
// CV is read only
#define LNCV_LACK_ERROR_READONLY (2)
// Value out of range
#define LNCV_LACK_ERROR_OUTOFRANGE (3)
// Everything OK
#define LNCV_LACK_OK (127)

// the valid range for module addresses (CV0) as per the LNCV spec.
#define LNCV_MIN_MODULEADDR (0)
#define LNCV_MAX_MODULEADDR (65534)

class RailNetClass
{
  private:
    LnBuf   LnBuffer ;
	void 		setTxPin(uint8_t txPin);
	void 	setDuplexPin(uint8_t txPin);

  public:
    RailNetClass();
    void        init(void);
    void        init(uint8_t txPin,uint8_t duplexPin);
    boolean 		available(void);
    uint8_t			length(void);
    lnMsg*      receive(void);
    RN_STATUS   send(lnMsg *TxPacket);
    RN_STATUS   send(lnMsg *TxPacket, uint8_t PrioDelay);
    RN_STATUS   send(uint8_t OpCode, uint8_t Data1, uint8_t Data2);
    RN_STATUS   send(uint8_t OpCode, uint8_t Data1, uint8_t Data2, uint8_t PrioDelay);
    RN_STATUS   sendAck(uint8_t DevCode);
    
    LnBufStats* getStats(void);
    
	const char*	getStatusStr(RN_STATUS Status);
    
    RN_STATUS reportPower( uint8_t State ) ;
};

extern RailNetClass RailNet;

/*
class RailNetControllerClass
{
  private:
	uint16_t MyID;
	uint8_t   InputRegisters;
	uint8_t  OutputRegisters;
	uint8_t * InputRegs;
	uint8_t * OutputRegs;
	uint8_t * Blink_Mask;

	void pushInput();
	void updateOutput();

  public:
	uint8_t checkInput();
	void fireBlink();
}*/


/************************************************************************************
    SV (System Variable Handling
************************************************************************************

typedef enum
{
  SV_EE_SZ_256 = 0,
  SV_EE_SZ_512 = 1,
  SV_EE_SZ_1024 = 2,
  SV_EE_SZ_2048 = 3,
  SV_EE_SZ_4096 = 4,
  SV_EE_SZ_8192 = 5
} SV_EE_SIZE ;

typedef enum
{
  SV_WRITE_SINGLE = 0x01,
  SV_READ_SINGLE = 0x02,
  SV_WRITE_MASKED = 0x03,
  SV_WRITE_QUAD = 0x05,
  SV_READ_QUAD = 0x06,
  SV_DISCOVER = 0x07,
  SV_IDENTIFY = 0x08,
  SV_CHANGE_ADDRESS = 0x09,
  SV_RECONFIGURE = 0x0F
} SV_CMD ;

typedef enum
{
  SV_ADDR_EEPROM_SIZE = 1,
  SV_ADDR_SW_VERSION = 2,
  SV_ADDR_NODE_ID_L = 3,
  SV_ADDR_NODE_ID_H = 4,
  SV_ADDR_SERIAL_NUMBER_L = 5,
  SV_ADDR_SERIAL_NUMBER_H = 6,
  SV_ADDR_USER_BASE = 7,
} SV_ADDR ;

typedef enum
{
  SV_OK = 0,
  SV_ERROR = 1,
  SV_DEFERRED_PROCESSING_NEEDED = 2
} SV_STATUS ;

class LocoNetSystemVariableClass
{
  private:
	uint8_t 	mfgId ;
	uint8_t 	devId ;
	uint16_t 	productId ;
  uint8_t   swVersion ;
    
  uint8_t DeferredProcessingRequired ;
  uint8_t DeferredSrcAddr ;
    
	/** Checks whether the given Offset is a valid value.
	 *
	 * Returns:
	 *		True - if the given Offset is valid. False Otherwise.
	 
    uint8_t isSVStorageValid(uint16_t Offset);
	
	/** Read the NodeId (Address) for SV programming of this module.
	 *
	 * This method accesses multiple special EEPROM locations.
	 
    uint16_t readSVNodeId(void);
	
	/** Write the NodeId (Address) for SV programming of this module.
	 *
	 * This method accesses multiple special EEPROM locations.
	 
    uint16_t writeSVNodeId(uint16_t newNodeId);
	
	/**
	 * Checks whether all addresses of an address range are valid (defers to
	 * isSVStorageValid()). Sends a notification for the first invalid address
	 * (long Ack with a value of 42).
	 *
	 *	TODO: There is a Type error in this method. Return type is bool, but
	 *		actual returned values are Integer.
	 *
	 * Returns:
	 *		0 if at least one address of the range is not valid.
	 *		1 if all addresses out of the range are valid.
	 
    bool CheckAddressRange(uint16_t startAddress, uint8_t Count);

  public:
	void init(uint8_t newMfgId, uint8_t newDevId, uint16_t newProductId, uint8_t newSwVersion);
	
	/**
	 * Check whether a message is an SV programming message. If so, the message
	 * is processed.
	 * Call this message in your main loop to implement SV programming.
	 *
	 * TODO: This method should be updated to reflect whether the message has
	 *	been consumed.
	 *
	 * Note that this method will not send out replies.
	 *
	 * Returns:
	 *		SV_OK - the message was or was not an SV programming message.
				It may or may not have been consumed.
	 *		SV_DEFERRED_PROCESSING_NEEDED - the message was an SV programming
				message and has been consumed. doDeferredProcessing() must be
				called to actually process the message.
	 *		SV_ERROR - the message was an SV programming message and carried
				an unsupported OPCODE.
	 *
	 
	SV_STATUS processMessage(lnMsg *LnPacket );
	
    /** Read a value from the given EEPROM offset.
     *
     * There are two special values for the Offset parameter:
     *	SV_ADDR_EEPROM_SIZE - Return the size of the EEPROM
     *  SV_ADDR_SW_VERSION - Return the value of swVersion
     *  3 and on - Return the byte stored in the EEPROM at location (Offset - 2)
     *
     * Parameters:
     *		Offset: The offset into the EEPROM. Despite the value being passed as 2 Bytes, only the lower byte is respected.
     *
     * Returns:
     *		A Byte containing the EEPROM size, the software version or contents of the EEPROM.
     *
     
    uint8_t readSVStorage(uint16_t Offset );
    
    /** Write the given value to the given Offset in EEPROM.
     *
     * TODO: Writes to Offset 0 and 1 will cause data corruption.
     *
     * Fires notifySVChanged(Offset), if the value actually chaned.
     *
     * Returns:
     *		A Byte containing the new EEPROM value (even if unchanged).
     
    uint8_t writeSVStorage(uint16_t Offset, uint8_t Value);
    
	/**
	 * Attempts to send a reply to an SV programming message.
	 * This method will repeatedly try to send the message, until it succeeds.
	 *
	 * Returns:
	 *		SV_OK - Reply was successfully sent.
	 *		SV_DEFERRED_PROCESSING_NEEDED - Reply was not sent, a later retry is needed.
	 
    SV_STATUS doDeferredProcessing( void );
};

/************************************************************************************
    Call-back functions
************************************************************************************/

#if defined (__cplusplus)
	extern "C" {
#endif

extern void notifySensor( uint16_t Address, uint8_t State ) __attribute__ ((weak));

// Address: Switch Address.
// Output: Value 0 for Coil Off, anything else for Coil On
// Direction: Value 0 for Closed/GREEN, anything else for Thrown/RED
// state: Value 0 for no input, anything else for activated
// Sensor: Value 0 for 'Aux'/'thrown' anything else for 'switch'/'closed'
extern void notifySwitchRequest( uint16_t Address, uint8_t Output, uint8_t Direction ) __attribute__ ((weak));
extern void notifySwitchReport( uint16_t Address, uint8_t Output, uint8_t Direction ) __attribute__ ((weak));
extern void notifySwitchOutputsReport( uint16_t Address, uint8_t ClosedOutput, uint8_t ThrownOutput ) __attribute__ ((weak));
extern void notifySwitchState( uint16_t Address, uint8_t Output, uint8_t Direction ) __attribute__ ((weak));
extern void notifyPower( uint8_t State ) __attribute__ ((weak));

// Power management, Transponding and Multi-Sense Device info Call-back functions
extern void notifyMultiSenseTransponder( uint16_t Address, uint8_t Zone, uint16_t LocoAddress, uint8_t Present ) __attribute__ ((weak));
extern void notifyMultiSensePower( uint8_t BoardID, uint8_t Subdistrict, uint8_t Mode, uint8_t Direction ) __attribute__ ((weak));

// System Variable notify Call-back functions
extern void notifySVChanged(uint16_t Offset) __attribute__ ((weak));

// LNCV notify Call-back functions

// Negative return codes will result in no message being sent.
// Where a value response is appropriate, a return value of LNCV_LACK_OK will trigger the
// response being sent.
// Other values greater than 0 will result in a LACK message being sent.
// When no value result is appropriate, LNCV_LACK_OK will be sent as a LACK.

/**
 * TODO: General LNCV documentation
 * Pick an ArtNr
 * Implement your code to the following behaviour...
 */

/**
 * Notification that an LNCVDiscover message was sent. If a module wants to react to this,
 * It should return LNCV_LACK_OK and set ArtNr and ModuleAddress accordingly.
 * A response just as in the case of notifyLNCVProgrammingStart will be generated.
 * If a module responds to a LNCVDiscover, it should apparently enter programming mode immediately.
 */
extern int8_t notifyLNCVdiscover( uint16_t & ArtNr, uint16_t & ModuleAddress ) __attribute__ ((weak));;

/**
 * Notification that a LNCVProgrammingStart message was received. Application code should process this message and
 * set the return code to LNCV_LACK_OK in case this message was intended for this module (i.e., the addresses match).
 * In case ArtNr and/or ModuleAddress were Broadcast addresses, the Application Code should replace them by their
 * real values.
 * The calling code will then generate an appropriate ACK message.
 * A return code different than LACK_LNCV_OK will result in no response being sent.
 */
extern int8_t notifyLNCVprogrammingStart ( uint16_t & ArtNr, uint16_t & ModuleAddress ) __attribute__ ((weak));

/**
 * Notification that a LNCV read request message was received. Application code should process this message,
 * set the lncvValue to its respective value and set an appropriate return code.
 * return LNCV_LACK_OK leads the calling code to create a response containing lncvValue.
 * return code >= 0 leads to a NACK being sent.
 * return code < 0 will result in no reaction.
 */
extern int8_t notifyLNCVread ( uint16_t ArtNr, uint16_t lncvAddress, uint16_t, uint16_t & lncvValue ) __attribute__ ((weak));

/**
 * Notification that a LNCV value should be written. Application code should process this message and
 * set an appropriate return code.
 * Note 1: LNCV 0 is spec'd to be the ModuleAddress.
 * Note 2: Changes to LNCV 0 must be reflected IMMEDIATELY! E.g. the programmingStop command will
 * be sent using the new address.
 *
 * return codes >= 0 will result in a LACK containing the return code being sent.
 * return codes < 0 will result in no reaction.
 */
extern int8_t notifyLNCVwrite ( uint16_t ArtNr, uint16_t lncvAddress, uint16_t lncvValue ) __attribute__ ((weak));

/**
 * Notification that an LNCV Programming Stop message was received.
 * This message is noch ACKed, thus does not require a result to be returned from the application.
 */
extern void notifyLNCVprogrammingStop( uint16_t ArtNr, uint16_t ModuleAddress ) __attribute__ ((weak));

#if defined (__cplusplus)
}
#endif

#endif
