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
// InputAssemblerStateDX11
//
//--------------------------------------------------------------------------------
#ifndef InputAssemblerStateDX11_h
#define InputAssemblerStateDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include "TStateMonitor.h"
#include "TStateArrayMonitor.h"
#include "PrimitiveTopology.h"
#include "DataFormat.h"
#include "render/ResourceSystem/DeviceResource.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class InputAssemblerStateDX11
	{
	public:
		InputAssemblerStateDX11();
		virtual ~InputAssemblerStateDX11();

		void SetFeautureLevel( D3D_FEATURE_LEVEL level );
		void ClearState( );
		void SetSisterState( InputAssemblerStateDX11* pState );
		void ResetUpdateFlags( );

		u32 GetAvailableSlotCount();

		TStateMonitor< ResourcePtr > IndexBuffer;
		TStateMonitor< DataFormatType > IndexBufferFormat;
		TStateArrayMonitor< ResourcePtr, IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT > VertexBuffers;
		TStateArrayMonitor< u32, IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT > VertexBufferStrides;
		TStateArrayMonitor< u32, IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT > VertexBufferOffsets;
		TStateMonitor< i32 > InputLayout;
		TStateMonitor< PrimitiveTopologyType > PrimitiveTopology;

	protected:

		D3D_FEATURE_LEVEL				m_FeatureLevel;

		InputAssemblerStateDX11*		m_pSisterState;

		u32					AvailableSlotCount;
	};

};
//--------------------------------------------------------------------------------
#endif // InputAssemblerStateDX11_h
//--------------------------------------------------------------------------------

