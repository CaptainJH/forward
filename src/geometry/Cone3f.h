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
// Cone3f
//
// This class represents a cone with two points, and the cone radius at each of 
// those two points.  This configuration can also be used to represent a cylinder
// when the two radii are equal, or even a partial cone with its top cut off.
//--------------------------------------------------------------------------------
#ifndef Cone3f_h
#define Cone3f_h
//--------------------------------------------------------------------------------
#include "Shape3D.h"
#include "math/Vector3f.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Cone3f : public Shape3D
	{
	public:
		Cone3f( );
		Cone3f( const Vector3f& p1, f32 r1, const Vector3f& p2, f32 r2 );
		virtual ~Cone3f( );

		void SamplePosition( Vector3f& position, f32 theta, f32 height ) const;
		void SampleNormal( Vector3f& normal, f32 theta, f32 height ) const;
		void SamplePositionAndNormal( Vector3f& position, Vector3f& normal, f32 theta, f32 height ) const;

		virtual eSHAPE GetShapeType( ) const;

		Vector3f	P1;
		Vector3f	P2;
		f32		R1;
		f32		R2;
	};
};
//--------------------------------------------------------------------------------
#endif // Cone3f_h
