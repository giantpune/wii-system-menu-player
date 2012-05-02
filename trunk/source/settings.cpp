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
#include <cstdio>
#include <gctypes.h>
#include <gccore.h>

#include "gecko.h"
#include "settings.h"

namespace Settings
{

bool useDumpedBanners = true;
bool useNandBanners = true;
bool useHomebrewForBanners = true;
bool mountDVD = true;
std::string sdBannerPath = "sd:/banners/";
std::string resourcePath;

#define CURRENT_SETTINGS_VERSION	1
#define SAVE_FILE_NAME "smPlayerSettings.xml"
static std::string saveFilePath = "sd:/config/";


bool GetXMLBool( TiXmlElement *node, bool defaultValue )
{
	const char *val = NULL;

	if( !node || !node->FirstChild() || !( val = node->FirstChild()->Value() ) )
	{
		return defaultValue;
	}

	if( !strcasecmp( val, "1" )
			|| !strcasecmp( val, "yes" )
			|| !strcasecmp( val, "true" ) )
	{
		return true;
	}

	if( !strcasecmp( val, "0" )
			|| !strcasecmp( val, "no" )
			|| !strcasecmp( val, "false" ) )
	{
		return false;
	}

	return defaultValue;
}

const std::string &GetXMLString( TiXmlElement *node, const std::string &defaultValue )
{
	if( !node || !node->FirstChild() )
	{
		return defaultValue;
	}

	return node->FirstChild()->ValueStr();
}



// save and load
void Load( int argc, char *argv[] )
{
	if( argc > 0 && !strncasecmp( argv[ 0 ], "sd:/", 4 ) )
	{
		const char* test = argv[ 0 ];
		u32 len = strlen( test );
		int slashes = 0;
		int lastSlash = 0;
		for( u32 i = 0; i < len; i++ )
		{
			if( test[ i ] == '/' )
			{
				lastSlash = i;

				// ignore double slash
				if( test[ i + 1 ] != '/' )
				{
					slashes++;
				}
			}
		}

		if( slashes > 1 && lastSlash < 0x100 )
		{
			char derp[ 0x100 ];
			strncpy( derp, test, lastSlash + 1 );
			saveFilePath = derp;
		}
	}

	// set all default values
	useDumpedBanners = true;
	useNandBanners = true;
	useHomebrewForBanners = true;
	mountDVD = true;
	sdBannerPath = "sd:/banners/";
	resourcePath.clear();

	std::string fileName( saveFilePath + SAVE_FILE_NAME );

	// try to load xml file
	TiXmlDocument xmlDoc( fileName );
	int version = 0;
	if( !xmlDoc.LoadFile() )
	{
		return;
	}

	TiXmlElement *appNode =  xmlDoc.FirstChildElement( "settings" );
	if( !appNode )
	{
		return;
	}

	if( !appNode->Attribute( "version", &version ) || version < CURRENT_SETTINGS_VERSION )
	{
		return;
	}

	// read values
	useDumpedBanners = GetXMLBool( appNode->FirstChildElement( "useDumpedBanners" ), true );
	useNandBanners = GetXMLBool( appNode->FirstChildElement( "useNandBanners" ), true );
	useHomebrewForBanners = GetXMLBool( appNode->FirstChildElement( "useHomebrewForBanners" ), true );
	mountDVD = GetXMLBool( appNode->FirstChildElement( "mountDVD" ), true );
	sdBannerPath = GetXMLString( appNode->FirstChildElement( "sdBannerPath" ), "sd:/banners/" );
	resourcePath = GetXMLString( appNode->FirstChildElement( "resourcePath" ), std::string() );

}

/*
  example xml file:

<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<settings version="1">
	<useDumpedBanners>true</useDumpedBanners>
	<useNandBanners>true</useNandBanners>
	<useHomebrewForBanners>true</useHomebrewForBanners>
	<mountDVD>true</mountDVD>
	<sdBannerPath>sd:/banners/</sdBannerPath>
	<resourcePath>sd:/darkwii_orange_No-Spin_4.1U.csm</resourcePath>
</settings>





  */

}
