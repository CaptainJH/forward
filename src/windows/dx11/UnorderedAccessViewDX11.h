
//--------------------------------------------------------------------------------
// UnorderedAccessViewDX11
//
//--------------------------------------------------------------------------------
#ifndef UnorderedAccessViewDX11_h
#define UnorderedAccessViewDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include <wrl.h>
//--------------------------------------------------------------------------------
namespace forward
{
	typedef Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> UnorderedAccessViewComPtr;

	class UnorderedAccessViewDX11
	{
	public:
		UnorderedAccessViewDX11( UnorderedAccessViewComPtr pView );
		~UnorderedAccessViewDX11();

	protected:
		UnorderedAccessViewComPtr			m_pUnorderedAccessView;
		
		//friend OutputMergerStageDX11;
		//friend PipelineManagerDX11;
		//friend RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // UnorderedAccessViewDX11_h
//--------------------------------------------------------------------------------

