//--------------------------------------------------------------------------------
// StructuredBufferDX11 
//
//--------------------------------------------------------------------------------
#ifndef StructuredBufferDX11_h
#define StructuredBufferDX11_h
//--------------------------------------------------------------------------------
#include "BufferDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class StructuredBufferDX11 : public BufferDX11
	{
	public:
		StructuredBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer );
		virtual ~StructuredBufferDX11();

		virtual ResourceType				GetType();

	protected:

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // StructuredBufferDX11_h
