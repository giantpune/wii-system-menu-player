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
#ifndef GREYBACKGROUND_H
#define GREYBACKGROUND_H

#include "object.h"
#include "systemmenuresource.h"
#include "utils/char16.h"

class GreyBackground: public SystemMenuResource
{
public:
	GreyBackground();
	~GreyBackground();

	bool Load( const u8* boardAshData, u32 boardAshSize );
	void Render( Mtx &modelview, const Vec2f &ScreenProps, bool widescreen );

	// set the date that appears on the screen
	//! the grey background contains the pane for the date
	//! it shows the day of the week, when month/date (when the region is set to USA,
	//! i still have to figure out how it is formatted for other languages)
	//! the weekDay parameter expects the index of the weekday, with sunday being 0
	//! the other 2 parameters are just as you would expect them to be displayed
	void SetDate( u8 weekDay, u8 dayOfTheMonth, u8 month );

protected:
	Layout *layout;
	Object *obj;

	std::map< std::string, Animation *>brlans;

	char16 dateTextBuffer[ 64 ];
};


#endif // GREYBACKGROUND_H
