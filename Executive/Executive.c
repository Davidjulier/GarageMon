/*-----------------------------------------------------------------------------
 --|
 --| Includes
 --|
 -----------------------------------------------------------------------------*/
#include "Executive.h"
#include "Executive_pvt.h"

#include "Utilities.h"

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

// Must be called every frame
static void manageHeartBeat(unsigned frameCount)
{
	static const unsigned HEART_BEAT_RATE_MS = 500;
	static bool ledCmd = false;

	// During overflow to zero this will blink twice
	if (frameCount % (HEART_BEAT_RATE_MS / MS_PER_FRAME) == 0)
	{
		digitalWrite(HEARTBEAT_LED, ledCmd);
		ledCmd = !ledCmd;
	}

	return;
}

/*------------------------------------------------------------------------------
 --|
 --| Public Function Bodies
 --|
 ------------------------------------------------------------------------------*/

extern void ExecInit(void) {

	execOut = execOutputInit;

	for (unsigned i = 0; i < NELEMS(ExecInitTable); i++)
		ExecInitTable[i]();

	// Main blinks the LED
	pinMode(HEARTBEAT_LED, OUTPUT);

	return;

}


// Must be called in an infinite loop with no delay
extern void ManageExec()
{
	// Count frames away. Basic unit of measurement.
	static unsigned overFlowingFrameCounter = 0;
			overFlowingFrameCounter++;

	unsigned delayTimeTillNextFrame;

	// Used to measure our foreground frame duration in MS
	unsigned long foregroundDuration;

	/*-----------Foreground Frame-----------*/

	// Get current time in milliseconds
	foregroundDuration = millis();

	// Blink an LED so we know we're alive
	manageHeartBeat(overFlowingFrameCounter);

	// Run all of our tasks

	// Note: The overhead of division here is low. The alternative would be
	// to do it at compile time (or have an interrupt driven scheme) but this
	// was not worth the tradeoff of added complexity / diminished readability
	for (unsigned i = 0; i < NELEMS(ExecFrameTable); i++)
	{
		// In 13.6 years of run-time overflowing framecounter will become zero
		// and all functions will be called regardless of their rate. This is
		// deemed acceptable for simplicity.
		if (overFlowingFrameCounter
				% (ExecFrameTable[i].rateCalledInMS / MS_PER_FRAME) == 0)
			ExecFrameTable[i].functPtr();
	}

	// Let's see how long that took (overflow safe)
	foregroundDuration = millis() - foregroundDuration;

	/*-----------End Foreground Frame-----------*/

	// Calculate stats about foreground execution
	// (1) Current frame time
	execOut.lastFrameTimeMS = foregroundDuration;
	// (2) Keep Track of Max
	if (execOut.lastFrameTimeMS > execOut.worstCaseFrameTimeMS)
	{
		execOut.worstCaseFrameTimeMS = execOut.lastFrameTimeMS;
	}

	// Calculate new delay time
	delayTimeTillNextFrame = MS_PER_FRAME - execOut.lastFrameTimeMS;

	// If we over-ran
	if (execOut.lastFrameTimeMS > MS_PER_FRAME)
	{
		// We could fault here, but MQTT libraries are non deterministic
		// and I did not feel like re-writing them. No real 'real time'
		// requirements here, so let's just move on with our lives

		// We'll accept the overrun and restart the frame ASAP
		delayTimeTillNextFrame = 0;

	}

	// Mostly start tasks every MS_PER_FRAME
	delay(delayTimeTillNextFrame);

	return;

}
extern void GetExecData(ExecOutput_structType *output)
{
	*output = execOut;

	return;
}
