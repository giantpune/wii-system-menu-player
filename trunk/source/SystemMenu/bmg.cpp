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
#include "bmg.h"
#include "utils/crc32.h"
#include "gecko.h"

// functions for converting indexes from 514 to other system menu versions
u16 MapBmg_97( u16 idx );
u16 MapBmg_128( u16 idx );
u16 MapBmg_224( u16 idx );
u16 MapBmg_288( u16 idx );
u16 MapBmg_384( u16 idx );
u16 MapBmg_416( u16 idx );
u16 MapBmg_418( u16 idx );
u16 MapBmg_512( u16 idx );

Bmg *Bmg::instance = NULL;
Bmg::Bmg( const u8* stuff, u32 len )
{
	if( stuff )
	{
		SetResource( stuff, len );
	}
}

bool Bmg::SetResource( const u8* stuff, u32 len )
{
	Reset();
	if( !stuff || len < 0x40 )
	{
		gprintf( "data is too small\n" );
		return false;
	}

	// i didnt see the system menu checking the header, but it cant be a bad idea to take a peek at it
	bmgHeader = (BmgHeader*)stuff;
	if( bmgHeader->magic != 0x4d455347626d6731ull || bmgHeader->size < len || bmgHeader->sectionCnt != 2 )
	{
		gprintf( "bad file\n" );
		Reset();
		return false;
	}

	u32 r8 = 0x20;
	for( u32 i = 0; i < bmgHeader->sectionCnt; i++ )
	{
		u32 magic = *(u32*)( stuff + r8 );
		u32 sectionSize = *(u32*)( stuff + r8 + 4 );

		if( sectionSize + r8 > len )
		{
			gprintf( "bad file 2  %08x %08x %08x\n", sectionSize, r8, len );
			hexdump( stuff, 0x40 );
			Reset();
			return false;
		}

		if( magic == 0x494E4631 )// INF1
		{
			inf1Header = (Inf1Header*)( stuff + r8 );
		}
		else if( magic == 0x44415431 )// DAT1
		{
			dat1Header = (Dat1Header*)( stuff + r8 );
		}
		r8 += sectionSize;
	}
	if( !inf1Header || !dat1Header )
	{
		gprintf( "failed to find the sections\n" );
		Reset();
		return false;
	}
	if( inf1Header->size <
			inf1Header->numMessages *
			(u32)inf1Header->entrySize )// cast here because g++ issues a wrong warning about signed vs unsigned when using u16
	{
		gprintf( "bad INF1 header\n" );
		Reset();
		return false;
	}

	// set index resolver
	u32 crc = Crc32( stuff, len );
	switch( crc )
	{
	case 0x44b151d6: // "message/eng/ipl_common.bmg"
	case 0xa725e9c2: // "message/fra/ipl_common.bmg"
	case 0xcb51cf76: // "message/ger/ipl_common.bmg"
	case 0x9593aed0: // "message/ita/ipl_common.bmg"
	case 0x51aaecee: // "message/jpn/ipl_common.bmg"
	case 0x08d5a95f: // "message/ned/ipl_common.bmg"
	case 0xf3dee08b: // "message/spa/ipl_common.bmg"
		IndexResolver = MapBmg_97; break;
	case 0xe839bb11: // "message/eng/ipl_common.bmg"
	case 0x80a8bed1: // "message/fra/ipl_common.bmg"
	case 0xbff3aefb: // "message/ger/ipl_common.bmg"
	case 0x99357f87: // "message/ita/ipl_common.bmg"
	case 0x80b9b19c: // "message/jpn/ipl_common.bmg"
	case 0x80f5c8a1: // "message/ned/ipl_common.bmg"
	case 0x9898bf1b: // "message/spa/ipl_common.bmg"
		IndexResolver = MapBmg_128; break;
	case 0x8ad22a50: // "message/eng/ipl_common.bmg"
	case 0x7cc25c85: // "message/fra/ipl_common.bmg"
	case 0xe4cc617f: // "message/ger/ipl_common.bmg"
	case 0xf74a70b2: // "message/ita/ipl_common.bmg"
	case 0x90246050: // "message/jpn/ipl_common.bmg"
	case 0x161f9460: // "message/ned/ipl_common.bmg"
	case 0xde5098b2: // "message/spa/ipl_common.bmg"
		IndexResolver = MapBmg_224; break;
	case 0xc2883d99: // "message/eng/ipl_common.bmg"
	case 0x0a15ad8a: // "message/fra/ipl_common.bmg"
	case 0x56afc607: // "message/ger/ipl_common.bmg"
	case 0xca2b2c12: // "message/ita/ipl_common.bmg"
	case 0x3ca42b32: // "message/jpn/ipl_common.bmg"
	case 0x0c5cdb5e: // "message/ned/ipl_common.bmg"
	case 0xf7e8cff5: // "message/spa/ipl_common.bmg"
		IndexResolver = MapBmg_288; break;
	case 0x59d9b62d: // "message/eng/ipl_common.bmg"
	case 0x93d2b987: // "message/fra/ipl_common.bmg"
	case 0x528e96c2: // "message/ger/ipl_common.bmg"
	case 0x4887b884: // "message/ita/ipl_common.bmg"
	case 0x0c114af0: // "message/jpn/ipl_common.bmg"
	case 0xcf05006d: // "message/ned/ipl_common.bmg"
	case 0x09c0d18e: // "message/spa/ipl_common.bmg"
		IndexResolver = MapBmg_384; break;
	case 0x210b7261: // "message/eng/ipl_common.bmg"
	case 0x2f48b22b: // "message/fra/ipl_common.bmg"
	case 0x8b1a4e55: // "message/ger/ipl_common.bmg"
	case 0xdc902e36: // "message/ita/ipl_common.bmg"
	case 0x8478bf4b: // "message/jpn/ipl_common.bmg"
	case 0x1e86c53e: // "message/ned/ipl_common.bmg"
	case 0x1a7796d6: // "message/spa/ipl_common.bmg"
		IndexResolver = MapBmg_416; break;
	case 0x65fc7883: // "message/eng/ipl_common_noe.bmg"
	case 0x1b0ac940: // "message/fra/ipl_common_noe.bmg"
//	case 0x8b1a4e55: // "message/ger/ipl_common.bmg"
//	case 0xdc902e36: // "message/ita/ipl_common.bmg"
//	case 0x8478bf4b: // "message/jpn/ipl_common.bmg"
//	case 0x1e86c53e: // "message/ned/ipl_common.bmg"
	case 0x32ed6cc4: // "message/spa/ipl_common_noe.bmg"
		IndexResolver = MapBmg_418; break;
	case 0x42a976fa: // "message/eng/ipl_common.bmg"
	case 0xf5267aad: // "message/fra/ipl_common.bmg"
	case 0x3f3bdd44: // "message/ger/ipl_common.bmg"
	case 0x153a3265: // "message/ita/ipl_common.bmg"
	case 0x236dd768: // "message/jpn/ipl_common.bmg"
	case 0x584e9135: // "message/ned/ipl_common.bmg"
	case 0x098b386a: // "message/spa/ipl_common.bmg"
		IndexResolver = MapBmg_512; break;
	case 0x6854cacf: // "message/eng/ipl_common_noe.bmg"
	case 0x41f2bcee: // "message/fra/ipl_common_noe.bmg"
//	case 0x3f3bdd44: // "message/ger/ipl_common.bmg"
//	case 0x153a3265: // "message/ita/ipl_common.bmg"
//	case 0x236dd768: // "message/jpn/ipl_common.bmg"
//	case 0x584e9135: // "message/ned/ipl_common.bmg"
	case 0x8cc2b58a: // "message/spa/ipl_common_noe.bmg"
		break;
	default:
//		gprintf( "unrecognized bmg file.  no index resolver is being used %08x\n", crc );
		break;
	}

	return true;
}

void Bmg::Reset()
{
	bmgHeader = NULL;
	inf1Header = NULL;
	dat1Header = NULL;
	IndexResolver = NULL;
}

const char16* Bmg::GetMessage( u16 index ) const
{
	if( IndexResolver )
	{
		//gprintf( "converting message %u to ", index );
		index = IndexResolver( index );
		//gprintf( "%u\n", index );
	}
	if( !bmgHeader || index >= inf1Header->numMessages )
	{
		return NULL;
	}
	u32 offset = *(u32*)(((u8*)inf1Header) + sizeof( Inf1Header ) + ( index * inf1Header->entrySize ));
	if( offset >= dat1Header->size )
	{
		gprintf( "index out of range\n" );
		return NULL;
	}
	return (char16*)( ((u8*)dat1Header) + sizeof( Dat1Header ) + offset );
}

