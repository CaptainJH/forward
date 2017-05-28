
//--------------------------------------------------------------------------------
// DepthStencilStateConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef DepthStencilStateConfigDX11_h
#define DepthStencilStateConfigDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class DepthStencilStateConfigDX11
	{
	public:
		DepthStencilStateConfigDX11();
		~DepthStencilStateConfigDX11();

		void SetDefaults();

		const D3D11_DEPTH_STENCIL_DESC& GetDesc() const;
		D3D11_DEPTH_STENCIL_DESC& GetDesc();

	protected:
		D3D11_DEPTH_STENCIL_DESC m_state;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // DepthStencilStateConfigDX11_h
//--------------------------------------------------------------------------------

