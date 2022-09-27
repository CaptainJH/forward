#include "ShaderDX12.h"
#include "RHI/ShaderSystem/Shader.h"
#include "dxCommon/ShaderFactoryDX.h"

using namespace forward;

ShaderDX12::ShaderDX12(forward::Shader* shader)
	: ShaderDX(shader)
{
	auto type = shader->GetType();
	switch (type)
	{
	case FGOT_VERTEX_SHADER:
		m_shaderType = VERTEX_SHADER;
		ShaderModel = L"vs";
		break;

	case FGOT_PIXEL_SHADER:
		m_shaderType = PIXEL_SHADER;
		ShaderModel = L"ps";
		break;

	case FGOT_GEOMETRY_SHADER:
		m_shaderType = GEOMETRY_SHADER;
		ShaderModel = L"gs";
		break;

	case FGOT_COMPUTE_SHADER:
		m_shaderType = COMPUTE_SHADER;
		ShaderModel = L"cs";
		break;

	default:
		assert(false);
	}

	ShaderModel += L"_5_0";

	m_pCompiledShader = ShaderFactoryDX::GenerateShader(GetType(),
		shader->GetShaderFile(), shader->GetShaderEntry(), ShaderModel);
	assert(m_pCompiledShader);

	ReflectShader();
}

ShaderDX12::~ShaderDX12()
{}

ShaderType ShaderDX12::GetType() const
{
	return m_shaderType;
}

