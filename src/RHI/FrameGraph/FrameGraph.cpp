//***************************************************************************************
// FrameGraph.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "FrameGraph.h"

using namespace forward;


#define Add_Input_Resource_To_RenderPassInfo(it) \
	if (std::find(passInfo.m_inputResources.begin(), passInfo.m_inputResources.end(), &*it) == passInfo.m_inputResources.end())\
	{\
		passInfo.m_inputResources.push_back(&*it);\
		registerReadFrameGraphResource(it->GetFrameGraphResource(), pass_ptr);\
	}

#define Add_DrawingState_To_RenderPassInfo(it)\
	if (std::find(passInfo.m_drawingStates.begin(), passInfo.m_drawingStates.end(), &*it) == passInfo.m_drawingStates.end())\
	{\
		passInfo.m_drawingStates.push_back(&*it);\
		registerDrawingState(it->GetFrameGraphDrawingState(), pass_ptr);\
	}

void FrameGraph::Reset()
{
	m_passDB.clear();
	m_allUsedResources.clear();
}

void FrameGraph::DrawRenderPass(RenderPass* pass)
{
	registerRenderPass(pass);
}

std::vector<RenderPassInfo>& FrameGraph::GetRenderPassDB()
{
	return m_passDB;
}

FrameGraphObjectInfo* FrameGraph::registerShader(Shader* shader, RenderPass* pass)
{
	auto it = std::find_if(m_allUsedShaders.begin(), m_allUsedShaders.end(), [shader](FrameGraphObjectInfo& obj)->bool {
		return obj.m_object == shader;
	});

	if (it == m_allUsedShaders.end())
	{
		m_allUsedShaders.push_back(FrameGraphObjectInfo(shader));
		it = m_allUsedShaders.end() - 1;
	}

	auto itPass = std::find_if(it->m_readerPassList.begin(), it->m_readerPassList.end(), [pass](RenderPassInfo* passInfo)->bool {
		return passInfo->m_renderPass == pass;
	});
	if (itPass == it->m_readerPassList.end())
	{
		auto itPassInfo = std::find_if(m_passDB.begin(), m_passDB.end(), [pass](RenderPassInfo& info)->bool {
			return info.m_renderPass == pass;
		});
		assert(itPassInfo != m_passDB.end());
		it->m_readerPassList.push_back(&*itPassInfo);
	}

	return &*it;
}

FrameGraphObjectInfo* FrameGraph::registerDrawingState(DrawingState* state, RenderPass* pass)
{
	auto it = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [state](FrameGraphObjectInfo& obj)->bool {
		return obj.m_object == state;
	});

	if (it == m_allUsedDrawingStates.end())
	{
		m_allUsedDrawingStates.push_back(FrameGraphObjectInfo(state));
		it = m_allUsedVertexFormats.end() - 1;
	}

	auto itPass = std::find_if(it->m_readerPassList.begin(), it->m_readerPassList.end(), [pass](RenderPassInfo* passInfo)->bool {
		return passInfo->m_renderPass == pass;
	});
	if (itPass == it->m_readerPassList.end())
	{
		auto itPassInfo = std::find_if(m_passDB.begin(), m_passDB.end(), [pass](RenderPassInfo& info)->bool {
			return info.m_renderPass == pass;
		});
		assert(itPassInfo != m_passDB.end());
		it->m_readerPassList.push_back(&*itPassInfo);
	}

	return &*it;
}

