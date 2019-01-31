#pragma once
#include <d3dcommon.h>
#include "utilities/Utils.h"

namespace forward
{
	class HLSLParameter
	{
	public:
		struct Description
		{
			std::string semanticName;
			u32 semanticIndex;
			u32 registerIndex;
			D3D_NAME systemValueType;
			D3D_REGISTER_COMPONENT_TYPE componentType;
			u32 mask;
			u32 readWriteMask;
			u32 stream;
			D3D_MIN_PRECISION minPrecision;
		};

		// Construction.  Parameters are reported for inputs, outputs, and patch
		// constants.
		template<class D3D_SIGNATURE_PARAMETER_DESC>
		HLSLParameter(D3D_SIGNATURE_PARAMETER_DESC const& desc)
		{
			m_desc.semanticName = std::string(desc.SemanticName);
			m_desc.semanticIndex = desc.SemanticIndex;
			m_desc.registerIndex = desc.Register;
			m_desc.systemValueType = desc.SystemValueType;
			m_desc.componentType = desc.ComponentType;
			m_desc.mask = desc.Mask;
			m_desc.readWriteMask = desc.ReadWriteMask;
			m_desc.stream = desc.Stream;
			m_desc.minPrecision = desc.MinPrecision;
		}

		// Member access.
		std::string const& GetSemanticName() const;
		u32 GetSemanticIndex() const;
		u32 GetRegisterIndex() const;
		D3D_NAME GetSystemValueType() const;
		D3D_REGISTER_COMPONENT_TYPE GetComponentType() const;
		u32 GetMask() const;
		u32 GetReadWriteMask() const;
		u32 GetStream() const;
		D3D_MIN_PRECISION GetMinPrecision() const;

		// Print to a text file for human readability.
		void Print(std::ofstream& output) const;

	private:
		Description m_desc;

		// Support for Print.
		static std::string const msSVName[];
		static std::string const msComponentType[];
		static std::string const msMinPrecision[];
	};


}