
//--------------------------------------------------------------------------------
// Quaternion
//
//--------------------------------------------------------------------------------
#ifndef Quaternion_h
#define Quaternion_h
//--------------------------------------------------------------------------------
#include <cmath>
//--------------------------------------------------------------------------------
namespace forward
{
	template <typename Real>
	class Quaternion
	{
	public:
		Quaternion( );
		Quaternion( Real w, Real x, Real y, Real z );
		~Quaternion( );

		Real length() const;
		Real lengthSquared() const;
		Real dot( const Quaternion& a ) const;

		Quaternion conjugate() const;
		Quaternion inverse() const;

		Quaternion operator+( const Quaternion& a ) const;
		Quaternion operator-( const Quaternion& a ) const;
		Quaternion operator*( const Quaternion& a ) const;
		Quaternion operator/( const Quaternion& a ) const;
		Quaternion operator*( const Real& real ) const;
		Quaternion operator/( const Real& real ) const;

	public:
		Real w;
		Real x;
		Real y;
		Real z;
	};

	#include "Quaternion.inl"
};
//--------------------------------------------------------------------------------
#endif // Quaternion_h
//--------------------------------------------------------------------------------