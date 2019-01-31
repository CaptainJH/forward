#include "HLSLShaderVariable.h"
#include <iomanip>

using namespace forward;

HLSLShaderVariable::HLSLShaderVariable()
{
}

std::string const& HLSLShaderVariable::GetName() const
{
	return m_desc.name;
}

u32 HLSLShaderVariable::GetOffset() const
{
	return m_desc.offset;
}

u32 HLSLShaderVariable::GetNumBytes() const
{
	return m_desc.numBytes;
}

u32 HLSLShaderVariable::GetFlags() const
{
	return m_desc.flags;
}

u32 HLSLShaderVariable::GetTextureStart() const
{
	return m_desc.textureStart;
}

u32 HLSLShaderVariable::GetTextureNumSlots() const
{
	return m_desc.textureNumSlots;
}

u32 HLSLShaderVariable::GetSamplerStart() const
{
	return m_desc.samplerStart;
}

u32 HLSLShaderVariable::GetSamplerNumSlots() const
{
	return m_desc.samplerNumSlots;
}

std::vector<u8> const& HLSLShaderVariable::GetDefaultValue() const
{
	return m_desc.defaultValue;
}

void HLSLShaderVariable::Print(std::ofstream& output) const
{
	output << "name = " << m_desc.name << std::endl;
	output << "offset = " << m_desc.offset << std::endl;
	output << "numBytes = " << m_desc.numBytes << std::endl;
	output << "flags = " << m_desc.flags << std::endl;

	if (m_desc.textureStart == 0xFFFFFFFF)
	{
		output << "textureStart = -1" << std::endl;
	}
	else
	{
		output << "textureStart = " << m_desc.textureStart << std::endl;
	}
	output << "textureNumSlots = " << m_desc.textureNumSlots << std::endl;

	if (m_desc.samplerStart == 0xFFFFFFFF)
	{
		output << "samplerStart = -1" << std::endl;
	}
	else
	{
		output << "samplerStart = " << m_desc.samplerStart << std::endl;
	}
	output << "textureNumSlots = " << m_desc.samplerNumSlots << std::endl;

	output << "default value = ";
	auto size = m_desc.defaultValue.size();
	if (size > 0)
	{
		output << std::hex << std::endl;
		auto j = 0;
		for (auto c : m_desc.defaultValue)
		{
			u32 hc = static_cast<u32>(c);
			output << "0x" << std::setw(2) << std::setfill('0') << hc;
			if ((++j % 16) == 0)
			{
				if (j != size)
				{
					output << std::endl;
				}
			}
			else
			{
				output << ' ';
			}
		}
		output << std::dec;
	}
	else
	{
		output << "none";
	}
	output << std::endl;
}