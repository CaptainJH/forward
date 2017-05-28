//--------------------------------------------------------------------------------
// BlendStateConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef BlendStateConfigDX11_h
#define BlendStateConfigDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class BlendStateConfigDX11
	{
	public:
		BlendStateConfigDX11();
		~BlendStateConfigDX11();

		void SetDefaults();

		const D3D11_BLEND_DESC& GetDesc() const;
		D3D11_BLEND_DESC& GetDesc();

	protected:
		D3D11_BLEND_DESC m_state;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // BlendStateConfigDX11_h
//--------------------------------------------------------------------------------

