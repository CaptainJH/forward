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
#include "PCH.h"
#include "VertexElementDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;

std::string VertexElementDX11::PositionSemantic = "POSITION";
std::string VertexElementDX11::NormalSemantic = "NORMAL";
std::string VertexElementDX11::TexCoordSemantic = "TEXCOORDS";
std::string VertexElementDX11::BoneIDSemantic = "BONEIDS";
std::string VertexElementDX11::BoneWeightSemantic = "BONEWEIGHTS";
std::string VertexElementDX11::TangentSemantic = "TANGENT";

//--------------------------------------------------------------------------------
VertexElementDX11::VertexElementDX11( i32 float_tuple, i32 elementCount )
{
	m_iTuple = float_tuple;
	m_iCount = elementCount;
	m_pfData = new f32[ m_iTuple * m_iCount ];
}
//--------------------------------------------------------------------------------
VertexElementDX11::~VertexElementDX11()
{
	delete[] m_pfData;
}
//--------------------------------------------------------------------------------
i32	VertexElementDX11::SizeInBytes()
{
	return( m_iTuple * sizeof(f32) );
}
//--------------------------------------------------------------------------------
i32	VertexElementDX11::Tuple()
{
	return( m_iTuple );
}
//--------------------------------------------------------------------------------
i32 VertexElementDX11::Count()
{
	return( m_iCount );
}
//--------------------------------------------------------------------------------
f32* VertexElementDX11::Get1f( i32 i )
{
	return( &m_pfData[ i ] );
}
//--------------------------------------------------------------------------------
Vector2f* VertexElementDX11::Get2f( i32 i )
{
	return( (Vector2f*)&m_pfData[ 2*i ] );
}
//--------------------------------------------------------------------------------
Vector3f* VertexElementDX11::Get3f( i32 i )
{
	return( (Vector3f*)&m_pfData[ 3*i ] );
}
//--------------------------------------------------------------------------------
Vector4f* VertexElementDX11::Get4f( i32 i )
{
	return( (Vector4f*)&m_pfData[ 4*i ] );
}
//--------------------------------------------------------------------------------
i32* VertexElementDX11::Get1i( i32 i )
{
	return( (i32*)&m_pfData[ i ] );
}
//--------------------------------------------------------------------------------
u32* VertexElementDX11::Get1ui( i32 i )
{
	return( (u32*)&m_pfData[ i ] );
}
//--------------------------------------------------------------------------------
f32* VertexElementDX11::operator[]( i32 i )
{
	return( &m_pfData[ m_iTuple * i ] );
}
//--------------------------------------------------------------------------------
const f32* VertexElementDX11::operator[]( i32 i ) const
{
	return( &m_pfData[ m_iTuple * i ] );
}
//--------------------------------------------------------------------------------
void* VertexElementDX11::GetPtr( i32 i )
{
	return( &m_pfData[ m_iTuple * i ] );
}
//--------------------------------------------------------------------------------