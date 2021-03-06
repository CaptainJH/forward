//--------------------------------------------------------------------------------
// Texture3dConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture3dConfigDX11_h
#define Texture3dConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <d3d11_2.h>
#include "dx11_Hieroglyph/ResourceSystem/ResourceDX11.h"
#include "dx11_Hieroglyph/ResourceSystem/ResourceView/ShaderResourceViewConfigDX11.h"
#include "dx11_Hieroglyph/ResourceSystem/ResourceView/RenderTargetViewConfigDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture3dConfigDX11
	{
	public:
		Texture3dConfigDX11();
		Texture3dConfigDX11(const Texture3dConfig& config);
		virtual ~Texture3dConfigDX11();

		void SetDefaults();

		void SetWidth( u32 state );
		void SetHeight( u32 state );
		void SetDepth( u32 state );
		void SetMipLevels( u32 state );
		void SetFormat( DXGI_FORMAT state );
		void SetUsage( D3D11_USAGE state ); 
		void SetBindFlags( u32 state );
		void SetCPUAccessFlags( u32 state );
		void SetMiscFlags( u32 state );

		void MakeShaderResource();
		void MakeRenderTarget();

		D3D11_TEXTURE3D_DESC GetTextureDesc();

		ShaderResourceViewConfigDX11* CreateSRV();
		RenderTargetViewConfigDX11* CreateRTV();

		bool IsShaderResource() const;
		bool IsRenderTarget() const;

	protected:
		D3D11_TEXTURE3D_DESC 		m_State;

		friend class RendererDX11;
		friend class ResourceProxyDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture3dConfigDX11_h
//--------------------------------------------------------------------------------

