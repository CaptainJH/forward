//--------------------------------------------------------------------------------
// Texture2dConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture2dConfigDX11_h
#define Texture2dConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <d3d11_2.h>
#include "render/ResourceSystem/Texture/Texture2dConfig.h"
#include "dx11/ResourceSystem/ResourceView/ShaderResourceViewConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/RenderTargetViewConfigDX11.h"
#include "dx11/ResourceSystem/ResourceView/DepthStencilViewConfigDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture2dConfigDX11 : public Texture2dConfig
	{
	public:
		Texture2dConfigDX11();
		virtual ~Texture2dConfigDX11();

		void SetDefaults();

		void SetDepthBuffer( u32 width, u32 height ) override;
		void SetColorBuffer( u32 width, u32 height ) override;

		void SetWidth( u32 state )				override;
		void SetHeight( u32 state )				override;
		void SetMipLevels( u32 state )			override;
		void SetArraySize( u32 state )			override;
		void SetFormat( DataFormatType state )	override;

		void SetSampleDesc( DXGI_SAMPLE_DESC state );
		void SetUsage( D3D11_USAGE state ); 
		void SetBindFlags( u32 state );
		void SetCPUAccessFlags( u32 state );
		void SetMiscFlags( u32 state );

		DataFormatType GetFormat() const		override;

		void MakeShaderResource()				override;
		void MakeRenderTarget()					override;
		void MakeDepthStencil()					override;
		void MakeRenderTargetAndShaderResource()override;
		void MakeDepthStencilAndShaderResource()override;

		D3D11_TEXTURE2D_DESC GetTextureDesc();

	protected:
		D3D11_TEXTURE2D_DESC 		m_State;

		ShaderResourceViewConfigDX11* CreateSRV();
		DepthStencilViewConfigDX11* CreateDSV();

		bool IsShaderResource() const			override;
		bool IsRenderTarget() const				override;
		bool IsDepthStencil() const				override;
		bool IsTextureArray() const				override;

		static DataFormatType GetDepthResourceFormat(DataFormatType depthFormat);
		static DataFormatType GetDepthSRVFormat(DataFormatType depthFormat);

		friend class RendererDX11;
		friend class ResourceProxyDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture2dConfigDX11_h
//--------------------------------------------------------------------------------

