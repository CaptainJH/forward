//***************************************************************************************
// ShaderTable.cpp by Heqi Ju (C) 2023 All Rights Reserved.
//***************************************************************************************

#include "ShaderTable.h"
#include "Utils.h"

using namespace forward;

// D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT
static const u32 RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT = 32;
//D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
static const u32 SHADER_IDENTIFIER_SIZE_IN_BYTES = 32;

void ShaderTable::DebugPrint(std::unordered_map<void*, WString> shaderIdToStringMap)
{
    std::wstringstream wstr;
    wstr << L"|--------------------------------------------------------------------\n";
    wstr << L"|Shader table - " << m_name.c_str() << L": "
        << GetElementSize() << L" | "
        << GetNumBytes() << L" bytes\n";

    for (auto i = 0U; i < GetNumElements(); ++i)
    {
        wstr << L"| [" << i << L"]: ";
        wstr << shaderIdToStringMap[(*this)[i].shaderIdentifier.ptr] << L", ";
        wstr << (*this)[i].shaderIdentifier.size << L" + " << (*this)[i].localRootArguments.size << L" bytes \n";
    }
    wstr << L"|--------------------------------------------------------------------\n";
    wstr << L"\n";
    OutputDebugStringW(wstr.str().c_str());
}

ShaderRecord& ShaderTable::operator[](u32 index)
{
    assert(index < m_numElements);
    ShaderRecord* ret = (ShaderRecord*)(GetData()[index * m_elementSize]);
    return *ret;
}

ShaderTable::ShaderTable(const String& name, u32 numShaderRecords, u32 payloadSize)
    : Resource(name)
{
    m_type = FGOT_SHADER_TABLE;
    SetUsage(RU_DYNAMIC_UPDATE);
    auto shaderRecordSizeAligned = Align(payloadSize + SHADER_IDENTIFIER_SIZE_IN_BYTES, RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    Initialize(numShaderRecords, shaderRecordSizeAligned);
}

bool ShaderTable::ContainPayload() const
{
    return GetElementSize() > RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
}

void ShaderTable::SetupShaderRecords(const std::unordered_map<WString, void*>& name2identifier)
{
    auto dst = GetData();
    for (auto& record : m_shaderRecords)
    {
        void* ident = nullptr;
        if (name2identifier.contains(record.shaderName))
            ident = name2identifier.at(record.shaderName);
        else
        {
            const auto sepIt = record.shaderName.find(L"_");
            assert(sepIt != WString::npos);
            auto groupName = record.shaderName.substr(0, sepIt);
            assert(name2identifier.contains(groupName));
            ident = name2identifier.at(groupName);
        }

        if (record.shaderArguments.empty())
        {
            ShaderRecord sr(ident, SHADER_IDENTIFIER_SIZE_IN_BYTES);
            sr.CopyTo(dst);
        }
        else
        {
            ShaderRecord sr(ident, SHADER_IDENTIFIER_SIZE_IN_BYTES, 
                record.shaderArguments.data(), static_cast<u32>(record.shaderArguments.size()));
            sr.CopyTo(dst);
        }

        dst += GetElementSize();
    }
}
