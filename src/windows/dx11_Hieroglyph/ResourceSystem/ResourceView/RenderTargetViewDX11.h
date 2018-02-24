//--------------------------------------------------------------------------------
// RenderTargetViewDX11
//
//--------------------------------------------------------------------------------
#ifndef RenderTargetViewDX11_h
#define RenderTargetViewDX11_h
//--------------------------------------------------------------------------------
#include <wrl.h>
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	typedef Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetViewComPtr;

	class RenderTargetViewDX11
	{
	public:
		RenderTargetViewDX11( RenderTargetViewComPtr pView );
		~RenderTargetViewDX11();

		ID3D11RenderTargetView* GetRTV();

	protected:
		RenderTargetViewComPtr			m_pRenderTargetView;
		
		friend class PipelineManagerDX11;
		friend class OutputMergerStageDX11;
		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // RenderTargetViewDX11_h
//--------------------------------------------------------------------------------

