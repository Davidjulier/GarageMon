/*
 * GarageSensor.h
 *
 *  Created on: May 3, 2018
 *      Author: Lenny
 */

#ifndef GARAGESENSOR_GARAGESENSOR_H_
#define GARAGESENSOR_GARAGESENSOR_H_

/*------------------------------------------------------------------------------
--|
--| Includes
--|
------------------------------------------------------------------------------*/

/* None */

/*------------------------------------------------------------------------------
--|
--| Defines
--|
------------------------------------------------------------------------------*/

/* None */

/*------------------------------------------------------------------------------
--|
--| Types
--|
------------------------------------------------------------------------------*/
// Garage door possible states
typedef enum
{
	UNKNOWN,
	CLOSED,
	OPENED,
	doorState_enumTypeSize
} doorState_enumType;

// Garage door sensor possible states
typedef enum
{
	// Fresh bootup. We don't know anything at all.
	SENSOR_INIT_0,
	// Open OR Closed measurement established (dont know which is which)
	SENSOR_INIT_1,
	// Open AND Closed measurement established (we DO know which is which)
	SENSOR_OPERATIONAL,
	sensorState_enumTypeSize
}sensorState_enumType;


// Output data from this module is provided via this type
typedef struct
{
	// Keeps track of whether we're calibrated yet. On bootup system will try
	// to determine what closed and open garage doors look like
	// NOTE! Caller should check this prior to assuming any of the data is valid
	sensorState_enumType sensorState;
	doorState_enumType doorState;

	// Not for conventional use (might need these later or for debugging)
	unsigned long currentDistanceInCM;
	unsigned long closedDistanceInCM;
	unsigned long openedDistanceInCM;
}GarageSensorOutput_structType;

/*------------------------------------------------------------------------------
--|
--| Constants
--|
------------------------------------------------------------------------------*/

/* None */

/*------------------------------------------------------------------------------
--|
--| Function Specifications
--|
------------------------------------------------------------------------------*/
//If we want to use functions linked in as externals that were compiled using
//the plain C compiler, we'll need their function declarations enclosed in an
//extern "C" {} block which suppresses C++ name mangling for everything declared
// or defined inside
#if defined (__cplusplus)
extern "C"
{
#endif

// Must be called prior to any of the other functions within this module
extern void InitGarageSensor();

// Must be called once per software frame. Note, the speed of sound is slow.
// Therefore, the duration of this function is variable depending on sensor
// target. It is capped at 90ms to ensure we can have a 100ms software frame.
extern void ManageGarageSensor();

// This is the only interface to data within this module. Caller should be
// careful to check that the garage sensor has valid data
extern void GetGarageSensorData(GarageSensorOutput_structType *data);

#if defined (__cplusplus)
}
#endif

#endif /* GARAGESENSOR_GARAGESENSOR_H_ */
