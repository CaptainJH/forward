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

shared_ptr<Resource> LoadedResourceManager::FindBufferByName(const String& n)
{
	auto it = std::find_if(mAllLoadedBuffers.begin(), mAllLoadedBuffers.end(), [&](auto rp) {
		return rp->Name() == n; });
	return it == mAllLoadedBuffers.end() ? nullptr : *it;
}

shared_ptr<Resource> LoadedResourceManager::FindVertexBufferByName(const String& n)
{
	return FindBufferByName(n + "_VB");
}

shared_ptr<Resource> LoadedResourceManager::FindIndexBufferByName(const String& n)
{
	return FindBufferByName(n + "_IB");
}

shared_ptr<Resource> LoadedResourceManager::FindTextureByName(const String& n)
{
	auto it = std::find_if(mAllLoadedTextures.begin(), mAllLoadedTextures.end(), [&](auto rp) {
		return rp->Name() == n; });
	return it == mAllLoadedTextures.end() ? nullptr : *it;
}