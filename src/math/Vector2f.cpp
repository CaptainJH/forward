// Standard C Library includes
#include <math.h>
#include <memory>
#include <assert.h>

//----------------------------------------------------------------------------------------------------
#include "Vector2f.h"
//----------------------------------------------------------------------------------------------------
using namespace forward;
//----------------------------------------------------------------------------------------------------
Vector2f::Vector2f( )
{
}
//----------------------------------------------------------------------------------------------------
Vector2f::Vector2f( f32 X, f32 Y )
{
	x = X;
	y = Y;
}
//----------------------------------------------------------------------------------------------------
Vector2f::Vector2f( const Vector2f& Vector )
{
	x = Vector.x;
	y = Vector.y;
}
//----------------------------------------------------------------------------------------------------
Vector2f& Vector2f::operator= ( const Vector2f& Vector )
{
	x = Vector.x;
	y = Vector.y;

    return( *this );
}
//----------------------------------------------------------------------------------------------------
void Vector2f::MakeZero( )
{
	x = 0.0f;
	y = 0.0f;
}
//----------------------------------------------------------------------------------------------------
void Vector2f::Normalize( )
{
	f32 fInvMag = ( 1.0f / Magnitude() );

	x *= fInvMag;
	y *= fInvMag;
}
//----------------------------------------------------------------------------------------------------
f32 Vector2f::Magnitude( )
{
	f32 fLength = 0.0f;

	fLength += x * x;
	fLength += y * y;

	return( sqrtf( fLength ) );
}
//----------------------------------------------------------------------------------------------------
void Vector2f::Clamp()
{
	if ( x > 1.0f ) x = 1.0f;
	if ( x < 0.0f ) x = 0.0f;

	if ( y > 1.0f ) y = 1.0f;
	if ( y < 0.0f ) y = 0.0f;
}
//----------------------------------------------------------------------------------------------------
f32 Vector2f::operator[] ( i32 iPos ) const
{
	if ( iPos == 0 ) return( x );
	return( y );
}
//----------------------------------------------------------------------------
f32& Vector2f::operator[] ( i32 iPos )
{
	if ( iPos == 0 ) return( x );
	return( y );
}
//----------------------------------------------------------------------------
bool Vector2f::operator== ( const Vector2f& Vector ) const
{
	if ( ( x - Vector.x ) * ( x - Vector.x ) > 0.01f )
		return false;
	if ( ( y - Vector.y ) * ( y - Vector.y ) > 0.01f )
		return false;

	return( true );
}
//----------------------------------------------------------------------------------------------------
bool Vector2f::operator!= ( const Vector2f& Vector ) const
{
    return( !( *this == Vector ) );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector2f::operator+ ( const Vector2f& Vector ) const
{
	Vector2f sum;

	sum.x = x + Vector.x;
	sum.y = y + Vector.y;

	return( sum );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector2f::operator- ( const Vector2f& Vector ) const
{
	Vector2f diff;

	diff.x = x - Vector.x;
	diff.y = y - Vector.y;

	return( diff );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector2f::operator* ( const Vector2f& Vector ) const
{
	Vector2f prod;

	prod.x = x * Vector.x;
	prod.y = y * Vector.y;

	return( prod );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector2f::operator* ( f32 fScalar ) const
{
	Vector2f prod;

	prod.x = x * fScalar;
	prod.y = y * fScalar;

	return( prod );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector2f::operator/ ( f32 fScalar ) const
{
	Vector2f quot;
	if ( fScalar != 0.0f )
	{
		f32 fInvScalar = 1.0f / fScalar;
		quot.x = x * fInvScalar;
		quot.y = y * fInvScalar;
	}
	else
	{
		quot.MakeZero();
	}

	return( quot );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector2f::operator- ( ) const
{
	Vector2f neg;

	neg.x = -x;
	neg.y = -y;

	return( neg );
}
//----------------------------------------------------------------------------------------------------
Vector2f& Vector2f::operator+= ( const Vector2f& Vector )
{
	x += Vector.x;
	y += Vector.y;

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector2f& Vector2f::operator-= ( const Vector2f& Vector )
{
	x -= Vector.x;
	y -= Vector.y;

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector2f& Vector2f::operator*= ( f32 fScalar )
{
	x *= fScalar;
	y *= fScalar;

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector2f& Vector2f::operator/= ( f32 fScalar )
{
	if ( fScalar != 0.0f )
	{
		f32 fInvScalar = 1.0f / fScalar;	
		x *= fInvScalar;
		y *= fInvScalar;
	}
	else
	{
		MakeZero();
	}

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector2f::Zero()
{
	Vector2f ret;
	ret.MakeZero();
	return ret;
}
//----------------------------------------------------------------------------------------------------
