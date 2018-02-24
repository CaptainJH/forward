//--------------------------------------------------------------------------------
// BlendStateConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef BlendStateConfigDX11_h
#define BlendStateConfigDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include "render/FrameGraph/PipelineStateObjects.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class BlendStateConfigDX11
	{
	public:
		BlendStateConfigDX11();
		BlendStateConfigDX11(const BlendState& blendState);
		~BlendStateConfigDX11();

		void SetDefaults();

		const D3D11_BLEND_DESC& GetDesc() const;
		D3D11_BLEND_DESC& GetDesc();

		bool operator==(const BlendStateConfigDX11& rhs) const;

	protected:
		D3D11_BLEND_DESC m_state;

	private:
		// Conversions from GTEngine values to DX11 values.
		static D3D11_BLEND const msMode[];
		static D3D11_BLEND_OP const msOperation[];
	};
};
//--------------------------------------------------------------------------------
#endif // BlendStateConfigDX11_h
//--------------------------------------------------------------------------------

