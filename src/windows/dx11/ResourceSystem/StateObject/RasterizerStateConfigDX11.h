
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
	class RasterizerStateConfigDX11
	{
	public:
		RasterizerStateConfigDX11();
		~RasterizerStateConfigDX11();

		void SetDefaults();

		const D3D11_RASTERIZER_DESC& GetDesc() const;
		D3D11_RASTERIZER_DESC& GetDesc();

	protected:
		D3D11_RASTERIZER_DESC m_state;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // RasterizerStateConfigDX11_h
//--------------------------------------------------------------------------------

