
//----------------------------------------------------------------------------------------------------
// Vector3f
//----------------------------------------------------------------------------------------------------
#ifndef Vector3f_h
#define Vector3f_h
//----------------------------------------------------------------------------------------------------
#include "Types.h"
//----------------------------------------------------------------------------------------------------
namespace forward
{
	class Vector3f
	{
	public:
		Vector3f( );
		Vector3f( f32 x, f32 y, f32 z );
		Vector3f( const Vector3f& Vector );

		// vector operations
		void Clamp( );
		Vector3f Cross( const Vector3f& A ) const;
		f32 Dot( const Vector3f& A ) const;
		void MakeZero( );
		f32 Magnitude( );
		void Normalize( );
        Vector3f Perpendicular( );

        // static vector ops
        static Vector3f Clamp( const Vector3f& A );
        static Vector3f Cross( const Vector3f& A, const Vector3f& B );
        static f32 Dot( const Vector3f& A, const Vector3f& B );
        static f32 LengthSq( const Vector3f& A );
		static Vector3f Normalize( const Vector3f& A );
        static Vector3f Perpendicular( const Vector3f& A );
		static Vector3f Random( );


		// assignment
		Vector3f& operator= (const Vector3f& Vector);

		// member access
		f32 operator[] (i32 iPos) const;
		f32& operator[] (i32 iPos);

		// comparison
		bool operator== ( const Vector3f& Vector ) const;
		bool operator!= ( const Vector3f& Vector ) const;

		// arithmetic operations
		Vector3f operator+ ( const Vector3f& Vector ) const;
		Vector3f operator- ( const Vector3f& Vector ) const;
        Vector3f operator* ( const Vector3f& Vector ) const;
		Vector3f operator* ( f32 fScalar ) const;
        Vector3f operator/ ( const Vector3f& Vector ) const;
		Vector3f operator/ ( f32 fScalar ) const;
		Vector3f operator- ( ) const;

		// arithmetic updates
		Vector3f& operator+= ( const Vector3f& Vector );
		Vector3f& operator-= ( const Vector3f& Vector );
        Vector3f& operator*= ( const Vector3f& Vector );
		Vector3f& operator*= ( f32 fScalar );
        Vector3f& operator/= ( const Vector3f& Vector );
		Vector3f& operator/= ( f32 fScalar );

	public:
		f32 x;
		f32 y;
		f32 z;
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Vector3f_h