u16 MapBmg_97( u16 idx )
{
	switch( idx )
	{
		case 0 ... 21: return idx;
		//case 22: return ??;	// '"Channel Time Remaining:"'
		//case 23: return ??;	// '"Less than a minute"'
		case 24: return 102;
		case 25: return 3;
		//case 26: return ??;	// '"Your licence for this Channel has expired."'
		//case 27: return ??;	// '"The system is operating in maintenance mode. The Wii Message Board cannot be used."'
		case 28: return 3;
		//case 29: return ??;	// '"The period of use for this Channel has ended."'
		//case 30: return ??;	// '"Press the A Button."'
		case 31: return 102;
		case 32: return 3;
		//case 33: return ??;	// '"This cannot be used in maintenance mode."'
		//case 34: return ??;	// '"Photos can be sent only to other Wii consoles."'
		case 35: return 22;
		case 36: return 23;
		case 37: return 24;
		case 38: return 25;
		case 39: return 26;
		case 40: return 27;
		case 41: return 28;
		case 42: return 29;
		case 43: return 30;
		case 44: return 31;
		case 45: return 32;
		case 46: return 33;
		case 47: return 34;
		//case 48: return ??;	// '"Erase this data?"'
		case 49: return 31;
		case 50: return 32;
		case 51: return 26;
		case 52: return 39;
		case 53: return 40;
		case 54: return 41;
		case 55: return 42;
		case 56: return 43;
		case 57: return 44;
		case 58: return 45;
		case 59: return 46;
		case 60: return 47;
		case 61: return 48;
		case 62: return 49;
		case 63: return 50;
		case 64: return 35;
		case 65: return 52;
		case 66: return 53;
		case 67: return 54;
		case 68: return 55;
		case 69: return 56;
		case 70: return 57;
		case 71: return 58;
		case 72: return 59;
		//case 73: return ??;	// '"Enter a nickname."'
		case 74: return 61;
		case 75: return 62;
		case 76: return 63;
		case 77: return 64;
		case 78: return 65;
		case 79: return 22;
		case 80: return 67;
		case 81: return 68;
		case 82: return 69;
		case 83: return 70;
		case 84: return 71;
		case 85: return 72;
		case 86: return 73;
		//case 87: return ??;	// '"Registration to be confirmed. You must register one another to be able to exchange messages."'
		//case 88: return ??;	// '"This will launch the Internet Channel. OK?"'
		//case 89: return ??;	// '"You do not have the Internet Channel."'
		//case 90: return ??;	// '"Unable to jump using your current Internet Channel."'
		//case 91: return ??;	// '"You do not have this Channel."'
		//case 92: return ??;	// '"Opt Out"'
		//case 93: return ??;	// '"Opt out of:"'
		//case 94: return ??;	// '"This message service"'
		//case 95: return ??;	// '"All message services"'
		//case 96: return ??;	// '"Cancel"'
		case 97: return 249;
		//case 98: return ??;	// '"You have chosen to opt out of this Channel's message service."'
		//case 99: return ??;	// '"You have chosen to opt out of message services for all Channels and Games."'
		//case 100: return ??;	// '"There is a problem with WiiConnect24. Transmission could not be halted. Please try again later."'
		//case 101: return ??;	// '"Opt Out"'
		//case 102: return ??;	// '"Opt out of:"'
		//case 103: return ??;	// '"This message service"'
		//case 104: return ??;	// '"All message services"'
		//case 105: return ??;	// '"Cancel"'
		//case 106: return ??;	// '"Are you sure ?"'
		//case 107: return ??;	// '"You have chosen to opt out of this Channel's message service."'
		//case 108: return ??;	// '"You have chosen to opt out of message services for all Channels and Games."'
		case 109: return 3;
		case 110: return 74;
		case 111: return 75;
		case 112: return 76;
		case 113: return 77;
		case 114: return 78;
		case 115: return 79;
		case 116: return 80;
		case 117: return 3;
		case 118: return 82;
		case 119: return 83;
		case 120: return 84;
		case 121: return 85;
		case 122: return 86;
		case 123: return 87;
		case 124: return 88;
		case 125: return 89;
		case 126: return 90;
		case 127: return 91;
		case 128: return 92;
		case 129: return 93;
		case 130: return 3;
		case 131: return 3;
		case 132: return 96;
		case 133: return 97;
		case 134: return 98;
		case 135: return 99;
		case 136: return 100;
		case 137: return 57;
		case 138: return 102;
		//case 139: return ??;	// '"← Add a Mii"'
		case 140: return 104;
		case 141: return 105;
		case 142: return 106;
		//case 143: return ??;	// '"Sending message..."'
		//case 144: return ??;	// '"Messages Sent:"'
		//case 145: return ??;	// '"And"'
		//case 146: return ??;	// '"more"'
		case 147: return 107;
		case 148: return 108;
		case 149: return 109;
		case 150: return 110;
		case 151: return 111;
		case 152: return 112;
		case 153: return 113;
		case 154: return 114;
		case 155: return 113;
		case 156: return 116;
		//case 157: return ??;	// '"Welcome to the SD Card Menu! Here, you can launch Channels saved on SD Cards by temporarily utilising the Wii System Memory."'
		//case 158: return ??;	// '"About Save Data Save data cannot be loaded directly from SD Cards. To use save data stored on SD Cards, first move it to the Wii System Memory."'
		//case 159: return ??;	// '"Caution! When you see this icon appear, do not remove the SD Card or turn off the power on the Wii console."'
		//case 160: return ??;	// '"SD Card Menu"'
		//case 161: return ??;	// '"SD Card Menu"'
		//case 162: return ??;	// '"SD Card Menu"'
		//case 163: return ??;	// '"Next"'
		//case 164: return ??;	// '"Close"'
		case 165: return 22;
		//case 166: return ??;	// '"About the SD Card Menu"'
		case 167: return 117;
		case 168: return 1;
		case 169: return 205;
		//case 170: return ??;	// '"Loading from the SD Card..."'
		case 171: return 43;
		//case 172: return ??;	// '"Launch this Channel?"'
		//case 173: return ??;	// '"Launching..."'
		//case 174: return ??;	// '"Failed to launch."'
		//case 175: return ??;	// '"This Channel cannot be launched on this Wii console."'
		//case 176: return ??;	// '"This Channel could not be launched. Manage your Wii System Memory to launch this Channel."'
		//case 177: return ??;	// '"Moving data to the SD Card..."'
		case 178: return 118;
		//case 179: return ??;	// '"Close the SD Card Menu and open the Data Management Screen in Wii Options? This requires **** blocks in the Wii System Memory."'
		//case 180: return ??;	// '"There are no Channels that can be automatically managed. Use the Data Management Screen in Wii Options to free up **** blocks of space."'
		//case 181: return ??;	// '"There is not enough free space on the SD Card. Please manage the SD Card to free up additional space."'
		//case 182: return ??;	// '"Please select the criteria for Channels the Wii console will move to an SD Card."'
		//case 183: return ??;	// '"Not Launched Recently"'
		//case 184: return ??;	// '"Right side of the Wii Menu"'
		//case 185: return ??;	// '"Lots of Blocks"'
		//case 186: return ??;	// '"A Few Blocks"'
		//case 187: return ??;	// '"Auto Manage"'
		//case 188: return ??;	// '"Use Data Management"'
		case 189: return 34;
		//case 190: return ??;	// '"The Channels shown below have been selected. Move them to an SD Card?"'
		case 191: return 49;
		//case 192: return ??;	// '"There is still not enough available space in Wii System Memory. Please manage data on the Data Management Screen in Wii Options."'
		//case 193: return ??;	// '"There are so many Channels stored on this SD Card that not all of them can be displayed on the SD Card Menu."'
		//case 194: return ??;	// '"This SD Card must be formatted."'
		case 195: return 111;
		//case 196: return ??;	// '"Next"'
		case 197: return 120;
		//case 198: return ??;	// '"This SD Card is write-protected. No changes can be made to the order of Channels."'
		//case 199: return ??;	// '"This Channel cannot be launched because there is a newer version available."'
		//case 200: return ??;	// '"Continue"'
		//case 201: return ??;	// '"About the SD Card Menu On the SD Card Menu, you can temporarily utilise the Wii System Memory to launch a Channel stored on an SD Card."'
		//case 202: return ??;	// '"To view this information again, go to the SD Card Menu and select the icon shown here."'
		//case 203: return ??;	// '"(Blue Channels will be replaced on the SD Card with the ones stored on your Wii Console.)"'
		//case 204: return ??;	// '"Searching..."'
		//case 205: return ??;	// '"This data cannot be moved."'
		case 206: return 122;
		case 207: return 123;
		case 208: return 124;
		case 209: return 125;
		case 210: return 126;
		case 211: return 127;
		case 212: return 128;
		case 213: return 129;
		case 214: return 130;
		//case 215: return ??;	// '"The data may not have been moved."'
		case 216: return 132;
		case 217: return 133;
		case 218: return 134;
		case 219: return 135;
		case 220: return 124;
		case 221: return 125;
		case 222: return 126;
		case 223: return 127;
		case 224: return 140;
		case 225: return 141;
		case 226: return 142;
		//case 227: return ??;	// '"The data may not have been copied."'
		case 228: return 144;
		//case 229: return ??;	// '"The data may not have been erased."'
		case 230: return 146;
		case 231: return 147;
		case 232: return 124;
		case 233: return 125;
		case 234: return 126;
		case 235: return 127;
		case 236: return 152;
		case 237: return 153;
		case 238: return 154;
		case 239: return 154;
		case 240: return 156;
		//case 241: return ??;	// '"The Memory Card may not have been formatted."'
		case 242: return 3;
		case 243: return 159;
		case 244: return 160;
		case 245: return 161;
		case 246: return 162;
		case 247: return 34;
		//case 248: return ??;	// '"The data has been erased."'
		case 249: return 165;
		case 250: return 166;
		case 251: return 167;
		case 252: return 22;
		case 253: return 169;
		case 254: return 170;
		case 255: return 171;
		case 256: return 64;
		case 257: return 173;
		case 258: return 118;
		case 259: return 117;
		case 260: return 34;
		case 261: return 177;
		case 262: return 178;
		case 263: return 179;
		case 264: return 180;
		case 265: return 181;
		case 266: return 182;
		case 267: return 183;
		case 268: return 184;
		case 269: return 185;
		case 270: return 186;
		case 271: return 48;
		case 272: return 188;
		case 273: return 189;
		case 274: return 190;
		case 275: return 191;
		//case 276: return ??;	// '"Moving data to the Wii System Memory. Do not remove the SD Card or power off your Wii console."'
		case 277: return 193;
		case 278: return 194;
		case 279: return 195;
		case 280: return 196;
		case 281: return 197;
		case 282: return 198;
		case 283: return 199;
		case 284: return 133;
		case 285: return 201;
		case 286: return 202;
		case 287: return 203;
		case 288: return 204;
		case 289: return 205;
		case 290: return 111;
		//case 291: return ??;	// '"Some data could not be copied. For details, please refer to the game's Instruction Booklet."'
		case 292: return 208;
		case 293: return 209;
		case 294: return 210;
		case 295: return 211;
		case 296: return 212;
		case 297: return 213;
		//case 298: return ??;	// '"The data has been copied. This includes some data that cannot be used on other Wii consoles. Please refer to the game's Instruction Booklet for details."'
		//case 299: return ??;	// '"Data has been copied. There was some data that could not be copied to an SD Card. Refer to the game's Instruction Booklet for details."'
		case 300: return 216;
		//case 301: return ??;	// '"The data has been copied. This data cannot be copied to another Wii console. See the operations guide for more information."'
		//case 302: return ??;	// '"This data cannot be copied to this Wii console."'
		//case 303: return ??;	// '"SD Card Menu Data"'
		//case 304: return ??;	// '"This Channel cannot be moved to this Wii console."'
		//case 305: return ??;	// '"This file cannot be moved, but it can be copied."'
		//case 306: return ??;	// '"The data has been moved. This includes some data that cannot be used on other Wii consoles. See the game's Instruction Booklet for details."'
		//case 307: return ??;	// '"Some of the data could not be moved. See the game's Instruction Booklet for details."'
		//case 308: return ??;	// '"The data has been moved. However, it cannot be moved to another Wii console. See the game's Instruction Booklet for details."'
		//case 309: return ??;	// '"This data cannot be moved to this Wii console."'
		//case 310: return ??;	// '"You must first play this game on your Wii Console to move save data."'
		//case 311: return ??;	// '"This data can't be moved because a portion of it can't be copied."'
		//case 312: return ??;	// '"This Channel cannot be copied because there is a newer version available."'
		//case 313: return ??;	// '"This Channel cannot be moved because there is a newer version available."'
		//case 314: return ??;	// '"Erasing... Do not remove the SD Card or power off your Wii console."'
		case 315: return 22;
		case 316: return 218;
		case 317: return 64;
		case 318: return 220;
		case 319: return 221;
		case 320: return 222;
		case 321: return 223;
		case 322: return 224;
		case 323: return 225;
		case 324: return 226;
		case 325: return 227;
		case 326: return 228;
		case 327: return 229;
		case 328: return 230;
		case 329: return 231;
		case 330: return 232;
		case 331: return 233;
		case 332: return 234;
		//case 333: return ??;	// '"Enter your IP Address."'
		//case 334: return ??;	// '"Enter your Primary DNS."'
		//case 335: return ??;	// '"Enter your Secondary DNS."'
		//case 336: return ??;	// '"Enter the Subnet Mask."'
		//case 337: return ??;	// '"Enter your Gateway."'
		//case 338: return ??;	// '"Enter your SSID."'
		//case 339: return ??;	// '"Enter the key."'
		//case 340: return ??;	// '"Enter the name of your proxy server."'
		//case 341: return ??;	// '"Enter the port number."'
		//case 342: return ??;	// '"Enter your user name."'
		//case 343: return ??;	// '"Enter your password."'
		//case 344: return ??;	// '"Enter your MTU value."'
		//case 345: return ??;	// '"Enter your PIN."'
		//case 346: return ??;	// '"Enter the answer to your secret question."'
		//case 347: return ??;	// '"Enter the master key."'
		//case 348: return ??;	// '"Enter a nickname for your Wii."'
		//case 349: return ??;	// '"Enter the mail address."'
		//case 350: return ??;	// '"Enter a nickname."'
		//case 351: return ??;	// '"Remember, if you forget your PIN or the answer to your secret question, you will be unable to remove Parental Controls restrictions."'
		//case 352: return ??;	// '"If you forget your PIN and the answer to your secret question, you won't be able to remove Parental Controls restrictions."'
		case 353: return 235;
		case 354: return 236;
		case 355: return 237;
		case 356: return 238;
		//case 357: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update."'
		case 358: return 240;
		case 359: return 241;
		case 360: return 242;
		case 361: return 243;
		case 362: return 244;
		case 363: return 245;
		case 364: return 246;
		case 365: return 247;
		case 366: return 248;
		case 367: return 249;
		case 368: return 250;
		case 369: return 251;
		case 370: return 252;
		case 371: return 253;
		//case 372: return ??;	// '"To use WiiConnect24 and the Wii Shop Channel, please confirm the Wii Network Services Agreement after configuring Internet settings."'
		//case 373: return ??;	// '"Please review the Wii Network Services Agreement in Wii Settings to use the Wii Shop Channel or WiiConnect24."'
		//case 374: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 375: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 376: return ??;	// '"I Accept"'
		//case 377: return ??;	// '"I Accept"'
		//case 378: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. Failure to accept this update may also render this and future games un"'
		//case 379: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update. Please note that this update will add new Channels to the Wii Menu."'
		case 380: return 254;
		case 381: return 255;
		case 382: return 256;
		case 383: return 257;
		case 384: return 258;
		case 385: return 259;
		case 386: return 260;
		case 387: return 261;
		case 388 ... 399: return idx;
		case 400: return 274;
		case 401: return 275;
		case 402: return 276;
		case 403: return 277;
		case 404: return 278;
		case 405: return 279;
		case 406: return 280;
		case 407: return 281;
		case 408: return 282;
		case 409: return 283;
		case 410: return 284;
		case 411: return 285;
		case 412: return 285;
		case 413: return 285;
		case 414: return 275;
		case 415: return 289;
		case 416: return 290;
		case 417: return 275;
		case 418: return 292;
		case 419: return 293;
		case 420: return 294;
		case 421: return 275;
		case 422: return 276;
		case 423: return 277;
		case 424: return 278;
		case 425: return 279;
		case 426: return 280;
		case 427: return 281;
		case 428: return 282;
		case 429: return 283;
		case 430: return 284;
		case 431: return 285;
		case 432: return 285;
		case 433: return 285;
		case 434: return 275;
		case 435: return 289;
		case 436: return 290;
		case 437: return 275;
		case 438: return 292;
		case 439: return 293;
		case 440: return 294;
		case 441: return 315;
		case 442: return 316;
		//case 443: return ??;	// '"The answer to your secret question must be at least six characters."'
		case 444: return 318;
		case 445: return 319;
		case 446: return 320;
		//case 447: return ??;	// '"Aim the Wii Remote at the TV. Press  or  to adjust sensitivity until only two blinking lights appear."'
		case 448: return 322;
		case 449: return 323;
		//case 450: return ??;	// '"AOSS setup failed. Move closer or make sure the access point is powered on. Then try again. For help, visit support.nintendo.com"'
		case 451: return idx;
		case 452: return 326;
		case 453: return 327;
		case 454: return 328;
		//case 455: return ??;	// '"The action you wish to perform cannot be completed at this time. Please try again later."'
		case 456: return 330;
		//case 457: return ??;	// '"Be sure to set your TV to display in standard format when choosing the Standard (4:3) setting on your Wii."'
	}
	return 0xffff;
}

