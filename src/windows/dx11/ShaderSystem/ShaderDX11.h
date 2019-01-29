#pragma once

#include "dxCommon/ShaderDX.h"
#include "dx11/dx11Util.h"
#include "HLSLParameter.h"
#include "HLSLConstantBuffer.h"
#include "HLSLResourceBindInfo.h"
#include "HLSLTextureBuffer.h"
#include "HLSLTexture.h"
#include "HLSLTextureArray.h"

namespace forward
{
	class ShaderDX11 : public ShaderDX
	{
	public:
		ShaderDX11(forward::FrameGraphObject* obj);
		virtual ~ShaderDX11();

		// Calls to ID3D11DeviceContext::XSSetShader.
		virtual void Bind(ID3D11DeviceContext* context) = 0;

		// Calls to ID3D11DeviceContext::XSSetConstantBuffers.
		virtual void BindCBuffer(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11Buffer* buffer) = 0;
		virtual void UnbindCBuffer(ID3D11DeviceContext* context,
			u32 bindPoint) = 0;

		// Calls to ID3D11DeviceContext::XSSetShaderResources.
		virtual void BindSRView(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11ShaderResourceView* srView) = 0;
		virtual void UnbindSRView(ID3D11DeviceContext* context,
			u32 bindPoint) = 0;

		// Calls to ID3D11DeviceContext::XSSetUnorderedAccessViews.
		//virtual void BindUAView(ID3D11DeviceContext* context,
		//	u32 bindPoint, ID3D11UnorderedAccessView* uaView,
		//	u32 initialCount) = 0;
		//virtual void UnbindUAView(ID3D11DeviceContext* context,
		//	u32 bindPoint) = 0;

		// Calls to ID3D11DeviceContext::XSSetSamplers.
		virtual void BindSampler(ID3D11DeviceContext* context,
			u32 bindPoint, ID3D11SamplerState* state) = 0;
		virtual void UnbindSampler(ID3D11DeviceContext* context,
			u32 bindPoint) = 0;

		DeviceObjComPtr		GetDeviceObject();

		void ReflectShader();
		void PostSetDeviceObject(forward::FrameGraphObject* obj) override;

		std::vector<HLSLConstantBuffer> const& GetCBuffers() const;
		std::vector<HLSLTextureBuffer> const& GetTBuffers() const;
		std::vector<HLSLTexture> const& GetTextures() const;
		std::vector<HLSLTextureArray> const& GetTextureArrays() const;

	protected:
		struct Description
		{
			struct InstructionCount
			{
				u32 numInstructions;
				u32 numTemporaryRegisters;
				u32 numTemporaryArrays;
				u32 numDefines;
				u32 numDeclarations;
				u32 numTextureNormal;
				u32 numTextureLoad;
				u32 numTextureComparison;
				u32 numTextureBias;
				u32 numTextureGradient;
				u32 numFloatArithmetic;
				u32 numSIntArithmetic;
				u32 numUIntArithmetic;
				u32 numStaticFlowControl;
				u32 numDynamicFlowControl;
				u32 numMacro;
				u32 numArray;
			};


			struct GSParameters
			{
				u32 numCutInstructions;
				u32 numEmitInstructions;
				D3D_PRIMITIVE inputPrimitive;
				D3D_PRIMITIVE_TOPOLOGY outputTopology;
				u32 maxOutputVertices;
			};

			struct TSParameters
			{
				u32 numPatchConstants;
				u32 numGSInstances;
				u32 numControlPoints;
				D3D_PRIMITIVE inputPrimitive;
				D3D_TESSELLATOR_OUTPUT_PRIMITIVE outputPrimitive;
				D3D_TESSELLATOR_PARTITIONING partitioning;
				D3D_TESSELLATOR_DOMAIN domain;
			};

			struct CSParameters
			{
				u32 numBarrierInstructions;
				u32 numInterlockedInstructions;
				u32 numTextureStoreInstructions;
			};

			std::string creator;
			D3D11_SHADER_VERSION_TYPE shaderType;
			u32 majorVersion;
			u32 minorVersion;
			u32 flags;
			u32 numConstantBuffers;
			u32 numBoundResources;
			u32 numInputParameters;
			u32 numOutputParameters;
			InstructionCount instructions;
			GSParameters gs;
			TSParameters ts;
			CSParameters cs;
		};

		void SetDescription(const D3D11_SHADER_DESC& desc);
		void InsertInput(HLSLParameter const& parameter);
		void InsertOutput(HLSLParameter const& parameter);
		void Insert(HLSLConstantBuffer const& cbuffer);
		void Insert(HLSLResourceBindInfo const& rbinfo);
		void Insert(HLSLTextureBuffer const& tbuffer);
		void Insert(HLSLTexture const& texture);
		void Insert(HLSLTextureArray const& tarray);

		static bool GetVariables(ID3D11ShaderReflectionConstantBuffer* cbuffer, 
			u32 numVariables, std::vector<HLSLBaseBuffer::Member>& members);
		static bool GetTypes(ID3D11ShaderReflectionType* rtype,
			u32 numMembers, HLSLShaderType& stype);
		static bool ShaderDX11::IsTextureArray(D3D_SRV_DIMENSION dim);

	protected:
		DeviceObjComPtr		m_deviceObjPtr;
		Description			m_desc;
		std::vector<HLSLParameter>	m_inputs;
		std::vector<HLSLParameter>	m_outputs;
		std::vector<HLSLConstantBuffer> m_CBuffers;
		std::vector<HLSLResourceBindInfo> m_RBInfos;
		std::vector<HLSLTextureBuffer> m_TBuffers;
		std::vector<HLSLTexture> m_Textures;
		std::vector<HLSLTextureArray> m_TextureArrays;
	};
}