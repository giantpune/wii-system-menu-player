#ifndef __SC_H__
#define __SC_H__


#include <gctypes.h>

// extension for conf.h
const char* CONF_GetLanguageString();

// copies of a couple functions already in the conf.h, but storing the result so getting the values it faster after the first time
s32 _CONF_GetEuRGB60();
s32 _CONF_GetAspectRatio();
s32 _CONF_GetVideo();


#endif
