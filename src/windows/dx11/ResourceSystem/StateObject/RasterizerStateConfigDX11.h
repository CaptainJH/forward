
//--------------------------------------------------------------------------------
// RasterizerStateConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef RasterizerStateConfigDX11_h
#define RasterizerStateConfigDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class RasterizerStateConfigDX11 : public D3D11_RASTERIZER_DESC
	{
	public:
		RasterizerStateConfigDX11();
		~RasterizerStateConfigDX11();

		void SetDefaults();

	protected:

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // RasterizerStateConfigDX11_h
//--------------------------------------------------------------------------------

