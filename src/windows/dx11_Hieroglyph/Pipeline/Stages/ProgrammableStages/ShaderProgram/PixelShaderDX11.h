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
// PixelShaderDX11
//
//--------------------------------------------------------------------------------
#ifndef PixelShaderDX11_h
#define PixelShaderDX11_h
//--------------------------------------------------------------------------------
#include "dxCommon/ShaderDX.h"
#include "dx11_Hieroglyph/dx11Util.h"
//#include "PipelineManagerDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class PixelShaderDX11 : public ShaderDX
	{
	public:
		PixelShaderDX11( ID3D11PixelShader* pShader );
		virtual ~PixelShaderDX11();

		ShaderType GetType() const override;

	protected:
		ID3D11PixelShader*			m_pPixelShader;

		friend class PixelStageDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // PixelShaderDX11_h
//--------------------------------------------------------------------------------

