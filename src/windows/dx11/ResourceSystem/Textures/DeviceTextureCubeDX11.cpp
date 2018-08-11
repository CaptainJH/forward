//***************************************************************************************
// DeviceTextureCubeDX11.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "DeviceTextureCubeDX11.h"

using namespace forward;

DeviceTextureCubeDX11::DeviceTextureCubeDX11(ID3D11Device* device, FrameGraphTextureCube* tex)
	: DeviceTextureDX11(tex)
{
	D3D11_TEXTURE2D_DESC desc;
	// Specify the texture description.
	desc.Width = tex->GetWidth();
	desc.Height = tex->GetHeight();
	desc.MipLevels = tex->GetMipLevelNum();
	desc.ArraySize = 6;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	const auto TBP = tex->GetBindPosition();
	assert(TBP == TBP_Shader);

	auto usage = tex->GetUsage();
	if (usage == ResourceUsage::RU_IMMUTABLE)
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}
	else if (usage == ResourceUsage::RU_DYNAMIC_UPDATE)
	{
		// DX11 does not allow a cube map to be a dynamic-update resource.
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}
	else if (usage == ResourceUsage::RU_SHADER_OUTPUT)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
	}

	if (tex->WantAutoGenerateMips() && CanAutoGenerateMips(tex, device))
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_NONE;
		desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	// Create the texture.
	Texture2DComPtr dxTexture;
	if (tex->GetData() && tex->IsFileTexture())
	{

	}
}

ID3D11Texture2D* DeviceTextureCubeDX11::GetDXTexture2DPtr()
{
	if (m_deviceResPtr)
	{
		return static_cast<ID3D11Texture2D*>(m_deviceResPtr.Get());
	}

	return nullptr;
}

shared_ptr<FrameGraphTextureCube> DeviceTextureCubeDX11::GetFrameGraphTextureCube()
{
	auto ptr = FrameGraphObject();
	forward::FrameGraphObject* p_obj = ptr.get();
	auto p = dynamic_cast<FrameGraphTextureCube*>(p_obj);

	return shared_ptr<FrameGraphTextureCube>(p);
}