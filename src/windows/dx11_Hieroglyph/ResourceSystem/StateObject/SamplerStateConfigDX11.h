
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
	class SamplerStateConfigDX11
	{
	public:
		SamplerStateConfigDX11();
		~SamplerStateConfigDX11();

		void SetDefaults();

		const D3D11_SAMPLER_DESC& GetDesc() const;
		D3D11_SAMPLER_DESC& GetDesc();

	protected:
		D3D11_SAMPLER_DESC m_state;

		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // SamplerStateConfigDX11_h
//--------------------------------------------------------------------------------

