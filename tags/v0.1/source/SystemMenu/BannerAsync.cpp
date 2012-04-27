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
#include <unistd.h>
#include <stdio.h>
#include <gccore.h>
#include "BannerAsync.h"
#include "contentbin.h"
#include "fileops.h"
#include "nandtitle.h"
#include "TextureConverter.h"

extern const u8 homebrewBanner_bin[];
extern const u32 homebrewBanner_bin_size;

vector<BannerAsync *> BannerAsync::List;
queue<BannerAsync *> BannerAsync::DeleteList;
lwp_t BannerAsync::Thread = LWP_THREAD_NULL;
mutex_t BannerAsync::ListLock = LWP_THREAD_NULL;
BannerAsync * BannerAsync::InUse = NULL;
u32 BannerAsync::ThreadCount = 0;
bool BannerAsync::CloseThread = false;

BannerAsync::BannerAsync()
	: Banner(0, 0)
	, buffer(0)
	, bufferSize(0)
{

}

BannerAsync::BannerAsync(const string &path)
	: Banner(0, 0)
	, filepath(path)
	, buffer(0)
	, bufferSize(0)
{
	ThreadInit();
	ThreadAdd(this);
}

BannerAsync::~BannerAsync()
{
	ThreadRemove(this);
	ThreadExit();
	while(InUse == this)
		usleep(100);

	free( buffer );
}

void BannerAsync::ThreadAdd(BannerAsync *Image)
{
	LWP_MutexLock(ListLock);
	List.push_back(Image);
	LWP_MutexUnlock(ListLock);
	LWP_ResumeThread(Thread);
}

void BannerAsync::ThreadRemove(BannerAsync *Image)
{
	for(u32 i = 0; i < List.size(); ++i)
	{
		if(List[i] == Image)
		{
			LWP_MutexLock(ListLock);
			List.erase(List.begin()+i);
			LWP_MutexUnlock(ListLock);
			break;
		}
	}
}

void BannerAsync::RemoveBanner(BannerAsync *img)
{
	LWP_MutexLock(ListLock);
    DeleteList.push(img);
	LWP_MutexUnlock(ListLock);
	LWP_ResumeThread(Thread);
}

void BannerAsync::ClearQueue()
{
	LWP_MutexLock(ListLock);
	List.clear();
	LWP_MutexUnlock(ListLock);
}

void * BannerAsync::BannerAsyncThread(void *arg)
{
	while(!CloseThread)
	{
		while(!List.empty() && !CloseThread)
		{
			LWP_MutexLock(ListLock);
			while(!DeleteList.empty())
			{
			    delete DeleteList.front();
			    DeleteList.pop();
			}
			if(List.empty())
				break;
			InUse = List.front();
			List.erase(List.begin());
			LWP_MutexUnlock(ListLock);

			if (!InUse )
				continue;


			if( InUse->filepath[ 0 ] == '/' )// cheating...  if the path starts with a '/', assume it is from the nand
			{
				LoadNandBanner( InUse );
			}
			else if( strcasestr( InUse->filepath.c_str(), "/apps/" ) )
			{
				LoadHomebrewBanner( InUse );
			}
			else if( strcasestr( InUse->filepath.c_str(), "content.bin" ) )
			{
				LoadContentBinBanner( InUse );
			}
			else
			{
				LoadBannerFile( InUse );
			}
			InUse = NULL;
		}

		if(!DeleteList.empty())
        {
			LWP_MutexLock(ListLock);
			while(!DeleteList.empty())
			{
			    delete DeleteList.front();
			    DeleteList.pop();
			}
			LWP_MutexUnlock(ListLock);
        }

		LWP_SuspendThread(Thread);
	}

	return NULL;
}

u32 BannerAsync::ThreadInit()
{
	if (Thread == LWP_THREAD_NULL)
	{
		LWP_MutexInit(&ListLock, false);
		LWP_CreateThread(&Thread, BannerAsyncThread, NULL, NULL, 32768, 80);
	}
	return ++ThreadCount;
}

u32 BannerAsync::ThreadExit()
{
	//! We don't need to always shutdown and startup the thread, especially
	//! since this is a nested startup/shutdown from the gui thread.
	//! It's fine with being put to suspended only.
	/*
	if (--ThreadCount == 0)
	{
		CloseThread = true;
		LWP_ResumeThread(Thread);
		LWP_JoinThread(Thread, NULL);
		LWP_MutexUnlock(ListLock);
		LWP_MutexDestroy(ListLock);
		Thread = LWP_THREAD_NULL;
		ListLock = LWP_MUTEX_NULL;
		ListLock = LWP_MUTEX_NULL;
	}
	*/
	return --ThreadCount;
}

