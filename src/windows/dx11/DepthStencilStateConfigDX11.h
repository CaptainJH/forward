
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
	class DepthStencilStateConfigDX11 : public D3D11_DEPTH_STENCIL_DESC
	{
	public:
		DepthStencilStateConfigDX11();
		~DepthStencilStateConfigDX11();

		void SetDefaults();

	protected:

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // DepthStencilStateConfigDX11_h
//--------------------------------------------------------------------------------

