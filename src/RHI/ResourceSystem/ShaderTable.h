//***************************************************************************************
// ShaderTable.h by Heqi Ju (C) 2023 All Rights Reserved.
//***************************************************************************************
#pragma once

#include <assert.h>
#include <unordered_map>
#include "RHI/ResourceSystem/Resource.h"

namespace forward
{
    // Shader record = {{Shader ID}, {RootArguments}}
    class ShaderRecord
    {
    public:
        ShaderRecord(void* pShaderIdentifier, u32 shaderIdentifierSize) :
            shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
        {}

        ShaderRecord(void* pShaderIdentifier, u32 shaderIdentifierSize, void* pLocalRootArguments, u32 localRootArgumentsSize) :
            shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
            localRootArguments(pLocalRootArguments, localRootArgumentsSize)
        {}

        void CopyTo(void* dest) const
        {
            u8* byteDest = static_cast<u8*>(dest);
            memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
            if (localRootArguments.ptr)
            {
                memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
            }
        }

        struct PointerWithSize {
            void* ptr;
            u32 size;

            PointerWithSize() : ptr(nullptr), size(0) {}
            PointerWithSize(void* _ptr, u32 _size) : ptr(_ptr), size(_size) {};
        };
        PointerWithSize shaderIdentifier;
        PointerWithSize localRootArguments;
    };

    struct ShaderRecordDesc
    {
        WString shaderName = L"";
        Vector<u8> shaderArguments = {};
    };


	class ShaderTable final : public Resource
	{
	public:
        ShaderTable() = delete;
        ShaderTable(const String& name, WString shaderRecordName);
        ShaderTable(const String& name, Vector<WString> shaderRecordNames);

        ShaderRecord& operator[](u32 index);
        u32 GetShaderRecordSize() const { return GetElementSize(); }
        void SetupShaderRecords(const std::unordered_map<WString, void*>& name2identifier);
        bool ContainPayload() const;

        Vector<ShaderRecordDesc> m_shaderRecords;

    private:
        // Pretty-print the shader records.
        void DebugPrint(std::unordered_map<void*, WString> shaderIdToStringMap);
	};
}