//***************************************************************************************
// FrameGraphTexture.cpp by Heqi Ju (C) 2018 All Rights Reserved.
//***************************************************************************************

#include "FrameGraphTexture.h"

using namespace forward;

FrameGraphTexture::FrameGraphTexture(const std::string& name)
	: FrameGraphResource(name)
{}

FrameGraphTexture2D::FrameGraphTexture2D(const std::string& name)
	: FrameGraphTexture(name)
{}

ResourceType FrameGraphTexture2D::GetResourceType() const
{
	return RT_TEXTURE2D;
}