FrameGraphObjectInfo* FrameGraph::registerVertexFormat(VertexFormat* vformat, RenderPass* pass)
{
	auto it = std::find_if(m_allUsedVertexFormats.begin(), m_allUsedVertexFormats.end(), [vformat](FrameGraphObjectInfo& obj)->bool {
		return obj.m_object == vformat;
	});

	if (it == m_allUsedVertexFormats.end())
	{
		m_allUsedVertexFormats.push_back(FrameGraphObjectInfo(vformat));
		it = m_allUsedVertexFormats.end() - 1;
	}

	auto itPass = std::find_if(it->m_readerPassList.begin(), it->m_readerPassList.end(), [pass](RenderPassInfo* passInfo)->bool {
		return passInfo->m_renderPass == pass;
	});
	if (itPass == it->m_readerPassList.end())
	{
		auto itPassInfo = std::find_if(m_passDB.begin(), m_passDB.end(), [pass](RenderPassInfo& info)->bool {
			return info.m_renderPass == pass;
		});
		assert(itPassInfo != m_passDB.end());
		it->m_readerPassList.push_back(&*itPassInfo);
	}

	return &*it;
}

FrameGraphResourceInfo* FrameGraph::registerReadFrameGraphResource(Resource* res, RenderPass* pass)
{
	auto it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [res](FrameGraphResourceInfo& resInfo)->bool {
		return resInfo.m_object == res;
	});

	if (it == m_allUsedResources.end())
	{
		m_allUsedResources.push_back(FrameGraphResourceInfo(res));
		it = m_allUsedResources.end() - 1;
	}

	auto itPass = std::find_if(it->m_readerPassList.begin(), it->m_readerPassList.end(), [pass](RenderPassInfo* passInfo)->bool {
		return passInfo->m_renderPass == pass;
	});
	if (itPass == it->m_readerPassList.end())
	{
		auto itPassInfo = std::find_if(m_passDB.begin(), m_passDB.end(), [pass](RenderPassInfo& info)->bool {
			return info.m_renderPass == pass;
		});
		assert(itPassInfo != m_passDB.end());
		it->m_readerPassList.push_back(&*itPassInfo);
	}

	return &*it;
}

FrameGraphResourceInfo* FrameGraph::registerWriteFrameGraphResource(Resource* res, RenderPass* pass)
{
	auto it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [res](FrameGraphResourceInfo& resInfo)->bool {
		return resInfo.m_object == res;
	});

	if (it == m_allUsedResources.end())
	{
		m_allUsedResources.push_back(FrameGraphResourceInfo(res));
		it = m_allUsedResources.end() - 1;
	}

	auto itPass = std::find_if(it->m_writerPassList.begin(), it->m_writerPassList.end(), [pass](RenderPassInfo* passInfo)->bool {
		return passInfo->m_renderPass == pass;
	});
	if (itPass == it->m_writerPassList.end())
	{
		auto itPassInfo = std::find_if(m_passDB.begin(), m_passDB.end(), [pass](RenderPassInfo& info)->bool {
			return info.m_renderPass == pass;
		});
		assert(itPassInfo != m_passDB.end());
		it->m_writerPassList.push_back(&*itPassInfo);
	}

	return &*it;
}

