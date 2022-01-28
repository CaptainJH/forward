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
// DXGIAdapter
//
//--------------------------------------------------------------------------------
#include "PCH.h"
#ifdef DX11
#include <dxgi.h>
#elif DX12
#include <dxgi1_6.h>
#endif
//--------------------------------------------------------------------------------
#ifndef DXGIAdapter_h
#define DXGIAdapter_h
//--------------------------------------------------------------------------------
namespace forward
{
	class DXGIOutput;

	class DXGIAdapter
	{
	public:
		virtual ~DXGIAdapter() {}
#ifdef DX11
		DXGIAdapter(Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter);

		Microsoft::WRL::ComPtr<IDXGIAdapter1>		m_pAdapter;
#elif DX12
		DXGIAdapter( Microsoft::WRL::ComPtr<IDXGIAdapter4> pAdapter );

		Microsoft::WRL::ComPtr<IDXGIAdapter4>		m_pAdapter;
#endif
		std::vector<DXGIOutput>						m_vOutputs;
	};
};
//--------------------------------------------------------------------------------
#endif // DXGIAdapter_h
//--------------------------------------------------------------------------------

