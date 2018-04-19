#pragma once

#include "render/FrameGraph/RenderPassHelper.h"
#include "Vector2f.h"

namespace forward
{
	struct Vertex_Font_POS2_TEX2
	{
		Vector2f Pos;
		Vector2f TexCoord;
	};

	class Font : public IRenderPassSource
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
		shared_ptr<FrameGraphVertexBuffer>	mVertexBuffer;
		shared_ptr<FrameGraphIndexBuffer>	mIndexBuffer;
		shared_ptr<FrameGraphTexture2D>		mTexture;
		shared_ptr<FrameGraphVertexShader>	mVertexShader;
		shared_ptr<FrameGraphPixelShader>	mPixelShader;
		shared_ptr<FrameGraphConstantBuffer<Vector4f>> mConstantBufferVS;
		shared_ptr<FrameGraphConstantBuffer<Vector4f>> mConstantBufferPS;
		shared_ptr<SamplerState>			mSampler;
		f32 mCharacterData[257];

	private:
		void OnRenderPassBuilding(RenderPass&) override;
	};
}