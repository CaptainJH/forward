
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
		Matrix4f( float fM11, float fM12, float fM13, float fM14,
				   float fM21, float fM22, float fM23, float fM24,
				   float fM31, float fM32, float fM33, float fM34,
				   float fM41, float fM42, float fM43, float fM44 );
		Matrix4f Inverse() const;

		void RotationX( float fRadians );
		void RotationY( float fRadians );
		void RotationZ( float fRadians );
		void Scale( float fScale );
		void Translate( float fX, float fY, float fZ );

		Vector3f GetBasisX() const;
		Vector3f GetBasisY() const;
		Vector3f GetBasisZ() const;

		Vector3f GetTranslation() const;
		Matrix3f GetRotation() const;

		void SetRotation( const Matrix3f& Rot );
		void SetTranslation( const Vector3f& Trans );

		static Matrix4f RotationMatrixXYZ( float fRadiansX, float fRadiansY, float fRadiansZ );
		static Matrix4f RotationMatrixX( float fRadians );
		static Matrix4f RotationMatrixY( float fRadians );
		static Matrix4f RotationMatrixZ( float fRadians );
		static Matrix4f ScaleMatrix( float fScale );
		static Matrix4f ScaleMatrix( const Vector3f& scale );
		static Matrix4f ScaleMatrixXYZ( float fX, float fY, float fZ );
		static Matrix4f TranslationMatrix( float fX, float fY, float fZ );
		static Matrix4f LookAtLHMatrix( Vector3f& eye, Vector3f& at, Vector3f& up );
		static Matrix4f PerspectiveFovLHMatrix( float fovy, float aspect, float zn, float zf );
		static Matrix4f OrthographicLHMatrix( float zn, float zf, float width, float height );

		void MakeZero( );
		void MakeIdentity( );
		void MakeTranspose( );

		static Matrix4f Zero();
		static Matrix4f Identity();
		Matrix4f Transpose();

		// Operators
		Matrix4f& operator= ( const Matrix4f& Matrix );

		// member access
		float operator() ( i32 iRow, i32 iCol ) const;
		float& operator() ( i32 iRow, i32 iCol );
		float operator[] ( i32 iPos ) const;
		float& operator[] ( i32 iPos );

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
		Matrix4f operator* ( float fScalar ) const;
		Matrix4f operator/ ( float fScalar ) const;
		Matrix4f operator- () const;

		// arithmetic updates
		Matrix4f& operator+= ( const Matrix4f& Matrix );
		Matrix4f& operator-= ( const Matrix4f& Matrix );
		Matrix4f& operator*= ( const Matrix4f& Matrix );
		Matrix4f& operator*= ( float fScalar );
		Matrix4f& operator/= ( float fScalar );

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
		float m_afEntry[4*4];

		static i32 I(i32 iRow, i32 iCol); // iRow*N + iCol
	};
};
//----------------------------------------------------------------------------------------------------
#endif // Matrix4f_h