u16 MapBmg_128( u16 idx )
{
	switch( idx )
	{
		case 0 ... 21: return idx;
		//case 22: return ??;	// '"Channel Time Remaining:"'
		//case 23: return ??;	// '"Less than a minute"'
		case 24: return 102;
		case 25: return 3;
		//case 26: return ??;	// '"Your licence for this Channel has expired."'
		//case 27: return ??;	// '"The system is operating in maintenance mode. The Wii Message Board cannot be used."'
		case 28: return 3;
		//case 29: return ??;	// '"The period of use for this Channel has ended."'
		//case 30: return ??;	// '"Press the A Button."'
		case 31: return 102;
		case 32: return 3;
		//case 33: return ??;	// '"This cannot be used in maintenance mode."'
		//case 34: return ??;	// '"Photos can be sent only to other Wii consoles."'
		case 35: return 22;
		case 36: return 23;
		case 37: return 24;
		case 38: return 25;
		case 39: return 26;
		case 40: return 27;
		case 41: return 28;
		case 42: return 29;
		case 43: return 30;
		case 44: return 31;
		case 45: return 32;
		case 46: return 33;
		case 47: return 34;
		//case 48: return ??;	// '"Erase this data?"'
		case 49: return 31;
		case 50: return 32;
		case 51: return 26;
		case 52: return 39;
		case 53: return 40;
		case 54: return 41;
		case 55: return 42;
		case 56: return 43;
		case 57: return 44;
		case 58: return 45;
		case 59: return 46;
		case 60: return 47;
		case 61: return 48;
		case 62: return 49;
		case 63: return 50;
		case 64: return 35;
		case 65: return 52;
		case 66: return 53;
		case 67: return 54;
		case 68: return 55;
		case 69: return 56;
		case 70: return 57;
		case 71: return 58;
		case 72: return 59;
		//case 73: return ??;	// '"Enter a nickname."'
		case 74: return 61;
		case 75: return 62;
		case 76: return 63;
		case 77: return 64;
		case 78: return 65;
		case 79: return 22;
		case 80: return 67;
		case 81: return 68;
		case 82: return 69;
		case 83: return 70;
		case 84: return 71;
		case 85: return 72;
		case 86: return 73;
		//case 87: return ??;	// '"Registration to be confirmed. You must register one another to be able to exchange messages."'
		//case 88: return ??;	// '"This will launch the Internet Channel. OK?"'
		//case 89: return ??;	// '"You do not have the Internet Channel."'
		//case 90: return ??;	// '"Unable to jump using your current Internet Channel."'
		//case 91: return ??;	// '"You do not have this Channel."'
		//case 92: return ??;	// '"Opt Out"'
		//case 93: return ??;	// '"Opt out of:"'
		//case 94: return ??;	// '"This message service"'
		//case 95: return ??;	// '"All message services"'
		//case 96: return ??;	// '"Cancel"'
		case 97: return 249;
		//case 98: return ??;	// '"You have chosen to opt out of this Channel's message service."'
		//case 99: return ??;	// '"You have chosen to opt out of message services for all Channels and Games."'
		//case 100: return ??;	// '"There is a problem with WiiConnect24. Transmission could not be halted. Please try again later."'
		//case 101: return ??;	// '"Opt Out"'
		//case 102: return ??;	// '"Opt out of:"'
		//case 103: return ??;	// '"This message service"'
		//case 104: return ??;	// '"All message services"'
		//case 105: return ??;	// '"Cancel"'
		//case 106: return ??;	// '"Are you sure ?"'
		//case 107: return ??;	// '"You have chosen to opt out of this Channel's message service."'
		//case 108: return ??;	// '"You have chosen to opt out of message services for all Channels and Games."'
		case 109: return 3;
		case 110: return 74;
		case 111: return 75;
		case 112: return 76;
		case 113: return 77;
		case 114: return 78;
		case 115: return 79;
		case 116: return 80;
		case 117: return 3;
		case 118: return 82;
		case 119: return 83;
		case 120: return 84;
		case 121: return 85;
		case 122: return 86;
		case 123: return 87;
		case 124: return 88;
		case 125: return 89;
		case 126: return 90;
		case 127: return 91;
		case 128: return 92;
		case 129: return 93;
		case 130: return 3;
		case 131: return 3;
		case 132: return 96;
		case 133: return 97;
		case 134: return 98;
		case 135: return 99;
		case 136: return 100;
		case 137: return 57;
		case 138: return 102;
		//case 139: return ??;	// '"← Add a Mii"'
		case 140: return 104;
		case 141: return 105;
		case 142: return 106;
		//case 143: return ??;	// '"Sending message..."'
		//case 144: return ??;	// '"Messages Sent:"'
		//case 145: return ??;	// '"And"'
		//case 146: return ??;	// '"more"'
		case 147: return 107;
		case 148: return 108;
		case 149: return 109;
		case 150: return 110;
		case 151: return 111;
		case 152: return 112;
		case 153: return 113;
		case 154: return 114;
		case 155: return 113;
		case 156: return 116;
		//case 157: return ??;	// '"Welcome to the SD Card Menu! Here, you can launch Channels saved on SD Cards by temporarily utilising the Wii System Memory."'
		//case 158: return ??;	// '"About Save Data Save data cannot be loaded directly from SD Cards. To use save data stored on SD Cards, first move it to the Wii System Memory."'
		//case 159: return ??;	// '"Caution! When you see this icon appear, do not remove the SD Card or turn off the power on the Wii console."'
		//case 160: return ??;	// '"SD Card Menu"'
		//case 161: return ??;	// '"SD Card Menu"'
		//case 162: return ??;	// '"SD Card Menu"'
		//case 163: return ??;	// '"Next"'
		//case 164: return ??;	// '"Close"'
		case 165: return 22;
		//case 166: return ??;	// '"About the SD Card Menu"'
		case 167: return 117;
		case 168: return 1;
		case 169: return 205;
		//case 170: return ??;	// '"Loading from the SD Card..."'
		case 171: return 43;
		//case 172: return ??;	// '"Launch this Channel?"'
		//case 173: return ??;	// '"Launching..."'
		//case 174: return ??;	// '"Failed to launch."'
		//case 175: return ??;	// '"This Channel cannot be launched on this Wii console."'
		//case 176: return ??;	// '"This Channel could not be launched. Manage your Wii System Memory to launch this Channel."'
		//case 177: return ??;	// '"Moving data to the SD Card..."'
		case 178: return 118;
		//case 179: return ??;	// '"Close the SD Card Menu and open the Data Management Screen in Wii Options? This requires **** blocks in the Wii System Memory."'
		//case 180: return ??;	// '"There are no Channels that can be automatically managed. Use the Data Management Screen in Wii Options to free up **** blocks of space."'
		//case 181: return ??;	// '"There is not enough free space on the SD Card. Please manage the SD Card to free up additional space."'
		//case 182: return ??;	// '"Please select the criteria for Channels the Wii console will move to an SD Card."'
		//case 183: return ??;	// '"Not Launched Recently"'
		//case 184: return ??;	// '"Right side of the Wii Menu"'
		//case 185: return ??;	// '"Lots of Blocks"'
		//case 186: return ??;	// '"A Few Blocks"'
		//case 187: return ??;	// '"Auto Manage"'
		//case 188: return ??;	// '"Use Data Management"'
		case 189: return 34;
		//case 190: return ??;	// '"The Channels shown below have been selected. Move them to an SD Card?"'
		case 191: return 49;
		//case 192: return ??;	// '"There is still not enough available space in Wii System Memory. Please manage data on the Data Management Screen in Wii Options."'
		//case 193: return ??;	// '"There are so many Channels stored on this SD Card that not all of them can be displayed on the SD Card Menu."'
		//case 194: return ??;	// '"This SD Card must be formatted."'
		case 195: return 111;
		//case 196: return ??;	// '"Next"'
		case 197: return 120;
		//case 198: return ??;	// '"This SD Card is write-protected. No changes can be made to the order of Channels."'
		//case 199: return ??;	// '"This Channel cannot be launched because there is a newer version available."'
		//case 200: return ??;	// '"Continue"'
		//case 201: return ??;	// '"About the SD Card Menu On the SD Card Menu, you can temporarily utilise the Wii System Memory to launch a Channel stored on an SD Card."'
		//case 202: return ??;	// '"To view this information again, go to the SD Card Menu and select the icon shown here."'
		//case 203: return ??;	// '"(Blue Channels will be replaced on the SD Card with the ones stored on your Wii Console.)"'
		//case 204: return ??;	// '"Searching..."'
		//case 205: return ??;	// '"This data cannot be moved."'
		case 206: return 122;
		case 207: return 123;
		case 208: return 124;
		case 209: return 125;
		case 210: return 126;
		case 211: return 127;
		case 212: return 128;
		case 213: return 129;
		case 214: return 130;
		//case 215: return ??;	// '"The data may not have been moved."'
		case 216: return 132;
		case 217: return 133;
		case 218: return 134;
		case 219: return 135;
		case 220: return 124;
		case 221: return 125;
		case 222: return 126;
		case 223: return 127;
		case 224: return 140;
		case 225: return 141;
		case 226: return 142;
		//case 227: return ??;	// '"The data may not have been copied."'
		case 228: return 144;
		//case 229: return ??;	// '"The data may not have been erased."'
		case 230: return 146;
		case 231: return 147;
		case 232: return 124;
		case 233: return 125;
		case 234: return 126;
		case 235: return 127;
		case 236: return 152;
		case 237: return 153;
		case 238: return 154;
		case 239: return 154;
		case 240: return 156;
		//case 241: return ??;	// '"The Memory Card may not have been formatted."'
		case 242: return 3;
		case 243: return 159;
		case 244: return 160;
		case 245: return 161;
		case 246: return 162;
		case 247: return 34;
		//case 248: return ??;	// '"The data has been erased."'
		case 249: return 165;
		case 250: return 166;
		case 251: return 167;
		case 252: return 22;
		case 253: return 169;
		case 254: return 170;
		case 255: return 171;
		case 256: return 64;
		case 257: return 173;
		case 258: return 118;
		case 259: return 117;
		case 260: return 34;
		case 261: return 177;
		case 262: return 178;
		case 263: return 179;
		case 264: return 180;
		case 265: return 181;
		case 266: return 182;
		case 267: return 183;
		case 268: return 184;
		case 269: return 185;
		case 270: return 186;
		case 271: return 48;
		case 272: return 188;
		case 273: return 189;
		case 274: return 190;
		case 275: return 191;
		//case 276: return ??;	// '"Moving data to the Wii System Memory. Do not remove the SD Card or power off your Wii console."'
		case 277: return 193;
		case 278: return 194;
		case 279: return 195;
		case 280: return 196;
		case 281: return 197;
		case 282: return 198;
		case 283: return 199;
		case 284: return 133;
		case 285: return 201;
		case 286: return 202;
		case 287: return 203;
		case 288: return 204;
		case 289: return 205;
		case 290: return 111;
		//case 291: return ??;	// '"Some data could not be copied. For details, please refer to the game's Instruction Booklet."'
		case 292: return 208;
		case 293: return 209;
		case 294: return 210;
		case 295: return 211;
		case 296: return 212;
		case 297: return 213;
		//case 298: return ??;	// '"The data has been copied. This includes some data that cannot be used on other Wii consoles. Please refer to the game's Instruction Booklet for details."'
		//case 299: return ??;	// '"Data has been copied. There was some data that could not be copied to an SD Card. Refer to the game's Instruction Booklet for details."'
		case 300: return 216;
		//case 301: return ??;	// '"The data has been copied. This data cannot be copied to another Wii console. See the operations guide for more information."'
		//case 302: return ??;	// '"This data cannot be copied to this Wii console."'
		//case 303: return ??;	// '"SD Card Menu Data"'
		//case 304: return ??;	// '"This Channel cannot be moved to this Wii console."'
		//case 305: return ??;	// '"This file cannot be moved, but it can be copied."'
		//case 306: return ??;	// '"The data has been moved. This includes some data that cannot be used on other Wii consoles. See the game's Instruction Booklet for details."'
		//case 307: return ??;	// '"Some of the data could not be moved. See the game's Instruction Booklet for details."'
		//case 308: return ??;	// '"The data has been moved. However, it cannot be moved to another Wii console. See the game's Instruction Booklet for details."'
		//case 309: return ??;	// '"This data cannot be moved to this Wii console."'
		//case 310: return ??;	// '"You must first play this game on your Wii Console to move save data."'
		//case 311: return ??;	// '"This data can't be moved because a portion of it can't be copied."'
		//case 312: return ??;	// '"This Channel cannot be copied because there is a newer version available."'
		//case 313: return ??;	// '"This Channel cannot be moved because there is a newer version available."'
		//case 314: return ??;	// '"Erasing... Do not remove the SD Card or power off your Wii console."'
		case 315: return 22;
		case 316: return 218;
		case 317: return 64;
		case 318: return 220;
		case 319: return 221;
		case 320: return 222;
		case 321: return 223;
		case 322: return 224;
		case 323: return 225;
		case 324: return 226;
		case 325: return 227;
		case 326: return 228;
		case 327: return 229;
		case 328: return 230;
		case 329: return 231;
		case 330: return 232;
		case 331: return 233;
		case 332: return 234;
		//case 333: return ??;	// '"Enter your IP Address."'
		//case 334: return ??;	// '"Enter your Primary DNS."'
		//case 335: return ??;	// '"Enter your Secondary DNS."'
		//case 336: return ??;	// '"Enter the Subnet Mask."'
		//case 337: return ??;	// '"Enter your Gateway."'
		//case 338: return ??;	// '"Enter your SSID."'
		//case 339: return ??;	// '"Enter the key."'
		//case 340: return ??;	// '"Enter the name of your proxy server."'
		//case 341: return ??;	// '"Enter the port number."'
		//case 342: return ??;	// '"Enter your user name."'
		//case 343: return ??;	// '"Enter your password."'
		//case 344: return ??;	// '"Enter your MTU value."'
		//case 345: return ??;	// '"Enter your PIN."'
		//case 346: return ??;	// '"Enter the answer to your secret question."'
		//case 347: return ??;	// '"Enter the master key."'
		//case 348: return ??;	// '"Enter a nickname for your Wii."'
		//case 349: return ??;	// '"Enter the mail address."'
		//case 350: return ??;	// '"Enter a nickname."'
		//case 351: return ??;	// '"Remember, if you forget your PIN or the answer to your secret question, you will be unable to remove Parental Controls restrictions."'
		//case 352: return ??;	// '"If you forget your PIN and the answer to your secret question, you won't be able to remove Parental Controls restrictions."'
		case 353: return 235;
		case 354: return 236;
		case 355: return 237;
		case 356: return 238;
		//case 357: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update."'
		case 358: return 240;
		case 359: return 241;
		case 360: return 242;
		case 361: return 243;
		case 362: return 244;
		case 363: return 245;
		case 364: return 246;
		case 365: return 247;
		case 366: return 248;
		case 367: return 249;
		case 368: return 250;
		case 369: return 251;
		case 370: return 252;
		case 371: return 253;
		case 372: return 254;
		case 373: return 255;
		//case 374: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 375: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 376: return ??;	// '"I Accept"'
		//case 377: return ??;	// '"I Accept"'
		//case 378: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. Failure to accept this update may also render this and future games un"'
		//case 379: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update. Please note that this update will add new Channels to the Wii Menu."'
		case 380: return 256;
		case 381: return 257;
		case 382: return 258;
		case 383: return 259;
		case 384: return 260;
		case 385: return 261;
		case 386: return 262;
		case 387: return 263;
		case 388 ... 399: return idx;
		case 400: return 276;
		case 401: return 277;
		case 402: return 278;
		case 403: return 279;
		case 404: return 280;
		case 405: return 281;
		case 406: return 282;
		case 407: return 283;
		case 408: return 284;
		case 409: return 285;
		case 410: return 286;
		case 411: return 287;
		case 412: return 287;
		case 413: return 287;
		case 414: return 277;
		case 415: return 291;
		case 416: return 292;
		case 417: return 277;
		case 418: return 294;
		case 419: return 295;
		case 420: return 296;
		case 421: return 277;
		case 422: return 278;
		case 423: return 279;
		case 424: return 280;
		case 425: return 281;
		case 426: return 282;
		case 427: return 283;
		case 428: return 284;
		case 429: return 285;
		case 430: return 286;
		case 431: return 287;
		case 432: return 287;
		case 433: return 287;
		case 434: return 277;
		case 435: return 291;
		case 436: return 292;
		case 437: return 277;
		case 438: return 294;
		case 439: return 295;
		case 440: return 296;
		case 441: return 317;
		case 442: return 318;
		//case 443: return ??;	// '"The answer to your secret question must be at least six characters."'
		case 444: return 320;
		case 445: return 321;
		case 446: return 322;
		//case 447: return ??;	// '"Aim the Wii Remote at the TV. Press  or  to adjust sensitivity until only two blinking lights appear."'
		case 448: return 324;
		case 449: return 325;
		//case 450: return ??;	// '"AOSS setup failed. Move closer or make sure the access point is powered on. Then try again. For help, visit support.nintendo.com"'
		case 451: return idx;
		case 452: return 328;
		case 453: return 329;
		case 454: return 330;
		//case 455: return ??;	// '"The action you wish to perform cannot be completed at this time. Please try again later."'
		case 456: return 332;
		//case 457: return ??;	// '"Be sure to set your TV to display in standard format when choosing the Standard (4:3) setting on your Wii."'
	}
	return 0xffff;
}

