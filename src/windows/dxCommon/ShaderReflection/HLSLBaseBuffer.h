#pragma once
#include "HLSLResource.h"
#include "HLSLShaderVariable.h"
#include "HLSLShaderType.h"
#include "RHI/MemberLayout.h"

namespace forward
{
	class HLSLBaseBuffer : public HLSLResource
	{
	public:
		typedef std::pair<HLSLShaderVariable, HLSLShaderType> Member;

		// Construction and destruction.
		virtual ~HLSLBaseBuffer();

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLBaseBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 numBytes, std::vector<Member> const& members)
			: HLSLResource(desc, numBytes)
			, mMembers(members)
		{}

		template<class D3D_SHADER_INPUT_BIND_DESC>
		HLSLBaseBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc,
			u32 index, u32 numBytes,
			std::vector<Member> const& members)
			: HLSLResource(desc, index, numBytes)
			, mMembers(members)
		{}

		// Member access.
		std::vector<Member> const& GetMembers() const;

		// Print to a text file for human readability.
		virtual void Print(std::ofstream& output) const;

		// Generation of lookup tables for member layout.
		void GenerateLayout(std::vector<MemberLayout>& layout) const;

	private:
		void GenerateLayout(HLSLShaderType const& type, u32 parentOffset,
			std::string const& parentName,
			std::vector<MemberLayout>& layout) const;

		std::vector<Member> mMembers;
	};
}