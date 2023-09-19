#pragma once
#include "HLSLResource.h"

namespace forward
{
    class HLSLByteAddressBuffer : public HLSLResource
    {
    public:
        // Construction and destruction.
        virtual ~HLSLByteAddressBuffer() = default;

        template<class D3D_SHADER_INPUT_BIND_DESC>
        HLSLByteAddressBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc)
            : HLSLResource(desc, 0)
            , mGpuWritable(desc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
        {}

        template<class D3D_SHADER_INPUT_BIND_DESC>
        HLSLByteAddressBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index)
            : HLSLResource(desc, index, 0)
            , mGpuWritable(desc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
        {}

        // Member access.
        inline bool IsGpuWritable() const
        {
            return mGpuWritable;
        }

    private:
        bool mGpuWritable;
    };
}