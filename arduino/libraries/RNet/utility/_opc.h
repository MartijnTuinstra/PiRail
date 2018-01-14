#ifndef RN_OPC_INCLUDED
#define RN_OPC_INCLUDED
/*****************************************************************************
 *                                                                           *
 *      Copyright (C) 2001 Ron W. Auld                                       *
 *      Copyright (C) 2004 Alex Shepherd                                                                      *
 *                                                                           *
 *      Portions Copyright (C) Digitrax Inc.                                 * 
 *                                                                           *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU Lesser General Public               *
 *  License as published by the Free Software Foundation; either             *
 *  version 2.1 of the License, or (at your option) any later version.       *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Lesser General Public License for more details.                          *
 *                                                                           *
 *  You should have received a copy of the GNU Lesser General Public         *
 *  License along with this library; if not, write to the Free Software      *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*
 *                                                                           *
 *****************************************************************************
 *
 *  IMPORTANT:
 *
 *  Some of the message formats used in this code are Copyright Digitrax, Inc.
 *  and are used with permission as part of the EmbeddedLocoNet project. That
 *  permission does not extend to uses in other software products. If you wish
 *  to use this code, algorithm or these message formats outside of
 *  EmbeddedLocoNet, please contact Digitrax Inc, for specific permission.
 *
 *  Note: The sale any LocoNet device hardware (including bare PCB's) that
 *  uses this or any other LocoNet software, requires testing and certification
 *  by Digitrax Inc. and will be subject to a licensing agreement.
 *
 *  Please contact Digitrax Inc. for details.
 *
 *****************************************************************************
 *  NOTE                                                                     *
 *                                                                           *
 *  This file was first released under the GPL license but Ron Auld has      *
 *  given his permission for this file to be included in the EmbeddedLoconet *
 *  project under the LGPL license                                           *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *  Some of this code is based on work done by John Kabat and I thank him    *
 *  for the use of that code and his contributions to the understanding      *
 *  and use of the Digitrax LocoNet.                                         *
 *                                                                           *
 *****************************************************************************
 *
 * File Name:     loconet.h
 * Module:        Generic Loconet (r) message definitions.
 * Language:
 * Contributors:  Ron W. Auld (RwA), John Kabat
 * Remarks:
 *
 *    This file contains the definitions and structures pertinant to the
 *    Loconet (r) message protocol.
 *
 * Version Control:
 * $Log: loconet.h,v $
 * Plocher: changed byte to uint8_t
 *
 */

#include <stdint.h>

#if defined (__cplusplus)
	extern "C" {
#endif

#define RN_OPC_REPORT_ID     0x00
#define RN_OPC_EMERGENCY_SET 0x01
#define RN_OPC_EMERGENCY_REL 0x02
#define RN_OPC_POWER_ON      0x03
#define RN_OPC_POWER_OFF     0x04
#define RN_OPC_ACK 0x7F

#define RN_OPC_T_S_OUT     0x10
#define RN_OPC_P_S_OUT     0x11
#define RN_OPC_TBS_OUT     0x12
#define RN_OPC_T_M_OUT     0x13
#define RN_OPC_S_ALL_OUT   0x14
#define RN_OPC_S_BL_MASK   0x15
#define RN_OPC_POST_OUT    0x16
#define RN_OPC_REQUEST_OUT 0x47

#define RN_OPC_S_IN       0x05
#define RN_OPC_M_IN       0x06
#define RN_OPC_A_IN       0x07
#define RN_OPC_POST_IN    0x08
#define RN_OPC_REQUEST_IN 0x4C

#define RN_OPC_DEVID 0x50
#define RN_OPC_IN_OUT_REGS 0x51
#define RN_OPC_BLINK_INTERVAL 0x52
#define RN_OPC_PULSE_LENGTH   0x53
#define RN_OPC_INPUT_INTERVAL 0x54
#define RN_OPC_POST_EEPROM    0x55
#define RN_OPC_REQUEST_EEPROM 0x59


#if defined (__cplusplus)
}
#endif

#endif