u16 MapBmg_224( u16 idx )
{
	switch( idx )
	{
		case 0 ... 25: return idx;
		//case 26: return ??;	// '"Your licence for this Channel has expired."'
		case 27 ... 47: return idx;
		//case 48: return ??;	// '"Erase this data?"'
		case 49 ... 72: return idx;
		case 73: return 294;
		case 74 ... 86: return idx;
		//case 87: return ??;	// '"Registration to be confirmed. You must register one another to be able to exchange messages."'
		case 88 ... 96: return idx;
		case 97: return 309;
		case 98 ... 138: return idx;
		//case 139: return ??;	// '"← Add a Mii"'
		case 140 ... 156: return idx;
		//case 157: return ??;	// '"Welcome to the SD Card Menu! Here, you can launch Channels saved on SD Cards by temporarily utilising the Wii System Memory."'
		//case 158: return ??;	// '"About Save Data Save data cannot be loaded directly from SD Cards. To use save data stored on SD Cards, first move it to the Wii System Memory."'
		//case 159: return ??;	// '"Caution! When you see this icon appear, do not remove the SD Card or turn off the power on the Wii console."'
		//case 160: return ??;	// '"SD Card Menu"'
		//case 161: return ??;	// '"SD Card Menu"'
		//case 162: return ??;	// '"SD Card Menu"'
		//case 163: return ??;	// '"Next"'
		//case 164: return ??;	// '"Close"'
		case 165: return 35;
		//case 166: return ??;	// '"About the SD Card Menu"'
		case 167: return 157;
		case 168: return 1;
		case 169: return 245;
		//case 170: return ??;	// '"Loading from the SD Card..."'
		case 171: return 56;
		//case 172: return ??;	// '"Launch this Channel?"'
		//case 173: return ??;	// '"Launching..."'
		//case 174: return ??;	// '"Failed to launch."'
		//case 175: return ??;	// '"This Channel cannot be launched on this Wii console."'
		//case 176: return ??;	// '"This Channel could not be launched. Manage your Wii System Memory to launch this Channel."'
		//case 177: return ??;	// '"Moving data to the SD Card..."'
		case 178: return 158;
		//case 179: return ??;	// '"Close the SD Card Menu and open the Data Management Screen in Wii Options? This requires **** blocks in the Wii System Memory."'
		//case 180: return ??;	// '"There are no Channels that can be automatically managed. Use the Data Management Screen in Wii Options to free up **** blocks of space."'
		//case 181: return ??;	// '"There is not enough free space on the SD Card. Please manage the SD Card to free up additional space."'
		//case 182: return ??;	// '"Please select the criteria for Channels the Wii console will move to an SD Card."'
		//case 183: return ??;	// '"Not Launched Recently"'
		//case 184: return ??;	// '"Right side of the Wii Menu"'
		//case 185: return ??;	// '"Lots of Blocks"'
		//case 186: return ??;	// '"A Few Blocks"'
		//case 187: return ??;	// '"Auto Manage"'
		//case 188: return ??;	// '"Use Data Management"'
		case 189: return 47;
		//case 190: return ??;	// '"The Channels shown below have been selected. Move them to an SD Card?"'
		case 191: return 62;
		//case 192: return ??;	// '"There is still not enough available space in Wii System Memory. Please manage data on the Data Management Screen in Wii Options."'
		//case 193: return ??;	// '"There are so many Channels stored on this SD Card that not all of them can be displayed on the SD Card Menu."'
		//case 194: return ??;	// '"This SD Card must be formatted."'
		case 195: return 151;
		//case 196: return ??;	// '"Next"'
		case 197: return 160;
		//case 198: return ??;	// '"This SD Card is write-protected. No changes can be made to the order of Channels."'
		//case 199: return ??;	// '"This Channel cannot be launched because there is a newer version available."'
		//case 200: return ??;	// '"Continue"'
		//case 201: return ??;	// '"About the SD Card Menu On the SD Card Menu, you can temporarily utilise the Wii System Memory to launch a Channel stored on an SD Card."'
		//case 202: return ??;	// '"To view this information again, go to the SD Card Menu and select the icon shown here."'
		//case 203: return ??;	// '"(Blue Channels will be replaced on the SD Card with the ones stored on your Wii Console.)"'
		//case 204: return ??;	// '"Searching..."'
		//case 205: return ??;	// '"This data cannot be moved."'
		case 206: return 162;
		case 207: return 163;
		case 208: return 164;
		case 209: return 165;
		case 210: return 166;
		case 211: return 167;
		case 212: return 168;
		case 213: return 169;
		case 214: return 170;
		//case 215: return ??;	// '"The data may not have been moved."'
		case 216: return 172;
		case 217: return 173;
		case 218: return 174;
		case 219: return 175;
		case 220: return 164;
		case 221: return 165;
		case 222: return 166;
		case 223: return 167;
		case 224: return 180;
		case 225: return 181;
		case 226: return 182;
		//case 227: return ??;	// '"The data may not have been copied."'
		case 228: return 184;
		//case 229: return ??;	// '"The data may not have been erased."'
		case 230: return 186;
		case 231: return 187;
		case 232: return 164;
		case 233: return 165;
		case 234: return 166;
		case 235: return 167;
		case 236: return 192;
		case 237: return 193;
		case 238: return 194;
		case 239: return 194;
		case 240: return 196;
		//case 241: return ??;	// '"The Memory Card may not have been formatted."'
		case 242: return 3;
		case 243: return 199;
		case 244: return 200;
		case 245: return 201;
		case 246: return 202;
		case 247: return 47;
		//case 248: return ??;	// '"The data has been erased."'
		case 249: return 205;
		case 250: return 206;
		case 251: return 207;
		case 252: return 35;
		case 253: return 209;
		case 254: return 210;
		case 255: return 211;
		case 256: return 77;
		case 257: return 213;
		case 258: return 158;
		case 259: return 157;
		case 260: return 47;
		case 261: return 217;
		case 262: return 218;
		case 263: return 219;
		case 264: return 220;
		case 265: return 221;
		case 266: return 222;
		case 267: return 223;
		case 268: return 224;
		case 269: return 225;
		case 270: return 226;
		case 271: return 61;
		case 272: return 228;
		case 273: return 229;
		case 274: return 230;
		case 275: return 231;
		//case 276: return ??;	// '"Moving data to the Wii System Memory. Do not remove the SD Card or power off your Wii console."'
		case 277: return 233;
		case 278: return 234;
		case 279: return 235;
		case 280: return 236;
		case 281: return 237;
		case 282: return 238;
		case 283: return 239;
		case 284: return 173;
		case 285: return 241;
		case 286: return 242;
		case 287: return 243;
		case 288: return 244;
		case 289: return 245;
		case 290: return 151;
		case 291: return 247;
		case 292: return 248;
		case 293: return 249;
		case 294: return 250;
		case 295: return 251;
		case 296: return 252;
		case 297: return 253;
		case 298: return 254;
		case 299: return 255;
		case 300: return 256;
		//case 301: return ??;	// '"The data has been copied. This data cannot be copied to another Wii console. See the operations guide for more information."'
		//case 302: return ??;	// '"This data cannot be copied to this Wii console."'
		//case 303: return ??;	// '"SD Card Menu Data"'
		//case 304: return ??;	// '"This Channel cannot be moved to this Wii console."'
		//case 305: return ??;	// '"This file cannot be moved, but it can be copied."'
		//case 306: return ??;	// '"The data has been moved. This includes some data that cannot be used on other Wii consoles. See the game's Instruction Booklet for details."'
		//case 307: return ??;	// '"Some of the data could not be moved. See the game's Instruction Booklet for details."'
		//case 308: return ??;	// '"The data has been moved. However, it cannot be moved to another Wii console. See the game's Instruction Booklet for details."'
		//case 309: return ??;	// '"This data cannot be moved to this Wii console."'
		//case 310: return ??;	// '"You must first play this game on your Wii Console to move save data."'
		//case 311: return ??;	// '"This data can't be moved because a portion of it can't be copied."'
		//case 312: return ??;	// '"This Channel cannot be copied because there is a newer version available."'
		//case 313: return ??;	// '"This Channel cannot be moved because there is a newer version available."'
		//case 314: return ??;	// '"Erasing... Do not remove the SD Card or power off your Wii console."'
		case 315: return 35;
		case 316: return 260;
		case 317: return 77;
		case 318: return 262;
		case 319: return 263;
		case 320: return 264;
		case 321: return 265;
		case 322: return 266;
		case 323: return 267;
		case 324: return 268;
		case 325: return 269;
		case 326: return 270;
		case 327: return 271;
		case 328: return 272;
		case 329: return 273;
		case 330: return 274;
		case 331: return 275;
		case 332: return 276;
		case 333: return 277;
		case 334: return 278;
		case 335: return 279;
		case 336: return 280;
		case 337: return 281;
		case 338: return 282;
		case 339: return 283;
		case 340: return 284;
		case 341: return 285;
		case 342: return 286;
		case 343: return 287;
		case 344: return 288;
		case 345: return 289;
		case 346: return 290;
		case 347: return 291;
		case 348: return 292;
		case 349: return 293;
		case 350: return 294;
		//case 351: return ??;	// '"Remember, if you forget your PIN or the answer to your secret question, you will be unable to remove Parental Controls restrictions."'
		//case 352: return ??;	// '"If you forget your PIN and the answer to your secret question, you won't be able to remove Parental Controls restrictions."'
		case 353: return 295;
		case 354: return 296;
		case 355: return 297;
		case 356: return 298;
		//case 357: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update."'
		case 358: return 300;
		case 359: return 301;
		case 360: return 302;
		case 361: return 303;
		case 362: return 304;
		case 363: return 305;
		case 364: return 306;
		case 365: return 307;
		case 366: return 308;
		case 367: return 309;
		case 368: return 310;
		case 369: return 311;
		case 370: return 312;
		case 371: return 313;
		case 372: return 314;
		case 373: return 315;
		//case 374: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 375: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		case 376: return 318;
		case 377: return 318;
		//case 378: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. Failure to accept this update may also render this and future games un"'
		//case 379: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update. Please note that this update will add new Channels to the Wii Menu."'
		case 380: return 320;
		case 381: return 321;
		case 382: return 322;
		case 383: return 323;
		case 384: return 324;
		case 385: return 325;
		case 386: return 326;
		case 387: return 327;
		case 388: return 3;
		case 389: return 3;
		case 390: return 3;
		case 391: return idx;
		case 392: return 3;
		case 393: return 3;
		case 394: return 3;
		case 395: return 3;
		case 396: return 3;
		case 397 ... 399: return idx;
		case 400: return 340;
		case 401: return 341;
		case 402: return 342;
		case 403: return 343;
		case 404: return 344;
		case 405: return 345;
		case 406: return 346;
		case 407: return 347;
		case 408: return 348;
		case 409: return 349;
		case 410: return 350;
		case 411: return 351;
		case 412: return 351;
		case 413: return 351;
		case 414: return 341;
		case 415: return 355;
		case 416: return 356;
		case 417: return 341;
		case 418: return 358;
		case 419: return 359;
		case 420: return 360;
		case 421: return 341;
		case 422: return 342;
		case 423: return 343;
		case 424: return 344;
		case 425: return 345;
		case 426: return 346;
		case 427: return 347;
		case 428: return 348;
		case 429: return 349;
		case 430: return 350;
		case 431: return 351;
		case 432: return 351;
		case 433: return 351;
		case 434: return 341;
		case 435: return 355;
		case 436: return 356;
		case 437: return 341;
		case 438: return 358;
		case 439: return 359;
		case 440: return 360;
		case 441: return 381;
		case 442: return 382;
		//case 443: return ??;	// '"The answer to your secret question must be at least six characters."'
		case 444: return 384;
		case 445: return 385;
		case 446: return 386;
		case 447: return 387;
		case 448: return 388;
		case 449: return 389;
		//case 450: return ??;	// '"AOSS setup failed. Move closer or make sure the access point is powered on. Then try again. For help, visit support.nintendo.com"'
		case 451: return idx;
		case 452: return 392;
		case 453: return 393;
		case 454: return 394;
		//case 455: return ??;	// '"The action you wish to perform cannot be completed at this time. Please try again later."'
		case 456: return 396;
		//case 457: return ??;	// '"Be sure to set your TV to display in standard format when choosing the Standard (4:3) setting on your Wii."'
	}
	return 0xffff;
}

