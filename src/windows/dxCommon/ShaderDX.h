//***************************************************************************************
// ShaderDX.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "dxCommon/d3dUtil.h"
#include <d3dCompiler.h>
#include "render/ResourceSystem/DeviceObject.h"
#include "dxCommon/ShaderReflection/HLSLParameter.h"
#include "dxCommon/ShaderReflection/HLSLConstantBuffer.h"
#include "dxCommon/ShaderReflection/HLSLResourceBindInfo.h"
#include "dxCommon/ShaderReflection/HLSLTextureBuffer.h"
#include "dxCommon/ShaderReflection/HLSLTexture.h"
#include "dxCommon/ShaderReflection/HLSLTextureArray.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class GraphicsObject;

	enum ShaderType
	{
		VERTEX_SHADER = 0,
		HULL_SHADER = 1,
		DOMAIN_SHADER = 2,
		GEOMETRY_SHADER = 3,
		PIXEL_SHADER = 4,
		COMPUTE_SHADER = 5
	};


	enum ShaderMask
	{
		VERTEX_SHADER_MSK = 0x0001,
		HULL_SHADER_MSK = 0x0002,
		DOMAIN_SHADER_MSK = 0x0004,
		GEOMETRY_SHADER_MSK = 0x0008,
		PIXEL_SHADER_MSK = 0x0010,
		COMPUTE_SHADER_MSK = 0x0020
	};

	class ShaderDX : public DeviceObject
	{
	public:
		/// TODO: just for backward compatibility to project forwardDX11_Hieroglyph
		ShaderDX();
		ShaderDX(forward::GraphicsObject* obj);
		virtual ~ShaderDX();

		virtual ShaderType GetType() const = 0;

		ID3DBlob*	GetCompiledCode();	

		std::wstring ToString();

		std::vector<HLSLConstantBuffer> const& GetCBuffers() const;
		std::vector<HLSLTextureBuffer> const& GetTBuffers() const;
		std::vector<HLSLTexture> const& GetTextures() const;
		std::vector<HLSLTextureArray> const& GetTextureArrays() const;

	public:
		std::wstring							FileName;
		std::wstring							Function;
		std::wstring							ShaderModel;
		std::string								ShaderText;
		ID3DBlob*								m_pCompiledShader;

	protected:
		void InsertInput(HLSLParameter const& parameter);
		void InsertOutput(HLSLParameter const& parameter);
		void Insert(HLSLConstantBuffer const& cbuffer);
		void Insert(HLSLResourceBindInfo const& rbinfo);
		void Insert(HLSLTextureBuffer const& tbuffer);
		void Insert(HLSLTexture const& texture);
		void Insert(HLSLTextureArray const& tarray);

		template<class D3D_SHADER_DESC>
		void SetDescription(const D3D_SHADER_DESC& desc)
		{
			m_desc.creator = std::string(desc.Creator);
			m_desc.flags = desc.Flags;
			m_desc.numConstantBuffers = desc.ConstantBuffers;
			m_desc.numBoundResources = desc.BoundResources;
			m_desc.numInputParameters = desc.InputParameters;
			m_desc.numOutputParameters = desc.OutputParameters;

			m_desc.instructions.numInstructions = desc.InstructionCount;
			m_desc.instructions.numTemporaryRegisters = desc.TempRegisterCount;
			m_desc.instructions.numTemporaryArrays = desc.TempArrayCount;
			m_desc.instructions.numDefines = desc.DefCount;
			m_desc.instructions.numDeclarations = desc.DclCount;
			m_desc.instructions.numTextureNormal = desc.TextureNormalInstructions;
			m_desc.instructions.numTextureLoad = desc.TextureLoadInstructions;
			m_desc.instructions.numTextureComparison = desc.TextureCompInstructions;
			m_desc.instructions.numTextureBias = desc.TextureBiasInstructions;
			m_desc.instructions.numTextureGradient = desc.TextureGradientInstructions;
			m_desc.instructions.numFloatArithmetic = desc.FloatInstructionCount;
			m_desc.instructions.numSIntArithmetic = desc.IntInstructionCount;
			m_desc.instructions.numUIntArithmetic = desc.UintInstructionCount;
			m_desc.instructions.numStaticFlowControl = desc.StaticFlowControlCount;
			m_desc.instructions.numDynamicFlowControl = desc.DynamicFlowControlCount;
			m_desc.instructions.numMacro = desc.MacroInstructionCount;
			m_desc.instructions.numArray = desc.ArrayInstructionCount;

			m_desc.gs.numCutInstructions = desc.CutInstructionCount;
			m_desc.gs.numEmitInstructions = desc.EmitInstructionCount;
			m_desc.gs.inputPrimitive = desc.InputPrimitive;
			m_desc.gs.outputTopology = desc.GSOutputTopology;
			m_desc.gs.maxOutputVertices = desc.GSMaxOutputVertexCount;

			m_desc.ts.numPatchConstants = desc.PatchConstantParameters;
			m_desc.ts.numGSInstances = desc.cGSInstanceCount;
			m_desc.ts.numControlPoints = desc.cControlPoints;
			m_desc.ts.inputPrimitive = desc.InputPrimitive;
			m_desc.ts.outputPrimitive = desc.HSOutputPrimitive;
			m_desc.ts.partitioning = desc.HSPartitioning;
			m_desc.ts.domain = desc.TessellatorDomain;

			m_desc.cs.numBarrierInstructions = desc.cBarrierInstructions;
			m_desc.cs.numInterlockedInstructions = desc.cInterlockedInstructions;
			m_desc.cs.numTextureStoreInstructions = desc.cTextureStoreInstructions;
		}

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
			ShaderType shaderType;
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

		Description			m_desc;
		std::vector<HLSLParameter>	m_inputs;
		std::vector<HLSLParameter>	m_outputs;
		std::vector<HLSLConstantBuffer> m_CBuffers;
		std::vector<HLSLResourceBindInfo> m_RBInfos;
		std::vector<HLSLTextureBuffer> m_TBuffers;
		std::vector<HLSLTexture> m_Textures;
		std::vector<HLSLTextureArray> m_TextureArrays;
	};
};
//--------------------------------------------------------------------------------

