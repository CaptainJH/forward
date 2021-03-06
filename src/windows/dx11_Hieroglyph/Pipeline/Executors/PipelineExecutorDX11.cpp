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
#include "PCH.h"
#include "PipelineExecutorDX11.h"
#include "dx11_Hieroglyph/RendererDX11.h"
//--------------------------------------------------------------------------------
using namespace forward;
//--------------------------------------------------------------------------------
PipelineExecutorDX11::PipelineExecutorDX11( )
{
}
//--------------------------------------------------------------------------------
PipelineExecutorDX11::~PipelineExecutorDX11()
{
	std::map<i32, InputLayoutKey*>::iterator it = m_InputLayouts.begin();
	for( ; it != m_InputLayouts.end(); it++ )
        SAFE_DELETE( (*it).second );
}
//--------------------------------------------------------------------------------
i32 PipelineExecutorDX11::GetInputLayout( i32 ShaderID )
{
	i32 layout = -1;

	// Automatically generate the layout if it doesn't already exist.

	if ( m_InputLayouts[ShaderID] == 0 )
		GenerateInputLayout( ShaderID );
	
	// The layout should be created now...
	layout = m_InputLayouts[ShaderID]->layout;

	return( layout );
}
//--------------------------------------------------------------------------------
void PipelineExecutorDX11::GenerateInputLayout( i32 ShaderID )
{
	// Create the input layout for the given shader index

	RendererDX11* pRenderer = RendererDX11::Get();
	if ( m_InputLayouts[ShaderID] == 0 )
	{
		InputLayoutKey* pKey = new InputLayoutKey();
		pKey->shader = ShaderID;
		pKey->layout = pRenderer->CreateInputLayout( m_elements, ShaderID );
		m_InputLayouts[ShaderID] = pKey;
	}
}
//--------------------------------------------------------------------------------
void PipelineExecutorDX11::SetLayoutElements( u32 count, D3D11_INPUT_ELEMENT_DESC* pElements )
{
	for ( u32 i = 0; i < count; i++ )
	{
		m_elements.push_back( pElements[i] );
	}
}
//--------------------------------------------------------------------------------