u16 MapBmg_288( u16 idx )
{
	switch( idx )
	{
		case 0 ... 25: return idx;
		//case 26: return ??;	// '"Your licence for this Channel has expired."'
		case 27 ... 47: return idx;
		//case 48: return ??;	// '"Erase this data?"'
		case 49 ... 72: return idx;
		case 73: return 294;
		case 74 ... 86: return idx;
		//case 87: return ??;	// '"Registration to be confirmed. You must register one another to be able to exchange messages."'
		case 88 ... 96: return idx;
		case 97: return 309;
		case 98 ... 138: return idx;
		//case 139: return ??;	// '"← Add a Mii"'
		case 140 ... 156: return idx;
		//case 157: return ??;	// '"Welcome to the SD Card Menu! Here, you can launch Channels saved on SD Cards by temporarily utilising the Wii System Memory."'
		//case 158: return ??;	// '"About Save Data Save data cannot be loaded directly from SD Cards. To use save data stored on SD Cards, first move it to the Wii System Memory."'
		//case 159: return ??;	// '"Caution! When you see this icon appear, do not remove the SD Card or turn off the power on the Wii console."'
		//case 160: return ??;	// '"SD Card Menu"'
		//case 161: return ??;	// '"SD Card Menu"'
		//case 162: return ??;	// '"SD Card Menu"'
		//case 163: return ??;	// '"Next"'
		//case 164: return ??;	// '"Close"'
		case 165: return 35;
		//case 166: return ??;	// '"About the SD Card Menu"'
		case 167: return 157;
		case 168: return 1;
		case 169: return 245;
		//case 170: return ??;	// '"Loading from the SD Card..."'
		case 171: return 56;
		//case 172: return ??;	// '"Launch this Channel?"'
		//case 173: return ??;	// '"Launching..."'
		//case 174: return ??;	// '"Failed to launch."'
		//case 175: return ??;	// '"This Channel cannot be launched on this Wii console."'
		//case 176: return ??;	// '"This Channel could not be launched. Manage your Wii System Memory to launch this Channel."'
		//case 177: return ??;	// '"Moving data to the SD Card..."'
		case 178: return 158;
		//case 179: return ??;	// '"Close the SD Card Menu and open the Data Management Screen in Wii Options? This requires **** blocks in the Wii System Memory."'
		//case 180: return ??;	// '"There are no Channels that can be automatically managed. Use the Data Management Screen in Wii Options to free up **** blocks of space."'
		//case 181: return ??;	// '"There is not enough free space on the SD Card. Please manage the SD Card to free up additional space."'
		//case 182: return ??;	// '"Please select the criteria for Channels the Wii console will move to an SD Card."'
		//case 183: return ??;	// '"Not Launched Recently"'
		//case 184: return ??;	// '"Right side of the Wii Menu"'
		//case 185: return ??;	// '"Lots of Blocks"'
		//case 186: return ??;	// '"A Few Blocks"'
		//case 187: return ??;	// '"Auto Manage"'
		//case 188: return ??;	// '"Use Data Management"'
		case 189: return 47;
		//case 190: return ??;	// '"The Channels shown below have been selected. Move them to an SD Card?"'
		case 191: return 62;
		//case 192: return ??;	// '"There is still not enough available space in Wii System Memory. Please manage data on the Data Management Screen in Wii Options."'
		//case 193: return ??;	// '"There are so many Channels stored on this SD Card that not all of them can be displayed on the SD Card Menu."'
		//case 194: return ??;	// '"This SD Card must be formatted."'
		case 195: return 151;
		//case 196: return ??;	// '"Next"'
		case 197: return 160;
		//case 198: return ??;	// '"This SD Card is write-protected. No changes can be made to the order of Channels."'
		//case 199: return ??;	// '"This Channel cannot be launched because there is a newer version available."'
		//case 200: return ??;	// '"Continue"'
		//case 201: return ??;	// '"About the SD Card Menu On the SD Card Menu, you can temporarily utilise the Wii System Memory to launch a Channel stored on an SD Card."'
		//case 202: return ??;	// '"To view this information again, go to the SD Card Menu and select the icon shown here."'
		//case 203: return ??;	// '"(Blue Channels will be replaced on the SD Card with the ones stored on your Wii Console.)"'
		//case 204: return ??;	// '"Searching..."'
		//case 205: return ??;	// '"This data cannot be moved."'
		case 206: return 162;
		case 207: return 163;
		case 208: return 164;
		case 209: return 165;
		case 210: return 166;
		case 211: return 167;
		case 212: return 168;
		case 213: return 169;
		case 214: return 170;
		//case 215: return ??;	// '"The data may not have been moved."'
		case 216: return 172;
		case 217: return 173;
		case 218: return 174;
		case 219: return 175;
		case 220: return 164;
		case 221: return 165;
		case 222: return 166;
		case 223: return 167;
		case 224: return 180;
		case 225: return 181;
		case 226: return 182;
		//case 227: return ??;	// '"The data may not have been copied."'
		case 228: return 184;
		//case 229: return ??;	// '"The data may not have been erased."'
		case 230: return 186;
		case 231: return 187;
		case 232: return 164;
		case 233: return 165;
		case 234: return 166;
		case 235: return 167;
		case 236: return 192;
		case 237: return 193;
		case 238: return 194;
		case 239: return 194;
		case 240: return 196;
		//case 241: return ??;	// '"The Memory Card may not have been formatted."'
		case 242: return 3;
		case 243: return 199;
		case 244: return 200;
		case 245: return 201;
		case 246: return 202;
		case 247: return 47;
		//case 248: return ??;	// '"The data has been erased."'
		case 249: return 205;
		case 250: return 206;
		case 251: return 207;
		case 252: return 35;
		case 253: return 209;
		case 254: return 210;
		case 255: return 211;
		case 256: return 77;
		case 257: return 213;
		case 258: return 158;
		case 259: return 157;
		case 260: return 47;
		case 261: return 217;
		case 262: return 218;
		case 263: return 219;
		case 264: return 220;
		case 265: return 221;
		case 266: return 222;
		case 267: return 223;
		case 268: return 224;
		case 269: return 225;
		case 270: return 226;
		case 271: return 61;
		case 272: return 228;
		case 273: return 229;
		case 274: return 230;
		case 275: return 231;
		//case 276: return ??;	// '"Moving data to the Wii System Memory. Do not remove the SD Card or power off your Wii console."'
		case 277: return 233;
		case 278: return 234;
		case 279: return 235;
		case 280: return 236;
		case 281: return 237;
		case 282: return 238;
		case 283: return 239;
		case 284: return 173;
		case 285: return 241;
		case 286: return 242;
		case 287: return 243;
		case 288: return 244;
		case 289: return 245;
		case 290: return 151;
		case 291: return 247;
		case 292: return 248;
		case 293: return 249;
		case 294: return 250;
		case 295: return 251;
		case 296: return 252;
		case 297: return 253;
		case 298: return 254;
		case 299: return 255;
		case 300: return 256;
		//case 301: return ??;	// '"The data has been copied. This data cannot be copied to another Wii console. See the operations guide for more information."'
		//case 302: return ??;	// '"This data cannot be copied to this Wii console."'
		//case 303: return ??;	// '"SD Card Menu Data"'
		//case 304: return ??;	// '"This Channel cannot be moved to this Wii console."'
		//case 305: return ??;	// '"This file cannot be moved, but it can be copied."'
		//case 306: return ??;	// '"The data has been moved. This includes some data that cannot be used on other Wii consoles. See the game's Instruction Booklet for details."'
		//case 307: return ??;	// '"Some of the data could not be moved. See the game's Instruction Booklet for details."'
		//case 308: return ??;	// '"The data has been moved. However, it cannot be moved to another Wii console. See the game's Instruction Booklet for details."'
		//case 309: return ??;	// '"This data cannot be moved to this Wii console."'
		//case 310: return ??;	// '"You must first play this game on your Wii Console to move save data."'
		case 311: return 240;	// probably not right.  but close enough -- '"This data can't be moved because a portion of it can't be copied."'
		//case 312: return ??;	// '"This Channel cannot be copied because there is a newer version available."'
		//case 313: return ??;	// '"This Channel cannot be moved because there is a newer version available."'
		//case 314: return ??;	// '"Erasing... Do not remove the SD Card or power off your Wii console."'
		case 315: return 35;
		case 316: return 260;
		case 317: return 77;
		case 318: return 262;
		case 319: return 263;
		case 320: return 264;
		case 321: return 265;
		case 322: return 266;
		case 323: return 267;
		case 324: return 268;
		case 325: return 269;
		case 326: return 270;
		case 327: return 271;
		case 328: return 272;
		case 329: return 273;
		case 330: return 274;
		case 331: return 275;
		case 332: return 276;
		case 333: return 277;
		case 334: return 278;
		case 335: return 279;
		case 336: return 280;
		case 337: return 281;
		case 338: return 282;
		case 339: return 283;
		case 340: return 284;
		case 341: return 285;
		case 342: return 286;
		case 343: return 287;
		case 344: return 288;
		case 345: return 289;
		case 346: return 290;
		case 347: return 291;
		case 348: return 292;
		case 349: return 293;
		case 350: return 294;
		//case 351: return ??;	// '"Remember, if you forget your PIN or the answer to your secret question, you will be unable to remove Parental Controls restrictions."'
		//case 352: return ??;	// '"If you forget your PIN and the answer to your secret question, you won't be able to remove Parental Controls restrictions."'
		case 353: return 295;
		case 354: return 296;
		case 355: return 297;
		case 356: return 298;
		case 357: return 299;
		case 358: return 300;
		case 359: return 301;
		case 360: return 302;
		case 361: return 303;
		case 362: return 304;
		case 363: return 305;
		case 364: return 306;
		case 365: return 307;
		case 366: return 308;
		case 367: return 309;
		case 368: return 310;
		case 369: return 311;
		case 370: return 312;
		case 371: return 313;
		case 372: return 314;
		case 373: return 315;
		//case 374: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 375: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		case 376: return 318;
		case 377: return 318;
		//case 378: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. Failure to accept this update may also render this and future games un"'
		//case 379: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update. Please note that this update will add new Channels to the Wii Menu."'
		case 380: return 320;
		case 381: return 321;
		case 382: return 322;
		case 383: return 323;
		case 384: return 324;
		case 385: return 325;
		case 386: return 326;
		case 387: return 327;
		case 388: return 3;
		case 389: return 3;
		case 390: return 3;
		case 391: return idx;
		case 392: return 3;
		case 393: return 3;
		case 394: return 3;
		case 395: return 3;
		case 396: return 3;
		case 397 ... 399: return idx;
		case 400: return 340;
		case 401: return 341;
		case 402: return 342;
		case 403: return 343;
		case 404: return 344;
		case 405: return 345;
		case 406: return 346;
		case 407: return 347;
		case 408: return 348;
		case 409: return 349;
		case 410: return 350;
		case 411: return 351;
		case 412: return 351;
		case 413: return 351;
		case 414: return 341;
		case 415: return 355;
		case 416: return 356;
		case 417: return 341;
		case 418: return 358;
		case 419: return 359;
		case 420: return 360;
		case 421: return 341;
		case 422: return 342;
		case 423: return 343;
		case 424: return 344;
		case 425: return 345;
		case 426: return 346;
		case 427: return 347;
		case 428: return 348;
		case 429: return 349;
		case 430: return 350;
		case 431: return 351;
		case 432: return 351;
		case 433: return 351;
		case 434: return 341;
		case 435: return 355;
		case 436: return 356;
		case 437: return 341;
		case 438: return 358;
		case 439: return 359;
		case 440: return 360;
		case 441: return 381;
		case 442: return 382;
		//case 443: return ??;	// '"The answer to your secret question must be at least six characters."'
		case 444: return 384;
		case 445: return 385;
		case 446: return 386;
		case 447: return 387;
		case 448: return 388;
		case 449: return 389;
		//case 450: return ??;	// '"AOSS setup failed. Move closer or make sure the access point is powered on. Then try again. For help, visit support.nintendo.com"'
		case 451: return idx;
		case 452: return 392;
		case 453: return 393;
		case 454: return 394;
		//case 455: return ??;	// '"The action you wish to perform cannot be completed at this time. Please try again later."'
		case 456: return 396;
		//case 457: return ??;	// '"Be sure to set your TV to display in standard format when choosing the Standard (4:3) setting on your Wii."'
	}
	return 0xffff;
}

