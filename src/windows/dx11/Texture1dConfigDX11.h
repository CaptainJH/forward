//--------------------------------------------------------------------------------
// Texture1dConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture1dConfigDX11_h
#define Texture1dConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture1dConfigDX11
	{
	public:
		Texture1dConfigDX11();
		virtual ~Texture1dConfigDX11();

		void SetDefaults();

		void SetWidth( UINT state );
		void SetMipLevels( UINT state );
		void SetArraySize( UINT state );
		void SetFormat( DXGI_FORMAT state );
		void SetUsage( D3D11_USAGE state ); 
		void SetBindFlags( UINT state );
		void SetCPUAccessFlags( UINT state );
		void SetMiscFlags( UINT state );

		D3D11_TEXTURE1D_DESC GetTextureDesc();

	protected:
		D3D11_TEXTURE1D_DESC 		m_State;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture1dConfigDX11_h
//--------------------------------------------------------------------------------

