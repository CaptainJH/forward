#pragma once

#include "RHI/FrameGraph/RenderPassHelper.h"
#include "Vector2f.h"

namespace forward
{
	struct Vertex_Font_POS2_TEX2
	{
		Vector2f Pos;
		Vector2f TexCoord;
	};

	class Font : public IRenderPassGenerator
	{
	public:
		// Construction.
		Font(u32 width, u32 height, i8 const* texels, 
			f32 const* characterData, u32 maxMessageLength);

		// Populate the vertex buffer for the specified string.
		void Typeset(i32 viewportWidth, i32 viewportHeight, i32 x, i32 y,
			Vector4f const& color, std::string const& message) const;

		u32 GetIndexCount() const;

	protected:

		u32 mMaxMessageLength;
		shared_ptr<VertexBuffer>	mVertexBuffer;
		shared_ptr<IndexBuffer>	mIndexBuffer;
		shared_ptr<Texture2D>		mTexture;
		shared_ptr<VertexShader>	mVertexShader;
		shared_ptr<PixelShader>	mPixelShader;
		shared_ptr<ConstantBuffer<Vector4f>> mConstantBufferVS;
		shared_ptr<ConstantBuffer<Vector4f>> mConstantBufferPS;
		shared_ptr<SamplerState>			mSampler;
		f32 mCharacterData[257];

	private:
		void OnRenderPassBuilding(RenderPass&) override;
	};
}