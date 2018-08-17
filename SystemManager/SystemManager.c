/*
 * stateManager.c
 *
 *  Created on: May 1, 2018
 *      Author: Lenny
 */

/*-----------------------------------------------------------------------------
 --|
 --| Includes
 --|
 -----------------------------------------------------------------------------*/
#include "Utilities.h"
#include "GarageSensor.h"
#include "MQTT.h"

#include "SystemManager_pvt.h"
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
// Will roll over in 136 years. I probably wont mind at that point.
#define SECONDS_PER_DAY (24 * 60 * 60)
#define SECONDS_PER_HOUR (60 * 60)
#define SECONDS_PER_MIN 60
static void getUpTime(uptime_structType *upTime)
{
	static unsigned long lastMillis = 0;
	static unsigned rollover = 0;

	unsigned long currentMillis = millis();

	if (currentMillis < lastMillis)
		rollover++;

	unsigned seconds = currentMillis/1000 + (rollover * ULONG_MAX/1000);

	lastMillis = currentMillis;

	upTime->days = seconds / SECONDS_PER_DAY;

	upTime->hours = (seconds
			- upTime->days * SECONDS_PER_DAY)
						   / SECONDS_PER_HOUR;

	upTime->minutes = (seconds
			- upTime->days * SECONDS_PER_DAY
			- upTime->hours * SECONDS_PER_HOUR)
						   / SECONDS_PER_MIN;

	upTime->seconds = (seconds
			- upTime->days * SECONDS_PER_DAY
			- upTime->hours * SECONDS_PER_HOUR
			- upTime->minutes * SECONDS_PER_MIN);

	return;

}
// Function called to inspect the data from our garage door sensor module
static void processSensor()
{

	// Sensor module's thoughts about its environment
	switch (sysMgrInputData.garageDoorData.sensorState)
	{
	case SENSOR_INIT_0:
		SetStatus(BAD);
		digitalWrite(STATE1_LED, HIGH);
		break;
	case SENSOR_INIT_1:
		SetStatus(BAD);
		digitalWrite(SYS_CALIBRATED_LED, HIGH);
		break;
	case SENSOR_OPERATIONAL:
		SetStatus(GOOD);
		break;
	default:
		// Added a default case to keep compiler warning from popping up
		// Should never end up here. Maybe due to SEU.
		FatalFault(true);
		break;
	}

	// Sensor module's thoughts about the garage door status
	switch (sysMgrInputData.garageDoorData.doorState)
	{
	case UNKNOWN:
		// If sensor is operational AND doesn't know the door status
		if (sysMgrInputData.garageDoorData.sensorState
				== SENSOR_OPERATIONAL)
		{
			// We have a problem.
			SetStatus(BAD);
			FatalFault(true);
		}
		break;
	case OPENED:
		//digitalWrite(DOOR_CLOSED_LED, LOW); - could. But should?
		//digitalWrite(DOOR_OPEN_LED, HIGH); - could. But should?
		break;
	case CLOSED:
		//digitalWrite(DOOR_CLOSED_LED, HIGH); - could. But should?
		//digitalWrite(DOOR_OPEN_LED, LOW); - could. But should?
		break;
	default:
		// Added a default case to keep compiler warning from popping up
		FatalFault(true);
		break;
	}
}

