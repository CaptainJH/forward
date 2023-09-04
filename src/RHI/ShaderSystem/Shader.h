//***************************************************************************************
// Shader.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "RHI/ResourceSystem/GraphicsObject.h"
#include "RHI/MemberLayout.h"
#include <unordered_map>

namespace forward
{

	class Shader : public GraphicsObject
	{
	public:
		Shader(const i8* name, const WString shaderFile, const i8* entryFunction);
		Shader(const i8* name, const String shaderText);

		const std::string& GetShaderText() const;
		const std::wstring& GetShaderFile() const;
		const std::string& GetShaderEntry() const;

		enum
		{
			ConstantBufferShaderDataLookup = 0,     // CB
			TextureBufferShaderDataLookup = 1,      // TB
			StructuredBufferShaderDataLookup = 2,   // SB
			RawBufferShaderDataLookup = 3,          // RB
			TextureSingleShaderDataLookup = 4,      // TX
			TextureArrayShaderDataLookup = 5,       // TS
			SamplerStateShaderDataLookup = 6,       // SS
			NUM_LOOKUP_INDICES = 7
		};

		struct Data
		{
			Data(GraphicsObjectType inType, std::string const& inName,
				i32 inBindPoint, i32 inNumBytes, u32 inExtra,
				bool inIsGpuWritable);

			std::shared_ptr<GraphicsObject>	object;			// CB, TB, SB, RB, TX, TA, SS
			GraphicsObjectType				type;			// CB, TB, SB, RB, TX, TA, SS
			std::string							name;			// CB, TB, SB, RB, TX, TA, SS
			i32									bindPoint;		// CB, TB, SB, RB, TX, TA, SS
			i32									numBytes;		// CB, TB, SB, RB
			u32									extra;			// TX, TA (dims), SB (ctrtype)
			bool								isGpuWritable;	// SB, RB, TX, TA
		};

	public:
		std::wstring m_shaderFile;
		std::string m_entryFunction;
		std::string m_shader;

		std::vector<Data> m_Data[NUM_LOOKUP_INDICES];
		std::vector<BufferLayout> m_CBufferLayouts;
		std::vector<BufferLayout> m_TBufferLayouts;

		struct ShaderParamData
		{
			u32 size;
			u32 offset;
			u32 bind;
			String typeName;
		};
		std::unordered_map<String, ShaderParamData> m_shaderParamsInfo;
	};


	class VertexShader : public Shader
	{
	public:
		VertexShader(const i8* name, const WString shaderFile, const i8* entryFunction);
		VertexShader(const i8* name, const String shaderText);

	};

	class PixelShader : public Shader
	{
	public:
		PixelShader(const i8* name, const WString shaderFile, const i8* entryFunction);
		PixelShader(const i8* name, const String shaderText);
	};

	class GeometryShader : public Shader
	{
	public:
		GeometryShader(const i8* name, const WString shaderFile, const i8* entryFunction);
	};

	class ComputeShader : public Shader
	{
	public:
		ComputeShader(const i8* name, const WString shaderFile, const i8* entryFunction);
	};

}