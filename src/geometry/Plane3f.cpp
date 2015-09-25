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
#include "Plane3f.h"
#include <math.h>
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
Plane3f::Plane3f()
{
	m_fComponents[0] = 0.0f;
	m_fComponents[1] = 0.0f;
	m_fComponents[2] = 0.0f;
	m_fComponents[3] = 0.0f;
}
//--------------------------------------------------------------------------------
Plane3f::Plane3f(float a, float b, float c, float d)
{
	m_fComponents[0] = a;
	m_fComponents[1] = b;
	m_fComponents[2] = c;
	m_fComponents[3] = d;
}
//--------------------------------------------------------------------------------
Plane3f::~Plane3f()
{
}
//--------------------------------------------------------------------------------
void Plane3f::Normalize()
{
	float fMagnitude = static_cast<float>(sqrt(a()*a() + b()*b() + c()*c()));

	for (int i = 0; i < 4; i++)
	{
        m_fComponents[i] /= fMagnitude;
	}
}
//--------------------------------------------------------------------------------
float Plane3f::DistanceToPoint( const Vector3f& pt ) const
{
	return (a() * pt.x +
			b() * pt.y +
			c() * pt.z +
			d());
}
//--------------------------------------------------------------------------------
float Plane3f::a() const
{
	return(m_fComponents[0]);
}
//--------------------------------------------------------------------------------
float& Plane3f::a()
{
	return(m_fComponents[0]);
}
//--------------------------------------------------------------------------------
float Plane3f::b() const
{
	return(m_fComponents[1]);
}
//--------------------------------------------------------------------------------
float& Plane3f::b()
{
	return(m_fComponents[1]);
}
//--------------------------------------------------------------------------------
float Plane3f::c() const
{
	return(m_fComponents[2]);
}
//--------------------------------------------------------------------------------
float& Plane3f::c()
{
	return(m_fComponents[2]);
}
//--------------------------------------------------------------------------------
float Plane3f::d() const
{
	return(m_fComponents[3]);
}
//--------------------------------------------------------------------------------
float& Plane3f::d()
{
	return(m_fComponents[3]);
}
//--------------------------------------------------------------------------------
eSHAPE Plane3f::GetShapeType( ) const
{
	return( PLANE );
}
//--------------------------------------------------------------------------------