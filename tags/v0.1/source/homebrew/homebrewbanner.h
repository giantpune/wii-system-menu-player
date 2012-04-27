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
#ifndef HOMEBREWBANNER_H
#define HOMEBREWBANNER_H

#include <string>
#include <vector>

#include "Banner.h"

class HomebrewXML
{
public:
	HomebrewXML();
	HomebrewXML( const char* filename );
	~HomebrewXML();

	int LoadHomebrewXMLData(const char* filename);
	void SetName( const char * newName);

	const char16* GetName() const { return Name; }
	const char16* GetCoder() const { return Coder; }
	const char16 * GetVersion() const { return Version; }
	const char16 * GetReleasedate() const { return Releasedate; }
	const char16 * GetShortDescription() const { return ShortDescription; }
	const char16 * GetLongDescription() const { return LongDescription; }

	const std::vector<std::string> & GetArguments() const { return Arguments; }

protected:
	char16* Name;
	char16* Coder;
	char16* Version;
	char16* Releasedate;
	char16* ShortDescription;
	char16* LongDescription;
	std::vector<std::string> Arguments;
};

#endif // HOMEBREWBANNER_H
