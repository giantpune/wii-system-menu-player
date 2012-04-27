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
#include <gctypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "tinyxml/tinyxml.h"


#include "fileops.h"
#include "gecko.h"

#include "homebrewbanner.h"

//#define ENTRIE_SIZE	 8192

HomebrewXML::HomebrewXML()
	: Name( NULL ),
	  Coder( NULL ),
	  Version( NULL ),
	  Releasedate( NULL ),
	  ShortDescription( NULL ),
	  LongDescription( NULL )
{

}

HomebrewXML::HomebrewXML( const char* filename )
	: Name( NULL ),
	  Coder( NULL ),
	  Version( NULL ),
	  Releasedate( NULL ),
	  ShortDescription( NULL ),
	  LongDescription( NULL )
{
	LoadHomebrewXMLData( filename );
}

HomebrewXML::~HomebrewXML()
{
	free( Name );
	free( Coder );
	free( Version );
	free( Releasedate );
	free( ShortDescription );
	free( LongDescription );
}

int HomebrewXML::LoadHomebrewXMLData( const char* filename )
{
	FREE( Name );
	FREE( Coder );
	FREE( Version );
	FREE( Releasedate );
	FREE( ShortDescription );
	FREE( LongDescription );
	Arguments.clear();

	TiXmlDocument xmlDoc(filename);
	if(!xmlDoc.LoadFile())
		return 0;

	TiXmlElement *appNode =  xmlDoc.FirstChildElement("app");
	if( !appNode )
	{
		return 0;
	}

	TiXmlElement *node = NULL;

	node = appNode->FirstChildElement("name");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		Name = strdup16( node->FirstChild()->Value() );

	node = appNode->FirstChildElement("coder");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		Coder = strdup16( node->FirstChild()->Value() );

	node = appNode->FirstChildElement("version");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		Version = strdup16( node->FirstChild()->Value() );

	node = appNode->FirstChildElement("short_description");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		ShortDescription = strdup16( node->FirstChild()->Value() );

	node = appNode->FirstChildElement("long_description");
	if(node && node->FirstChild() && node->FirstChild()->Value())
		LongDescription = strdup16( node->FirstChild()->Value() );

	char ReleaseText[ 20 ];
	u32 len = 0;
	memset(ReleaseText, 0, sizeof(ReleaseText));

	const char *dNode = NULL;

	node = appNode->FirstChildElement("release_date");
	if( node && node->FirstChild() && ( dNode = node->FirstChild()->Value() ) )
	{
		len = strlen( dNode );

		// check if its aready represented as a date with '/'
		bool done = false;
		for( u32 i = 0; i < len && !done; i++ )
		{
			if( ReleaseText[ i ] == '/' )
			{
				done = true;
			}
		}

		if( !done && ( len == 14 || len == 8 ) )
		{
			snprintf(ReleaseText, sizeof(ReleaseText), "%.2s/%.2s/%.4s", &dNode[4], &dNode[6], &dNode[0] );
			Releasedate = strdup16( ReleaseText );
		}
		else if( !done && len == 12 )
		{
			snprintf(ReleaseText, sizeof(ReleaseText), "%.2s/%.4s", &dNode[4], &dNode[0] );
			Releasedate = strdup16( ReleaseText );
		}
		else
		{
			Releasedate = strdup16( dNode );
		}
	}

	node = appNode->FirstChildElement("arguments");
	if(!node)
		return 1;

	TiXmlElement *argNode = node->FirstChildElement("arg");

	while(argNode)
	{
		if(argNode->FirstChild() && argNode->FirstChild()->Value())
			Arguments.push_back(std::string(argNode->FirstChild()->Value()));

		argNode = argNode->NextSiblingElement();
	}

	return 1;
}

/* Set Name */
void HomebrewXML::SetName( const char * newName )
{
	free( Name );
	Name = strdup16( newName );
}

