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
#pragma once

#include "Types.h"
#include "render/ResourceSystem/DeviceObject.h"
#include "dx11/dx11Util.h"
#include <d3d11_2.h>
//--------------------------------------------------------------------------------
namespace forward
{

	class DeviceObjectDX11 : public DeviceObject
	{
	public:
		DeviceObjectDX11();

		virtual ~DeviceObjectDX11();

		DeviceObjComPtr GetDeviceObject();

	protected:
		DeviceObjComPtr		m_deviceObjPtr;
	};

};
//--------------------------------------------------------------------------------

