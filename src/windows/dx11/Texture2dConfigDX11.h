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
		void SetDepthBuffer( UINT width, UINT height );
		void SetColorBuffer( UINT width, UINT height );

		void SetWidth( UINT state );
		void SetHeight( UINT state );
		void SetMipLevels( UINT state );
		void SetArraySize( UINT state );
		void SetFormat( DXGI_FORMAT state );
		void SetSampleDesc( DXGI_SAMPLE_DESC state );
		void SetUsage( D3D11_USAGE state ); 
		void SetBindFlags( UINT state );
		void SetCPUAccessFlags( UINT state );
		void SetMiscFlags( UINT state );

		D3D11_TEXTURE2D_DESC GetTextureDesc();

	protected:
		D3D11_TEXTURE2D_DESC 		m_State;

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture2dConfigDX11_h
//--------------------------------------------------------------------------------

