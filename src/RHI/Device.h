//***************************************************************************************
// Device.h by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#pragma once
#include <map>
#include "Types.h"
#include "ResourceSystem/DeviceResource.h"
#include "FrameGraph/PipelineStateObjects.h"
#include "CommandQueue.h"

struct SDL_Window;
struct SDL_Renderer;
namespace forward
{
	enum class DeviceType
	{
		Device_Hieroglyph,
		Device_Forward_DX11,
		Device_Forward_DX12,
        Device_Forward_Metal,
	};

	enum class RendererAPI
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

	struct LoadedResourceManager
	{
		Vector<shared_ptr<Resource>> mAllLoadedBuffers;
		Vector<shared_ptr<Resource>> mAllLoadedTextures;

		shared_ptr<Resource> FindBufferByName(const String& n);
		shared_ptr<Resource> FindVertexBufferByName(const String& n);
		shared_ptr<Resource> FindIndexBufferByName(const String& n);
		shared_ptr<Resource> FindTextureByName(const String& n);
	};


	class Device
	{
	public:
		virtual ~Device();
		virtual RendererAPI GetRendererAPI() const = 0;

		virtual void DeleteResource(ResourcePtr ptr) = 0;

		virtual void DrawRenderPass(RenderPass& pass) = 0;

		virtual void OnResize(u32 width, u32 height) = 0;

		virtual void BeginDrawFrameGraph(FrameGraph* fg);
		virtual void EndDrawFrameGraph() = 0;

		virtual void SaveRenderTarget(const std::wstring& filename, RasterPipelineStateObject* pso) = 0;
		virtual void SaveTexture(const std::wstring& filename, Texture2D* tex) = 0;

		virtual void DrawScreenText(const std::string& msg, i32 x, i32 y, const float4& color) = 0;

		virtual shared_ptr<Texture2D> GetDefaultRT() const = 0;
		virtual shared_ptr<Texture2D> GetDefaultDS() const = 0;
		virtual shared_ptr<Texture2D> GetCurrentSwapChainRT() = 0;
		virtual void FlushDefaultQueue() = 0;

		virtual shared_ptr<CommandQueue> MakeCommandQueue(QueueType t, u32 maxCmdListCount) = 0;

		void AddExternalResource(const char* name, void* res);

		void EnableImGUI(bool b) { m_imguiEnabled = b; }
		bool IsImGUIEnabled() const { return m_imguiEnabled; }

		LoadedResourceManager mLoadedResourceMgr;

	protected:
		Device() = default;
		Device(SDL_Renderer* r) : m_sdlRenderer(r) {}
		virtual bool Initialize(SwapChainConfig&, bool bOffScreen=false) = 0;

		FrameGraph* m_currentFrameGraph = nullptr;
		bool		m_imguiEnabled = true;
		SDL_Renderer* const m_sdlRenderer = nullptr;
		SDL_Window* m_sdlWnd = nullptr;

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
	};
}
