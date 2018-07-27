//***************************************************************************************
// FrameGraphShader.h by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#pragma once

#include "render/FrameGraph/FrameGraphObject.h"
#include "render/MemberLayout.h"

namespace forward
{

	class FrameGraphShader : public FrameGraphObject
	{
	public:
		FrameGraphShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction);

		const std::string& GetShaderText() const;
		const std::wstring& GetShaderFile() const;
		const std::wstring& GetShaderEntry() const;

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
			Data(FrameGraphObjectType inType, std::string const& inName,
				i32 inBindPoint, i32 inNumBytes, u32 inExtra,
				bool inIsGpuWritable);

			std::shared_ptr<FrameGraphObject>	object;			// CB, TB, SB, RB, TX, TA, SS
			FrameGraphObjectType				type;			// CB, TB, SB, RB, TX, TA, SS
			std::string							name;			// CB, TB, SB, RB, TX, TA, SS
			i32									bindPoint;		// CB, TB, SB, RB, TX, TA, SS
			i32									numBytes;		// CB, TB, SB, RB
			u32									extra;			// TX, TA (dims), SB (ctrtype)
			bool								isGpuWritable;	// SB, RB, TX, TA
		};

	protected:
		std::wstring m_shaderFile;
		std::wstring m_entryFunction;
		std::string m_shader;

		std::vector<Data> m_Data[NUM_LOOKUP_INDICES];
		std::vector<BufferLayout> m_CBufferLayouts;
		std::vector<BufferLayout> m_TBufferLayouts;

	private:
		void PostSetDeviceObject() override;
		
	};


	class FrameGraphVertexShader : public FrameGraphShader
	{
	public:
		FrameGraphVertexShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction);

	};

	class FrameGraphPixelShader : public FrameGraphShader
	{
	public:
		FrameGraphPixelShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction);
	};

	class FrameGraphGeometryShader : public FrameGraphShader
	{
	public:
		FrameGraphGeometryShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction);
	};

	class FrameGraphComputeShader : public FrameGraphShader
	{
	public:
		FrameGraphComputeShader(const std::string& name, const std::wstring& shaderFile, const std::wstring& entryFunction);
	};

}