u16 MapBmg_384( u16 idx )
{
	switch( idx )
	{
		case 0 ... 25: return idx;
		//case 26: return ??;	// '"Your licence for this Channel has expired."'
		case 27 ... 47: return idx;
		//case 48: return ??;	// '"Erase this data?"'
		case 49 ... 72: return idx;
		case 73: return 294;
		case 74 ... 86: return idx;
		//case 87: return ??;	// '"Registration to be confirmed. You must register one another to be able to exchange messages."'
		case 88 ... 96: return idx;
		case 97: return 311;
		case 98 ... 138: return idx;
		//case 139: return ??;	// '"← Add a Mii"'
		case 140 ... 156: return idx;
		//case 157: return ??;	// '"Welcome to the SD Card Menu! Here, you can launch Channels saved on SD Cards by temporarily utilising the Wii System Memory."'
		//case 158: return ??;	// '"About Save Data Save data cannot be loaded directly from SD Cards. To use save data stored on SD Cards, first move it to the Wii System Memory."'
		//case 159: return ??;	// '"Caution! When you see this icon appear, do not remove the SD Card or turn off the power on the Wii console."'
		//case 160: return ??;	// '"SD Card Menu"'
		//case 161: return ??;	// '"SD Card Menu"'
		//case 162: return ??;	// '"SD Card Menu"'
		//case 163: return ??;	// '"Next"'
		//case 164: return ??;	// '"Close"'
		case 165: return 35;
		//case 166: return ??;	// '"About the SD Card Menu"'
		case 167: return 157;
		case 168: return 1;
		case 169: return 245;
		//case 170: return ??;	// '"Loading from the SD Card..."'
		case 171: return 56;
		//case 172: return ??;	// '"Launch this Channel?"'
		//case 173: return ??;	// '"Launching..."'
		//case 174: return ??;	// '"Failed to launch."'
		//case 175: return ??;	// '"This Channel cannot be launched on this Wii console."'
		//case 176: return ??;	// '"This Channel could not be launched. Manage your Wii System Memory to launch this Channel."'
		//case 177: return ??;	// '"Moving data to the SD Card..."'
		case 178: return 158;
		//case 179: return ??;	// '"Close the SD Card Menu and open the Data Management Screen in Wii Options? This requires **** blocks in the Wii System Memory."'
		//case 180: return ??;	// '"There are no Channels that can be automatically managed. Use the Data Management Screen in Wii Options to free up **** blocks of space."'
		//case 181: return ??;	// '"There is not enough free space on the SD Card. Please manage the SD Card to free up additional space."'
		//case 182: return ??;	// '"Please select the criteria for Channels the Wii console will move to an SD Card."'
		//case 183: return ??;	// '"Not Launched Recently"'
		//case 184: return ??;	// '"Right side of the Wii Menu"'
		//case 185: return ??;	// '"Lots of Blocks"'
		//case 186: return ??;	// '"A Few Blocks"'
		//case 187: return ??;	// '"Auto Manage"'
		//case 188: return ??;	// '"Use Data Management"'
		case 189: return 47;
		//case 190: return ??;	// '"The Channels shown below have been selected. Move them to an SD Card?"'
		case 191: return 62;
		//case 192: return ??;	// '"There is still not enough available space in Wii System Memory. Please manage data on the Data Management Screen in Wii Options."'
		//case 193: return ??;	// '"There are so many Channels stored on this SD Card that not all of them can be displayed on the SD Card Menu."'
		//case 194: return ??;	// '"This SD Card must be formatted."'
		case 195: return 151;
		//case 196: return ??;	// '"Next"'
		case 197: return 160;
		//case 198: return ??;	// '"This SD Card is write-protected. No changes can be made to the order of Channels."'
		//case 199: return ??;	// '"This Channel cannot be launched because there is a newer version available."'
		//case 200: return ??;	// '"Continue"'
		//case 201: return ??;	// '"About the SD Card Menu On the SD Card Menu, you can temporarily utilise the Wii System Memory to launch a Channel stored on an SD Card."'
		//case 202: return ??;	// '"To view this information again, go to the SD Card Menu and select the icon shown here."'
		//case 203: return ??;	// '"(Blue Channels will be replaced on the SD Card with the ones stored on your Wii Console.)"'
		//case 204: return ??;	// '"Searching..."'
		//case 205: return ??;	// '"This data cannot be moved."'
		case 206: return 162;
		case 207: return 163;
		case 208: return 164;
		case 209: return 165;
		case 210: return 166;
		case 211: return 167;
		case 212: return 168;
		case 213: return 169;
		case 214: return 170;
		//case 215: return ??;	// '"The data may not have been moved."'
		case 216: return 172;
		case 217: return 173;
		case 218: return 174;
		case 219: return 175;
		case 220: return 164;
		case 221: return 165;
		case 222: return 166;
		case 223: return 167;
		case 224: return 180;
		case 225: return 181;
		case 226: return 182;
		//case 227: return ??;	// '"The data may not have been copied."'
		case 228: return 184;
		//case 229: return ??;	// '"The data may not have been erased."'
		case 230: return 186;
		case 231: return 187;
		case 232: return 164;
		case 233: return 165;
		case 234: return 166;
		case 235: return 167;
		case 236: return 192;
		case 237: return 193;
		case 238: return 194;
		case 239: return 194;
		case 240: return 196;
		//case 241: return ??;	// '"The Memory Card may not have been formatted."'
		case 242: return 3;
		case 243: return 199;
		case 244: return 200;
		case 245: return 201;
		case 246: return 202;
		case 247: return 47;
		//case 248: return ??;	// '"The data has been erased."'
		case 249: return 205;
		case 250: return 206;
		case 251: return 207;
		case 252: return 35;
		case 253: return 209;
		case 254: return 210;
		case 255: return 211;
		case 256: return 77;
		case 257: return 213;
		case 258: return 158;
		case 259: return 157;
		case 260: return 47;
		case 261: return 217;
		case 262: return 218;
		case 263: return 219;
		case 264: return 220;
		case 265: return 221;
		case 266: return 222;
		case 267: return 223;
		case 268: return 224;
		case 269: return 225;
		case 270: return 226;
		case 271: return 61;
		case 272: return 228;
		case 273: return 229;
		case 274: return 230;
		case 275: return 231;
		//case 276: return ??;	// '"Moving data to the Wii System Memory. Do not remove the SD Card or power off your Wii console."'
		case 277: return 233;
		case 278: return 234;
		case 279: return 235;
		case 280: return 236;
		case 281: return 237;
		case 282: return 238;
		case 283: return 239;
		case 284: return 173;
		case 285: return 241;
		case 286: return 242;
		case 287: return 243;
		case 288: return 244;
		case 289: return 245;
		case 290: return 151;
		case 291: return 247;
		case 292: return 248;
		case 293: return 249;
		case 294: return 250;
		case 295: return 251;
		case 296: return 252;
		case 297: return 253;
		case 298: return 254;
		case 299: return 255;
		case 300: return 256;
		//case 301: return ??;	// '"The data has been copied. This data cannot be copied to another Wii console. See the operations guide for more information."'
		//case 302: return ??;	// '"This data cannot be copied to this Wii console."'
		//case 303: return ??;	// '"SD Card Menu Data"'
		//case 304: return ??;	// '"This Channel cannot be moved to this Wii console."'
		//case 305: return ??;	// '"This file cannot be moved, but it can be copied."'
		//case 306: return ??;	// '"The data has been moved. This includes some data that cannot be used on other Wii consoles. See the game's Instruction Booklet for details."'
		//case 307: return ??;	// '"Some of the data could not be moved. See the game's Instruction Booklet for details."'
		//case 308: return ??;	// '"The data has been moved. However, it cannot be moved to another Wii console. See the game's Instruction Booklet for details."'
		//case 309: return ??;	// '"This data cannot be moved to this Wii console."'
		//case 310: return ??;	// '"You must first play this game on your Wii Console to move save data."'
		//case 311: return ??;	// '"This data can't be moved because a portion of it can't be copied."'
		//case 312: return ??;	// '"This Channel cannot be copied because there is a newer version available."'
		//case 313: return ??;	// '"This Channel cannot be moved because there is a newer version available."'
		//case 314: return ??;	// '"Erasing... Do not remove the SD Card or power off your Wii console."'
		case 315: return 35;
		case 316: return 260;
		case 317: return 77;
		case 318: return 262;
		case 319: return 263;
		case 320: return 264;
		case 321: return 265;
		case 322: return 266;
		case 323: return 267;
		case 324: return 268;
		case 325: return 269;
		case 326: return 270;
		case 327: return 271;
		case 328: return 272;
		case 329: return 273;
		case 330: return 274;
		case 331: return 275;
		case 332: return 276;
		case 333: return 277;
		case 334: return 278;
		case 335: return 279;
		case 336: return 280;
		case 337: return 281;
		case 338: return 282;
		case 339: return 283;
		case 340: return 284;
		case 341: return 285;
		case 342: return 286;
		case 343: return 287;
		case 344: return 288;
		case 345: return 289;
		case 346: return 290;
		case 347: return 291;
		case 348: return 292;
		case 349: return 293;
		case 350: return 294;
		case 351: return 295;
		case 352: return 296;
		case 353: return 297;
		case 354: return 298;
		case 355: return 299;
		case 356: return 300;
		case 357: return 301;
		case 358: return 302;
		case 359: return 303;
		case 360: return 304;
		case 361: return 305;
		case 362: return 306;
		case 363: return 307;
		case 364: return 308;
		case 365: return 309;
		case 366: return 310;
		case 367: return 311;
		case 368: return 312;
		case 369: return 313;
		case 370: return 314;
		case 371: return 315;
		case 372: return 316;
		case 373: return 317;
		//case 374: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 375: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		case 376: return 320;
		case 377: return 320;
		//case 378: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. Failure to accept this update may also render this and future games un"'
		//case 379: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update. Please note that this update will add new Channels to the Wii Menu."'
		case 380: return 322;
		case 381: return 323;
		case 382: return 324;
		case 383: return 325;
		case 384: return 326;
		case 385: return 327;
		case 386: return 328;
		case 387: return 329;
		case 388: return 3;
		case 389: return 3;
		case 390: return 3;
		case 391: return 3;
		case 392: return 3;
		case 393: return idx;
		case 394: return 3;
		case 395: return 3;
		case 396: return 3;
		case 397: return 3;
		case 398: return 3;
		case 399: return idx;
		case 400: return 342;
		case 401: return 343;
		case 402: return 344;
		case 403: return 345;
		case 404: return 346;
		case 405: return 347;
		case 406: return 348;
		case 407: return 349;
		case 408: return 350;
		case 409: return 351;
		case 410: return 352;
		case 411: return 353;
		case 412: return 353;
		case 413: return 353;
		case 414: return 343;
		case 415: return 357;
		case 416: return 358;
		case 417: return 343;
		case 418: return 360;
		case 419: return 361;
		case 420: return 362;
		case 421: return 343;
		case 422: return 344;
		case 423: return 345;
		case 424: return 346;
		case 425: return 347;
		case 426: return 348;
		case 427: return 349;
		case 428: return 350;
		case 429: return 351;
		case 430: return 352;
		case 431: return 353;
		case 432: return 353;
		case 433: return 353;
		case 434: return 343;
		case 435: return 357;
		case 436: return 358;
		case 437: return 343;
		case 438: return 360;
		case 439: return 361;
		case 440: return 362;
		case 441: return 383;
		case 442: return 384;
		//case 443: return ??;	// '"The answer to your secret question must be at least six characters."'
		case 444: return 386;
		case 445: return 387;
		case 446: return 388;
		case 447: return 389;
		case 448: return 390;
		case 449: return 391;
		//case 450: return ??;	// '"AOSS setup failed. Move closer or make sure the access point is powered on. Then try again. For help, visit support.nintendo.com"'
		case 451: return idx;
		case 452: return 394;
		case 453: return 395;
		case 454: return 396;
		//case 455: return ??;	// '"The action you wish to perform cannot be completed at this time. Please try again later."'
		case 456: return 398;
		//case 457: return ??;	// '"Be sure to set your TV to display in standard format when choosing the Standard (4:3) setting on your Wii."'
	}
	return 0xffff;
}

