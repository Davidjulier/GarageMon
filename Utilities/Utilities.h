/*
 * Utilities.h
 *
 *  Created on: Apr 30, 2018
 *      Author: Lenny
 */

#ifndef UTILITIES_UTILITIES_H_
#define UTILITIES_UTILITIES_H_
/*------------------------------------------------------------------------------
--|
--| Includes
--|
------------------------------------------------------------------------------*/
#include "arduino.h"
/*------------------------------------------------------------------------------
--|
--| Defines
--|
------------------------------------------------------------------------------*/
// Utilities handles fatal faults, and needs to know these LEDs therefore
#define RED_LED D3
#define GREEN_LED D4

#define ABS_DIFF(x,y) (x > y ? x - y : y - x)

#define NELEMS(n) (sizeof(n) / sizeof(n[0]))

// Compile-time assertion macro.  If the assertion evaluates to false,
// an enum value (1/0) is created, generating a compiler error.  Otherwise,
// no code is generated.  Must be used within a function.
#define COMPILE_TIME_ASSERT(e)             do {                          \
                                              enum {                     \
                                                 assert_static = 1/(e)   \
                                              };                         \
                                           } while(0)

/*------------------------------------------------------------------------------
--|
--| Defines
--|
------------------------------------------------------------------------------*/

#define ULONG_MAX 0xFFFFFFFF

/*------------------------------------------------------------------------------
--|
--| Types
--|
------------------------------------------------------------------------------*/

typedef enum
{
	BAD,
	GOOD
} sysState_enumType;

// Generic function pointer
typedef void (*genericFunct)(void);

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
// Name mangling...
#if defined (__cplusplus)
extern "C"
{
#endif
extern void SetStatus(sysState_enumType status);
extern void FatalFault(bool alarm);
#if defined (__cplusplus)
}
#endif
#endif /* UTILITIES_UTILITIES_H_ */
