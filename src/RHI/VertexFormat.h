#pragma once
#include "PCH.h"
#include "DataFormat.h"
#include "RHI/ResourceSystem/GraphicsObject.h"

namespace forward
{
	// Enumerations for DX11.
	enum VASemantic
	{
		VA_NO_SEMANTIC,
		VA_POSITION,
		VA_BLENDWEIGHT,
		VA_BLENDINDICES,
		VA_NORMAL,
		VA_PSIZE,
		VA_TEXCOORD,
		VA_TANGENT,
		VA_BINORMAL,
		VA_TESSFACTOR,
		VA_POSITIONT,
		VA_COLOR,
		VA_FOG,
		VA_DEPTH,
		VA_SAMPLE,
		VA_NUM_SEMANTICS
	};

	enum VAConstant
	{
		// TODO:  Modify to the numbers for Shader Model 5 (DX11).

		// The maximum number of attributes for a vertex format.
		VA_MAX_ATTRIBUTES = 16,

		// The maximum number of texture coordinate units.
		VA_MAX_TCOORD_UNITS = 8,

		// The maximum number of color units.
		VA_MAX_COLOR_UNITS = 2
	};

	class VertexFormat : public GraphicsObject
	{
	public:
		// Construction.
		VertexFormat();

		// Support for reusing a VertexFormat object within a scope.  This
		// call resets the object to the state produced by the default
		// constructor call.
		void Reset();

		// Create a packed vertex format, where all attributes are contiguous in
		// memory.  The order of the attributes is determined by the order of
		// Bind calls.
		bool Bind(VASemantic semantic, DataFormatType type, u32 unit);

		// Member access.  GetAttribute returns 'true' when the input i is
		// such that 0 <= i < GetNumAttributes(), in which case the returned
		// semantic, type, unit, and offset are valid.
		u32 GetVertexSize() const;
		i32 GetNumAttributes() const;
		bool GetAttribute(i32 i, VASemantic& semantic, DataFormatType& type,
			u32& unit, u32& offset) const;

		// Determine whether a semantic/unit exists.  If so, return the
		// index i that can be used to obtain more information about the
		// attribute by the functions after this.  If not, return -1.
		i32 GetIndex(VASemantic semantic, u32 unit) const;
		DataFormatType GetType(i32 i) const;
		u32 GetOffset(i32 i) const;

		bool ContainSemantic(VASemantic semantic) const;

		static const i8* GetSemanticName(VASemantic semantic);

	private:
		struct Attribute
		{
			Attribute();
			VASemantic semantic;
			DataFormatType type;
			u32 unit;
			u32 offset;
		};

		i32 mNumAttributes;
		u32 mVertexSize;
		Attribute mAttributes[VA_MAX_ATTRIBUTES];

		static const i8* msSemantic[VA_NUM_SEMANTICS];
	};
}