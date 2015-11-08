//--------------------------------------------------------------------------------
// ShaderResourceViewDX11
//
//--------------------------------------------------------------------------------
#ifndef ShaderResourceViewDX11_h
#define ShaderResourceViewDX11_h
//--------------------------------------------------------------------------------
#include <d3d11_2.h>
#include <wrl.h>
//--------------------------------------------------------------------------------
namespace forward
{
	typedef Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceViewComPtr;

	class ShaderResourceViewDX11
	{
	public:
		ShaderResourceViewDX11( ShaderResourceViewComPtr pView );
		~ShaderResourceViewDX11();

		ID3D11ShaderResourceView* GetSRV();

	protected:
		ShaderResourceViewComPtr		m_pShaderResourceView;
		
		friend class PipelineManagerDX11;
		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // ShaderResourceViewDX11_h
//--------------------------------------------------------------------------------

