#include "DeviceSamplerStateDX11.h"

using namespace forward;

DeviceSamplerStateDX11::DeviceSamplerStateDX11(ID3D11Device* device, SamplerState* sampState)
	: DeviceDrawingStateDX11(sampState)
{
	// Specify the sampler state description.
	D3D11_SAMPLER_DESC desc;
	desc.Filter = msFilter[sampState->filter];
	desc.AddressU = msMode[sampState->mode[0]];
	desc.AddressV = msMode[sampState->mode[1]];
	desc.AddressW = msMode[sampState->mode[2]];
	desc.MipLODBias = sampState->mipLODBias;
	desc.MaxAnisotropy = sampState->maxAnisotropy;
	desc.ComparisonFunc = msComparison[sampState->comparison];
	desc.BorderColor[0] = sampState->borderColor[0];
	desc.BorderColor[1] = sampState->borderColor[1];
	desc.BorderColor[2] = sampState->borderColor[2];
	desc.BorderColor[3] = sampState->borderColor[3];
	desc.MinLOD = sampState->minLOD;
	desc.MaxLOD = sampState->maxLOD;

	// Create the sampler state
	SamplerStateComPtr sampler;
	HR(device->CreateSamplerState(&desc, sampler.GetAddressOf()));
	m_deviceObjPtr = sampler;
}

DeviceSamplerStateDX11::~DeviceSamplerStateDX11()
{

}

ID3D11SamplerState* DeviceSamplerStateDX11::GetSamplerStateDX11()
{
	return static_cast<ID3D11SamplerState*>(m_deviceObjPtr.Get());
}

shared_ptr<SamplerState> DeviceSamplerStateDX11::GetSamplerState()
{
	return m_frameGraphObjPtr.lock_down<SamplerState>();
}

D3D11_FILTER const DeviceSamplerStateDX11::msFilter[] =
{
	D3D11_FILTER_MIN_MAG_MIP_POINT,
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR,
	D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_MIN_MAG_MIP_LINEAR,
	D3D11_FILTER_ANISOTROPIC,
	D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
	D3D11_FILTER_COMPARISON_ANISOTROPIC
};

D3D11_TEXTURE_ADDRESS_MODE const DeviceSamplerStateDX11::msMode[] =
{
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_BORDER,
	D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
};

D3D11_COMPARISON_FUNC const DeviceSamplerStateDX11::msComparison[] =
{
	D3D11_COMPARISON_NEVER,
	D3D11_COMPARISON_LESS,
	D3D11_COMPARISON_EQUAL,
	D3D11_COMPARISON_LESS_EQUAL,
	D3D11_COMPARISON_GREATER,
	D3D11_COMPARISON_NOT_EQUAL,
	D3D11_COMPARISON_GREATER_EQUAL,
	D3D11_COMPARISON_ALWAYS
};