void ShaderDX12::ReflectShader()
{
	assert(m_pCompiledShader);
	const void* buffer = m_pCompiledShader->GetBufferPointer();
	auto numBytes = m_pCompiledShader->GetBufferSize();

	ID3D12ShaderReflection* reflector = nullptr;
	HR(D3DReflect(buffer, numBytes, IID_ID3D12ShaderReflection, (void**)&reflector));

	{
		/// get description
		D3D12_SHADER_DESC desc;
		HR(reflector->GetDesc(&desc));
		SetDescription(desc);
	}

	{
		/// get shader inputs
		const auto numInputs = m_desc.numInputParameters;
		for (auto i = 0U; i < numInputs; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC spDesc;
			HR(reflector->GetInputParameterDesc(i, &spDesc));
			InsertInput(HLSLParameter(spDesc));
		}
	}

	{
		/// get shader outputs
		const auto numOutputs = m_desc.numOutputParameters;
		for (auto i = 0U; i < numOutputs; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC spDesc;
			HR(reflector->GetOutputParameterDesc(i, &spDesc));
			InsertOutput(HLSLParameter(spDesc));
		}
	}

	{
		/// get constant buffers
		auto const numCBuffers = m_desc.numConstantBuffers;
		for (auto i = 0U; i < numCBuffers; ++i)
		{
			ID3D12ShaderReflectionConstantBuffer* cbuffer = reflector->GetConstantBufferByIndex(i);

			D3D12_SHADER_BUFFER_DESC cbDesc;
			HR(cbuffer->GetDesc(&cbDesc));

			D3D12_SHADER_INPUT_BIND_DESC resDesc;
			HR(reflector->GetResourceBindingDescByName(cbDesc.Name, &resDesc));

			if (cbDesc.Type == D3D_CT_CBUFFER)
			{
				std::vector<HLSLBaseBuffer::Member> members;
				if (!GetVariables(cbuffer, cbDesc.Variables, members))
				{
					continue;
				}

				if (resDesc.BindCount == 1)
				{
					Insert(HLSLConstantBuffer(resDesc, cbDesc.Size, members));
				}
				else
				{
					for (UINT j = 0; j < resDesc.BindCount; ++j)
					{
						Insert(HLSLConstantBuffer(resDesc, j, cbDesc.Size, members));
					}
				}
			}
			else if (cbDesc.Type == D3D_CT_TBUFFER)
			{
				std::vector<HLSLBaseBuffer::Member> members;
				if (!GetVariables(cbuffer, cbDesc.Variables, members))
				{
					continue;
				}

				if (resDesc.BindCount == 1)
				{
					Insert(HLSLTextureBuffer(resDesc, cbDesc.Size, members));
				}
				else
				{
					for (auto j = 0U; j < resDesc.BindCount; ++j)
					{
						Insert(HLSLTextureBuffer(resDesc, j,
							cbDesc.Size, members));
					}
				}
			}
			else if (cbDesc.Type == D3D_CT_RESOURCE_BIND_INFO)
			{
				std::vector<HLSLBaseBuffer::Member> members;
				if (!GetVariables(cbuffer, cbDesc.Variables, members))
				{
					continue;
				}

				if (resDesc.BindCount == 1)
				{
					Insert(HLSLResourceBindInfo(resDesc, cbDesc.Size, members));
				}
				else
				{
					for (auto j = 0U; j < resDesc.BindCount; ++j)
					{
						Insert(HLSLResourceBindInfo(resDesc, j, cbDesc.Size, members));
					}
				}
			}
			else  // cbDesc.Type == D3D_CT_INTERFACE_POINTERS
			{
				Log::Get().Write(L"Interface pointers are not yet supported in forward.");
			}
		}
	}

	{
		/// get bound resource
		// TODO: It appears that D3DCompile never produces a resource with a bind
		// count larger than 1.  For example, "Texture2D texture[2];" comes
		// through with names "texture[0]" and "texture[1]", each having a bind
		// count of 1 (with consecutive bind points).  So it appears that passing
		// a bind count in D3D interfaces that is larger than 1 is something a
		// programmer can do only if he manually combines the texture resources
		// into his own data structure.  If the statement about D3DCompiler is
		// true, we do not need the loops with upper bound resDesc.BindCount.

		UINT const numResources = m_desc.numBoundResources;
		for (UINT i = 0; i < numResources; ++i)
		{
			D3D12_SHADER_INPUT_BIND_DESC resDesc;
			HRESULT hr = reflector->GetResourceBindingDesc(i, &resDesc);
			if (FAILED(hr))
			{
				auto str = L"reflector->GetResourceBindingDesc error, hr = " + std::to_wstring(hr);
				Log::Get().Write(str);
				continue;
			}

			if (resDesc.Type == D3D_SIT_CBUFFER     // cbuffer
				|| resDesc.Type == D3D_SIT_TBUFFER)    // tbuffer
			{
				// These were processed in the previous loop.
			}
			else if (resDesc.Type == D3D_SIT_TEXTURE        // Texture*
				|| resDesc.Type == D3D_SIT_UAV_RWTYPED)   // RWTexture*
			{
				if (IsTextureArray(resDesc.Dimension))
				{
					if (resDesc.BindCount == 1)
					{
						Insert(HLSLTextureArray(resDesc));
					}
					else
					{
						for (auto j = 0U; j < resDesc.BindCount; ++j)
						{
							Insert(HLSLTextureArray(resDesc, j));
						}
					}
				}
				else
				{
					if (resDesc.BindCount == 1)
					{
						Insert(HLSLTexture(resDesc));
					}
					else
					{
						for (auto j = 0U; j < resDesc.BindCount; ++j)
						{
							Insert(HLSLTexture(resDesc, j));
						}
					}
				}
			}
			//else if (resDesc.Type == D3D_SIT_SAMPLER)   // SamplerState
			//{
			//	if (resDesc.BindCount == 1)
			//	{
			//		shader.Insert(HLSLSamplerState(resDesc));
			//	}
			//	else
			//	{
			//		for (UINT j = 0; j < resDesc.BindCount; ++j)
			//		{
			//			shader.Insert(HLSLSamplerState(resDesc, j));
			//		}
			//	}
			//}
			//else if (
			//	resDesc.Type == D3D_SIT_BYTEADDRESS         // ByteAddressBuffer
			//	|| resDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)  // RWByteAddressBuffer
			//{
			//	if (resDesc.BindCount == 1)
			//	{
			//		shader.Insert(HLSLByteAddressBuffer(resDesc));
			//	}
			//	else
			//	{
			//		for (UINT j = 0; j < resDesc.BindCount; ++j)
			//		{
			//			shader.Insert(HLSLByteAddressBuffer(resDesc, j));
			//		}
			//	}
			//}
			//else
			//{
			//	// D3D_SIT_STRUCTURED:  StructuredBuffer
			//	// D3D_SIT_UAV_RWSTRUCTURED:  RWStructuredBuffer
			//	// D3D_SIT_UAV_APPEND_STRUCTURED:  AppendStructuredBuffer
			//	// D3D_SIT_UAV_CONSUME_STRUCTURED:  ConsumeStructuredBuffer
			//	// D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:  RWStructuredBuffer

			//	if (resDesc.BindCount == 1)
			//	{
			//		shader.Insert(HLSLStructuredBuffer(resDesc));
			//	}
			//	else
			//	{
			//		for (UINT j = 0; j < resDesc.BindCount; ++j)
			//		{
			//			shader.Insert(HLSLStructuredBuffer(resDesc, j));
			//		}
			//	}
			//}
		}
	}

	reflector->Release();
}

