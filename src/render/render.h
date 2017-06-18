//***************************************************************************************
// render.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include "Types.h"
#include "DataFormat.h"
#include "ResourceSystem/ResourceConfig.h"


namespace forward
{
	enum RendererAPI
	{
		DirectX11,
		DirectX12
	};

	// Subresource information.
	struct Subresource
	{
		void* data;
		u32 rowPitch;
		u32 slicePitch;
	};


	class Renderer
	{
	public:
		virtual RendererAPI GetRendererAPI() const = 0;

		virtual Resource1Ptr CreateVertexBuffer(VertexBufferConfig* pConfig, Subresource* pData) = 0;
		virtual Resource1Ptr CreateIndexBuffer(IndexBufferConfig* pConfig, Subresource* pData) = 0;
		virtual Resource1Ptr CreateConstantBuffer(ConstantBufferConfig* pConfig, Subresource* pData) = 0;

		virtual Resource1Ptr CreateTexture1D(Texture1dConfig* pConfig, Subresource* pData) = 0;
		virtual Resource1Ptr CreateTexture2D(Texture2dConfig* pConfig, Subresource* pData) = 0;
		virtual Resource1Ptr CreateTexture3D(Texture3dConfig* pConfig, Subresource* pData) = 0;

		virtual void DeleteResource(Resource1Ptr ptr) = 0;

	protected:
		Renderer();
		virtual ~Renderer();

		// Static renderer access - used for accessing the renderer when no reference
		// is already available.

		static Renderer*					m_spRenderer;
	};
}