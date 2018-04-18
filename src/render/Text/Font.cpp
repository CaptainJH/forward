#include "Font.h"

using namespace forward;

Font::Font(u32 width, u32 height, i8 const* texels,
	f32 const* characterData, u32 maxMessageLength)
	: mMaxMessageLength(maxMessageLength)
{
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
	auto numVertices = 4 * mMaxMessageLength;
	mVertexBuffer = make_shared<FrameGraphVertexBuffer>("ArialVB", vformat, numVertices);
	mVertexBuffer->SetUsage(RU_DYNAMIC_UPDATE);

	// Set the y values for top vertex positions and all texture
	// coordinates, since they do not change.
	// 0 -- 2   4 -- 6  ... <-- pos.y = 0, tex.y = 0
	// | \  |   | \  | 
	// |  \ |   |  \ | 
	// 1 -- 3   5 -- 7  ... <-- tex.y = 1
	Vertex_Font_POS2_TEX2* vertices = mVertexBuffer->GetData<Vertex_Font_POS2_TEX2*>();
	for (auto i = 0U; i < numVertices; ++i)
	{
		vertices[i].Pos = Vector2f::Zero();
		vertices[i].TexCoord = Vector2f::Zero();
	}
	for (u32 i = 0U; i < mMaxMessageLength; ++i)
	{
		auto& v0 = vertices[4 * i + 0];
		auto& v1 = vertices[4 * i + 1];
		auto& v2 = vertices[4 * i + 2];
		auto& v3 = vertices[4 * i + 3];

		v0.Pos[1] = 0.0f;
		v0.TexCoord[1] = 0.0f;
		v1.TexCoord[1] = 1.0f;
		v2.Pos[1] = 0.0f;
		v2.TexCoord[1] = 0.0f;
		v3.TexCoord[1] = 1.0f;
	}

	// Set the x coordinates on the first two vertices to zero, 
	// since they do not change.
	vertices[0].Pos[0] = 0.0f;
	vertices[1].Pos[0] = 0.0f;

	// Create and set the index buffer data.
	// 0 -- 2   4 -- 6  ...
	// | \  |   | \  | 
	// |  \ |   |  \ | 
	// 1 -- 3   5 -- 7  ...
	auto numTriangles = 2 * mMaxMessageLength;
	mIndexBuffer = make_shared<FrameGraphIndexBuffer>("ArialIB", PT_TRIANGLELIST, numTriangles * 3);
	u32* ibuf = mIndexBuffer->GetData<u32*>();
	for (auto i = 0U; i < mMaxMessageLength; ++i)
	{
		// Bottom triangle
		ibuf[6 * i + 0] = 4 * i;
		ibuf[6 * i + 1] = 4 * i + 3;
		ibuf[6 * i + 2] = 4 * i + 1;

		// Top triangle
		ibuf[6 * i + 3] = 4 * i;
		ibuf[6 * i + 4] = 4 * i + 2;
		ibuf[6 * i + 5] = 4 * i + 3;
	}

	// Create a texture from the specified monochrome bitmap.
	mTexture = make_shared<FrameGraphTexture2D>("ArialTex", DF_R8_UNORM, width, height, TextureBindPosition::TBP_Shader);
	memcpy(mTexture->GetData(), texels, mTexture->GetNumBytes());
	memcpy(mCharacterData, characterData, 257 * sizeof(f32));

	mVertexShader = make_shared<FrameGraphVertexShader>("ArialVS", L"TextEffect.hlsl", L"VSMain");
	mPixelShader = make_shared<FrameGraphPixelShader>("ArialPS", L"TextEffect.hlsl", L"PSMain");
	mConstantBuffer = make_shared<FrameGraphConstantBuffer<Vector2f>>("ArialCB");
}

void Font::OnRenderPassBuilding(RenderPass&)
{

}