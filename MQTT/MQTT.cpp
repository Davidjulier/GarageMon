/*
 * MQTT.c
 *
 *  Created on: May 10, 2018
 *      Author: Lenny
 */


/*-----------------------------------------------------------------------------
--|
--| Includes
--|
-----------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>

// This library provides a client for doing simple publish/subscribe messaging
// with a server that supports MQTT
#include <PubSubClient.h>

#include "MQTT.h"
#include "MQTT_pvt.h"

// Used for latching a fatal fault if we can't connect to MQTT
#include "Utilities.h"

/*------------------------------------------------------------------------------
--|
--| Private Function Bodies
--|
------------------------------------------------------------------------------*/
static void callback(const char topic[], unsigned char* payload, unsigned int length)
{
	std::string msg = std::string((const char*)payload, length);

	if (msg.compare("open") == 0)
		mqttOut.doorCmd = OPEN;
	else if (msg.compare("close") == 0)
		mqttOut.doorCmd = CLOSE;

	return;

}

// This wrapper tries to connect to our MQTT server a few times. If not
// successful, latches a fatal fault and kills the unit. Should be called
// Only after wifi, callbacks, and sever have been configured
static void mqttConnect()
{
	// How long to wait between reconnect attempts
	const unsigned RECONNECT_DELAY_SEC  = 2;
	// Two minutes worth of attempts (enough for any server reboot situation)
	const unsigned RETRY_MAX = 120 / RECONNECT_DELAY_SEC;

	unsigned totalRetries = 0;
	while (client.connect(mqttID, mqttUser, mqttPassword) != true)
	{

		if (totalRetries++ == RETRY_MAX)
			// Welp, that didn't work. Wait for human intervention.
			FatalFault(false);

		// Wait before retrying
		delay(RECONNECT_DELAY_SEC * 1000);
	}

	// Register for the topics we're interested in
	client.subscribe(SUB_GARAGE_CMD);

	return;
}

/*------------------------------------------------------------------------------
--|
--| Public Function Bodies
--|
------------------------------------------------------------------------------*/

extern void InitMQTTCom()
{

	// Set up our output data
	mqttOut = mqttOutputInit;

	// Connect to the WiFI Network
	WiFi.begin(ssid, password);

	// Temp...for debug...
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(100);
		printf("Connecting to WiFi..\n");
	}

	// Specify the address and the port of the MQTT server.
	client.setServer(mqttServer, mqttPort);

	// Register our callback function
	client.setCallback(callback);

	// Connect to our raspberry pi MQTT server and register for the topics
	// we're interested in
	mqttConnect();

	// Say hello
	std::string msg = "Hello from Garage Sensor: ";
	msg += WiFi.localIP().toString().c_str();
	msg += "!\r\n";
	client.publish(PUB_GARAGE_GENERAL, msg.c_str(), true);

	return;

}


// Must be called every frame
extern void ManageMQTTCom()
{

	// If disconnected, reconnect
	if (!client.connected())
	{
		mqttConnect();
		mqttOut.numReconnects++;
	}
	// Process incoming messages, and maintain connection to our Raspberry Pi
	// MQTT broker server
	client.loop();
	return;

}

extern void SendMQTTMsg(mqttMsgType_StructType msg)
{
	client.publish(msg.topic, msg.str, true);
	return;

}
extern void GetMQTTData(MQTTOutput_structType *output)
{
	*output = mqttOut;

	// Register receipt
	mqttOut.doorCmd = NO_CHANGE;

	return;

}
