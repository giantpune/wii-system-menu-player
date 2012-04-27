/*
Copyright (c) 2012 - Dimok and giantpune

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#ifndef DIHANDLER_H
#define DIHANDLER_H

#include <gctypes.h>
#include <gccore.h>
#include <stdio.h>

#include "sigslot.h"

// class to handle talking to the DVD drive and stuff like that

class DiHandler: public sigslot::has_slots<>
{
public:
	static DiHandler *Instance()
	{
		if( !instance )
		{
			instance = new DiHandler;
		}
		return instance;
	}

	enum ErrorCode
	{
		E_None,					// no error
		E_Init,					// failed to open /dev/di.  fatal
		E_Reset,
		E_CheckCover,			// error checking of there is a disc in the drive
		E_OpenPartition,		// error opening the partition.  the real system menu treats this as a fatal error
								// and shows the "An error has occurred..." screen.  we can do better than that.
								// this will happen for fakesigned discs if the IOS knows how to check the signature
		E_DVD_ReadError,		// some read error happened
		E_NoMem,				// failed to allocate some memory.  this one is fatal
		E_NoOpeningBnr			// didnt find a banner in a wii disc

	};

	// error code, fatal
	//! probably most of them will be fatal, otherwise handled internally
	sigslot::signal2< ErrorCode, bool > ErrorHappened;

	enum DiscType
	{
		T_GC,
		T_Wii,
		T_Unknown
	};

	sigslot::signal0<> StartingToReadDisc;
	sigslot::signal1< DiscType > DiscInserted;
	sigslot::signal0<> DiscEjected;

	// this will only fire 1 time for the opening.bnr
	//! the buffer is created with memalign()
	//! if anybody gets this data, set this bool to true so the thread knows not to free it.
	//! otherwise it will be freed imediately
	sigslot::signal3< u8*, u32, bool & > OpeningBnrReady;

	// have the working thread sleep/wake up
	void Wake();
	void Sleep();

	// closes /dev/di
	void DeInit();




private:
    DiHandler();
	static DiHandler *instance;

	static lwp_t thread;
	static mutex_t mutex;
	static bool threadSleep;
	static bool threadExit;
	static void *ThreadMain( void *arg );

};

#endif // DIHANDLER_H
