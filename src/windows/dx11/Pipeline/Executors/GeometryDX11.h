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
// GeometryDX11
//
//
//--------------------------------------------------------------------------------
#ifndef GeometryDX11_h
#define GeometryDX11_h
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "ResourceSystem\ResourceProxyDX11.h"
#include "VertexElementDX11.h"
#include "TriangleIndices.h"
#include "LineIndices.h"
#include "PointIndices.h"
#include "PipelineExecutorDX11.h"
#include "Pipeline\Stages\FixedStages\InputAssembler\InputAssemblerStateDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class VertexBufferDX11;
	class IndexBufferDX11;

	class GeometryDX11 : public PipelineExecutorDX11
	{
	public:
		GeometryDX11( );
		virtual ~GeometryDX11( );
	
		virtual void Execute( PipelineManagerDX11* pPipeline );

		void AddElement( VertexElementDX11* element );
		void AddFace( const TriangleIndices& face );
		void AddLine( const LineIndices& line );
		void AddPoint( const PointIndices& point );
		void AddIndex( u32 index );

        VertexElementDX11* GetElement( std::string name );
		VertexElementDX11* GetElement( std::wstring name );
		VertexElementDX11* GetElement( i32 index );

		u32 GetIndex( u32 index );

		D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveType();
		void SetPrimitiveType( D3D11_PRIMITIVE_TOPOLOGY type );

		i32 GetPrimitiveCount();
		u32 GetIndexCount();
		
		i32 GetVertexCount();
		u32 GetElementCount();
		i32 GetVertexSize();

		i32 CalculateVertexSize();
		i32 CalculateVertexCount();

		void GenerateInputLayout( i32 ShaderID );

		void LoadToBuffers( );

        bool ComputeTangentFrame( std::string positionSemantic = VertexElementDX11::PositionSemantic,
                                  std::string normalSemantic = VertexElementDX11::NormalSemantic,
                                  std::string texCoordSemantic = VertexElementDX11::TexCoordSemantic, 
                                  std::string tangentSemantic = VertexElementDX11::TangentSemantic );

		std::vector<VertexElementDX11*>		m_vElements;
		std::vector<u32>					m_vIndices;
		
		ResourcePtr m_VB;
		ResourcePtr m_IB;

		// The size 
		i32 m_iVertexSize;
		i32 m_iVertexCount;

		// The type of primitives listed in the index buffer
		D3D11_PRIMITIVE_TOPOLOGY m_ePrimType;
	};

	typedef std::shared_ptr<GeometryDX11> GeometryPtr;
};
//--------------------------------------------------------------------------------
#endif // GeometryDX11_h
