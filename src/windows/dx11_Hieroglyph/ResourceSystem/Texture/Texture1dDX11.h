
//--------------------------------------------------------------------------------
// Texture1dDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture1dDX11_h
#define Texture1dDX11_h
//--------------------------------------------------------------------------------
#include "dx11_Hieroglyph/ResourceSystem/ResourceDX11.h"
#include <wrl.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture1dDX11 : public ResourceDX11
	{
	public:
		explicit Texture1dDX11( Microsoft::WRL::ComPtr<ID3D11Texture1D> pTex );
		virtual ~Texture1dDX11();

		D3D11_TEXTURE1D_DESC		GetActualDescription();
		D3D11_TEXTURE1D_DESC		GetDesiredDescription();
		void						SetDesiredDescription( D3D11_TEXTURE1D_DESC description );

		virtual ResourceType		GetType();
		virtual ID3D11Resource*		GetResource();

		virtual u32					GetEvictionPriority();
		virtual void				SetEvictionPriority( u32 EvictionPriority );

		ID3D11ShaderResourceView*	GetSRV() const;

	protected:
		i32												m_iResourceSRV = -1;
		Microsoft::WRL::ComPtr<ID3D11Texture1D>			m_pTexture;
		D3D11_TEXTURE1D_DESC							m_DesiredDesc;
		D3D11_TEXTURE1D_DESC							m_ActualDesc;

	protected:
		void CreateSRV();

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture1dDX11_h
//--------------------------------------------------------------------------------