bool BannerAsync::LoadBannerFile( BannerAsync *bann )
{
	//gprintf( "BannerAsync::LoadBannerFile( \"%s\" )\n", bann->filepath.c_str() );
	FILE * f = fopen( bann->filepath.c_str(), "rb" );
	if(!f)
	{
		gprintf( "  error opening %s\n", bann->filepath.c_str() );
		return false;
	}

	fseek( f, 0, SEEK_END );
	int size = ftell( f );
	rewind( f );

	u8 *buffer = (u8*)memalign( 32, size );
	if( !buffer )
	{
		return false;
	}
	fread( buffer, 1, size, f );
	fclose( f );

	bann->Load(buffer, size);
	bann->LoadIcon();
	bann->buffer = buffer;
	bann->bufferSize = size;
	return true;
}

bool BannerAsync::LoadNandBanner( BannerAsync *bann )
{
	u8* buffer;
	u32 size;

	if( NandTitle::LoadFileFromNand( bann->filepath.c_str(), &buffer, &size ) )
	{
		gprintf( "error loading: \"%s\"\n", bann->filepath.c_str() );
		return false;
	}
	bann->Load( buffer, size );
	bann->LoadIcon();
	bann->buffer = buffer;
	bann->bufferSize = size;
	return true;
}

bool BannerAsync::LoadContentBinBanner( BannerAsync *bann )
{
	const char *path = bann->filepath.c_str();
	u32 tid32 = *(u32*)( path + 22 );
	FILE *file = fopen( path, "rb" );
	if( !file )
	{
		return false;
	}

	u32 bannerLen;
	u32 installedBytes = 0;
	u8* bannerData = ContentBin::GetIconAsBannerData( file, tid32, bannerLen, installedBytes );
	fclose( file );
	if( !bannerData )
	{
		return false;
	}
	bann->Load( bannerData, bannerLen );
	bann->LoadIcon();
	bann->buffer = bannerData;
	bann->bufferSize = bannerLen;
	return true;
}

bool BannerAsync::LoadHomebrewBanner( BannerAsync *bann )
{
	bann->Load( homebrewBanner_bin, homebrewBanner_bin_size );
	bann->LoadIcon();
	return true;
}

BannerAsyncHB::BannerAsyncHB( const string &path, HomebrewXML *hbXml )
	: xml( hbXml ),
	  rawTexData( NULL )
{
	// not using BannerAsync's second constructor because it creates a race condition where
	// this class gets treated as the base class for a brief second on the working thread until the
	// main thread catches up

	filepath = path;
	ThreadInit();
	ThreadAdd(this);
}

BannerAsyncHB::~BannerAsyncHB()
{
	free( rawTexData );
}

Object *BannerAsyncHB::LoadBanner()
{
	if( !arc || !xml )
		return NULL;

	if( bannerObj )
	{
		return bannerObj;
	}
	u32 arcLen;
	if( !( banner_bin = arc->GetFileAllocated( "/meta/banner.bin", &arcLen ) ) )
	{
		return NULL;
	}

	U8Archive theArc( banner_bin, arcLen );

	// create layout
	if( !( layout_banner = LoadLayout( theArc, "banner" ) ) )
	{
		return NULL;
	}

	// create object
	bannerObj = new Object;
	bannerObj->BindPane( layout_banner->FindPane( "RootPane" ) );
	bannerObj->BindMaterials( layout_banner->Materials() );

	// get animations
	std::string brlanName = "banner_Start";
	Animation *anim = LoadAnimation( theArc, brlanName );

	// we have a starting animation
	if( anim )
	{
		layout_banner->LoadBrlanTpls( anim, theArc );
		bannerBrlans[ brlanName ] = anim;
		bannerObj->AddAnimation( anim );
		bannerObj->SetAnimation( brlanName, 0, -1, -1, false );
		bannerObj->Start();
	}

	brlanName = "banner_Loop";
	anim = LoadAnimation( theArc, brlanName );

	// we have a loop animation
	if( anim )
	{
		layout_banner->LoadBrlanTpls( anim, theArc );
		bannerBrlans[ brlanName ] = anim;
		bannerObj->AddAnimation( anim );
		bannerObj->ScheduleAnimation( brlanName );
		bannerObj->Start();
	}

	// now patch in text from the xml
	SetText( layout_banner, "T_coder", "T_Coded_by", "Line", xml->GetCoder() );
	SetText( layout_banner, "T_release_date", "T_Released", "Line1", xml->GetReleasedate() );
	SetText( layout_banner, "T_version", "T_versiontext", "Line2", xml->GetVersion() );
	SetText( layout_banner, "T_name", NULL, NULL, xml->GetName() );
	SetText( layout_banner, "T_short_descript", NULL, NULL, xml->GetShortDescription() );

	// patch in the png icon
	CreatePngTexture();

	if( rawTexData )
	{
		Texture *tex;
		if( ( tex = layout_banner->FindTexture( "HBPic.tpl" ) ) )
		{
			tex->LoadFromRawData( rawTexData, pngWidth, pngHeight, GX_TF_RGBA8 );
		}

		f32 sc = 1.5f;
		Vec2f v;
		v.x = sc;
		v.y = sc;

		Pane *pane;
		if( (pane = layout_banner->FindPane( "HBPic" ) ) )
		{
			pane->SetScale( v );
		}
		if( (pane = layout_banner->FindPane( "HBPicSha" ) ) )
		{
			pane->SetScale( v );
		}
	}
	else
	{
		// theres no icon.png, so hide some panes
		Pane *pane;
		if( (pane = layout_banner->FindPane( "HBPic" ) ) )
		{
			pane->SetVisible( false );
		}
		if( (pane = layout_banner->FindPane( "N_HBPicSha" ) ) )
		{
			pane->SetVisible( false );
		}
	}

	return bannerObj;
}

