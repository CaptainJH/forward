//--------------------------------------------------------------------------------
#include "ResourceProxyDX11.h"
#include "ResourceSystem\Buffer\BufferConfigDX11.h"
#include "ResourceSystem\Texture\Texture1dConfigDX11.h"
#include "ResourceSystem\Texture\Texture2dConfigDX11.h"
#include "ResourceSystem\Texture\Texture3dConfigDX11.h"
#include "ResourceSystem\ResourceView\DepthStencilViewConfigDX11.h"
#include "ResourceSystem\ResourceView\ShaderResourceViewConfigDX11.h"
#include "ResourceSystem\ResourceView\UnorderedAccessViewConfigDX11.h"
#include "ResourceSystem\ResourceView\RenderTargetViewConfigDX11.h"
#include "RendererDX11.h"
#include "d3dUtil.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ResourceProxyDX11::ResourceProxyDX11( i32 ResourceID, BufferConfigDX11* pConfig, 
                                        RendererDX11* pRenderer)
{	    
    D3D11_BUFFER_DESC desc = pConfig->GetBufferDesc();
    CommonConstructor( desc.BindFlags, ResourceID, pRenderer, nullptr, nullptr, nullptr );	

    // Retain the renderer's configuration.  

    m_pBufferConfig = new BufferConfigDX11();
    *m_pBufferConfig = *pConfig;	
}
//--------------------------------------------------------------------------------
ResourceProxyDX11::ResourceProxyDX11( i32 ResourceID, Texture1dConfigDX11* pConfig, 
                                     RendererDX11* pRenderer )
{    
    D3D11_TEXTURE1D_DESC desc = pConfig->GetTextureDesc();
    CommonConstructor( desc.BindFlags, ResourceID, pRenderer, nullptr, nullptr, nullptr );	

    // Retain the renderer's configuration.  

    m_pTexture1dConfig = new Texture1dConfigDX11();
    *m_pTexture1dConfig = *pConfig;	
}
//--------------------------------------------------------------------------------
ResourceProxyDX11::ResourceProxyDX11( i32 ResourceID, Texture2dConfigDX11* pConfig, 
                                     RendererDX11* pRenderer)
{    
    D3D11_TEXTURE2D_DESC desc = pConfig->GetTextureDesc();
	ShaderResourceViewConfigDX11* pSRVConfig = nullptr;
	RenderTargetViewConfigDX11* pRTVConfig = nullptr;
	UnorderedAccessViewConfigDX11* pUAVConfig = nullptr;
	DepthStencilViewConfigDX11* pDSVConfig = nullptr;
	if (pConfig->IsShaderResource())
	{
		pSRVConfig = pConfig->CreateSRV();
	}
	if (pConfig->IsDepthStencil())
	{
		pDSVConfig = pConfig->CreateDSV();

		if (pConfig->IsShaderResource())
			pSRVConfig->SetFormat(Texture2dConfigDX11::GetDepthSRVFormat(pDSVConfig->GetFormat()));
	}
    CommonConstructor( desc.BindFlags, ResourceID, pRenderer, pSRVConfig, pRTVConfig, pUAVConfig, pDSVConfig );	

    // Retain the renderer's configuration.  

    m_pTexture2dConfig = new Texture2dConfigDX11();
    *m_pTexture2dConfig = *pConfig;
}
//--------------------------------------------------------------------------------
ResourceProxyDX11::ResourceProxyDX11( i32 ResourceID, Texture3dConfigDX11* pConfig,
                                     RendererDX11* pRenderer)
{
    D3D11_TEXTURE3D_DESC desc = pConfig->GetTextureDesc();
	ShaderResourceViewConfigDX11* pSRVConfig = nullptr;
	RenderTargetViewConfigDX11* pRTVConfig = nullptr;
	UnorderedAccessViewConfigDX11* pUAVConfig = nullptr;
	if (pConfig->IsShaderResource())
	{
		pSRVConfig = pConfig->CreateSRV();
	}
	if (pConfig->IsRenderTarget())
	{
		pRTVConfig = pConfig->CreateRTV();
	}

    CommonConstructor( desc.BindFlags, ResourceID, pRenderer, pSRVConfig, pRTVConfig, pUAVConfig );	

    // Retain the renderer's configuration.

    m_pTexture3dConfig = new Texture3dConfigDX11();
    *m_pTexture3dConfig = *pConfig;	
}
//--------------------------------------------------------------------------------
ResourceProxyDX11::ResourceProxyDX11()
{
	// Initialize all indices and pointers to a neutral state.

	m_iResource = -1;
	m_iResourceSRV = m_iResourceRTV = m_iResourceDSV = m_iResourceUAV = 0;

	m_pBufferConfig = nullptr;
	m_pTexture1dConfig = nullptr;
	m_pTexture2dConfig = nullptr;
	m_pTexture3dConfig = nullptr;
    m_pSRVConfig = nullptr;
    m_pRTVConfig = nullptr;
    m_pDSVConfig = nullptr;
    m_pUAVConfig = nullptr;
}
//--------------------------------------------------------------------------------
ResourceProxyDX11::~ResourceProxyDX11()
{
	SAFE_DELETE( m_pBufferConfig );
	SAFE_DELETE( m_pTexture1dConfig );
	SAFE_DELETE( m_pTexture2dConfig );
	SAFE_DELETE( m_pTexture3dConfig );
	SAFE_DELETE( m_pSRVConfig );
	SAFE_DELETE( m_pRTVConfig );
	SAFE_DELETE( m_pUAVConfig );
	SAFE_DELETE( m_pDSVConfig );
}
//--------------------------------------------------------------------------------
void ResourceProxyDX11::CommonConstructor( u32 BindFlags, i32 ResourceID, RendererDX11* pRenderer, 
                                        ShaderResourceViewConfigDX11* pSRVConfig, 
                                        RenderTargetViewConfigDX11* pRTVConfig, 
                                        UnorderedAccessViewConfigDX11* pUAVConfig, 
                                        DepthStencilViewConfigDX11* pDSVConfig )
{
    // Initialize all indices and pointers to a neutral state.    
    m_iResource = ResourceID;
	m_iResourceSRV = 0;
	m_iResourceRTV = 0;
	m_iResourceDSV = 0;
	m_iResourceUAV = 0;


    m_pBufferConfig = nullptr;
    m_pTexture1dConfig = nullptr;
    m_pTexture2dConfig = nullptr;
    m_pTexture3dConfig = nullptr;
    m_pSRVConfig = nullptr;
    m_pRTVConfig = nullptr;
    m_pUAVConfig = nullptr;
    m_pDSVConfig = nullptr;

    // Copy the config structures
    if ( pSRVConfig )
    {
        m_pSRVConfig = new ShaderResourceViewConfigDX11();
        *m_pSRVConfig = *pSRVConfig;
    }

    if ( pRTVConfig )
    {
        m_pRTVConfig = new RenderTargetViewConfigDX11();
        *m_pRTVConfig = *pRTVConfig;
    }

    if ( pUAVConfig )
    {
        m_pUAVConfig = new UnorderedAccessViewConfigDX11();
        *m_pUAVConfig = *pUAVConfig;
    }

    if ( pDSVConfig )
    {
        m_pDSVConfig = new DepthStencilViewConfigDX11();
        *m_pDSVConfig = *pDSVConfig;
    }

    // Depending on the bind flags used to create the resource, we create a set
    // of default views to be used.

    if ( ( BindFlags & D3D11_BIND_SHADER_RESOURCE ) == D3D11_BIND_SHADER_RESOURCE )
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc = pSRVConfig ? &pSRVConfig->GetSRVDesc() : nullptr;
        m_iResourceSRV = pRenderer->CreateShaderResourceView( m_iResource, pDesc );
    }

    if ( ( BindFlags & D3D11_BIND_RENDER_TARGET ) == D3D11_BIND_RENDER_TARGET )
    {
        D3D11_RENDER_TARGET_VIEW_DESC* pDesc = pRTVConfig ? &pRTVConfig->GetRTVDesc() : nullptr;
        m_iResourceRTV = pRenderer->CreateRenderTargetView( m_iResource, pDesc );
    }

    if ( ( BindFlags & D3D11_BIND_DEPTH_STENCIL ) == D3D11_BIND_DEPTH_STENCIL )
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc = pDSVConfig ? &pDSVConfig->GetDSVDesc() : nullptr;
        m_iResourceDSV = pRenderer->CreateDepthStencilView( m_iResource, pDesc );
    }

    if ( ( BindFlags & D3D11_BIND_UNORDERED_ACCESS ) == D3D11_BIND_UNORDERED_ACCESS )
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc = pUAVConfig ? &pUAVConfig->GetUAVDesc() : nullptr;
        m_iResourceUAV = pRenderer->CreateUnorderedAccessView( m_iResource, pDesc );
    }
}
