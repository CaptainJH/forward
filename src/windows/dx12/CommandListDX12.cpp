//***************************************************************************************
// CommandListDX12.cpp by Heqi Ju (C) 2022 All Rights Reserved.
//***************************************************************************************
#include "CommandListDX12.h"

using namespace forward;

void CommandListDX12::Reset()
{

}

void CommandListDX12::Close()
{

}

CommandListDX12::CommandListDX12(Device& d) 
	: CommandList(d) 
{}

CommandListDX12::~CommandListDX12() 
{}

void CommandListDX12::DrawScreenText(const std::string& msg, i32 x, i32 y, const Vector4f& color)
{
	msg; x; y; color;

}

void CommandListDX12::Draw(u32 vertexNum, u32 startVertexLocation)
{
	vertexNum; startVertexLocation;
}

void CommandListDX12::DrawIndexed(u32 indexCount)
{
	indexCount;
}

void CommandListDX12::BeginDrawFrameGraph(FrameGraph* fg)
{
	fg;
}

void CommandListDX12::EndDrawFrameGraph()
{

}

void CommandListDX12::DrawRenderPass(RenderPass& pass)
{
	pass;
}