u16 MapBmg_416( u16 idx )
{
	switch( idx )
	{
		case 0 ... 25: return idx;
		//case 26: return ??;	// '"Your licence for this Channel has expired."'
		case 27 ... 47: return idx;
		//case 48: return ??;	// '"Erase this data?"'
		case 49 ... 72: return idx;
		case 73: return 350;
		case 74 ... 86: return idx;
		//case 87: return ??;	// '"Registration to be confirmed. You must register one another to be able to exchange messages."'
		case 88 ... 96: return idx;
		case 97: return 106;
		case 98 ... 105: return idx;
		case 106: return 97;
		case 107 ... 108: return idx;
		case 109: return 3;
		case 110 ... 138: return idx;
		//case 139: return ??;	// '"← Add a Mii"'
		case 140 ... 156: return idx;
		//case 157: return ??;	// '"Welcome to the SD Card Menu! Here, you can launch Channels saved on SD Cards by temporarily utilising the Wii System Memory."'
		case 158 ... 178: return idx;
		//case 179: return ??;	// '"Close the SD Card Menu and open the Data Management Screen in Wii Options? This requires **** blocks in the Wii System Memory."'
		//case 180: return ??;	// '"There are no Channels that can be automatically managed. Use the Data Management Screen in Wii Options to free up **** blocks of space."'
		case 181 ... 183: return idx;
		//case 184: return ??;	// '"Right side of the Wii Menu"'
		case 185 ... 187: return idx;
		//case 188: return ??;	// '"Use Data Management"'
		case 189: return idx;
		//case 190: return ??;	// '"The Channels shown below have been selected. Move them to an SD Card?"'
		case 191 ... 200: return idx;
		//case 201: return ??;	// '"About the SD Card Menu On the SD Card Menu, you can temporarily utilise the Wii System Memory to launch a Channel stored on an SD Card."'
		//case 202: return ??;	// '"To view this information again, go to the SD Card Menu and select the icon shown here."'
		case 203 ... 204: return idx;
		//case 205: return ??;	// '"This data cannot be moved."'
		case 206 ... 247: return idx;
		//case 248: return ??;	// '"The data has been erased."'
		case 249 ... 283: return idx;
		case 284: return 217;
		case 285 ... 300: return idx;
		//case 301: return ??;	// '"The data has been copied. This data cannot be copied to another Wii console. See the operations guide for more information."'
		//case 302: return ??;	// '"This data cannot be copied to this Wii console."'
		case 303 ... 304: return idx;
		//case 305: return ??;	// '"This file cannot be moved, but it can be copied."'
		//case 306: return ??;	// '"The data has been moved. This includes some data that cannot be used on other Wii consoles. See the game's Instruction Booklet for details."'
		case 307 ... 309: return idx;
		//case 310: return ??;	// '"You must first play this game on your Wii Console to move save data."'
		case 311 ... 373: return idx;
		//case 374: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 375: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		case 376 ... 377: return idx;
		//case 378: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. Failure to accept this update may also render this and future games un"'
		//case 379: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update. Please note that this update will add new Channels to the Wii Menu."'
		case 380: return 379;
		case 381: return 380;
		case 382: return 381;
		case 383: return 382;
		case 384: return 383;
		case 385: return 384;
		case 386: return 385;
		case 387: return 386;
		case 388 ... 398: return idx;
		case 399: return 3;
		case 400: return 399;
		case 401: return 400;
		case 402: return 401;
		case 403: return 402;
		case 404: return 403;
		case 405: return 404;
		case 406: return 405;
		case 407: return 406;
		case 408: return 407;
		case 409: return 408;
		case 410: return 409;
		case 411 ... 412: return idx;
		case 413: return 410;
		case 414: return 400;
		case 415: return 414;
		case 416: return 415;
		case 417: return 400;
		case 418: return 417;
		case 419: return 418;
		case 420: return 419;
		case 421: return 400;
		case 422: return 401;
		case 423: return 402;
		case 424: return 403;
		case 425: return 404;
		case 426: return 405;
		case 427: return 406;
		case 428: return 407;
		case 429: return 408;
		case 430: return 409;
		case 431 ... 432: return idx;
		case 433: return 410;
		case 434: return 400;
		case 435: return 414;
		case 436: return 415;
		case 437: return 400;
		case 438: return 417;
		case 439: return 418;
		case 440: return 419;
		case 441: return 440;
		case 442: return 441;
		//case 443: return ??;	// '"The answer to your secret question must be at least six characters."'
		case 444: return 443;
		case 445: return 444;
		case 446: return 445;
		case 447: return 446;
		case 448: return 447;
		case 449: return 448;
		case 450: return 449;
		case 451: return 3;
		case 452: return 451;
		case 453: return 452;
		case 454: return 453;
		//case 455: return ??;	// '"The action you wish to perform cannot be completed at this time. Please try again later."'
		case 456: return 455;
		case 457: return 456;
	}
	return 0xffff;
}

