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
// DXGIOutput
//
//--------------------------------------------------------------------------------
#include "PCH.h"
#include <dxgi.h>
//--------------------------------------------------------------------------------
#ifndef DXGIOutput_h
#define DXGIOutput_h
//--------------------------------------------------------------------------------
namespace forward
{
	class DXGIOutput
	{
	public:
		DXGIOutput( Microsoft::WRL::ComPtr<IDXGIOutput> pOutput );
		virtual ~DXGIOutput();

		Microsoft::WRL::ComPtr<IDXGIOutput>	m_pOutput;
	};
};
//--------------------------------------------------------------------------------
#endif // DXGIOutput_h
//--------------------------------------------------------------------------------

