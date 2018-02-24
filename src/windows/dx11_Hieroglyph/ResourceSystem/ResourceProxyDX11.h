//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
//--------------------------------------------------------------------------------
// ResourceProxyDX11
//
//--------------------------------------------------------------------------------
#ifndef ResourceProxyDX11_h
#define ResourceProxyDX11_h
//--------------------------------------------------------------------------------
#include "render/ResourceSystem/ResourceProxy.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class BufferConfigDX11;
	class Texture1dConfigDX11;
	class Texture2dConfigDX11;
	class Texture3dConfigDX11;
	class DepthStencilViewConfigDX11;
	class ShaderResourceViewConfigDX11;
	class UnorderedAccessViewConfigDX11;
	class RenderTargetViewConfigDX11;
	class Renderer;

	class ResourceProxyDX11 : public ResourceProxy
	{
	public:
		ResourceProxyDX11();

		ResourceProxyDX11( i32 ResourceID, BufferConfigDX11* pConfig, Renderer* pRenderer );
		ResourceProxyDX11( i32 ResourceID, Texture1dConfigDX11* pConfig, Renderer* pRenderer);
		ResourceProxyDX11( i32 ResourceID, Texture2dConfigDX11* pConfig, Renderer* pRenderer);
		ResourceProxyDX11( i32 ResourceID, Texture3dConfigDX11* pConfig, Renderer* pRenderer);

		virtual ~ResourceProxyDX11();

	public:

		BufferConfigDX11*		        m_pBufferConfig;
		Texture1dConfigDX11*	        m_pTexture1dConfig;
		Texture2dConfigDX11*	        m_pTexture2dConfig;
		Texture3dConfigDX11*	        m_pTexture3dConfig;

		friend class Renderer;
		friend class PipelineManagerDX11;

    protected: 
        
        void CommonConstructor( u32 BindFlags, i32 ResourceID, Renderer* pRenderer, 
                                ShaderResourceViewConfigDX11* pSRVConfig,
                                RenderTargetViewConfigDX11* pRTVConfig,
                                UnorderedAccessViewConfigDX11* pUAVConfig,
                                DepthStencilViewConfigDX11* pDSVConfig = NULL );

        DepthStencilViewConfigDX11*     m_pDSVConfig;
        ShaderResourceViewConfigDX11*   m_pSRVConfig;
        RenderTargetViewConfigDX11*     m_pRTVConfig;
        UnorderedAccessViewConfigDX11*  m_pUAVConfig;
	};

	typedef std::shared_ptr<ResourceProxyDX11> ResourceProxyPtr;
};
//--------------------------------------------------------------------------------
#endif // ResourceProxyDX11_h
//--------------------------------------------------------------------------------

