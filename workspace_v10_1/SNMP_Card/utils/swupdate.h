//****************************************************************************
//*
//*
//* File: CANTask.h
//*
//* Synopsis: Software Bootloader define file - related Structures and interfaces
//*
//*
//****************************************************************************

#ifndef __SWUPDATE_H__
#define __SWUPDATE_H__

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------
#include <Common.h>
//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------
#define FORCE_UPDATE_ADDR 0x20004000
#define FORCE_UPDATE_VALUE 0x1234cdef

//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------
typedef void (*tSoftwareUpdateRequested)(void);

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

void swupdate_SoftwareUpdateInit(tSoftwareUpdateRequested pfnCallback);
void swupdate_SoftwareUpdateBegin(BOOL forceUpdate);
//----------------------------------------------------------------------------

#endif /* __SWUPDATE_H__ */
//==============================================================================
// END OF __SWUPDATE_H__
//==============================================================================

