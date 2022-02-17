//***************************************************************************************
// render.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include <map>
#include "Types.h"
#include "DataFormat.h"
#include "Vector4f.h"
#include "ResourceSystem/DeviceResource.h"


namespace forward
{
	enum RendererType
	{
		Renderer_Hieroglyph,
		Renderer_Forward_DX11,
		Renderer_Forward_DX12,
        Renderer_Forward_Metal,
	};

	enum RendererAPI
	{
		DirectX11,
		DirectX12,
        Metal,
	};

	// Subresource information.
	struct Subresource
	{
		void* data;
		u32 rowPitch;
		u32 slicePitch;
	};

	class RenderPass;
	class FrameGraph;
	class SwapChainConfig;
	class Texture2D;
	struct PipelineStateObject;

	typedef std::pair<shared_ptr<Resource>, shared_ptr<DeviceResource>> ResoucePairType;


	class Renderer
	{
	public:
		virtual ~Renderer();
		virtual RendererAPI GetRendererAPI() const = 0;

		virtual void DeleteResource(ResourcePtr ptr) = 0;

		virtual void DrawRenderPass(RenderPass& pass) = 0;

		virtual void OnResize(u32 width, u32 height) = 0;

		virtual bool Initialize(SwapChainConfig&, bool bOffScreen=false) = 0;
		virtual void Shutdown() = 0;

		virtual void Draw(u32 vertexNum, u32 startVertexLocation=0) = 0;
		virtual void DrawIndexed(u32 indexCount) = 0;

		virtual void BeginDrawFrameGraph(FrameGraph* fg);
		virtual void EndDrawFrameGraph() = 0;

		virtual void ResolveResource(Texture2D* dst, Texture2D* src) = 0;

		virtual void SaveRenderTarget(const std::wstring& filename, PipelineStateObject& pso) = 0;

		virtual void DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color) = 0;

		virtual shared_ptr<Texture2D> GetDefaultRT() const = 0;
		virtual shared_ptr<Texture2D> GetDefaultDS() const = 0;

		void AddExternalResource(const char* name, void* res);

	protected:
		Renderer();

		FrameGraph* m_currentFrameGraph = nullptr;

		std::map<std::string, void*> m_externalResourceContext;

		template<class T>
		T GetFromExternalResource(const std::string name)
		{
			if (m_externalResourceContext.find(name) != m_externalResourceContext.end())
			{
				void* p = m_externalResourceContext[name];
				return static_cast<T>(p);
			}
			return nullptr;
		}

		std::vector<ResoucePairType> m_allLoadedBuffers;
		std::vector<ResoucePairType> m_allLoadedTextures;

		// Static renderer access - used for accessing the renderer when no reference
		// is already available.
		static Renderer*					m_spRenderer;
	};
}
