//--------------------------------------------------------------------------------
// Texture2dConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture2dConfigDX11_h
#define Texture2dConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <d3d11_2.h>
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

		D3D11_TEXTURE2D_DESC GetTextureDesc();

	protected:
		D3D11_TEXTURE2D_DESC 		m_State;

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture2dConfigDX11_h
//--------------------------------------------------------------------------------

