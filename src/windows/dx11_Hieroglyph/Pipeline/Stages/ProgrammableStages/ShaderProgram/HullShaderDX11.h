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
// HullShaderDX11
//
//--------------------------------------------------------------------------------
#ifndef HullShaderDX11_h
#define HullShaderDX11_h
//--------------------------------------------------------------------------------
#include "dxCommon/ShaderDX.h"
#include "dx11_Hieroglyph/dx11Util.h"
//#include "PipelineManagerDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class HullShaderDX11 : public ShaderDX
	{
	public:
		HullShaderDX11( ID3D11HullShader* pShader );
		virtual ~HullShaderDX11();

		ShaderType GetType() const override;

	protected:
		ID3D11HullShader*			m_pHullShader;

		friend class HullStageDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // HullShaderDX11_h
//--------------------------------------------------------------------------------

