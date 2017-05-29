//***************************************************************************************
// render.cpp by Heqi Ju (C) 2017 All Rights Reserved.
//***************************************************************************************
#include "render.h"

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