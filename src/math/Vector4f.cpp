#include <math.h>
#include <memory>
#include <assert.h>

//----------------------------------------------------------------------------------------------------
#include "Vector4f.h"
#include "Vector3f.h"
#include "Vector2f.h"
//----------------------------------------------------------------------------------------------------
using namespace forward;
//----------------------------------------------------------------------------------------------------
Vector4f::Vector4f( )
{
}
//----------------------------------------------------------------------------------------------------
Vector4f::Vector4f( f32 X, f32 Y, f32 Z, f32 W )
{
	x = X;
	y = Y;
	z = Z;
	w = W;
}
//----------------------------------------------------------------------------------------------------
Vector4f::Vector4f( const Vector3f& Vector, f32 W )
{
    x = Vector.x;
    y = Vector.y;
    z = Vector.z;
    w = W;
}
//----------------------------------------------------------------------------------------------------
Vector4f::Vector4f( const Vector4f& Vector )
{
	x = Vector.x;
	y = Vector.y;
	z = Vector.z;
	w = Vector.w;
}
//----------------------------------------------------------------------------------------------------
Vector4f& Vector4f::operator= ( const Vector4f& Vector )
{
	x = Vector.x;
	y = Vector.y;
	z = Vector.z;
	w = Vector.w;

    return *this;
}
//----------------------------------------------------------------------------------------------------
void Vector4f::MakeZero( )
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}
//----------------------------------------------------------------------------------------------------
void Vector4f::Normalize( )
{
	f32 fInvMag = ( 1.0f / Magnitude() );

	x *= fInvMag;
	y *= fInvMag;
	z *= fInvMag;
	w *= fInvMag;
}
//----------------------------------------------------------------------------------------------------
f32 Vector4f::Magnitude( )
{
	f32 fLength = 0.0f;

	fLength += x * x;
	fLength += y * y;
	fLength += z * z;
	fLength += w * w;

	return( sqrt(fLength) );
}
//----------------------------------------------------------------------------------------------------
f32 Vector4f::Dot( Vector4f& Vector )
{
	f32 ret = 0.0f;
	
	ret += x * Vector.x;
	ret += y * Vector.y;
	ret += z * Vector.z;
	ret += w * Vector.w;

	return ret;
}
//----------------------------------------------------------------------------------------------------
f32 Vector4f::operator[] ( i32 iPos ) const
{
	if ( iPos == 0 ) return( x );
	if ( iPos == 1 ) return( y );
	if ( iPos == 2 ) return( z );
	return( w );
}
//----------------------------------------------------------------------------
f32& Vector4f::operator[] ( i32 iPos )
{
	if ( iPos == 0 ) return( x );
	if ( iPos == 1 ) return( y );
	if ( iPos == 2 ) return( z );
	return( w );
}
//----------------------------------------------------------------------------
bool Vector4f::operator== ( const Vector4f& Vector ) const
{

	if ( ( x - Vector.x ) * ( x - Vector.x ) > 0.01f )
		return false;
	if ( ( y - Vector.y ) * ( y - Vector.y ) > 0.01f )
		return false;
	if ( ( z - Vector.z ) * ( z - Vector.z ) > 0.01f )
		return false;
	if ( ( w - Vector.w ) * ( w - Vector.w ) > 0.01f )
		return false;

	return true;
}
//----------------------------------------------------------------------------------------------------
bool Vector4f::operator!= ( const Vector4f& Vector ) const
{
    return( !( *this == Vector ) );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::operator+ ( const Vector4f& Vector ) const
{
	Vector4f sum;

	sum.x = x + Vector.x;
	sum.y = y + Vector.y;
	sum.z = z + Vector.z;
	sum.w = w + Vector.w;

	return( sum );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::operator- ( const Vector4f& Vector ) const
{
	Vector4f diff;

	diff.x = x - Vector.x;
	diff.y = y - Vector.y;
	diff.z = z - Vector.z;
	diff.w = w - Vector.w;

	return( diff );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::operator* ( f32 fScalar ) const
{
	Vector4f prod;

	prod.x = x * fScalar;
	prod.y = y * fScalar;
	prod.z = z * fScalar;
	prod.w = w * fScalar;

	return( prod );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::operator* ( const Vector4f& Vector ) const
{
    Vector4f prod;

    prod.x = x * Vector.x;
    prod.y = y * Vector.y;
    prod.z = z * Vector.z;
    prod.w = w * Vector.w;

    return( prod );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::operator/ ( f32 fScalar ) const
{
	Vector4f quot;
	if ( fScalar != 0.0f )
	{
		f32 fInvScalar = 1.0f / fScalar;
		quot.x = x * fInvScalar;
		quot.y = y * fInvScalar;
		quot.z = z * fInvScalar;
		quot.w = w * fInvScalar;
	}
	else
	{
		quot.MakeZero();
	}

	return( quot );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::operator/ ( const Vector4f& Vector ) const
{
    Vector4f quot;
    quot.x = Vector.x != 0.0f ? x / Vector.x : 0.0f;
    quot.y = Vector.y != 0.0f ? y / Vector.y : 0.0f;
    quot.z = Vector.z != 0.0f ? z / Vector.z : 0.0f;
    quot.w = Vector.w != 0.0f ? w / Vector.w : 0.0f;

    return( quot );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::operator- ( ) const
{
	Vector4f neg;

	neg.x = -x;
	neg.y = -y;
	neg.z = -z;
	neg.w = -w;

	return( neg );
}
//----------------------------------------------------------------------------------------------------
Vector4f& Vector4f::operator+= ( const Vector4f& Vector )
{
	x += Vector.x;
	y += Vector.y;
	z += Vector.z;
	w += Vector.w;

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector4f& Vector4f::operator-= ( const Vector4f& Vector )
{
	x -= Vector.x;
	y -= Vector.y;
	z -= Vector.z;
	w -= Vector.w;

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector4f& Vector4f::operator*= ( f32 fScalar )
{
	x *= fScalar;
	y *= fScalar;
	z *= fScalar;
	w *= fScalar;

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector4f& Vector4f::operator*= ( const Vector4f& Vector )
{
    x *= Vector.x;
    y *= Vector.y;
    z *= Vector.z;
    w *= Vector.w;

    return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector4f& Vector4f::operator/= ( f32 fScalar )
{
	if ( fScalar != 0.0f )
	{
		f32 fInvScalar = 1.0f / fScalar;	
		x *= fInvScalar;
		y *= fInvScalar;
		z *= fInvScalar;
		w *= fInvScalar;
	}
	else
	{
		MakeZero();
	}

	return( *this );
}
//----------------------------------------------------------------------------------------------------
Vector4f& Vector4f::operator/= ( const Vector4f& Vector )
{
    x = Vector.x != 0.0f ? x / Vector.x : 0.0f;
    y = Vector.y != 0.0f ? y / Vector.y : 0.0f;
    z = Vector.z != 0.0f ? z / Vector.z : 0.0f;
    w = Vector.w != 0.0f ? w / Vector.w : 0.0f;

    return( *this );
}
//----------------------------------------------------------------------------------------------------
void Vector4f::Clamp()
{
	if ( x > 1.0f ) x = 1.0f;
	if ( x < 0.0f ) x = 0.0f;

	if ( y > 1.0f ) y = 1.0f;
	if ( y < 0.0f ) y = 0.0f;

	if ( z > 1.0f ) z = 1.0f;
	if ( z < 0.0f ) z = 0.0f;

	if ( w > 1.0f ) w = 1.0f;
	if ( w < 0.0f ) w = 0.0f;
}
//----------------------------------------------------------------------------------------------------
u32 Vector4f::toARGB( )
{
	u32 result = 0;

	Clamp();

	result += (u32)(255 * z);
	result += ((u32)(255 * y) << 8);
	result += ((u32)(255 * x) << 16);
	result += ((u32)(255 * w) << 24);

	return( result );
}
//----------------------------------------------------------------------------------------------------
u32 Vector4f::toRGBA( )
{
	u32 result = 0;

	Clamp();

	result += (u32)(255 * w);
	result += ((u32)(255 * z) << 8);
	result += ((u32)(255 * y) << 16);
	result += ((u32)(255 * x) << 24);
	
	return( result );
}
//----------------------------------------------------------------------------------------------------
void Vector4f::fromARGB( u32 color )
{
	x = (f32)((color & 0x00ff0000) >> 16)/(255.0f);	// red channel
	y = (f32)((color & 0x0000ff00) >> 8)/(255.0f);	// green channel
	z = (f32)((color & 0x000000ff))/(255.0f);		// blue channel
	w = (f32)((color & 0xff000000) >> 24)/(255.0f);	// alpha channel
}
//----------------------------------------------------------------------------------------------------
Vector3f Vector4f::xyz() const
{
	return( Vector3f( x, y, z ) );
}
//----------------------------------------------------------------------------------------------------
Vector2f Vector4f::xy() const
{
	return( Vector2f( x, y ) );
}
//----------------------------------------------------------------------------------------------------
Vector4f Vector4f::Zero()
{
	Vector4f ret;
	ret.MakeZero();
	return ret;
}
//----------------------------------------------------------------------------------------------------