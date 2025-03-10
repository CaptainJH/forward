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
// Sphere3f
//
// This class represents a sphere with data members of a center and a radius.
// It may be used for intersection tests as well as visibility tests.
//--------------------------------------------------------------------------------
#ifndef Sphere3f_h
#define Sphere3f_h
//--------------------------------------------------------------------------------
#include "Shape3D.h"
#include "math/Vector3f.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Sphere3f : public Shape3D
	{
	public:
		Sphere3f( );
		Sphere3f( const Vector3f& center, f32 radius );
		virtual ~Sphere3f( );

		void Update( const Vector3f& center, f32 radius );
		void UpdateCenter( const Vector3f& center );
		void UpdateRadius( f32 radius );

		bool Intersects( const Sphere3f& test ) const;
		bool Envelops( const Sphere3f& test ) const;

		void SamplePosition( Vector3f& position, f32 theta, f32 phi ) const;
		void SampleNormal( Vector3f& normal, f32 theta, f32 phi ) const;
		void SamplePositionAndNormal( Vector3f& position, Vector3f& normal, f32 theta, f32 phi ) const;

		virtual eSHAPE GetShapeType( ) const;

		Vector3f	Center;
		f32		Radius;
	};
};
//--------------------------------------------------------------------------------
#endif // Sphere3f_h
