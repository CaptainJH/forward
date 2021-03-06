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
// HullStageDX11
//
//--------------------------------------------------------------------------------
#ifndef HullStageDX11_h
#define HullStageDX11_h
//--------------------------------------------------------------------------------
#include "ShaderStageDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class HullStageDX11 : public ShaderStageDX11
	{
	public:
		HullStageDX11();
		virtual ~HullStageDX11();

	protected:
		virtual ShaderType GetType();

		virtual void BindShaderProgram( ID3D11DeviceContext* );
		virtual void BindConstantBuffers( ID3D11DeviceContext* pContext, i32 count );
		virtual void BindSamplerStates( ID3D11DeviceContext* pContext, i32 count );
		virtual void BindShaderResourceViews( ID3D11DeviceContext* pContext, i32 count );
		virtual void BindUnorderedAccessViews( ID3D11DeviceContext* pContext, i32 count );
	};
};
//--------------------------------------------------------------------------------
#endif // HullStageDX11_h
//--------------------------------------------------------------------------------

