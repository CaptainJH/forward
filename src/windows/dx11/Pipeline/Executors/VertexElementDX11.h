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
// VertexElementDX11
//
// This is a class to represent generic vertex information.  The elements
// themselves can be either 1 thru 4 floating point values per element.
//
// Currently the arrays are a fixed size.  This may change in the future, but for
// now the user must create new CVertexElements with the new size and manually
// copy over the vertex data to the new object.
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
#ifndef VertexElementDX11_h
#define VertexElementDX11_h
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "Vector4f.h"
#include "dx11/dx11Util.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class VertexElementDX11
	{

    public:

        // Standard semantic names
        static std::string PositionSemantic;
        static std::string NormalSemantic;
        static std::string TexCoordSemantic;
        static std::string BoneIDSemantic;
		static std::string BoneWeightSemantic;
        static std::string TangentSemantic;
		static std::string ColorSemantic;

	public:
		VertexElementDX11( i32 tuple, i32 elementCount );
		~VertexElementDX11( );
		
		i32				SizeInBytes();
		i32				Count();
		i32				Tuple();

		void*			GetPtr( i32 i );

		f32*			Get1f( i32 i );
		Vector2f*		Get2f( i32 i );
		Vector3f*		Get3f( i32 i );
		Vector4f*		Get4f( i32 i );

		i32*			Get1i( i32 i );

		u32*	Get1ui( i32 i );

		f32*					operator[]( i32 i );
		const f32*			operator[]( i32 i ) const;

		std::string						m_SemanticName;
		u32							m_uiSemanticIndex;
		DXGI_FORMAT						m_Format;
		u32							m_uiInputSlot;
		u32							m_uiAlignedByteOffset;
		D3D11_INPUT_CLASSIFICATION		m_InputSlotClass;
		u32							m_uiInstanceDataStepRate;

	protected:
		VertexElementDX11() = delete;

		f32*							m_pfData;
		i32								m_iTuple;
		i32								m_iCount;
	};
};
#endif // VertexElementDX11_h