void FrameGraph::registerRenderPass(RenderPass* pass)
{
	auto it_pass = std::find_if(m_passDB.begin(), m_passDB.end(), [pass](RenderPassInfo& info)->bool {
		return info.m_renderPass == pass;
	});

	if (it_pass == m_passDB.end())
	{
		RenderPassInfo passInfo(pass);
		m_passDB.push_back(passInfo);
		it_pass = m_passDB.end() - 1;
	}

	if (!pass->IsPSO<RasterPipelineStateObject>())
		return;
	auto& pso = pass->GetPSO<RasterPipelineStateObject>();

	// vertex format
	auto vformat = &pso.m_IAState.m_vertexLayout;
	auto it_vformat = std::find_if(m_allUsedVertexFormats.begin(), m_allUsedVertexFormats.end(), [vformat](FrameGraphObjectInfo& obj)->bool {
		return obj.m_object == vformat;
	});
	if (it_vformat == m_allUsedVertexFormats.end())
	{
		m_allUsedVertexFormats.push_back(FrameGraphObjectInfo(vformat));
	}

	// vertex buffers
	for (auto& vb : pso.m_IAState.m_vertexBuffers)
	{
		if (vb)
		{
			auto vbuffer_ptr = vb.get();
			auto it_vbuffer = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [vbuffer_ptr](FrameGraphResourceInfo& info)->bool {
				return info.m_object == vbuffer_ptr;
			});
			if (it_vbuffer == m_allUsedResources.end())
			{
				m_allUsedResources.push_back(FrameGraphResourceInfo(vbuffer_ptr));
			}
		}
	}

	// index buffer
	auto ibuffer_ptr = pso.m_IAState.m_indexBuffer.get();
	if (ibuffer_ptr)
	{
		auto it_ibuffer = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [ibuffer_ptr](FrameGraphResourceInfo& info)->bool {
			return info.m_object == ibuffer_ptr;
		});
		if (it_ibuffer == m_allUsedResources.end())
		{
			m_allUsedResources.push_back(FrameGraphResourceInfo(ibuffer_ptr));
		}
	}

	// vertex shader
	{
		auto vs_ptr = pso.m_VSState.m_shader.get();
		if (vs_ptr)
		{
			auto it_vshader = std::find_if(m_allUsedShaders.begin(), m_allUsedShaders.end(), [vs_ptr](FrameGraphObjectInfo& info)->bool {
				return info.m_object == vs_ptr;
			});
			if (it_vshader == m_allUsedShaders.end())
			{
				m_allUsedShaders.push_back(FrameGraphObjectInfo(vs_ptr));
			}

			for (auto& cb : pso.m_VSState.m_constantBuffers)
			{
				if (cb)
				{
					auto cbuffer_ptr = cb.get();
					auto cbuffer_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [cbuffer_ptr](FrameGraphResourceInfo& info)->bool {
						return info.m_object == cbuffer_ptr;
					});
					if (cbuffer_it == m_allUsedResources.end())
					{
						m_allUsedResources.push_back(FrameGraphResourceInfo(cbuffer_ptr));
					}
				}
			}

			for (auto& samp : pso.m_VSState.m_samplers)
			{
				if (samp)
				{
					auto samp_ptr = samp.get();
					auto samp_it = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [samp_ptr](FrameGraphObjectInfo& info)->bool {
						return info.m_object == samp_ptr;
					});
					if (samp_it == m_allUsedDrawingStates.end())
					{
						m_allUsedDrawingStates.push_back(FrameGraphObjectInfo(samp_ptr));
					}
				}
			}

			for (auto& shaderRes : pso.m_VSState.m_shaderResources)
			{
				if (shaderRes)
				{
					auto shaderRes_ptr = shaderRes.get();
					auto shaderRes_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [shaderRes_ptr](FrameGraphResourceInfo& info)->bool {
						return info.m_object == shaderRes_ptr;
					});
					if (shaderRes_it == m_allUsedResources.end())
					{
						m_allUsedResources.push_back(FrameGraphResourceInfo(shaderRes_ptr));
					}
				}
			}
		}
	}

	// geometry shader
	{
		auto gs_ptr = pso.m_GSState.m_shader.get();
		if (gs_ptr)
		{
			auto it_gshader = std::find_if(m_allUsedShaders.begin(), m_allUsedShaders.end(), [gs_ptr](FrameGraphObjectInfo& info)->bool {
				return info.m_object == gs_ptr;
			});
			if (it_gshader == m_allUsedShaders.end())
			{
				m_allUsedShaders.push_back(FrameGraphObjectInfo(gs_ptr));
			}

			for (auto& cb : pso.m_GSState.m_constantBuffers)
			{
				if (cb)
				{
					auto cbuffer_ptr = cb.get();
					auto cbuffer_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [cbuffer_ptr](FrameGraphResourceInfo& info)->bool {
						return info.m_object == cbuffer_ptr;
					});
					if (cbuffer_it == m_allUsedResources.end())
					{
						m_allUsedResources.push_back(FrameGraphResourceInfo(cbuffer_ptr));
					}
				}
			}

			for (auto& samp : pso.m_GSState.m_samplers)
			{
				if (samp)
				{
					auto samp_ptr = samp.get();
					auto samp_it = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [samp_ptr](FrameGraphObjectInfo& info)->bool {
						return info.m_object == samp_ptr;
					});
					if (samp_it == m_allUsedDrawingStates.end())
					{
						m_allUsedDrawingStates.push_back(FrameGraphObjectInfo(samp_ptr));
					}
				}
			}

			for (auto& shaderRes : pso.m_GSState.m_shaderResources)
			{
				if (shaderRes)
				{
					auto shaderRes_ptr = shaderRes.get();
					auto shaderRes_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [shaderRes_ptr](FrameGraphResourceInfo& info)->bool {
						return info.m_object == shaderRes_ptr;
					});
					if (shaderRes_it == m_allUsedResources.end())
					{
						m_allUsedResources.push_back(FrameGraphResourceInfo(shaderRes_ptr));
					}
				}
			}
		}
	}

	// rasterizor stage
	auto rs_ptr = &pso.m_RSState.m_rsState;
	auto it_rs = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [rs_ptr](FrameGraphObjectInfo& info)->bool {
		return info.m_object == rs_ptr;
	});
	if (it_rs == m_allUsedDrawingStates.end())
	{
		m_allUsedDrawingStates.push_back(FrameGraphObjectInfo(rs_ptr));
	}


	// pixel shader
	{
		auto ps_ptr = pso.m_PSState.m_shader.get();
		if (ps_ptr)
		{
			auto it_pshader = std::find_if(m_allUsedShaders.begin(), m_allUsedShaders.end(), [ps_ptr](FrameGraphObjectInfo& info)->bool {
				return info.m_object == ps_ptr;
			});
			if (it_pshader == m_allUsedShaders.end())
			{
				m_allUsedShaders.push_back(FrameGraphObjectInfo(ps_ptr));
			}

			for (auto& cb : pso.m_PSState.m_constantBuffers)
			{
				if (cb)
				{
					auto cbuffer_ptr = cb.get();
					auto cbuffer_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [cbuffer_ptr](FrameGraphResourceInfo& info)->bool {
						return info.m_object == cbuffer_ptr;
					});
					if (cbuffer_it == m_allUsedResources.end())
					{
						m_allUsedResources.push_back(FrameGraphResourceInfo(cbuffer_ptr));
					}
				}
			}

			for (auto& samp : pso.m_PSState.m_samplers)
			{
				if (samp)
				{
					auto samp_ptr = samp.get();
					auto samp_it = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [samp_ptr](FrameGraphObjectInfo& info)->bool {
						return info.m_object == samp_ptr;
					});
					if (samp_it == m_allUsedDrawingStates.end())
					{
						m_allUsedDrawingStates.push_back(FrameGraphObjectInfo(samp_ptr));
					}
				}
			}

			for (auto& shaderRes : pso.m_PSState.m_shaderResources)
			{
				if (shaderRes)
				{
					auto shaderRes_ptr = shaderRes.get();
					auto shaderRes_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [shaderRes_ptr](FrameGraphResourceInfo& info)->bool {
						return info.m_object == shaderRes_ptr;
					});
					if (shaderRes_it == m_allUsedResources.end())
					{
						m_allUsedResources.push_back(FrameGraphResourceInfo(shaderRes_ptr));
					}
				}
			}
		}
	}

	// output merger
	auto blendState_ptr = &pso.m_OMState.m_blendState;
	auto it_blendState = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [blendState_ptr](FrameGraphObjectInfo& info)->bool {
		return info.m_object == blendState_ptr;
	});
	if (it_blendState == m_allUsedDrawingStates.end())
	{
		m_allUsedDrawingStates.push_back(FrameGraphObjectInfo(blendState_ptr));
	}

	auto ds_ptr = &pso.m_OMState.m_dsState;
	auto it_ds = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [ds_ptr](FrameGraphObjectInfo& info)->bool{
		return info.m_object == ds_ptr;
	});
	if (it_ds == m_allUsedDrawingStates.end())
	{
		m_allUsedDrawingStates.push_back(FrameGraphObjectInfo(ds_ptr));
	}

	auto dsRes_ptr = pso.m_OMState.m_depthStencilResource.get();
	auto it_DSRes = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [dsRes_ptr](FrameGraphResourceInfo& info)->bool {
		return info.m_object == dsRes_ptr;
	});
	if (it_DSRes == m_allUsedResources.end())
	{
		m_allUsedResources.push_back(FrameGraphResourceInfo(dsRes_ptr));
	}

	for (auto rt : pso.m_OMState.m_renderTargetResources)
	{
		if (rt)
		{
			auto rt_ptr = rt.get();
			auto it_rt = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [rt_ptr](FrameGraphResourceInfo& info)->bool {
				return info.m_object == rt_ptr;
			});
			if (it_rt == m_allUsedResources.end())
			{
				m_allUsedResources.push_back(FrameGraphResourceInfo(rt_ptr));
			}
		}
	}
}

