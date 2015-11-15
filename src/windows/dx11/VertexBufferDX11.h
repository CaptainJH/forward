//--------------------------------------------------------------------------------
// VertexBufferDX11 
//
//--------------------------------------------------------------------------------
#ifndef VertexBufferDX11_h
#define VertexBufferDX11_h
//--------------------------------------------------------------------------------
#include "BufferDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class VertexBufferDX11 : public BufferDX11
	{
	public:
		VertexBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer );
		virtual ~VertexBufferDX11();

		virtual ResourceType				GetType();

		void						SetVertexSize( i32 size );
		void						SetVertexCount( i32 count );

	protected:
		i32							m_iVertexSize;
		i32							m_iVertexCount;

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // VertexBufferDX11_h
