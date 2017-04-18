//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) Jason Zink 
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// ResourceDX11
//
// This pure interface provides the function interface to the information 
// contained in the D3D11 resource class.  The functions are made abstract so that
// the concrete classes that implement this interface will provide their own
// implementations based on their individual types, instead of providing a 
// resource pointer here and forcing each subclass to cast its type.
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
#ifndef ResourceDX11_h
#define ResourceDX11_h

#include "Types.h"
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	enum ResourceType
	{
		RT_VERTEXBUFFER = 0x010000,
		RT_INDEXBUFFER = 0x020000,
		RT_CONSTANTBUFFER = 0x030000,
		RT_STRUCTUREDBUFFER = 0x040000,
		RT_BYTEADDRESSBUFFER = 0x050000,
		RT_INDIRECTARGSBUFFER = 0x060000,
		RT_TEXTURE1D = 0x070000,
		RT_TEXTURE2D = 0x080000,
		RT_TEXTURE3D = 0x090000
	};

	class ResourceDX11
	{
	public:
		ResourceDX11();

		virtual ~ResourceDX11();

		virtual ResourceType				GetType() = 0;
		virtual ID3D11Resource*				GetResource() = 0;

		virtual u32						GetEvictionPriority() = 0;
		virtual void						SetEvictionPriority( u32 EvictionPriority ) = 0;

		u32						GetInnerID();

		static u32				s_usResourceUID;
		u32						m_usInnerID;
	};

};
//--------------------------------------------------------------------------------
#endif // ResourceDX11_h
//--------------------------------------------------------------------------------

