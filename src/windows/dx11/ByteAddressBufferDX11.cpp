
//--------------------------------------------------------------------------------
#include "ByteAddressBufferDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
ByteAddressBufferDX11::ByteAddressBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer )
{
	m_pBuffer = pBuffer;
}
//--------------------------------------------------------------------------------
ByteAddressBufferDX11::~ByteAddressBufferDX11()
{
}
//--------------------------------------------------------------------------------
ResourceType ByteAddressBufferDX11::GetType()
{
	return( RT_BYTEADDRESSBUFFER );
}
//--------------------------------------------------------------------------------
