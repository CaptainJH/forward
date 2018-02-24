
//--------------------------------------------------------------------------------
// DepthStencilStateConfigDX11
//
//--------------------------------------------------------------------------------
#ifndef DepthStencilStateConfigDX11_h
#define DepthStencilStateConfigDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include "render/FrameGraph/PipelineStateObjects.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class DepthStencilStateConfigDX11
	{
	public:
		DepthStencilStateConfigDX11();
		DepthStencilStateConfigDX11(const DepthStencilState& depthStencilState);
		~DepthStencilStateConfigDX11();

		void SetDefaults();

		const D3D11_DEPTH_STENCIL_DESC& GetDesc() const;
		D3D11_DEPTH_STENCIL_DESC& GetDesc();

		bool operator==(const DepthStencilStateConfigDX11& rhs) const;

	protected:
		D3D11_DEPTH_STENCIL_DESC m_state;

	private:
		// Conversions from GTEngine values to DX11 values.
		static D3D11_DEPTH_WRITE_MASK const msWriteMask[];
		static D3D11_COMPARISON_FUNC const msComparison[];
		static D3D11_STENCIL_OP const msOperation[];
	};
};
//--------------------------------------------------------------------------------
#endif // DepthStencilStateConfigDX11_h
//--------------------------------------------------------------------------------

