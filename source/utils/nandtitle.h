#ifndef NANDTITLE_H
#define NANDTITLE_H

#include <gccore.h>
#include <ogcsys.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <map>

#include "char16.h"

using namespace std;

#define TITLE_ID(x,y)	   (((u64)(x) << 32) | (y))
#define TITLE_UPPER(x)	  ((u32)((x) >> 32))
#define TITLE_LOWER(x)	  ((u32)(x))

#define TITLE_1(x)	  ((u8)((x) >> 8))
#define TITLE_2(x)	  ((u8)((x) >> 16))
#define TITLE_3(x)	  ((u8)((x) >> 24))
#define TITLE_4(x)	  ((u8)((x) >> 32))
#define TITLE_5(x)	  ((u8)((x) >> 40))
#define TITLE_6(x)	  ((u8)((x) >> 48))
#define TITLE_7(x)	  ((u8)((x) >> 56))

#define IMET_MAX_NAME_LEN 0x15

#define IMET_OFFSET					 0x40
#define IMET_SIGNATURE		  0x494d4554
#define DOWNLOADED_CHANNELS	 0x00010001
#define SYSTEM_CHANNELS		 0x00010002
#define RF_NEWS_CHANNEL		 0x48414741
#define RF_FORECAST_CHANNEL	 0x48414641

struct ImetName
{
	char16 title[ IMET_MAX_NAME_LEN ];
	char16 subtitle[ IMET_MAX_NAME_LEN ];
}__attribute__(( packed ));

struct IMET
{
	u32 sig; // "IMET"
	u32 unk1;
	u32 unk2;
	u32 filesizes[3];
	u32 unk3;
	ImetName names[ 10 ];
	// jap, eng, ger, fre, spa, ita, ned, schin, tchin, kor

	u8 zeroes2[0x24c];
	u8 md5[0x10];
} __attribute__(( packed ));

class NandTitle
{
public:
	NandTitle();
	~NandTitle();

	s32 Get();
	u64 At(u32 i);
	int IndexOf(u64 tid);
	u32 Count()
	{
		return titleIds.size();
	}

	u16 VersionOf(u64 tid);
	u16 VersionFromIndex(u32 i);

	u32 CountType(u32 type);

	u32 SetType(u32 upper);
	u64 Next();
	void ResetCounter();

	void AsciiTID(u64 tid, char* out);
	void AsciiFromIndex(u32 i, char* out);

	bool Exists(u64 tid);
	bool ExistsFromIndex(u32 i);

	u64 FindU64(const char *s);
	u64 FindU32(const char *s);

	s32 GetTicketViews(u64 tid, tikview **outbuf, u32 *outlen);

	u64 operator[](u32 i) { return At(i); }

	static int LoadFileFromNand(const char *filepath, u8 **outbuffer, u32 *outfilesize);
	static int ExtractFile(const char *nandPath, const char *filepath);
	static int ExtractDir(const char *wiipath, const char *filepath);
	tmd* GetTMD(u64 tid);
private:
	static int InternalExtractDir(char *nandPath, std::string &filepath);

	std::vector<u64> titleIds;

	u32 currentIndex;
	u32 currentType;
};

extern NandTitle NandTitles;

#endif // NANDTITLE_H