u16 MapBmg_418( u16 idx )
{
	switch( idx )
	{
		case 0 ... 378: return idx;
		//case 379: return ??;	// '"In order to use this software, you must perform a Wii system update. Select OK to begin the update. Please note that this update will add new Channels to the Wii Menu."'
		case 380: return 379;
		case 381: return 380;
		case 382: return 381;
		case 383: return 382;
		case 384: return 383;
		case 385: return 384;
		case 386: return 385;
		case 387: return 386;
		case 388 ... 398: return idx;
		case 399: return 3;
		case 400: return 399;
		case 401: return 400;
		case 402: return 401;
		case 403: return 402;
		case 404: return 403;
		case 405: return 404;
		case 406: return 405;
		case 407: return 406;
		case 408: return 407;
		case 409: return 408;
		case 410: return 409;
		case 411 ... 412: return idx;
		case 413: return 410;
		case 414: return 400;
		case 415: return 414;
		case 416: return 415;
		case 417: return 400;
		case 418: return 417;
		case 419: return 418;
		case 420: return 419;
		case 421: return 400;
		case 422: return 401;
		case 423: return 402;
		case 424: return 403;
		case 425: return 404;
		case 426: return 405;
		case 427: return 406;
		case 428: return 407;
		case 429: return 408;
		case 430: return 409;
		case 431 ... 432: return idx;
		case 433: return 410;
		case 434: return 400;
		case 435: return 414;
		case 436: return 415;
		case 437: return 400;
		case 438: return 417;
		case 439: return 418;
		case 440: return 419;
		case 441: return 440;
		case 442: return 441;
		case 443: return 442;
		case 444: return 443;
		case 445: return 444;
		case 446: return 445;
		case 447: return 446;
		case 448: return 447;
		case 449: return 448;
		case 450: return 449;
		case 451: return 3;
		case 452: return 451;
		case 453: return 452;
		case 454: return 453;
		case 455: return 454;
		case 456: return 455;
		case 457: return 456;
	}
	return 0xffff;
}

u16 MapBmg_512( u16 idx )
{
	switch( idx )
	{
		case 0 ... 25: return idx;
		//case 26: return ??;	// '"Your licence for this Channel has expired."'
		case 27 ... 47: return idx;
		//case 48: return ??;	// '"Erase this data?"'
		case 49 ... 72: return idx;
		case 73: return 350;
		case 74 ... 86: return idx;
		//case 87: return ??;	// '"Registration to be confirmed. You must register one another to be able to exchange messages."'
		case 88 ... 96: return idx;
		case 97: return 106;
		case 98 ... 105: return idx;
		case 106: return 97;
		case 107 ... 108: return idx;
		case 109: return 3;
		case 110 ... 138: return idx;
		//case 139: return ??;	// '"← Add a Mii"'
		case 140 ... 156: return idx;
		//case 157: return ??;	// '"Welcome to the SD Card Menu! Here, you can launch Channels saved on SD Cards by temporarily utilising the Wii System Memory."'
		case 158 ... 178: return idx;
		//case 179: return ??;	// '"Close the SD Card Menu and open the Data Management Screen in Wii Options? This requires **** blocks in the Wii System Memory."'
		//case 180: return ??;	// '"There are no Channels that can be automatically managed. Use the Data Management Screen in Wii Options to free up **** blocks of space."'
		case 181 ... 183: return idx;
		//case 184: return ??;	// '"Right side of the Wii Menu"'
		case 185 ... 187: return idx;
		//case 188: return ??;	// '"Use Data Management"'
		case 189: return idx;
		//case 190: return ??;	// '"The Channels shown below have been selected. Move them to an SD Card?"'
		case 191 ... 200: return idx;
		//case 201: return ??;	// '"About the SD Card Menu On the SD Card Menu, you can temporarily utilise the Wii System Memory to launch a Channel stored on an SD Card."'
		//case 202: return ??;	// '"To view this information again, go to the SD Card Menu and select the icon shown here."'
		case 203 ... 204: return idx;
		//case 205: return ??;	// '"This data cannot be moved."'
		case 206 ... 247: return idx;
		//case 248: return ??;	// '"The data has been erased."'
		case 249 ... 283: return idx;
		case 284: return 217;
		case 285 ... 300: return idx;
		//case 301: return ??;	// '"The data has been copied. This data cannot be copied to another Wii console. See the operations guide for more information."'
		//case 302: return ??;	// '"This data cannot be copied to this Wii console."'
		case 303 ... 304: return idx;
		//case 305: return ??;	// '"This file cannot be moved, but it can be copied."'
		//case 306: return ??;	// '"The data has been moved. This includes some data that cannot be used on other Wii consoles. See the game's Instruction Booklet for details."'
		case 307 ... 309: return idx;
		//case 310: return ??;	// '"You must first play this game on your Wii Console to move save data."'
		case 311 ... 373: return idx;
		//case 374: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		//case 375: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. If you do not wish to accept this update, you can cancel it by holding"'
		case 376 ... 377: return idx;
		//case 378: return ??;	// '"IMPORTANT: If your Wii console has any unauthorised modification, this update may detect and remove unauthorised content causing the immediate or delayed inoperability of your console. Failure to accept this update may also render this and future games un"'
		case 379 ... 442: return idx;
		//case 443: return ??;	// '"The answer to your secret question must be at least six characters."'
		case 444 ... 454: return idx;
		//case 455: return ??;	// '"The action you wish to perform cannot be completed at this time. Please try again later."'
		case 456 ... 457: return idx;
	}
	return 0xffff;
}
