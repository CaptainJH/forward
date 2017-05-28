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
// PipelineExecutorDX11
//
//--------------------------------------------------------------------------------
#ifndef PipelineExecutorDX11_h
#define PipelineExecutorDX11_h
//--------------------------------------------------------------------------------
#include "PCH.h"
#include "dx11Util.h"
//--------------------------------------------------------------------------------
namespace forward
{
	class PipelineManagerDX11;

	// This simple structure provides the mapping between a vertex shader program
	// and the corresponding input layout.

	struct InputLayoutKey
	{
		i32 shader;
		i32 layout;
	};

	class PipelineExecutorDX11
	{
	public:
		PipelineExecutorDX11();
		virtual ~PipelineExecutorDX11();

		virtual void Execute( PipelineManagerDX11* pPipeline ) = 0;
		
		virtual void SetLayoutElements( u32 count, D3D11_INPUT_ELEMENT_DESC* pElements );
		virtual void GenerateInputLayout( i32 ShaderID );
		virtual i32 GetInputLayout( i32 ShaderID );

	protected:

		// A description of our vertex elements
		std::vector<D3D11_INPUT_ELEMENT_DESC>	m_elements;
		std::map<i32,InputLayoutKey*>			m_InputLayouts;

	};

	typedef std::shared_ptr<PipelineExecutorDX11> ExecutorPtr;
};
//--------------------------------------------------------------------------------
#endif // PipelineExecutorDX11_h
//--------------------------------------------------------------------------------