void FrameGraph::LinkInfo()
{
	for (auto& passInfo : m_passDB)
	{
		auto pass_ptr = passInfo.m_renderPass;

		if (!pass_ptr->IsPSO<RasterPipelineStateObject>())
			continue;
		auto& pso = pass_ptr->GetPSO<RasterPipelineStateObject>();

		// vertex format
		auto vformat = &pso.m_IAState.m_vertexLayout;
		auto it_vformat = std::find_if(m_allUsedVertexFormats.begin(), m_allUsedVertexFormats.end(), [vformat](FrameGraphObjectInfo& obj)->bool {
			return obj.m_object == vformat;
		});
		assert(it_vformat != m_allUsedVertexFormats.end());
		passInfo.m_vertexFormat = &*it_vformat;
		registerVertexFormat(it_vformat->GetVertexFormat(), pass_ptr);

		// vertex buffers
		for (auto& vb : pso.m_IAState.m_vertexBuffers)
		{
			if (vb)
			{
				auto vbuffer_ptr = vb.get();
				auto it_vbuffer = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [vbuffer_ptr](FrameGraphResourceInfo& info)->bool {
					return info.m_object == vbuffer_ptr;
				});
				assert(it_vbuffer != m_allUsedResources.end());
				Add_Input_Resource_To_RenderPassInfo(it_vbuffer);
			}
		}

		// index buffer
		auto ibuffer_ptr = pso.m_IAState.m_indexBuffer.get();
		if (ibuffer_ptr)
		{
			auto it_ibuffer = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [ibuffer_ptr](FrameGraphResourceInfo& info)->bool {
				return info.m_object == ibuffer_ptr;
			});
			assert(it_ibuffer != m_allUsedResources.end());
			Add_Input_Resource_To_RenderPassInfo(it_ibuffer);
		}

		// vertex shader
		{
			auto vs_ptr = pso.m_VSState.m_shader.get();
			if (vs_ptr)
			{
				auto it_vshader = std::find_if(m_allUsedShaders.begin(), m_allUsedShaders.end(), [vs_ptr](FrameGraphObjectInfo& info)->bool {
					return info.m_object == vs_ptr;
				});
				assert(it_vshader != m_allUsedShaders.end());
				passInfo.m_shaders.push_back(&*it_vshader);
				registerShader(it_vshader->GetFrameGraphShader(), pass_ptr);

				for (auto& cb : pso.m_VSState.m_constantBuffers)
				{
					if (cb)
					{
						auto cbuffer_ptr = cb.get();
						auto cbuffer_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [cbuffer_ptr](FrameGraphResourceInfo& info)->bool {
							return info.m_object == cbuffer_ptr;
						});
						assert(cbuffer_it != m_allUsedResources.end());
						Add_Input_Resource_To_RenderPassInfo(cbuffer_it);
					}
				}

				for (auto& samp : pso.m_VSState.m_samplers)
				{
					if (samp)
					{
						auto samp_ptr = samp.get();
						auto samp_it = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [samp_ptr](FrameGraphObjectInfo& info)->bool {
							return info.m_object == samp_ptr;
						});
						assert(samp_it != m_allUsedDrawingStates.end());
						Add_DrawingState_To_RenderPassInfo(samp_it);
					}
				}

				for (auto& shaderRes : pso.m_VSState.m_shaderResources)
				{
					if (shaderRes)
					{
						auto shaderRes_ptr = shaderRes.get();
						auto shaderRes_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [shaderRes_ptr](FrameGraphResourceInfo& info)->bool {
							return info.m_object == shaderRes_ptr;
						});
						assert(shaderRes_it != m_allUsedResources.end());
						Add_Input_Resource_To_RenderPassInfo(shaderRes_it);
					}
				}
			}
		}

		// geometry shader
		{
			auto gs_ptr = pso.m_GSState.m_shader.get();
			if (gs_ptr)
			{
				auto it_gshader = std::find_if(m_allUsedShaders.begin(), m_allUsedShaders.end(), [gs_ptr](FrameGraphObjectInfo& info)->bool {
					return info.m_object == gs_ptr;
				});
				assert(it_gshader != m_allUsedShaders.end());
				passInfo.m_shaders.push_back(&*it_gshader);
				registerShader(it_gshader->GetFrameGraphShader(), pass_ptr);

				for (auto& cb : pso.m_GSState.m_constantBuffers)
				{
					if (cb)
					{
						auto cbuffer_ptr = cb.get();
						auto cbuffer_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [cbuffer_ptr](FrameGraphResourceInfo& info)->bool {
							return info.m_object == cbuffer_ptr;
						});
						assert(cbuffer_it != m_allUsedResources.end());
						Add_Input_Resource_To_RenderPassInfo(cbuffer_it);
					}
				}

				for (auto& samp : pso.m_GSState.m_samplers)
				{
					if (samp)
					{
						auto samp_ptr = samp.get();
						auto samp_it = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [samp_ptr](FrameGraphObjectInfo& info)->bool {
							return info.m_object == samp_ptr;
						});
						assert(samp_it != m_allUsedDrawingStates.end());
						Add_DrawingState_To_RenderPassInfo(samp_it);
					}
				}

				for (auto& shaderRes : pso.m_GSState.m_shaderResources)
				{
					if (shaderRes)
					{
						auto shaderRes_ptr = shaderRes.get();
						auto shaderRes_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [shaderRes_ptr](FrameGraphResourceInfo& info)->bool {
							return info.m_object == shaderRes_ptr;
						});
						assert(shaderRes_it != m_allUsedResources.end());
						Add_Input_Resource_To_RenderPassInfo(shaderRes_it);
					}
				}
			}
		}

		// rasterizor stage
		auto rs_ptr = &pso.m_RSState.m_rsState;
		auto it_rs = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [rs_ptr](FrameGraphObjectInfo& info)->bool {
			return info.m_object == rs_ptr;
		});
		assert(it_rs != m_allUsedDrawingStates.end());
		passInfo.m_drawingStates.push_back(&*it_rs);
		registerDrawingState(it_rs->GetFrameGraphDrawingState(), pass_ptr);


		// pixel shader
		{
			auto ps_ptr = pso.m_PSState.m_shader.get();
			if (ps_ptr)
			{
				auto it_pshader = std::find_if(m_allUsedShaders.begin(), m_allUsedShaders.end(), [ps_ptr](FrameGraphObjectInfo& info)->bool {
					return info.m_object == ps_ptr;
				});
				assert(it_pshader != m_allUsedShaders.end());
				passInfo.m_shaders.push_back(&*it_pshader);
				registerShader(it_pshader->GetFrameGraphShader(), pass_ptr);

				for (auto& cb : pso.m_PSState.m_constantBuffers)
				{
					if (cb)
					{
						auto cbuffer_ptr = cb.get();
						auto cbuffer_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [cbuffer_ptr](FrameGraphResourceInfo& info)->bool {
							return info.m_object == cbuffer_ptr;
						});
						assert(cbuffer_it != m_allUsedResources.end());
						Add_Input_Resource_To_RenderPassInfo(cbuffer_it);
					}
				}

				for (auto& samp : pso.m_PSState.m_samplers)
				{
					if (samp)
					{
						auto samp_ptr = samp.get();
						auto samp_it = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [samp_ptr](FrameGraphObjectInfo& info)->bool {
							return info.m_object == samp_ptr;
						});
						assert(samp_it != m_allUsedDrawingStates.end());
						Add_DrawingState_To_RenderPassInfo(samp_it);
					}
				}

				for (auto& shaderRes : pso.m_PSState.m_shaderResources)
				{
					if (shaderRes)
					{
						auto shaderRes_ptr = shaderRes.get();
						auto shaderRes_it = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [shaderRes_ptr](FrameGraphResourceInfo& info)->bool {
							return info.m_object == shaderRes_ptr;
						});
						assert(shaderRes_it != m_allUsedResources.end());
						Add_Input_Resource_To_RenderPassInfo(shaderRes_it);
					}
				}
			}
		}

		// output merger
		auto blendState_ptr = &pso.m_OMState.m_blendState;
		auto it_blendState = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [blendState_ptr](FrameGraphObjectInfo& info)->bool {
			return info.m_object == blendState_ptr;
		});
		assert(it_blendState != m_allUsedDrawingStates.end());
		passInfo.m_drawingStates.push_back(&*it_blendState);
		registerDrawingState(it_blendState->GetFrameGraphDrawingState(), pass_ptr);

		auto ds_ptr = &pso.m_OMState.m_dsState;
		auto it_ds = std::find_if(m_allUsedDrawingStates.begin(), m_allUsedDrawingStates.end(), [ds_ptr](FrameGraphObjectInfo& info)->bool {
			return info.m_object == ds_ptr;
		});
		assert(it_ds != m_allUsedDrawingStates.end());
		passInfo.m_drawingStates.push_back(&*it_ds);
		registerDrawingState(it_ds->GetFrameGraphDrawingState(), pass_ptr);

		auto dsRes_ptr = pso.m_OMState.m_depthStencilResource.get();
		if (dsRes_ptr)
		{
			auto it_DSRes = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [dsRes_ptr](FrameGraphResourceInfo& info)->bool {
				return info.m_object == dsRes_ptr;
			});
			assert(it_DSRes != m_allUsedResources.end());
			passInfo.m_outputResources.push_back(&*it_DSRes);
			registerWriteFrameGraphResource(it_DSRes->GetFrameGraphResource(), pass_ptr);
		}

		for (auto rt : pso.m_OMState.m_renderTargetResources)
		{
			if (rt)
			{
				auto rt_ptr = rt.get();
				auto it_rt = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [rt_ptr](FrameGraphResourceInfo& info)->bool {
					return info.m_object == rt_ptr;
				});
				assert(it_rt != m_allUsedResources.end());
				passInfo.m_outputResources.push_back(&*it_rt);
				registerWriteFrameGraphResource(it_rt->GetFrameGraphResource(), pass_ptr);
			}
		}
	}
}


RenderPassInfo::RenderPassInfo(RenderPass* pass)
	: m_renderPass(pass)
{
}

FrameGraphObjectInfo::FrameGraphObjectInfo(GraphicsObject* obj)
	: m_object(obj)
{

}

FrameGraphResourceInfo::FrameGraphResourceInfo(Resource* res)
	: FrameGraphObjectInfo(res)
{

}

Resource* FrameGraphResourceInfo::GetFrameGraphResource()
{
	return dynamic_cast<Resource*>(m_object);
}

DrawingState* FrameGraphObjectInfo::GetFrameGraphDrawingState()
{
	return dynamic_cast<DrawingState*>(m_object);
}

Shader* FrameGraphObjectInfo::GetFrameGraphShader()
{
	return dynamic_cast<Shader*>(m_object);
}

VertexFormat* FrameGraphObjectInfo::GetVertexFormat()
{
	return dynamic_cast<VertexFormat*>(m_object);
}