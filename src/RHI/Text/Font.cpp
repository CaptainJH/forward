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
	mVertexBuffer = make_shared<VertexBuffer>("ArialVB", vformat, numVertices);
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
		vertices[i].Pos = { 0.0f, 0.0f };
		vertices[i].TexCoord = { 0.0f, 0.0f };
	}
	for (u32 i = 0U; i < mMaxMessageLength; ++i)
	{
		auto& v0 = vertices[4 * i + 0];
		auto& v1 = vertices[4 * i + 1];
		auto& v2 = vertices[4 * i + 2];
		auto& v3 = vertices[4 * i + 3];

		v0.Pos[1] = 0.0f;
		v0.TexCoord[1] = 1.0f;
		v1.TexCoord[1] = 0.0f;
		v2.Pos[1] = 0.0f;
		v2.TexCoord[1] = 1.0f;
		v3.TexCoord[1] = 0.0f;
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
	mIndexBuffer = make_shared<IndexBuffer>("ArialIB", PT_TRIANGLELIST, numTriangles * 3);
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
	mTexture = make_shared<Texture2D>("ArialTex", DF_R8_UNORM, width, height, TextureBindPosition::TBP_Shader);
	memcpy(mTexture->GetData(), texels, mTexture->GetNumBytes());
	memcpy(mCharacterData, characterData, 257 * sizeof(f32));

	mVertexShader = make_shared<VertexShader>("ArialVS", L"TextEffect", "VSMain");
	mPixelShader = make_shared<PixelShader>("ArialPS", L"TextEffect", "PSMain");
	mConstantBufferVS = make_shared<ConstantBuffer<float4>>("ArialCB_VS");
	mConstantBufferPS = make_shared<ConstantBuffer<float4>>("ArialCB_PS");
	mSampler = make_shared<SamplerState>("ArialSamp");
}

void Font::Typeset(i32 viewportWidth, i32 viewportHeight, i32 x, i32 y, float4 const& color, std::string const& message) const
{
	// Get texel translation units, depends on viewport width and height.
	auto const vdx = 1.0f / static_cast<f32>(viewportWidth);
	auto const vdy = 1.0f / static_cast<f32>(viewportHeight);

	// Get texture information.
	auto tw = static_cast<f32>(mTexture->GetWidth());
	auto th = static_cast<f32>(mTexture->GetHeight());

	// Get vertex buffer information.
	auto vertexSize = mVertexBuffer->GetVertexFormat().GetVertexSize();
	auto data = mVertexBuffer->GetData();

	f32 x0 = 0.0f;
	auto const length = std::min(static_cast<u32>(message.length()), mMaxMessageLength);
	for (auto i = 0U; i < length; ++i)
	{
		// Get character data.
		auto c = static_cast<i32>(message[i]);
		c -= 32; // starting from ASCII Space
		auto const tx0 = mCharacterData[c];
		auto const tx1 = mCharacterData[c + 1];
		auto charWidthM1 = (tx1 - tx0)*tw - 1.0f;  // in pixels

		// 0 -- 2   4 -- 6  ...
		// | \  |   | \  | 
		// |  \ |   |  \ | 
		// 1 -- 3   5 -- 7  ...
		auto* v0 = reinterpret_cast<f32*>(data + (4 * i + 0)*vertexSize);
		auto* v1 = reinterpret_cast<f32*>(data + (4 * i + 1)*vertexSize);
		auto* v2 = reinterpret_cast<f32*>(data + (4 * i + 2)*vertexSize);
		auto* v3 = reinterpret_cast<f32*>(data + (4 * i + 3)*vertexSize);

		// Set bottom left vertex y coordinate.
		v1[1] = vdy*th;

		// Set x-coordinates.
		auto x1 = x0 + charWidthM1*vdx;
		v0[0] = x0;
		v1[0] = x0;
		v2[0] = x1;
		v3[0] = x1;

		// Set bottom right-side y-coordinate.
		v3[1] = vdy*th;

		// Set the four texture x-coordinates.  The y-coordinates were set in
		// the constructor.
		v0[2] = tx0;
		v1[2] = tx0;
		v2[2] = tx1;
		v3[2] = tx1;

		// Update left x coordinate for next quad
		x0 = x1;
	}

	mVertexBuffer->SetDirty();

	//// Update the number of triangles that should be drawn.
	//mVertexBuffer->SetNumActiveElements(4 * length);
	//mIndexBuffer->SetNumActivePrimitives(2 * length);

	// Set effect parameters.
	auto trnX = vdx * static_cast<f32>(x);
	auto trnY = 1.0f - vdy * static_cast<f32>(y);
	mConstantBufferVS->SetTypedData(float4(trnX, trnY, 0.0f, 0.0f));
	mConstantBufferPS->SetTypedData(color);
}

void Font::OnRenderPassBuilding(RenderPass& pass)
{
	auto& pso = pass.GetPSO<RasterPipelineStateObject>();

	pass.m_ia_params.m_indexBuffer = mIndexBuffer;
	pass.m_ia_params.m_topologyType = mIndexBuffer->GetPrimitiveType();

	pass.m_ia_params.m_vertexBuffers[0] = mVertexBuffer;
	pso.m_IAState.m_vertexLayout = mVertexBuffer->GetVertexFormat();

	pass.m_vs.m_constantBuffers[0] = mConstantBufferVS;
	pso.m_VSState.m_shader = mVertexShader;

	pass.m_ps.m_constantBuffers[1] = mConstantBufferPS;
	pass.m_ps.m_shaderResources[0] = mTexture;
	pso.m_PSState.m_samplers[0] = mSampler;
	pso.m_PSState.m_shader = mPixelShader;
}

u32 Font::GetIndexCount() const
{
	return mIndexBuffer->GetNumElements();
}