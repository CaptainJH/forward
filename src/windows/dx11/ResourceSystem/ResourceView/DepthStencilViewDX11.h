//--------------------------------------------------------------------------------
// DepthStencilViewDX11
//
//--------------------------------------------------------------------------------
#ifndef DepthStencilViewDX11_h
#define DepthStencilViewDX11_h
//--------------------------------------------------------------------------------
#include <wrl.h>
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	typedef Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilViewComPtr;

	class DepthStencilViewDX11
	{
	public:
		DepthStencilViewDX11( DepthStencilViewComPtr pView );
		~DepthStencilViewDX11();

	protected:
		DepthStencilViewComPtr			m_pDepthStencilView;
		
		friend class PipelineManagerDX11;
		friend class OutputMergerStageDX11;
		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // DepthStencilViewDX11_h
//--------------------------------------------------------------------------------

