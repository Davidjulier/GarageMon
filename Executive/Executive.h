/*
 * executive.h
 *
 *  Created on: May 1, 2018
 *      Author: Lenny
 */

#ifndef EXECUTIVE_EXECUTIVE_H_
#define EXECUTIVE_EXECUTIVE_H_


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

#define MS_PER_FRAME 100UL
/*------------------------------------------------------------------------------
--|
--| Types
--|
------------------------------------------------------------------------------*/

typedef struct
{
	unsigned lastFrameTimeMS;
	unsigned worstCaseFrameTimeMS;

}ExecOutput_structType;
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
extern void ExecInit(void);
extern void ManageExec(void);
extern void GetExecData(ExecOutput_structType *output);


#if defined (__cplusplus)
}
#endif

#endif /* EXECUTIVE_EXECUTIVE_H_ */