bool ShaderDX12::GetVariables(ID3D12ShaderReflectionConstantBuffer* cbuffer,
	u32 numVariables, std::vector<HLSLBaseBuffer::Member>& members)
{
	for (auto i = 0U; i < numVariables; ++i)
	{
		ID3D12ShaderReflectionVariable* var = cbuffer->GetVariableByIndex(i);
		ID3D12ShaderReflectionType* varType = var->GetType();

		D3D12_SHADER_VARIABLE_DESC varDesc;
		HRESULT hr = var->GetDesc(&varDesc);
		if (FAILED(hr))
		{
			auto str = L"var->GetDesc error, hr = " + std::to_wstring(hr);
			Log::Get().Write(str);
			return false;
		}

		D3D12_SHADER_TYPE_DESC varTypeDesc;
		hr = varType->GetDesc(&varTypeDesc);
		if (FAILED(hr))
		{
			auto str = L"varType->GetDesc error, hr = " + std::to_wstring(hr);
			Log::Get().Write(str);
			return false;
		}

		// Get the top-level information about the shader variable.
		HLSLShaderVariable svar;
		svar.SetDescription(varDesc);

		// Get the type of the variable.  If this is a struct type, the
		// call recurses to build the type tree implied by the struct.
		HLSLShaderType stype;
		stype.SetName(svar.GetName());
		stype.SetDescription(varTypeDesc);
		if (!GetTypes(varType, varTypeDesc.Members, stype))
		{
			return false;
		}

		members.push_back(std::make_pair(svar, stype));
	}
	return true;
}

bool ShaderDX12::GetTypes(ID3D12ShaderReflectionType* rtype,
	u32 numMembers, HLSLShaderType& stype)
{
	for (auto i = 0U; i < numMembers; ++i)
	{
		ID3D12ShaderReflectionType* memType = rtype->GetMemberTypeByIndex(i);
		char const* memTypeName = rtype->GetMemberTypeName(i);
		std::string memName(memTypeName ? memTypeName : "");
		D3D12_SHADER_TYPE_DESC memTypeDesc;
		HRESULT hr = memType->GetDesc(&memTypeDesc);
		if (FAILED(hr))
		{
			auto str = L"memType->GetDesc error, hr = " + std::to_wstring(hr);
			Log::Get().Write(str);
			return false;
		}
		HLSLShaderType& child = stype.GetChild(i);
		child.SetName(memName);
		child.SetDescription(memTypeDesc);
		GetTypes(memType, memTypeDesc.Members, child);
	}
	return true;
}

bool ShaderDX12::IsTextureArray(D3D_SRV_DIMENSION dim)
{
	return dim == D3D_SRV_DIMENSION_TEXTURE1DARRAY
		|| dim == D3D_SRV_DIMENSION_TEXTURE2DARRAY
		|| dim == D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
		|| dim == D3D_SRV_DIMENSION_TEXTURECUBE
		|| dim == D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
}
