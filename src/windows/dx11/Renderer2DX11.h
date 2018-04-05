#pragma once
#include "render/render.h"
#include "dx11Util.h"
#include "dxCommon/SwapChain.h"
#include "render/FrameGraph/FrameGraphObject.h"

namespace forward
{
	class Renderer2DX11 : public Renderer
	{
	public:
		virtual ~Renderer2DX11();
		RendererAPI GetRendererAPI() const override;

		void DrawRenderPass(RenderPass& pass) override;

		void DeleteResource(ResourcePtr ptr) override;

		void OnResize(u32 width, u32 height) override;

		bool Initialize(SwapChainConfig& config) override;
		void Shutdown() override;

		void Draw(u32 vertexNum, u32 startVertexLocation=0) override;
		void DrawIndexed(u32 indexCount) override;

		void ResolveResource(FrameGraphTexture2D* dst, FrameGraphTexture2D* src) override;

		//ID3D11Device*	GetDevice();
		//ID3D11DeviceContext*	GetDeviceContext();

	private:
		bool InitializeD3D(D3D_DRIVER_TYPE DriverType, D3D_FEATURE_LEVEL FeatureLevel);
		i32 CreateSwapChain(SwapChainConfig* pConfig);
		void Present();

	private:
		// The main API interfaces used in the renderer.
		DeviceComPtr							m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11Debug>		m_pDebugger;

		DeviceContextComPtr						m_pContext;

		// In general, all resources and API objects are housed in expandable arrays
		// wrapper objects.  The position within the array is used to provide fast
		// random access to the renderer clients.
		std::vector<SwapChain*>					m_vSwapChains;

		UserDefinedAnnotationComPtr				m_pAnnotation;

		D3D_FEATURE_LEVEL						m_FeatureLevel;

		static const i32						NumQueries = 3;
		i32										m_iCurrentQuery;
		QueryComPtr								m_Queries[NumQueries];
		D3D11_QUERY_DATA_PIPELINE_STATISTICS	m_PipelineStatsData;

		u32		m_width;
		u32		m_height;
	};
}