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
// BufferConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef BufferConfigDX11_h
#define BufferConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include "dx11/ResourceSystem/ResourceDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class BufferConfigDX11
	{
	public:
		BufferConfigDX11();
		BufferConfigDX11(const BufferConfig& config);
		virtual ~BufferConfigDX11();

		void SetDefaults();

		void SetDefaultConstantBuffer( u32 size, bool dynamic );
		void SetDefaultVertexBuffer( u32 size, bool dynamic );
		void SetDefaultIndexBuffer( u32 size, bool dynamic );
		void SetDefaultStructuredBuffer( u32 size, u32 structsize );
		void SetDefaultByteAddressBuffer( u32 size );
		void SetDefaultIndirectArgsBuffer( u32 size );
		void SetDefaultStagingBuffer( u32 size );

		void SetByteWidth( u32 state );
		void SetUsage( D3D11_USAGE state );
	    void SetBindFlags( u32 state );
	    void SetCPUAccessFlags( u32 state );
	    void SetMiscFlags( u32 state );	
	    void SetStructureByteStride( u32 state );

		D3D11_BUFFER_DESC GetBufferDesc();

	protected:
		D3D11_BUFFER_DESC 		m_State;

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // BufferConfigDX11_h
//--------------------------------------------------------------------------------

