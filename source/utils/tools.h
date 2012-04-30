#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <gctypes.h>
#include "utils/gx_addons.h"
#include "gecko.h"

#define MAKE_FOURCC(a, b, c, d) ((a) * (1 << 24) + (b) * (1 << 16) + (c) * (1 << 8) + (d) * (1 << 0))

typedef struct
{
  u32 magic;
  u32 size;
} section_t;

typedef struct
{
	float x, y;
} Vec2f;

typedef struct
{
	float x, y, z;
} Vec3f;

#define ALIGN32(x) (((x) + 31) & ~31)
#define LIMIT(x, min, max)																	\
	({																						\
		typeof( x ) _x = x;																	\
		typeof( min ) _min = min;															\
		typeof( max ) _max = max;															\
		( ( ( _x ) < ( _min ) ) ? ( _min ) : ( ( _x ) > ( _max ) ) ? ( _max) : ( _x ) );	\
	})

#define MultiplyAlpha(a1, a2) ((u16) (a1) * (u16) (a2) / 0xFF)
#define FLOAT_2_U8(x) ((u8)((x) > 255.0f ? 255.0f : ((x) < 0.0f ? 0.0f : (x) + 0.5f)))
#define FLOAT_2_S16(x) ((s16)((x) > 32767.0f ? 32767.0f : ((x) < -32768.0f ? 32768.0f : (x) + 0.5f)))

#define FREE( x ) do{ free( x ); x = NULL; }while( 0 )
#define DELETE( x ) do{ delete x; x = NULL; }while( 0 )

#define RU(N, S) ((((N) + (S) - 1) / (S)) * (S))

#endif
