#pragma once
#include "HLSLResource.h"

namespace forward
{
	class HLSLShaderType
	{
	public:
		struct Description
		{
			D3D_SHADER_VARIABLE_CLASS varClass;
			D3D_SHADER_VARIABLE_TYPE varType;
			u32 numRows;
			u32 numColumns;
			u32 numElements;
			u32 numChildren;
			u32 offset;
			std::string typeName;
		};

		// Construction.
		HLSLShaderType();

		// Deferred construction for shader reflection.  These functions are
		// intended to be write-once.
		template<class D3D_SHADER_TYPE_DESC>
		void SetDescription(D3D_SHADER_TYPE_DESC const& desc)
		{
			mDesc.varClass = desc.Class;
			mDesc.varType = desc.Type;
			mDesc.numRows = desc.Rows;
			mDesc.numColumns = desc.Columns;
			mDesc.numElements = desc.Elements;
			mDesc.numChildren = desc.Members;
			mDesc.offset = desc.Offset;
			mDesc.typeName = std::string(desc.Name ? desc.Name : "");

			if (desc.Members > 0)
			{
				mChildren.resize(desc.Members);
			}
			else
			{
				mChildren.clear();
			}
		}
		void SetName(std::string const& name);

		// This is non-const and is intended to be used as part of the Set*
		// write-once system.  HLSLShaderFactory::{GetVariables,GetTypes} are
		// the clients and they ensure that i is a valid index.
		HLSLShaderType& GetChild(u32 i);

		// For use in construction of lookup tables for name-offset pairs.
		HLSLShaderType const& GetChild(u32 i) const;

		// Member access.
		std::string const& GetName() const;
		D3D_SHADER_VARIABLE_CLASS GetClass() const;
		D3D_SHADER_VARIABLE_TYPE GetType() const;
		u32 GetNumRows() const;
		u32 GetNumColumns() const;
		u32 GetNumElements() const;
		u32 GetNumChildren() const;
		u32 GetOffset() const;
		std::string const& GetTypeName() const;
		std::vector<HLSLShaderType> const& GetChildren() const;

		// Print to a text file for human readability.
		void Print(std::ofstream& output, i32 indent) const;

	private:
		Description mDesc;
		std::string mName;
		std::vector<HLSLShaderType> mChildren;

		// Support for Print.
		static std::string const msVarClass[];
		static std::string const msVarType[];
	};
}