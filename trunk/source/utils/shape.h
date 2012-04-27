#ifndef SHAPE_H
#define SHAPE_H

#include <gctypes.h>
#include <math.h>

#include "gecko.h"

// tools to deal with 2-D shapes and points for screen coords and rectangles and such

struct Point
{
	Point( f32 x = 0.0f, f32 y = 0.0f )
		: x( x ),
		  y( y )
	{
	}
	Point( const Point &other )
		: x( other.x ),
		  y( other.y )
	{
	}

	f32 x;
	f32 y;
};

// generic base class
class Shape
{
public:
	Shape( f32 posX = 0.0f, f32 posY = 0.0f )
		: pos( posX, posY )
	{
	}
	Shape( const Point &pos )
		: pos( pos )
	{
	}

	// check if a given point is inside the shape
	virtual bool Contains( const Point &pos __attribute__((unused)) ) const { return false; }
	virtual bool Contains( f32 posX __attribute__((unused)), f32 posY __attribute__((unused)) ) const { return false; }
	virtual bool Contains( int posX __attribute__((unused)), int posY __attribute__((unused)) ) const { return false; }

	// move the position
	virtual void SetPos( const Point &newPos )
	{
		pos.x = newPos.x;
		pos.y = newPos.y;
	}

	Point pos;
};

// position is treated as the center
class Circle : public Shape
{
public:
	Circle( f32 rad, f32 posX, f32 posY )
		: Shape( posX, posY ),
		  radius( rad )
	{
	}
	Circle( f32 rad, const Point &pos )
		: Shape( pos ),
		  radius( rad )
	{
	}
	f32 radius;

	virtual bool Contains( const Point &point  ) const
	{
		return sqrt( (double)( pos.x - point.x ) * ( pos.x - point.x )
					 + ( pos.y - point.y ) * ( pos.y - point.y ) ) < radius;
	}
	virtual bool Contains( f32 posX, f32 posY ) const
	{
		return sqrt( (double)( pos.x - posX ) * ( pos.x - posX ) + ( pos.y - posY ) * ( pos.y - posY ) ) < radius;
	}
	virtual bool Contains( int posX, int posY ) const
	{
		return sqrt( (double)( pos.x - posX ) * ( pos.x - posX ) + ( pos.y - posY ) * ( pos.y - posY ) ) < radius;
	}
};

// pos is treated as the top-left corner
class Quad : public Shape
{
public:
	Quad( f32 x, f32 y, f32 w, f32 h )
		: Shape( x, y ),
		  br( x + w, y + h )
	{
	}

	Quad( const Point &topLeft, const Point &bottomRight )
		: Shape( topLeft ),
		  br( bottomRight )
	{
	}

	virtual bool Contains( const Point &point  ) const
	{
		register f32 x = point.x;
		register f32 y = point.y;
		return ( y > pos.y  && y < br.y && x > pos.x && x < br.x );
	}

	virtual bool Contains( f32 posX, f32 posY ) const
	{
		return ( posY > pos.y  && posY < br.y && posX > pos.x && posX < br.x );
	}

	virtual bool Contains( int posX, int posY ) const
	{
		return ( posY > pos.y  && posY < br.y && posX > pos.x && posX < br.x );
	}

	void SetCoords( u32 x, u32 y, u32 w, u32 h )
	{
		pos.x = x;
		pos.y = y;
		br.x = x + w;
		br.y = y + h;
	}

	// bottom right corner
	Point br;
};

#endif // SHAPE_H
