#pragma once
#include "HLSLResource.h"

namespace forward
{
    class HLSLSamplerState : public HLSLResource
    {
    public:
        // Construction and destruction.
        virtual ~HLSLSamplerState() = default;

        template<class D3D_SHADER_INPUT_BIND_DESC>
        HLSLSamplerState(D3D_SHADER_INPUT_BIND_DESC const& desc)
            : HLSLResource(desc, 0)
        {}
        template<class D3D_SHADER_INPUT_BIND_DESC>
        HLSLSamplerState(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index)
            : HLSLResource(desc, index, 0)
        {}
    };
}
