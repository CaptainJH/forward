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
// ShaderStageStateDX11
//
//--------------------------------------------------------------------------------
#ifndef ShaderStageStateDX11_h
#define ShaderStageStateDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include "dx11/ResourceSystem/ResourceProxyDX11.h"
#include "render/ResourceSystem/Resource.h"
#include "TStateMonitor.h"
#include "TStateArrayMonitor.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class ShaderStageStateDX11
	{
	public:
		ShaderStageStateDX11();
		virtual ~ShaderStageStateDX11();

		void SetFeautureLevel( D3D_FEATURE_LEVEL level );

		
		void ClearState( );

		void SetSisterState( ShaderStageStateDX11* pState );
		void ResetUpdateFlags( );

		TStateMonitor< i32 > ShaderProgram;
		TStateArrayMonitor< Resource1Ptr, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT >  ConstantBuffers;
		TStateArrayMonitor< ID3D11SamplerState*, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT > SamplerStates;
		TStateArrayMonitor< Resource1Ptr, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT > ShaderResources;
		TStateArrayMonitor< ResourcePtr, D3D11_PS_CS_UAV_REGISTER_COUNT > UnorderedAccessResources;
		TStateArrayMonitor< u32, D3D11_PS_CS_UAV_REGISTER_COUNT > UAVInitialCounts;

	protected:

		D3D_FEATURE_LEVEL			m_FeatureLevel;

		ShaderStageStateDX11*		m_pSisterState;

	};
};
//--------------------------------------------------------------------------------
#endif // ShaderStageStateDX11_h
//--------------------------------------------------------------------------------

