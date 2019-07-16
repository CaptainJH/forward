//***************************************************************************************
// render.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "render.h"
#include "render/FrameGraph/FrameGraph.h"

using namespace forward;

//--------------------------------------------------------------------------------
Renderer* Renderer::m_spRenderer = nullptr;
//--------------------------------------------------------------------------------

Renderer::Renderer()
{
	if (m_spRenderer == nullptr)
		m_spRenderer = this;
}

Renderer::~Renderer()
{

}

void Renderer::BeginDrawFrameGraph(FrameGraph* fg)
{
	assert(m_currentFrameGraph == nullptr);
	m_currentFrameGraph = fg;

	m_currentFrameGraph->Reset();
}

void Renderer::AddExternalResource(const char* name, void* res)
{
	std::string str(name);
	m_externalResourceContext[str] = res;
}