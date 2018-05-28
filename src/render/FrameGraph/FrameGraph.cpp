//***************************************************************************************
// FrameGraph.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************
#include "FrameGraph.h"

using namespace forward;

void FrameGraph::Reset()
{
	m_passDB.clear();
	m_allUsedResources.clear();
}

void FrameGraph::DrawRenderPass(RenderPass* pass)
{
	registerRenderPass(pass);

	auto& pso = pass->GetPSO();
	registerVertexFormat(&pso.m_IAState.m_vertexLayout, pass);

	for (auto& vb : pso.m_IAState.m_vertexBuffers)
	{
		if (vb)
		{
			registerReadFrameGraphResource(vb.get(), pass);
		}
	}

	registerReadFrameGraphResource(pso.m_IAState.m_indexBuffer.get(), pass);

	// vertex shader
	{
		auto vs_ptr = pso.m_VSState.m_shader.get();
		registerShader(vs_ptr, pass);

		for (auto& cb : pso.m_VSState.m_constantBuffers)
		{
			if (cb)
			{
				auto cbuffer_ptr = cb.get();
				registerReadFrameGraphResource(cbuffer_ptr, pass);
			}
		}

		for (auto& samp : pso.m_VSState.m_samplers)
		{
			if (samp)
			{
				auto samp_ptr = samp.get();
				registerDrawingState(samp_ptr, pass);
			}
		}

		for (auto& shaderRes : pso.m_VSState.m_shaderResources)
		{
			if (shaderRes)
			{
				auto shaderRes_ptr = shaderRes.get();
				registerReadFrameGraphResource(shaderRes_ptr, pass);
			}
		}
	}

	// geometry shader
	{
		auto gs_ptr = pso.m_GSState.m_shader.get();
		registerShader(gs_ptr, pass);

		for (auto& cb : pso.m_GSState.m_constantBuffers)
		{
			if (cb)
			{
				auto cbuffer_ptr = cb.get();
				registerReadFrameGraphResource(cbuffer_ptr, pass);
			}
		}

		for (auto& samp : pso.m_GSState.m_samplers)
		{
			if (samp)
			{
				auto samp_ptr = samp.get();
				registerDrawingState(samp_ptr, pass);
			}
		}

		for (auto& shaderRes : pso.m_GSState.m_shaderResources)
		{
			if (shaderRes)
			{
				auto shaderRes_ptr = shaderRes.get();
				registerReadFrameGraphResource(shaderRes_ptr, pass);
			}
		}
	}

	// rasterizer
	auto rs_ptr = &pso.m_RSState.m_rsState;
	registerDrawingState(rs_ptr, pass);

	// pixel shader
	{
		auto ps_ptr = pso.m_PSState.m_shader.get();
		registerShader(ps_ptr, pass);

		for (auto& cb : pso.m_PSState.m_constantBuffers)
		{
			if (cb)
			{
				auto cbuffer_ptr = cb.get();
				registerReadFrameGraphResource(cbuffer_ptr, pass);
			}
		}

		for (auto& samp : pso.m_PSState.m_samplers)
		{
			if (samp)
			{
				auto samp_ptr = samp.get();
				registerDrawingState(samp_ptr, pass);
			}
		}

		for (auto& shaderRes : pso.m_PSState.m_shaderResources)
		{
			if (shaderRes)
			{
				auto shaderRes_ptr = shaderRes.get();
				registerReadFrameGraphResource(shaderRes_ptr, pass);
			}
		}
	}

	// output merger
	auto blendState_ptr = &pso.m_OMState.m_blendState;
	registerDrawingState(blendState_ptr, pass);

	auto ds_ptr = &pso.m_OMState.m_dsState;
	registerDrawingState(ds_ptr, pass);

}

std::vector<RenderPassInfo>& FrameGraph::GetRenderPassDB()
{
	return m_passDB;
}

FrameGraphObjectInfo* FrameGraph::registerShader(FrameGraphShader* shader, RenderPass* pass)
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

FrameGraphObjectInfo* FrameGraph::registerDrawingState(FrameGraphDrawingState* state, RenderPass* pass)
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

FrameGraphResourceInfo* FrameGraph::registerReadFrameGraphResource(FrameGraphResource* res, RenderPass* pass)
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

FrameGraphResourceInfo* FrameGraph::registerWriteFrameGraphResource(FrameGraphResource* res, RenderPass* pass)
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

	auto& pso = pass->GetPSO();

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
	auto it_ibuffer = std::find_if(m_allUsedResources.begin(), m_allUsedResources.end(), [ibuffer_ptr](FrameGraphResourceInfo& info)->bool {
		return info.m_object == ibuffer_ptr;
	});
	if (it_ibuffer == m_allUsedResources.end())
	{
		m_allUsedResources.push_back(FrameGraphResourceInfo(ibuffer_ptr));
	}

	// vertex shader
	{
		auto vs_ptr = pso.m_VSState.m_shader.get();
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

	// geometry shader
	{
		auto gs_ptr = pso.m_GSState.m_shader.get();
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
}


RenderPassInfo::RenderPassInfo(RenderPass* pass)
	: m_renderPass(pass)
{
}

FrameGraphObjectInfo::FrameGraphObjectInfo(FrameGraphObject* obj)
	: m_object(obj)
{

}

FrameGraphResourceInfo::FrameGraphResourceInfo(FrameGraphResource* res)
	: FrameGraphObjectInfo(res)
{

}