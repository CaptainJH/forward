//***************************************************************************************
// Texture2dConfig.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "PCH.h"
#include "DataFormat.h"

namespace forward
{

	class Texture2dConfig
	{
	public:
		Texture2dConfig();
		virtual ~Texture2dConfig();

		virtual void SetDepthBuffer(u32 width, u32 height) = 0;
		virtual void SetColorBuffer(u32 width, u32 height) = 0;

		virtual void SetWidth(u32 state) = 0;
		virtual void SetHeight(u32 state) = 0;
		virtual void SetMipLevels(u32 state) = 0;
		virtual void SetArraySize(u32 state) = 0;
		virtual void SetFormat(DataFormatType state) = 0;

		virtual DataFormatType GetFormat() const = 0;

		virtual void MakeShaderResource() = 0;
		virtual void MakeRenderTarget() = 0;
		virtual void MakeDepthStencil() = 0;
		virtual void MakeRenderTargetAndShaderResource() = 0;
		virtual void MakeDepthStencilAndShaderResource() = 0;

	protected:

		virtual bool IsShaderResource() const = 0;
		virtual bool IsRenderTarget() const = 0;
		virtual bool IsDepthStencil() const = 0;
		virtual bool IsTextureArray() const = 0;
	};
}