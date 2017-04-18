
//--------------------------------------------------------------------------------
// SamplerStateConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef SamplerStateConfigDX11_h
#define SamplerStateConfigDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class SamplerStateConfigDX11 : public D3D11_SAMPLER_DESC
	{
	public:
		SamplerStateConfigDX11();
		~SamplerStateConfigDX11();

		void SetDefaults();

	protected:

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // SamplerStateConfigDX11_h
//--------------------------------------------------------------------------------

