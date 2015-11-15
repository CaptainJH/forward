
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
		Vector4f( float x, float y, float z, float w );
        Vector4f( const Vector3f& vector, float w );
		Vector4f( const Vector4f& Vector );

		// vector operations
		void Clamp( );
		float Dot( Vector4f& vector );
		void MakeZero( );
		void Normalize( );
		float Magnitude( );

		// Operators
		Vector4f& operator= ( const Vector4f& Vector );

		// member access
		float operator[] ( i32 iPos ) const;
		float& operator[] ( i32 iPos );

		// comparison
		bool operator== ( const Vector4f& Vector ) const;
		bool operator!= ( const Vector4f& Vector ) const;

		// arithmetic operations
		Vector4f operator+ ( const Vector4f& Vector ) const;
		Vector4f operator- ( const Vector4f& Vector ) const;
		Vector4f operator* ( float fScalar ) const;
        Vector4f operator* ( const Vector4f& Vector ) const;
		Vector4f operator/ ( float fScalar ) const;
        Vector4f operator/ ( const Vector4f& Vector ) const;
		Vector4f operator- ( ) const;

		// arithmetic updates
		Vector4f& operator+= ( const Vector4f& Vector );
		Vector4f& operator-= ( const Vector4f& Vector );
		Vector4f& operator*= ( float fScalar );
        Vector4f& operator*= ( const Vector4f& Vector );
		Vector4f& operator/= ( float fScalar );
        Vector4f& operator/= ( const Vector4f& Vector );

		u32 toARGB( );
		u32 toRGBA( );
		void fromARGB( u32 color );

		Vector3f xyz() const;
		Vector2f xy() const;

	public:
		float x;
		float y;
		float z;
		float w;
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Vector4f_h
