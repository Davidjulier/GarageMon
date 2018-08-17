/*
 * Utilities.cpp
 *
 *  Created on: Apr 30, 2018
 *      Author: Lenny
 */

/*-----------------------------------------------------------------------------
--|
--| Includes
--|
-----------------------------------------------------------------------------*/
#include "Arduino.h"
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

/*------------------------------------------------------------------------------
--|
--| Public Function Bodies
--|
------------------------------------------------------------------------------*/
extern void SetStatus(sysState_enumType status)
{
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);
	switch (status)
	{
	case GOOD:
		digitalWrite(RED_LED, LOW);
		digitalWrite(GREEN_LED, HIGH);
		break;
	default:
	case BAD:
		digitalWrite(RED_LED, HIGH);
		digitalWrite(GREEN_LED, LOW);
		break;

	}
	return;
}

#define ALARM D8
extern void FatalFault(bool alarm)
{
	SetStatus(BAD);

	// Sound an alarm
	if (alarm)
	{
	pinMode(ALARM, OUTPUT);
	digitalWrite(ALARM, HIGH);
	}
	// Pet our dog to prevent reboot.
	while(1){ESP.wdtFeed();}

}
