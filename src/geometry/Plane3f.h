//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) Jason Zink 
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Plane3f
//
// This class represents a 3D plane.  It allows testing the distance from a point
// to the plane, which is typically used in frustum testing to see if a point or
// sphere is intersecting the frustum.
//--------------------------------------------------------------------------------
#ifndef Plane3f_h
#define Plane3f_h
//--------------------------------------------------------------------------------
#include "Shape3D.h"
#include "math/Vector3f.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Plane3f : public Shape3D
	{
	public:
		Plane3f( );
		Plane3f( f32 a, f32 b, f32 c, f32 d );
		Plane3f(const Vector3f& normal, const Vector3f& v);
		virtual ~Plane3f( );

		// Operations
		void Normalize();
		f32 DistanceToPoint( const Vector3f& pt ) const;

		// Member Access
		f32 a() const;
		f32& a();
		f32 b() const;
		f32& b();
		f32 c() const;
		f32& c();
		f32 d() const;
		f32& d();

		virtual eSHAPE GetShapeType( ) const;

	protected:
		f32 m_fComponents[4];

	};
};
//--------------------------------------------------------------------------------
#endif // Plane3f_h
