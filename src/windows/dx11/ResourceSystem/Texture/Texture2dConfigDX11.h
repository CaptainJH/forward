//--------------------------------------------------------------------------------
// Texture2dConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture2dConfigDX11_h
#define Texture2dConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <d3d11_2.h>
#include "render/ResourceSystem/ResourceConfig.h"
#include "dx11/ResourceSystem/ResourceDX11.h"
#include "dx11/ResourceSystem/ResourceView/ShaderResourceViewConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/RenderTargetViewConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/DepthStencilViewConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/UnorderedAccessViewConfigDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture2dConfigDX11
	{
	public:
		Texture2dConfigDX11();
		Texture2dConfigDX11(const Texture2dConfig& config);
		virtual ~Texture2dConfigDX11();

		void SetDefaults();

		void SetDepthBuffer( u32 width, u32 height );
		void SetColorBuffer( u32 width, u32 height );

		void SetWidth( u32 state );
		void SetHeight( u32 state );
		void SetMipLevels( u32 state );
		void SetArraySize( u32 state );
		void SetFormat( DataFormatType state );

		void SetSampleDesc( DXGI_SAMPLE_DESC state );
		void SetUsage( D3D11_USAGE state ); 
		void SetBindFlags( u32 state );
		void SetCPUAccessFlags( u32 state );
		void SetMiscFlags( u32 state );

		DataFormatType GetFormat() const;

		void MakeShaderResource();
		void MakeRenderTarget();
		void MakeDepthStencil();
		void MakeRenderTargetAndShaderResource();
		void MakeDepthStencilAndShaderResource();

		bool IsShaderResource() const;
		bool IsRenderTarget() const;
		bool IsDepthStencil() const;
		bool IsTextureArray() const;

		ShaderResourceViewConfigDX11* CreateSRV();
		DepthStencilViewConfigDX11* CreateDSV();

		D3D11_TEXTURE2D_DESC GetTextureDesc();

		static DataFormatType GetDepthResourceFormat(DataFormatType depthFormat);
		static DataFormatType GetDepthSRVFormat(DataFormatType depthFormat);

	protected:
		D3D11_TEXTURE2D_DESC 		m_State;

		friend class RendererDX11;
		friend class ResourceProxyDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture2dConfigDX11_h
//--------------------------------------------------------------------------------

