/*
 * GarageSensor.c
 *
 *  Created on: May 3, 2018
 *      Author: Lenny
 */


/*-----------------------------------------------------------------------------
--|
--| Includes
--|
-----------------------------------------------------------------------------*/
#include "Arduino.h"
#include "GarageSensor_pvt.h"

#include "Utilities.h"
// Exec has our frame time which we need to know to calculate # samples
#include "Executive.h"

/*-----------------------------------------------------------------------------
--|
--| Private Data
--|
-----------------------------------------------------------------------------*/
/* None */

/*------------------------------------------------------------------------------
--|
--| Private Function Bodies
--|
------------------------------------------------------------------------------*/
// Drive HC-04, wait for sound to echo, calculate distance by delta T
// Function duration is variable (sound is SLOW!) capped by US_TIMEOUT_CAP
static unsigned long garageSensorPingTarget() {

	unsigned long duration;
	unsigned long distance;

	// Clears the trigPin
	digitalWrite(TRIGGER, LOW);
	delayMicroseconds(2);

	// Sets the trigPin on HIGH state for 10 micro seconds
	digitalWrite(TRIGGER, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIGGER, LOW);

	// Reads the echoPin, returns the sound wave travel time in microseconds
	duration = pulseIn(ECHO, HIGH, US_TIMEOUT_CAP);

	// Calculating the distance
	// TODO: define these magic numbers. Something about speed of sound...
	distance = duration * 0.034 / 2;

	return distance;
}

// Handle sensor state. Should only be called when we have a stable measurement.
static void manageOutputData(unsigned long distance)
{
	static unsigned long distance1, distance2;

	// We trust the reading from caller
	garageSensorOut.currentDistanceInCM = distance;

	switch (garageSensorOut.sensorState)
	{

	// Our first real stable distance. Record it.
	case SENSOR_INIT_0:
		distance1 = garageSensorOut.currentDistanceInCM;
		garageSensorOut.sensorState = SENSOR_INIT_1;
		break;

		// This is getting interesting. We now have two stable distances.
	case SENSOR_INIT_1:

		distance2 = garageSensorOut.currentDistanceInCM;

		// If the two distances are different *enough* we are good
		// to determine what is closed and what is an OPENED garage door
		if (!WITHIN_TOLERANCE(distance2, distance1))
		{

			if (distance1 < distance2)
			{
				garageSensorOut.closedDistanceInCM = distance2;
				garageSensorOut.openedDistanceInCM = distance1;
				garageSensorOut.doorState = CLOSED;

			}
			else
			{
				garageSensorOut.closedDistanceInCM = distance1;
				garageSensorOut.openedDistanceInCM = distance2;
				garageSensorOut.doorState = OPENED;

			}

			// We can now go operational.
			garageSensorOut.sensorState = SENSOR_OPERATIONAL;
		}
		break;

		// While operational, all we do is compare current distance to known limits
	case SENSOR_OPERATIONAL:
		// If closed
		if (WITHIN_TOLERANCE(garageSensorOut.currentDistanceInCM,
				garageSensorOut.closedDistanceInCM))
		{
			garageSensorOut.doorState = CLOSED;

		}
		// If open
		else if (WITHIN_TOLERANCE(garageSensorOut.currentDistanceInCM,
				garageSensorOut.openedDistanceInCM))
		{
			garageSensorOut.doorState = OPENED;
		}
		// If neither
		else
		{
			// What happened?!
			// system manager should see this and latch a fatal fault w/ alarm
			garageSensorOut.doorState = UNKNOWN;
		}

		break;

	default:
		// Added a default case to keep compiler warning from popping up
		// Should never ever get called (maybe due to SEU)
		FatalFault(true);
		break;

	}

}

/*------------------------------------------------------------------------------
--|
--| Public Function Bodies
--|
------------------------------------------------------------------------------*/

//------------------------------------INIT------------------------------------
extern void InitGarageSensor()
{
	// Set up our hardware
	pinMode(TRIGGER, OUTPUT);
	pinMode(ECHO, INPUT);

	// Set up our output data
	garageSensorOut = GarageSensorOutputInit;

}

//------------------------------------MANAGE------------------------------------
// Must be called every frame

// The following function attempts to verify a distance measurement prior to
// using it for processing. In other words, we want to trust the data first. So,
// we specify, in milliseconds, how many measurements to get in a row
// for a measurement to be considered stable. We want to avoid creep so we do
// it in the following way:

// First, we get an average of SAMPLES_TO_AVERAGE measurements. Then, we compare
// samples right from our sonar sensor, for SENSOR_STABLE_MS milliseconds, to
// this average. If they all are within the tolerance, we consider the distance
// stable. Then, and only then, do we use it for processing.

// Rationale: When the garage door is closing, we may have wild measurements.
// so this adds robustness.
#define SAMPLES_TO_AVERAGE 5
#define SENSOR_STABLE_MS 3000
#define NUMBER_SAMPLES SENSOR_STABLE_MS/MS_PER_FRAME
extern void ManageGarageSensor()
{
	// Double check that our frame-time makes sense for logic in this function
	COMPILE_TIME_ASSERT(NUMBER_SAMPLES > 0);

	// Counter for when averaging samples
	static unsigned long avgSamplesCount=0;
	// Counter for how many stable distance samples we have gotten
	static unsigned long stableSamplesCount=0;
	// Stable distance we've calculated (known after multiple frames)
	static unsigned long stableDistance = 0;
	// Current distance we've calculated (known within 1 frame)
	unsigned long currentDistance;

	// Grab a sample
	currentDistance = garageSensorPingTarget();

	// STATE1 - Average some readings and use it as our 'stable distance'
	if (avgSamplesCount < SAMPLES_TO_AVERAGE)
	{
		stableDistance += currentDistance;
		avgSamplesCount++;
	}
	// Divide
	else if (avgSamplesCount == SAMPLES_TO_AVERAGE)
	{
		stableDistance = stableDistance / SAMPLES_TO_AVERAGE;
		avgSamplesCount++;
	}

	// STATE2 - Ensure distance is stable over the specified time
	else
	{
		// If this new sample is within tolerance of the established distance
		if (currentDistance > 0
				&& WITHIN_TOLERANCE(currentDistance, stableDistance))
			stableSamplesCount++;

		// Otherwise, reset our counters and distance and try again
		else
		{
			// Reset our counters
			stableSamplesCount = 0;
			avgSamplesCount = 0;
			// Reset our distance - calculate fresh next time
			stableDistance = 0;
		}


		//printf("ASC = %lu, Distance = %u, CTD = %u, this many stable samples: %u\r\n", avgSamplesCount,			currentDistance, stableDistance, stableSamplesCount);

	}

	// If we have determined a stable distance, we can do our main processing
	if (stableSamplesCount == NUMBER_SAMPLES)
	{
		// Prevent overflow and allow == instead of >= in this if statement
		stableSamplesCount--;
		manageOutputData(stableDistance);
	}


	return;

}
extern void GetGarageSensorData(GarageSensorOutput_structType *output)
{
	*output = garageSensorOut;
}