Object *BannerAsyncHB::LoadIcon()
{
	if(!arc)
		return NULL;

	if( iconObj )
	{
		return iconObj;
	}

	u32 arcLen;
	if( !( icon_bin = arc->GetFileAllocated( "/meta/icon.bin", &arcLen ) ) )
	{
		return NULL;
	}

	U8Archive theArc( icon_bin, arcLen );

	// create layout
	if( !( layout_icon = LoadLayout( theArc, "icon" ) ) )
	{
		return NULL;
	}

	// create object
	iconObj = new Object;
	iconObj->BindPane( layout_icon->FindPane( "RootPane" ) );
	iconObj->BindMaterials( layout_icon->Materials() );

	// get animations
	std::string brlanName = "icon";
	Animation *anim = LoadAnimation( theArc, brlanName );

	// we have a loop animation
	if( anim )
	{
		layout_icon->LoadBrlanTpls( anim, theArc );
		iconBrlans[ brlanName ] = anim;
		iconObj->AddAnimation( anim );
		iconObj->ScheduleAnimation( brlanName );
        if((u32)anim->FrameCount() != 0) // avoid division by 0 -> crash
            iconObj->SetFrame( ((u32)rand()) % ((u32)anim->FrameCount()) );
		iconObj->Start();
	}

	// patch in the png icon
	CreatePngTexture();

	if( rawTexData )
	{
		Texture *tex;
		if( ( tex = layout_icon->FindTexture( "Iconpng.tpl" ) ) )
		{
			tex->LoadFromRawData( rawTexData, pngWidth, pngHeight, GX_TF_RGBA8 );
		}
	}
	else
	{
		// theres no icon.png, so turn that material into another generic "homebrew" one
		Material *mat;
		if( (mat = layout_icon->FindMaterial( "P_Iconpng" ) ) )
		{
			mat->SetTextureIndex( 0, 2 );
		}
	}


	return iconObj;
}

void BannerAsyncHB::SetText( Layout *lyt, const char* tboxName, const char *tboxName2, const char *linePane, const char16 *txt )
{
	Textbox *tbox = lyt->FindTextbox( tboxName );
	Pane *pane = NULL;
	if( !tbox )
	{
		return;
	}
	if( !txt || !txt[ 0 ] )
	{
		tbox->SetVisible( false );
		if( tboxName2 && ( pane = lyt->FindPane( tboxName2 ) ) )
		{
			pane->SetVisible( false );
		}
		if( linePane && ( pane = lyt->FindPane( linePane ) ) )
		{
			pane->SetVisible( false );
		}
		return;
	}
	tbox->SetText( txt );
}

void BannerAsyncHB::CreatePngTexture()
{
	if( rawTexData )
	{
		return;
	}
	// load png
	char path[ 0x80 ];
	snprintf( path, sizeof( path ), "%s/icon.png", filepath.c_str() );

	u8* pngData = NULL;
	u32 pngLen;
	if( LoadFileToMem( path, &pngData, &pngLen ) < 1 )
	{
		return;
	}

	// convert png to raw data
	gdImagePtr gdImg = gdImageCreateFromPngPtr( pngLen, pngData );
	if( !gdImg )
	{
		free( pngData );
		return;
	}

	rawTexData = GDImageToRGBA8( &gdImg, &pngWidth, &pngHeight );
	gdImageDestroy( gdImg );

	// done with the png
	free( pngData );
}
