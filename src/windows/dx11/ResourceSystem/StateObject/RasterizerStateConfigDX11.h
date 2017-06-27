
//--------------------------------------------------------------------------------
// RasterizerStateConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef RasterizerStateConfigDX11_h
#define RasterizerStateConfigDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include "render/FrameGraph/PipelineStateObjects.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class RasterizerStateConfigDX11
	{
	public:
		RasterizerStateConfigDX11();
		RasterizerStateConfigDX11(const RasterizerState& rasterizerState);
		~RasterizerStateConfigDX11();

		void SetDefaults();

		const D3D11_RASTERIZER_DESC& GetDesc() const;
		D3D11_RASTERIZER_DESC& GetDesc();

	protected:
		D3D11_RASTERIZER_DESC m_state;

	private:
		// Conversions from GTEngine values to DX11 values.
		static D3D11_FILL_MODE const msFillMode[];
		static D3D11_CULL_MODE const msCullMode[];
	};
};
//--------------------------------------------------------------------------------
#endif // RasterizerStateConfigDX11_h
//--------------------------------------------------------------------------------

