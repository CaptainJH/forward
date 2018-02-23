#include "VertexFormat.h"

using namespace forward;

VertexFormat::VertexFormat()
	: mNumAttributes(0)
	, mVertexSize(0)
{
}

void VertexFormat::Reset()
{
	mNumAttributes = 0;
	mVertexSize = 0;
	for (auto i = 0; i < VA_MAX_ATTRIBUTES; ++i)
	{
		mAttributes[i] = Attribute();
	}
}

bool VertexFormat::Bind(VASemantic semantic, DataFormatType type, u32 unit)
{
	if (0 <= mNumAttributes && mNumAttributes < VA_MAX_ATTRIBUTES)
	{
		// Validate the inputs.
		if (semantic == VA_COLOR)
		{
			if (unit >= VA_MAX_COLOR_UNITS)
			{
				return false;
			}
		}
		else if (semantic == VA_TEXCOORD)
		{
			if (unit >= VA_MAX_TCOORD_UNITS)
			{
				return false;
			}
		}
		else
		{
			if (unit > 0)
			{
				return false;
			}
		}

		// Accept the attribute.
		Attribute& attribute = mAttributes[mNumAttributes];
		attribute.semantic = semantic;
		attribute.type = type;
		attribute.unit = unit;
		attribute.offset = mVertexSize;
		++mNumAttributes;

		// Advance the offset.
		mVertexSize += DataFormat::GetNumBytesPerStruct(type);
		return true;
	}

	return false;
}

u32 VertexFormat::GetVertexSize() const
{
	return mVertexSize;
}

i32 VertexFormat::GetNumAttributes() const
{
	return mNumAttributes;
}

bool VertexFormat::GetAttribute(i32 i, VASemantic& semantic, DataFormatType& type,
	u32& unit, u32& offset) const
{
	if (0 <= i && i < mNumAttributes)
	{
		Attribute const& attribute = mAttributes[i];
		semantic = attribute.semantic;
		type = attribute.type;
		unit = attribute.unit;
		offset = attribute.offset;
		return true;
	}

	return false;
}

i32 VertexFormat::GetIndex(VASemantic semantic, u32 unit) const
{
	for (auto i = 0; i < mNumAttributes; ++i)
	{
		Attribute const& attribute = mAttributes[i];
		if (attribute.semantic == semantic && attribute.unit == unit)
		{
			return i;
		}
	}

	return -1;
}

DataFormatType VertexFormat::GetType(i32 i) const
{
	if (0 <= i && i < mNumAttributes)
	{
		return mAttributes[i].type;
	}

	return DF_UNKNOWN;
}

u32 VertexFormat::GetOffset(i32 i) const
{
	if (0 <= i && i < mNumAttributes)
	{
		return mAttributes[i].offset;
	}

	return 0xFFFFFFFFu;
}

VertexFormat::Attribute::Attribute()
	: semantic(VA_NO_SEMANTIC)
	, type(DF_UNKNOWN)
	, unit(0)
	, offset(0)
{
}