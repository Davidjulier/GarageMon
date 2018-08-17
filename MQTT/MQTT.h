/*
 * MQTT.h
 *
 *  Created on: May 10, 2018
 *      Author: Lenny
 */

#ifndef MQTT_MQTT_H_
#define MQTT_MQTT_H_


/*------------------------------------------------------------------------------
--|
--| Includes
--|
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
--|
--| Defines
--|
------------------------------------------------------------------------------*/
#define PUB_GARAGE_GENERAL "home/garage/general"
#define PUB_GARAGE_SENSOR "home/garage/sensor"
#define PUB_GARAGE_DEBUG "home/garage/debug"

#define SUB_GARAGE_CMD "home/garage/command"

/*------------------------------------------------------------------------------
--|
--| Types
--|
------------------------------------------------------------------------------*/

typedef struct
{
	char *topic;
	char *str;
} mqttMsgType_StructType;

typedef enum
{
	NO_CHANGE,
	OPEN,
	CLOSE,
}doorCommand_enumType;

typedef struct
{
	doorCommand_enumType doorCmd;
	unsigned numReconnects;

}MQTTOutput_structType;


/*------------------------------------------------------------------------------
--|
--| Constants
--|
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
--|
--| Private Data
--|
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
--|
--| Function Specifications
--|
------------------------------------------------------------------------------*/
#if defined (__cplusplus)
// Name mangling...
extern "C"
{
#endif
// Must be called prior to any of the other functions within this module
extern void InitMQTTCom();
extern void ManageMQTTCom();
extern void SendMQTTMsg(mqttMsgType_StructType msg);
extern void GetMQTTData(MQTTOutput_structType *output);

#if defined (__cplusplus)
}
#endif
#endif /* MQTT_MQTT_H_ */
