#include "HLSLBaseBuffer.h"

using namespace forward;

HLSLBaseBuffer::~HLSLBaseBuffer()
{
}

std::vector<HLSLBaseBuffer::Member> const& HLSLBaseBuffer::GetMembers() const
{
	return mMembers;
}

void HLSLBaseBuffer::Print(std::ofstream& output) const
{
	auto i = 0;
	for (auto const& member : mMembers)
	{
		output << "Variable[" << i << "]:" << std::endl;
		member.first.Print(output);
		output << "Type[" << i << "]:" << std::endl;
		member.second.Print(output, 0);
		++i;
	}
}

void HLSLBaseBuffer::GenerateLayout(std::vector<MemberLayout>& layout) const
{
	for (auto const& m : mMembers)
	{
		HLSLShaderType const& parent = m.second;
		GenerateLayout(parent, m.first.GetOffset(), parent.GetName(),
			layout);
	}
}

void HLSLBaseBuffer::GenerateLayout(HLSLShaderType const& parent,
	u32 parentOffset, std::string const& parentName,
	std::vector<MemberLayout>& layout) const
{
	u32 const numChildren = parent.GetNumChildren();
	if (numChildren > 0)
	{
		for (u32 i = 0; i < numChildren; ++i)
		{
			HLSLShaderType const& child = parent.GetChild(i);
			GenerateLayout(child, parentOffset + child.GetOffset(),
				parentName + "." + child.GetName(), layout);
		}
	}
	else
	{
		MemberLayout item;
		item.name = parentName;
		item.offset = parentOffset;
		item.numElements = parent.GetNumElements();
		layout.push_back(item);
	}
}