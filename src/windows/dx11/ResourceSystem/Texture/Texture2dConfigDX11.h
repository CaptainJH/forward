//--------------------------------------------------------------------------------
// Texture2dConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture2dConfigDX11_h
#define Texture2dConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <d3d11_2.h>
#include "dx11/ResourceSystem/ResourceView/ShaderResourceViewConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/RenderTargetViewConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/DepthStencilViewConfigDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture2dConfigDX11
	{
	public:
		Texture2dConfigDX11();
		virtual ~Texture2dConfigDX11();

		void SetDefaults();
		void SetDepthBuffer( u32 width, u32 height );
		void SetColorBuffer( u32 width, u32 height );

		void SetWidth( u32 state );
		void SetHeight( u32 state );
		void SetMipLevels( u32 state );
		void SetArraySize( u32 state );
		void SetFormat( DXGI_FORMAT state );
		void SetSampleDesc( DXGI_SAMPLE_DESC state );
		void SetUsage( D3D11_USAGE state ); 
		void SetBindFlags( u32 state );
		void SetCPUAccessFlags( u32 state );
		void SetMiscFlags( u32 state );

		DXGI_FORMAT GetFormat() const;

		void MakeShaderResource();
		void MakeRenderTarget();
		void MakeDepthStencil();
		void MakeRenderTargetAndShaderResource();
		void MakeDepthStencilAndShaderResource();

		D3D11_TEXTURE2D_DESC GetTextureDesc();

	protected:
		D3D11_TEXTURE2D_DESC 		m_State;

		ShaderResourceViewConfigDX11* CreateSRV();
		DepthStencilViewConfigDX11* CreateDSV();

		bool IsShaderResource() const;
		bool IsRenderTarget() const;
		bool IsDepthStencil() const;
		bool IsTextureArray() const;

		static DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT depthFormat);
		static DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthFormat);

		friend class RendererDX11;
		friend class ResourceProxyDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture2dConfigDX11_h
//--------------------------------------------------------------------------------

