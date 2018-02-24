//--------------------------------------------------------------------------------
// Texture2dDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture2dDX11_h
#define Texture2dDX11_h
//--------------------------------------------------------------------------------
#include "dx11_Hieroglyph/ResourceSystem/ResourceDX11.h"
#include "Texture2dConfigDX11.h"
#include <wrl.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture2dDX11 : public ResourceDX11
	{
	public:
		explicit Texture2dDX11( Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex );
		explicit Texture2dDX11( Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex, Texture2dConfig* pConfig);
		virtual ~Texture2dDX11();

		D3D11_TEXTURE2D_DESC		GetActualDescription();
		D3D11_TEXTURE2D_DESC		GetDesiredDescription();
		void						SetDesiredDescription( D3D11_TEXTURE2D_DESC description );

		virtual ResourceType		GetType();
		virtual ID3D11Resource*		GetResource();

		virtual u32					GetEvictionPriority();
		virtual void				SetEvictionPriority( u32 EvictionPriority );

		ID3D11ShaderResourceView*	GetSRV() const;
		ID3D11RenderTargetView*		GetRTV() const;
		ID3D11DepthStencilView*		GetDSV() const;

		i32	GetSRVID() const { return m_iResourceSRV; }
		i32 GetRTVID() const { return m_iResourceRTV; }
		i32 GetDSVID() const { return m_iResourceDSV; }

	protected:
		i32												m_iResourceSRV = -1;
		i32												m_iResourceRTV = -1;
		i32												m_iResourceDSV = -1;
		i32												m_iResourceUAV = -1;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			m_pTexture;
		D3D11_TEXTURE2D_DESC							m_DesiredDesc;
		D3D11_TEXTURE2D_DESC							m_ActualDesc;

	protected:
		void CreateResourceViews(u32 BindFlags,
			ShaderResourceViewConfigDX11* pSRVConfig,
			RenderTargetViewConfigDX11* pRTVConfig,
			UnorderedAccessViewConfigDX11* pUAVConfig,
			DepthStencilViewConfigDX11* pDSVConfig);

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture2dDX11_h
//--------------------------------------------------------------------------------

