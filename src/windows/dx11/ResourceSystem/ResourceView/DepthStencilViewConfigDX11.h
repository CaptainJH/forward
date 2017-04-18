//--------------------------------------------------------------------------------
// DepthStencilViewConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef DepthStencilViewConfigDX11_h
#define DepthStencilViewConfigDX11_h
//--------------------------------------------------------------------------------
#include "Types.h"
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class DepthStencilViewConfigDX11
	{
	public:
		DepthStencilViewConfigDX11();
		virtual ~DepthStencilViewConfigDX11();

		void SetDefaults();

		void SetFormat( DXGI_FORMAT state );
		void SetViewDimensions( D3D11_DSV_DIMENSION state );
		void SetFlags( u32 state );

        void SetTexture1D( D3D11_TEX1D_DSV state );
        void SetTexture1DArray( D3D11_TEX1D_ARRAY_DSV state );
        void SetTexture2D( D3D11_TEX2D_DSV state );
        void SetTexture2DArray( D3D11_TEX2D_ARRAY_DSV Texture2DArray );
        void SetTexture2DMS( D3D11_TEX2DMS_DSV state );
        void SetTexture2DMSArray( D3D11_TEX2DMS_ARRAY_DSV state );
		
		D3D11_DEPTH_STENCIL_VIEW_DESC& GetDSVDesc();

	protected:
		D3D11_DEPTH_STENCIL_VIEW_DESC 		m_State;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // DepthStencilViewConfigDX11_h
//--------------------------------------------------------------------------------

