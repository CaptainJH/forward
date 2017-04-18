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
// ConstantBufferDX11 
//
//--------------------------------------------------------------------------------
#ifndef ConstantBufferDX11_h
#define ConstantBufferDX11_h
//--------------------------------------------------------------------------------
#include "BufferDX11.h"
//--------------------------------------------------------------------------------
namespace forward
{
	struct ConstantBufferMapping
	{
		//TODO:
		//RenderParameterDX11*		pParameter;
		u32				offset;
		u32				size;
		D3D_SHADER_VARIABLE_CLASS	varclass;
		u32				elements;
		u32				valueID;
	};



	class ConstantBufferDX11 : public BufferDX11
	{
	public:
		ConstantBufferDX11( Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer );
		virtual ~ConstantBufferDX11();

		virtual ResourceType				GetType( );

		void						AddMapping( ConstantBufferMapping& mapping );
		void						EmptyMappings( );
		//TODO:
		//void						EvaluateMappings( PipelineManagerDX11* pPipeline, IParameterManager* pParamManager );
		bool						ContainsMapping( i32 index, const ConstantBufferMapping& mapping );

		void						SetAutoUpdate( bool enable );
		bool						GetAutoUpdate( );

	protected:
		bool									m_bAutoUpdate;
		std::vector< ConstantBufferMapping >	m_Mappings;

		friend class RendererDX11;
	};
};
//--------------------------------------------------------------------------------
#endif // ConstantBufferDX11_h
