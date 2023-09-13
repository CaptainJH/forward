//***************************************************************************************
// ShaderTable.cpp by Heqi Ju (C) 2023 All Rights Reserved.
//***************************************************************************************

#include "ShaderTable.h"
#include "Utils.h"

using namespace forward;

// D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT
static const u32 RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT = 32;

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

ShaderTable::ShaderTable(const String& name, u32 numShaderRecords, u32 shaderRecordSize)
    : Resource(name)
{
    m_type = FGOT_SHADER_TABLE;
    auto shaderRecordSizeAligned = Align(shaderRecordSize, RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
    Initialize(numShaderRecords, shaderRecordSizeAligned);
}
