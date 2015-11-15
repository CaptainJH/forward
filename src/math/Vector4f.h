
//----------------------------------------------------------------------------------------------------
// Vector4f
//----------------------------------------------------------------------------------------------------
#ifndef Vector4f_h
#define Vector4f_h
//----------------------------------------------------------------------------------------------------
#include "Types.h"
//----------------------------------------------------------------------------------------------------
namespace forward
{
	class Vector2f;
    class Vector3f;

	class Vector4f
	{
	public:
		Vector4f( );
		Vector4f( f32 x, f32 y, f32 z, f32 w );
        Vector4f( const Vector3f& vector, f32 w );
		Vector4f( const Vector4f& Vector );

		// vector operations
		void Clamp( );
		f32 Dot( Vector4f& vector );
		void MakeZero( );
		void Normalize( );
		f32 Magnitude( );

		// Operators
		Vector4f& operator= ( const Vector4f& Vector );

		// member access
		f32 operator[] ( i32 iPos ) const;
		f32& operator[] ( i32 iPos );

		// comparison
		bool operator== ( const Vector4f& Vector ) const;
		bool operator!= ( const Vector4f& Vector ) const;

		// arithmetic operations
		Vector4f operator+ ( const Vector4f& Vector ) const;
		Vector4f operator- ( const Vector4f& Vector ) const;
		Vector4f operator* ( f32 fScalar ) const;
        Vector4f operator* ( const Vector4f& Vector ) const;
		Vector4f operator/ ( f32 fScalar ) const;
        Vector4f operator/ ( const Vector4f& Vector ) const;
		Vector4f operator- ( ) const;

		// arithmetic updates
		Vector4f& operator+= ( const Vector4f& Vector );
		Vector4f& operator-= ( const Vector4f& Vector );
		Vector4f& operator*= ( f32 fScalar );
        Vector4f& operator*= ( const Vector4f& Vector );
		Vector4f& operator/= ( f32 fScalar );
        Vector4f& operator/= ( const Vector4f& Vector );

		u32 toARGB( );
		u32 toRGBA( );
		void fromARGB( u32 color );

		Vector3f xyz() const;
		Vector2f xy() const;

	public:
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Vector4f_h
