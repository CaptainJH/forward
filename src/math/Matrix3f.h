
//----------------------------------------------------------------------------------------------------
// Matrix3f
//
//----------------------------------------------------------------------------------------------------
#ifndef Matrix3f_h
#define Matrix3f_h
//----------------------------------------------------------------------------------------------------
#include "Vector3f.h"
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
namespace forward
{
	class Matrix3f
	{
	public:
		Matrix3f( );
		Matrix3f( bool bZero );
		Matrix3f( const Matrix3f& Matrix );
		Matrix3f( f32 fM11, f32 fM12, f32 fM13,
				   f32 fM21, f32 fM22, f32 fM23,
				   f32 fM31, f32 fM32, f32 fM33 );
		
		void RotationX( f32 fRadians );
		void RotationY( f32 fRadians );
		void RotationZ( f32 fRadians );
		void Rotation(const Vector3f& Rot );
		void RotationZYX(const Vector3f& Rot );
		void RotationEuler(const Vector3f& Axis, f32 Angle );
		void Orthonormalize();

		void MakeZero();
		void MakeIdentity();
		void MakeTranspose();

		Matrix3f Zero();
		Matrix3f Identity();
		Matrix3f Transpose();

		// Operators
		Matrix3f& operator= ( const Matrix3f& Matrix );

		// member access
		f32 operator() ( i32 iRow, i32 iCol ) const;
		f32& operator() ( i32 iRow, i32 iCol );
		f32 operator[] ( i32 iPos ) const;
		f32& operator[] ( i32 iPos );

		void SetRow( i32 iRow, const Vector3f& Vector );
		Vector3f GetRow( i32 iRow ) const;
		void SetColumn( i32 iCol, const Vector3f& Vector );
		Vector3f GetColumn( i32 iCol ) const;

		// comparison
		bool operator== ( const Matrix3f& Matrix ) const;
		bool operator!= ( const Matrix3f& Matrix ) const;

		// arithmetic operations
		Matrix3f operator+ ( const Matrix3f& Matrix ) const;
		Matrix3f operator- ( const Matrix3f& Matrix ) const;
		Matrix3f operator* ( const Matrix3f& Matrix ) const;
		Matrix3f operator* ( f32 fScalar) const;
		Matrix3f operator/ ( f32 fScalar) const;
		Matrix3f operator- ( ) const;

		// arithmetic updates
		Matrix3f& operator+= ( const Matrix3f& Matrix );
		Matrix3f& operator-= ( const Matrix3f& Matrix );
		Matrix3f& operator*= ( const Matrix3f& Matrix );
		Matrix3f& operator*= ( f32 fScalar );
		Matrix3f& operator/= ( f32 fScalar );

		// matrix - vector operations
		Vector3f operator* ( const Vector3f& rkV ) const;  // M * v

	protected:
		f32 m_afEntry[3*3];

		static i32 I(i32 iRow, i32 iCol); // iRow*N + iCol
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Matrix3f_h
