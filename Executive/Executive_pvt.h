/*
 * executive_pvt.h
 *
 *  Created on: May 1, 2018
 *      Author: Lenny
 *
 *      WARNING: This is a private header and should not be included outside of
 *      the executive module.
 */

#ifndef EXECUTIVE_EXECUTIVE_PVT_H_
#define EXECUTIVE_EXECUTIVE_PVT_H_


/*------------------------------------------------------------------------------
--|
--| Includes
--|
------------------------------------------------------------------------------*/
// All submodules
#include "SystemManager.h"
#include "GarageSensor.h"
#include "MQTT.h"

/*------------------------------------------------------------------------------
--|
--| Defines
--|
------------------------------------------------------------------------------*/
#define HEARTBEAT_LED D2


/*------------------------------------------------------------------------------
--|
--| Types
--|
------------------------------------------------------------------------------*/
// All sub modules will have this type of manage function called by this exec
typedef void (*manageFunct)(void);

// Contains the funct to call and the min rate at which to call it
typedef struct {
		manageFunct functPtr;
		unsigned rateCalledInMS;
	} foregroundFunct_StructType;

/*------------------------------------------------------------------------------
--|
--| Constants
--|
------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * ExecInitTable is an ordered table of all of the software
 * initialization functions which must be called prior to the Foreground Frame
 *--------------------------------------------------------------------------*/
static const manageFunct ExecInitTable [] =
{
		InitSystemManager,
		InitGarageSensor,
		InitMQTTCom,
};

/*----------------------------------------------------------------------------
 * ExecFrameTable is an ordered table of all of the software
 * managers which must be called, and how often (in MS) during the primary
 * software Frame.
 * NOTE: resolution is limited by the MS_PER_FRAME define
 *--------------------------------------------------------------------------*/
static const foregroundFunct_StructType ExecFrameTable [] =
{
		{ManageGarageSensor, 100},
		{ManageMQTTCom, 100},
		{ManageSystemManager, 300},

};


/*-----Exec's Output Init-----*/

static const ExecOutput_structType execOutputInit =
{
	0,
	0
};
/*------------------------------------------------------------------------------
--|
--| Private Data
--|
------------------------------------------------------------------------------*/

static ExecOutput_structType execOut;
/*------------------------------------------------------------------------------
--|
--| Function Specifications
--|
------------------------------------------------------------------------------*/


#endif /* EXECUTIVE_EXECUTIVE_PVT_H_ */
