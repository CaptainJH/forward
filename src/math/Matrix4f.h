
//----------------------------------------------------------------------------------------------------
// Matrix4f
//
// The matrices are in row major form for compliance with DirectX.
//----------------------------------------------------------------------------------------------------
#ifndef Matrix4f_h
#define Matrix4f_h
//----------------------------------------------------------------------------------------------------
#include "Matrix3f.h"
#include "Vector4f.h"
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
namespace forward
{
	class Matrix4f
	{
	public:

		Matrix4f();
		Matrix4f( bool bZero );
		Matrix4f( const Matrix4f& Matrix );
		Matrix4f( f32 fM11, f32 fM12, f32 fM13, f32 fM14,
				   f32 fM21, f32 fM22, f32 fM23, f32 fM24,
				   f32 fM31, f32 fM32, f32 fM33, f32 fM34,
				   f32 fM41, f32 fM42, f32 fM43, f32 fM44 );
		Matrix4f Inverse() const;

		void RotationX( f32 fRadians );
		void RotationY( f32 fRadians );
		void RotationZ( f32 fRadians );
		void Scale( f32 fScale );
		void Translate( f32 fX, f32 fY, f32 fZ );

		Vector3f GetBasisX() const;
		Vector3f GetBasisY() const;
		Vector3f GetBasisZ() const;

		Vector3f GetTranslation() const;
		Matrix3f GetRotation() const;

		void SetRotation( const Matrix3f& Rot );
		void SetTranslation( const Vector3f& Trans );

		static Matrix4f RotationMatrixXYZ( f32 fRadiansX, f32 fRadiansY, f32 fRadiansZ );
		static Matrix4f RotationMatrixX( f32 fRadians );
		static Matrix4f RotationMatrixY( f32 fRadians );
		static Matrix4f RotationMatrixZ( f32 fRadians );
		static Matrix4f ScaleMatrix( f32 fScale );
		static Matrix4f ScaleMatrix( const Vector3f& scale );
		static Matrix4f ScaleMatrixXYZ( f32 fX, f32 fY, f32 fZ );
		static Matrix4f TranslationMatrix( f32 fX, f32 fY, f32 fZ );
		static Matrix4f LookAtLHMatrix(const Vector3f& eye, const Vector3f& at, const Vector3f& up );
		static Matrix4f PerspectiveFovLHMatrix( f32 fovy, f32 aspect, f32 zn, f32 zf );
		static Matrix4f PerspectiveFrustumLHMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 zn, f32 zf);
		static Matrix4f OrthographicLHMatrix( f32 zn, f32 zf, f32 width, f32 height );
		static Matrix4f OrthographicLHMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 zn, f32 zf);

		void MakeZero( );
		void MakeIdentity( );
		void MakeTranspose( );

		static Matrix4f Zero();
		static Matrix4f Identity();
		Matrix4f Transpose();

		// Operators
		Matrix4f& operator= ( const Matrix4f& Matrix );

		// member access
		f32 operator() ( i32 iRow, i32 iCol ) const;
		f32& operator() ( i32 iRow, i32 iCol );
		f32 operator[] ( i32 iPos ) const;
		f32& operator[] ( i32 iPos );

		void SetRow( i32 iRow, const Vector4f& Vector );
        void SetRow( i32 iRow, const Vector3f& Vector );		
        Vector4f GetRow( i32 iRow ) const;        
		void SetColumn( i32 iCol, const Vector4f& Vector );
		Vector4f GetColumn( i32 iCol ) const;

		// comparison
		bool operator== ( const Matrix4f& Matrix ) const;
		bool operator!= ( const Matrix4f& Matrix ) const;

		// arithmetic operations
		Matrix4f operator+ ( const Matrix4f& Matrix ) const;
		Matrix4f operator- ( const Matrix4f& Matrix ) const;
		Matrix4f operator* ( const Matrix4f& Matrix ) const;
		Matrix4f operator* ( f32 fScalar ) const;
		Matrix4f operator/ ( f32 fScalar ) const;
		Matrix4f operator- () const;

		// arithmetic updates
		Matrix4f& operator+= ( const Matrix4f& Matrix );
		Matrix4f& operator-= ( const Matrix4f& Matrix );
		Matrix4f& operator*= ( const Matrix4f& Matrix );
		Matrix4f& operator*= ( f32 fScalar );
		Matrix4f& operator/= ( f32 fScalar );

		// matrix - vector operations
		Vector4f operator* ( const Vector4f& V ) const;  // M * v

		static const i32 m11 = 0;
		static const i32 m12 = 1;
		static const i32 m13 = 2;
		static const i32 m14 = 3;

		static const i32 m21 = 4;
		static const i32 m22 = 5;
		static const i32 m23 = 6;
		static const i32 m24 = 7;
		
		static const i32 m31 = 8;
		static const i32 m32 = 9;
		static const i32 m33 = 10;
		static const i32 m34 = 11;

		static const i32 m41 = 12;
		static const i32 m42 = 13;
		static const i32 m43 = 14;
		static const i32 m44 = 15;


	protected:
		f32 m_afEntry[4*4];

		static i32 I(i32 iRow, i32 iCol); // iRow*N + iCol
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Matrix4f_h
