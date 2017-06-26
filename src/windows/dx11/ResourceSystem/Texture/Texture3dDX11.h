//--------------------------------------------------------------------------------
// Texture3dDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture3dDX11_h
#define Texture3dDX11_h
//--------------------------------------------------------------------------------
#include "dx11/ResourceSystem/ResourceDX11.h"
#include "Texture3dConfigDX11.h"
#include <wrl.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture3dDX11 : public ResourceDX11
	{
	public:
		explicit Texture3dDX11( Microsoft::WRL::ComPtr<ID3D11Texture3D> pTex );
		explicit Texture3dDX11(Microsoft::WRL::ComPtr<ID3D11Texture3D> pTex, Texture3dConfig* pConfig);
		virtual ~Texture3dDX11();

		D3D11_TEXTURE3D_DESC		GetActualDescription();
		D3D11_TEXTURE3D_DESC		GetDesiredDescription();
		void						SetDesiredDescription( D3D11_TEXTURE3D_DESC description );

		virtual ResourceType		GetType();
		virtual ID3D11Resource*		GetResource();

		virtual u32					GetEvictionPriority();
		virtual void				SetEvictionPriority( u32 EvictionPriority );

		ID3D11ShaderResourceView*	GetSRV() const;

		i32 GetSRVID() const { return m_iResourceSRV; }

	protected:
		i32											m_iResourceSRV = -1;
		Microsoft::WRL::ComPtr<ID3D11Texture3D>		m_pTexture;
		D3D11_TEXTURE3D_DESC						m_DesiredDesc;
		D3D11_TEXTURE3D_DESC						m_ActualDesc;

		void CreateSRV(u32 BindFlags, ShaderResourceViewConfigDX11* pSRVConfig);

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture3dDX11_h
//--------------------------------------------------------------------------------

