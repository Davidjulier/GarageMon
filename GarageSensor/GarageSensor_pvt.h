/*
 * GarageSensor_pvt.h
 *
 *  Created on: May 3, 2018
 *      Author: Lenny
 *
 *      WARNING: This is a private header and should not be included outside of
 *      the garage sensor module.
 */

#ifndef GARAGESENSOR_GARAGESENSOR_PVT_H_
#define GARAGESENSOR_GARAGESENSOR_PVT_H_


/*------------------------------------------------------------------------------
--|
--| Includes
--|
------------------------------------------------------------------------------*/
#include "GarageSensor.h"
/*------------------------------------------------------------------------------
--|
--| Defines
--|
------------------------------------------------------------------------------*/
// See the sensor documentation (HC-04) for an understanding of these pins.
#define TRIGGER D0
#define ECHO D1
// Max time to wait for sound to reflect. Limits our max distance but
// caps function duration
#define US_TIMEOUT_CAP 70000

// Sensor tolerance (cm)
#define SENSOR_TOLERANCE_CM 5

#define WITHIN_TOLERANCE(a, b) \
	(a > ( b-SENSOR_TOLERANCE_CM) && \
			a < (b+SENSOR_TOLERANCE_CM))
/*------------------------------------------------------------------------------
--|
--| Types
--|
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
--|
--| Constants
--|
------------------------------------------------------------------------------*/
/*-----Module's Output Init-----*/

static const GarageSensorOutput_structType GarageSensorOutputInit =
{
	SENSOR_INIT_0,      // sensorState
    UNKNOWN,       		// doorState
    0UL,        		// currentDistanceInCM
    0UL,        		// openedDistanceInCM
    0UL,        		// closedDistanceInCM

};

/*------------------------------------------------------------------------------
--|
--| Private Data
--|
------------------------------------------------------------------------------*/
// Output data for module
GarageSensorOutput_structType garageSensorOut;

/*------------------------------------------------------------------------------
--|
--| Function Specifications
--|
------------------------------------------------------------------------------*/


#endif /* GARAGESENSOR_GARAGESENSOR_PVT_H_ */
