//--------------------------------------------------------------------------------
// BufferDX11
//
// This interface provides the function interface to the information 
// contained in the D3D11 buffer class.  
//--------------------------------------------------------------------------------
#ifndef BufferDX11_h
#define BufferDX11_h
//--------------------------------------------------------------------------------
#include "ResourceSystem\ResourceDX11.h"
#include <wrl.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class BufferDX11 : public ResourceDX11
	{
	public:
		BufferDX11();
		virtual ~BufferDX11();

		D3D11_BUFFER_DESC			GetActualDescription();
		D3D11_BUFFER_DESC			GetDesiredDescription();
		void						SetDesiredDescription( D3D11_BUFFER_DESC desc );

		u32						GetByteWidth();
		void						SetByteWidth( u32 width );

		D3D11_USAGE					GetUsage();
		void						SetUsage( D3D11_USAGE usage );

		u32						GetBindFlags();
		void						SetBindFlags( u32 flags );

		u32						GetCPUAccessFlags();
		void						SetCPUAccessFlags( u32 flags );

		u32						GetMiscFlags();
		void						SetMiscFlags( u32 flags );

		u32						GetStructureByteStride();
		void						SetStructureByteStride( u32 stride );

		void*						Map();
		void						UnMap();

		virtual ID3D11Resource*				GetResource();

		virtual u32						GetEvictionPriority();
		virtual void						SetEvictionPriority( u32 EvictionPriority );

	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_pBuffer;
		D3D11_BUFFER_DESC							m_DesiredDesc;
		D3D11_BUFFER_DESC							m_ActualDesc;

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // BufferDX11_h
//--------------------------------------------------------------------------------

