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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
#include "bannerlist.h"
#include "SoundOperations/audio.h"
#include "SoundOperations/gui_sound.h"
#include "utils/nandtitle.h"
#include "DirList.h"
#include "video.h"
#include "Banner.h"
#include "menuhandler.h"
#include "settings.h"
#include "SystemMenu/SystemFont.h"
#include "SystemMenu/Inputs.h"

#include "utils/ash.h"
#include "U8Archive.h"
#include "SystemMenu/SystemMenuResources.h"

extern "C"
{
	extern s32 MagicPatches(s32);
	void __exception_setreload(int t);
}

int	main( int argc, char *argv[] )
{
	__exception_setreload(15);

	// good enough.  we're not using rand for any 1337 crpto stuff
	srand( time( NULL ) );

	InitGecko();

	// load sd before settings
	fatMount("sd", &__io_wiisd, 0, 32, 64);
	Settings::Load( argc, argv );
	InitVideo();
	InitAudio();
	MagicPatches(1);
	ISFS_Initialize();
	NandTitles.Get();


	if( !SystemMenuResources::Instance()->Init() )
	{
		gprintf( "error initializing system menu resources.  exiting\n" );
		exit( 0 );
	}

	MenuHandler::Instance()->Start();

	exit( 0 );

	return 0;
}
