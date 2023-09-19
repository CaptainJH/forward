#pragma once
#include "HLSLResource.h"

namespace forward
{
    class HLSLStructuredBuffer : public HLSLResource
    {
    public:
        enum Type
        {
            SBT_INVALID,
            SBT_BASIC,
            SBT_APPEND,
            SBT_CONSUME,
            SBT_COUNTER
        };

        // Construction and destruction.
        virtual ~HLSLStructuredBuffer() = default;

        template<class D3D_SHADER_INPUT_BIND_DESC>
        HLSLStructuredBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc)
            :
            HLSLResource(desc, 0)
        {
            Initialize(desc);
        }

        template<class D3D_SHADER_INPUT_BIND_DESC>
        HLSLStructuredBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index)
            :
            HLSLResource(desc, index, 0)
        {
            Initialize(desc);
        }

        // Member access.
        inline HLSLStructuredBuffer::Type GetType() const
        {
            return mType;
        }

        inline bool IsGpuWritable() const
        {
            return mGpuWritable;
        }

    private:
        template<class D3D_SHADER_INPUT_BIND_DESC>
        void Initialize(D3D_SHADER_INPUT_BIND_DESC const& desc)
        {
            if (desc.Type == D3D_SIT_STRUCTURED
                || desc.Type == D3D_SIT_UAV_RWSTRUCTURED)
            {
                mType = Type::SBT_BASIC;
            }
            else if (desc.Type == D3D_SIT_UAV_APPEND_STRUCTURED)
            {
                mType = Type::SBT_APPEND;
            }
            else if (desc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED)
            {
                mType = Type::SBT_CONSUME;
            }
            else if (desc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
            {
                mType = Type::SBT_COUNTER;
            }
            else
            {
                mType = Type::SBT_INVALID;
            }

            mGpuWritable =
                desc.Type == D3D_SIT_UAV_RWSTRUCTURED ||
                desc.Type == D3D_SIT_UAV_APPEND_STRUCTURED ||
                desc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED ||
                desc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER;
        }

        Type mType;
        bool mGpuWritable;
    };
}