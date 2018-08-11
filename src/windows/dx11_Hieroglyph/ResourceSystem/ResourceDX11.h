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
#include "dx11_Hieroglyph/ResourceSystem/ResourceConfig.h"
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{
	class ResourceDX11 : public DeviceResource
	{
	public:
		ResourceDX11();

		virtual ~ResourceDX11();

		virtual ID3D11Resource*				GetResource() = 0;

		virtual ResourceType				GetType() = 0;

		i32		GetResourceID() const;
		void	SetResourceID(i32 id);

		void	SyncCPUToGPU() override {}

	protected:
		i32		m_ResourceID = -1;
	};

};
//--------------------------------------------------------------------------------
#endif // ResourceDX11_h
//--------------------------------------------------------------------------------

