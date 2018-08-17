#include "Arduino.h"
#include "HardwareSerial.h"

#include "Utilities.h"
#include "Executive.h"

// Arduino setup entry point
extern void setup()
{
	// Used for debug
	Serial.begin(9600);
	Serial.setDebugOutput(true);

	ExecInit();

	return;
}

// Arduino loop entry point
extern void loop()
{
	// Exec handles execution. Expects to be called forever with no delay.
	// Function duration will be MS_PER_FRAME (but overruns are possible)
	ManageExec();

	return;

}