// Min amount of time it takes the garage door to close from cmd receipt
#define GARAGE_CLOSE_TIME_MS REMOTE_ASSERT_TIME_MS + 10000
// Min amount of time to assert the garage door remote button from cmd receipt
#define REMOTE_ASSERT_TIME_MS 500
static void handleDoorCmd()
{
	// Implemented as state machine to allow for deterministic function duration

	typedef enum
	{
		READY_FOR_COMMAND, ASSERTING_GARAGE_SIGNAL, WAITING_FOR_DOOR
	} doorHandler_State_enumType;

	static doorHandler_State_enumType doorHandlerState = READY_FOR_COMMAND;
	static unsigned long timeAtActuation = 0;
	unsigned long currentTime;

	// Grab the current time before doing anything else
	currentTime = millis();
	switch (doorHandlerState)
	{

	// In this state we will handle a MQTT command if we decide it's valid
	case READY_FOR_COMMAND:
	{
		bool actuate = false;
		// The door can be opened if it is closed
		if ((sysMgrInputData.mqttData.doorCmd == OPEN)
				&& (sysMgrInputData.garageDoorData.doorState == CLOSED))
		{
			actuate = true;
		}
		// The door can be closed if it is open
		if ((sysMgrInputData.mqttData.doorCmd == CLOSE)
				&& (sysMgrInputData.garageDoorData.doorState == OPENED))
		{
			actuate = true;
		}
		// If we've decided to actuate the garage remote signal
		if (actuate)
		{
			// Begin actuation of remote signal
			digitalWrite(GARAGE_DOOR_ACTUATOR, HIGH);
			// Capture time this occurred
			timeAtActuation = currentTime;
			// Move to next state
			doorHandlerState = ASSERTING_GARAGE_SIGNAL;
		}
	}
		break;

		// In this state we will wait for garage door assertion to terminate.
		// All incoming actuation commands from MQTT will simply be ignored.
	case ASSERTING_GARAGE_SIGNAL:
		// If time is up
		if ((unsigned long) (currentTime - timeAtActuation)
				>= REMOTE_ASSERT_TIME_MS)
		{
			// Terminate actuation of remote signal
			digitalWrite(GARAGE_DOOR_ACTUATOR, LOW);
			// Now wait for the door to open or close
			doorHandlerState = WAITING_FOR_DOOR;
		}
		break;

		// In this state we will wait for garage door to close or open
		// All incoming actuation commands from MQTT will simply be ignored.
	case WAITING_FOR_DOOR:
		// If time is up, go back to processing commands
		if ((unsigned long) (currentTime - timeAtActuation)
				>= GARAGE_CLOSE_TIME_MS)
		{
			doorHandlerState = READY_FOR_COMMAND;
		}
		break;
	}
	return;
}
// Input into garage sensor from MQTT interface
static void handleMQTTInputData()
{
	handleDoorCmd();
	// Could support other commands here
	return;
}
// This function is used to report door and system state info
static void publishDoorStateData()
{
	const char* doorStateTable[doorState_enumTypeSize];
	doorStateTable[OPENED] = "opened";
	doorStateTable[CLOSED] = "closed";
	doorStateTable[UNKNOWN] = "unknown";

	const char* sensorStateTable[sensorState_enumTypeSize];
	sensorStateTable[SENSOR_INIT_0] = "booting";
	sensorStateTable[SENSOR_INIT_1] = "calibrating";
	sensorStateTable[SENSOR_OPERATIONAL] = "nominal";

	char generalBuffer[100];

	mqttMsgType_StructType msg;
	msg.topic = PUB_GARAGE_SENSOR;
	msg.str = generalBuffer;

	// Extracted for readability
	doorState_enumType doorState = sysMgrInputData.garageDoorData.doorState;
	sensorState_enumType sensorState =
			sysMgrInputData.garageDoorData.sensorState;

	snprintf(generalBuffer, NELEMS(generalBuffer), "Door:%s State:%s",
			doorStateTable[doorState], sensorStateTable[sensorState]);

	SendMQTTMsg(msg);

	return;
}
// This function is used to report debug info about the system
static void publishDebugData()
{
	char generalBuffer[100];

	mqttMsgType_StructType msg;
	msg.topic = PUB_GARAGE_DEBUG;
	msg.str = generalBuffer;

	// Extracted for readability
	unsigned long open = sysMgrInputData.garageDoorData.openedDistanceInCM;
	unsigned long closed = sysMgrInputData.garageDoorData.closedDistanceInCM;
	unsigned long current = sysMgrInputData.garageDoorData.currentDistanceInCM;
	unsigned mqttReconnects = sysMgrInputData.mqttData.numReconnects;
	unsigned worstCaseFrameTime = sysMgrInputData.execData.worstCaseFrameTimeMS;

	// Get system uptime
	uptime_structType upTime;
	getUpTime(&upTime);

	snprintf(generalBuffer, NELEMS(generalBuffer),
			"Open:%lu " // Open door dist
			"Close:%lu " // Close door dist
			"Current:%lu " // Current door dist
			"Days:%u " // Up time in days
			"Hours:%u " // Up time in hours
			"Mins:%u " // Up time in minutes
			"Secs:%u " // Up time in seconds
			"Reconnects:%u " // Number of MQTT reconnects
			"WCF:%u ", // Worst case frame time
			open, closed, current, upTime.days, upTime.hours, upTime.minutes,
			upTime.seconds, mqttReconnects, worstCaseFrameTime);

	SendMQTTMsg(msg);

	return;

}
// Garage sensor publishes some data to MQTT subscribers. This is the function
// that will handle that.
static void handleMQTTOutputData()
{
	// Let Raspberry PI know about the door state
	publishDoorStateData();
	// Let Raspberry PI know about the system state
	publishDebugData();
	return;

}
static void processMQTT()
{
	// Sender(s)->MQTT->Garage Sensor
	handleMQTTInputData();
	// Garage Sensor->MQTT->Receiver(s)
	handleMQTTOutputData();

	return;
}
static void getInputData()
{

	GetMQTTData(&sysMgrInputData.mqttData);
	GetGarageSensorData(&sysMgrInputData.garageDoorData);
	GetExecData(&sysMgrInputData.execData);
	return;

}
/*------------------------------------------------------------------------------
 --|
 --| Public Function Bodies
 --|
 ------------------------------------------------------------------------------*/
extern void InitSystemManager()
{

	// Pin for garage door actuator (hacked remote)
	pinMode(GARAGE_DOOR_ACTUATOR, OUTPUT);

	// The two "state" LEDs
	pinMode(STATE1_LED, OUTPUT);
	pinMode(SYS_CALIBRATED_LED, OUTPUT);

	// The two wired up colors of the one system health LED
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);

	// Initialize all to logic-low
	digitalWrite(GARAGE_DOOR_ACTUATOR, LOW);
	digitalWrite(STATE1_LED, LOW);
	digitalWrite(SYS_CALIBRATED_LED, LOW);
	digitalWrite(RED_LED, LOW);
	digitalWrite(GREEN_LED, LOW);
	return;
}
// Must be called every frame
extern void ManageSystemManager()
{
	// Get the input data we need in order to make decisions
	getInputData();
	// Process sensor data (maybe latch a fatal fault, turn on some LEDs, etc)
	processSensor();
	// Process mqtt data (maybe actuate the door, publish some data, etc.)
	processMQTT();

	return;
}

