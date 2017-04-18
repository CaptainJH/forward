//--------------------------------------------------------------------------------
// Texture2dDX11
//
//--------------------------------------------------------------------------------
#ifndef Texture2dDX11_h
#define Texture2dDX11_h
//--------------------------------------------------------------------------------
#include "ResourceSystem\ResourceDX11.h"
#include <wrl.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class Texture2dDX11 : public ResourceDX11
	{
	public:
		explicit Texture2dDX11( Microsoft::WRL::ComPtr<ID3D11Texture2D> pTex );
		virtual ~Texture2dDX11();

		D3D11_TEXTURE2D_DESC		GetActualDescription();
		D3D11_TEXTURE2D_DESC		GetDesiredDescription();
		void						SetDesiredDescription( D3D11_TEXTURE2D_DESC description );

		virtual ResourceType				GetType();
		virtual ID3D11Resource*				GetResource();

		virtual u32						GetEvictionPriority();
		virtual void						SetEvictionPriority( u32 EvictionPriority );

	protected:
		Microsoft::WRL::ComPtr<ID3D11Texture2D>			m_pTexture;
		D3D11_TEXTURE2D_DESC							m_DesiredDesc;
		D3D11_TEXTURE2D_DESC							m_ActualDesc;

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // Texture2dDX11_h
//--------------------------------------------------------------------------------

