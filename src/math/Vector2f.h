
//----------------------------------------------------------------------------------------------------
// Vector2f
//----------------------------------------------------------------------------------------------------
#ifndef Vector2f_h
#define Vector2f_h
//----------------------------------------------------------------------------------------------------
#include "types.h"
//----------------------------------------------------------------------------------------------------
namespace forward
{
	class Vector2f
	{
	public:
		Vector2f( );
		Vector2f( f32 x, f32 y );
		Vector2f( const Vector2f& Vector );

		static Vector2f Zero();

		// vector operations
		void Clamp( );
		void MakeZero( );
		void Normalize( );
		f32 Magnitude( );

		// assignment
		Vector2f& operator= ( const Vector2f& Vector );

		// accessors
		f32 operator[] ( i32 iPos ) const;
		f32& operator[] ( i32 iPos );

		// boolean comparison
		bool operator== ( const Vector2f& Vector ) const;
		bool operator!= ( const Vector2f& Vector ) const;

		// arithmetic operations
		Vector2f operator+ ( const Vector2f& Vector ) const;
		Vector2f operator- ( const Vector2f& Vector ) const;
		Vector2f operator* ( const Vector2f& Vector ) const;
		Vector2f operator* ( f32 fScalar ) const;
		Vector2f operator/ ( f32 fScalar ) const;
		Vector2f operator- ( ) const;

		// arithmetic updates
		Vector2f& operator+= ( const Vector2f& Vector );
		Vector2f& operator-= ( const Vector2f& Vector );
		Vector2f& operator*= ( f32 fScalar );
		Vector2f& operator/= ( f32 fScalar );

	public:
		f32 x;
		f32 y;
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Vector2f_h
