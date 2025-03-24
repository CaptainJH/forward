#pragma once
#include <vector>
#include <functional>

struct nvg_vertex {
	forward::float3 position;
	forward::float4 color;
	forward::float2 tex_coord;
};

struct D3DNVGfragUniforms {
	float scissorMat[16];
	forward::float4 scissorExt;
	forward::float4 scissorScale;
	float paintMat[16];
	forward::float4 extent;
	forward::float4 radius;
	forward::float4 feather;
	struct NVGcolor innerCol;
	struct NVGcolor outerCol;
	forward::float4 strokeMult;
	int texType;
	int type;
};

struct VS_CONSTANTS {
	float dummy[16];
	float viewSize[2];
};

struct RenderItem {
	std::vector<nvg_vertex> vertex_buffer;
	std::vector<int> index_buffer;
	D3DNVGfragUniforms constant_buffer;

	forward::RasterizerState::CullMode cull_mode = forward::RasterizerState::CULL_NONE;
	forward::OutputMergerStageState om_state;

	forward::shared_ptr<forward::Texture2D> tex = nullptr;

//private:
	void SetupBlenderStateNoWrite() {
		om_state.m_blendState = forward::BlendState();
		om_state.m_blendState.target[0].mask = 0;
	}

	void SetupBlenderStateBlend() {
		om_state.m_blendState = forward::BlendState();
		om_state.m_blendState.target[0].enable = true;
		om_state.m_blendState.target[0].dstColor = forward::BlendState::BM_INV_SRC_ALPHA;
		om_state.m_blendState.target[0].dstAlpha = forward::BlendState::BM_INV_SRC_ALPHA;
	}

	void SetupDepthStencilDrawShapes() {
		auto& dsState = om_state.m_dsState;
		dsState = forward::DepthStencilState();
		dsState.depthEnable = false;
		dsState.stencilEnable = true;
		dsState.frontFace.pass = forward::DepthStencilState::OP_INCR;
		dsState.backFace.pass = forward::DepthStencilState::OP_DECR;
	}

	void SetupDepthStencilDrawAA() {
		auto& dsState = om_state.m_dsState;
		dsState = forward::DepthStencilState();
		dsState.depthEnable = false;
		dsState.stencilEnable = true;
		dsState.frontFace.comparison = forward::DepthStencilState::EQUAL;
		dsState.backFace.comparison = forward::DepthStencilState::EQUAL;
	}

	void SetupDepthStencilFill() {
		auto& dsState = om_state.m_dsState;
		dsState = forward::DepthStencilState();
		dsState.depthEnable = false;
		dsState.stencilEnable = true;
		dsState.frontFace.comparison = forward::DepthStencilState::NOT_EQUAL;
		dsState.frontFace.fail = forward::DepthStencilState::OP_ZERO;
		dsState.frontFace.pass = forward::DepthStencilState::OP_ZERO;
		dsState.backFace.comparison = forward::DepthStencilState::NOT_EQUAL;
		dsState.backFace.fail = forward::DepthStencilState::OP_ZERO;
		dsState.backFace.pass = forward::DepthStencilState::OP_ZERO;
	}

	void SetupDepthStencilDefault() {
		auto& dsState = om_state.m_dsState;
		dsState = forward::DepthStencilState();
		dsState.depthEnable = false;
		dsState.stencilEnable = false;
	}
};

enum NVGcreateFlags {
	// Flag indicating if geometry based anti-aliasing is used (may not be needed when using MSAA).
	NVG_ANTIALIAS = 1 << 0,
	// Flag indicating if strokes should be drawn using stencil buffer. The rendering will be a little
	// slower, but path overlaps (i.e. self-intersecting or sharp turns) will be drawn just once.
	NVG_STENCIL_STROKES = 1 << 1,
	// Flag indicating that additional debug checks are done.
	NVG_DEBUG = 1 << 2,
};

NVGcontext* nvgCreateForward(int flags);
void nvgDeleteForward(NVGcontext* ctx);

std::function<void(RenderItem& ri)> forward_nvg_fill_callback = nullptr;

#if defined NANOVG_FORWARD_IMPLEMENTATION

struct ForwardNVGcontext {

	float viewport[2];

	struct TextureInfo {
		forward::shared_ptr<forward::Texture2D> tex;
		int nvgId;
		int nvgType;
		int nvgFlags;
		int w;
		int h;

	};
	std::vector<TextureInfo> textures;
};

enum D3DNVGshaderType {
	NSVG_SHADER_FILLGRAD,
	NSVG_SHADER_FILLIMG,
	NSVG_SHADER_SIMPLE,
	NSVG_SHADER_IMG
};

static void D3Dnvg__vset(struct NVGvertex* vtx, float x, float y, float u, float v)
{
	vtx->x = x;
	vtx->y = y;
	vtx->u = u;
	vtx->v = v;
}

static void D3Dnvg_copyMatrix3to4(float* pDest, const float* pSource)
{
	unsigned int i;
	for (i = 0; i < 4; i++)
	{
		memcpy(&pDest[i * 4], &pSource[i * 3], sizeof(float) * 3);
	}
}

static void D3Dnvg__xformToMat3x3(float* m3, float* t)
{
	m3[0] = t[0];
	m3[1] = t[1];
	m3[2] = 0.0f;
	m3[3] = t[2];
	m3[4] = t[3];
	m3[5] = 0.0f;
	m3[6] = t[4];
	m3[7] = t[5];
	m3[8] = 1.0f;
}

static struct NVGcolor D3Dnvg__premulColor(struct NVGcolor c)
{
	c.r *= c.a;
	c.g *= c.a;
	c.b *= c.a;
	return c;
}

static int D3Dnvg__convertPaint(ForwardNVGcontext* ctx, struct D3DNVGfragUniforms* frag,
	struct NVGpaint* paint, struct NVGscissor* scissor,
	float width, float fringe, float strokeThr)
{
	float invxform[6], paintMat[9], scissorMat[9];

	memset(frag, 0, sizeof(*frag));

	frag->innerCol = D3Dnvg__premulColor(paint->innerColor);
	frag->outerCol = D3Dnvg__premulColor(paint->outerColor);

	if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f)
	{
		memset(scissorMat, 0, sizeof(scissorMat));
		frag->scissorExt[0] = 1.0f;
		frag->scissorExt[1] = 1.0f;
		frag->scissorScale[0] = 1.0f;
		frag->scissorScale[1] = 1.0f;
	}
	else
	{
		nvgTransformInverse(invxform, scissor->xform);
		D3Dnvg__xformToMat3x3(scissorMat, invxform);
		frag->scissorExt[0] = scissor->extent[0];
		frag->scissorExt[1] = scissor->extent[1];
		frag->scissorScale[0] = sqrtf(scissor->xform[0] * scissor->xform[0] + scissor->xform[2] * scissor->xform[2]) / fringe;
		frag->scissorScale[1] = sqrtf(scissor->xform[1] * scissor->xform[1] + scissor->xform[3] * scissor->xform[3]) / fringe;
	}
	D3Dnvg_copyMatrix3to4(frag->scissorMat, scissorMat);


	frag->extent[0] = paint->extent[0];
	frag->extent[1] = paint->extent[1];

	frag->strokeMult[0] = (width * 0.5f + fringe * 0.5f) / fringe;
	frag->strokeMult[1] = strokeThr;

	if (paint->image != 0)
	{
		const auto& tex = *std::find_if(ctx->textures.begin(), ctx->textures.end(), [&](auto& t)->bool {
			return paint->image == t.nvgId;
			});
		if (!tex.tex)
		{
			return 0;
		}

		if ((tex.nvgFlags & NVG_IMAGE_FLIPY) != 0)
		{
			float m1[6], m2[6];
			nvgTransformTranslate(m1, 0.0f, frag->extent[1] * 0.5f);
			nvgTransformMultiply(m1, paint->xform);
			nvgTransformScale(m2, 1.0f, -1.0f);
			nvgTransformMultiply(m2, m1);
			nvgTransformTranslate(m1, 0.0f, -frag->extent[1] * 0.5f);
			nvgTransformMultiply(m1, m2);
			nvgTransformInverse(invxform, m1);
		}
		else
		{
			nvgTransformInverse(invxform, paint->xform);
		}
		frag->type = NSVG_SHADER_FILLIMG;

#if NANOVG_GL_USE_UNIFORMBUFFER
		if (tex->type == NVG_TEXTURE_RGBA)
		{
			frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0 : 1;
		}
		else
		{
			frag->texType = 2;
		}
#else
		if (tex.nvgType == NVG_TEXTURE_RGBA)
			frag->texType = (tex.nvgFlags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
		else
			frag->texType = 2.0f;
#endif
	}
	else
	{
		frag->type = NSVG_SHADER_FILLGRAD;
		frag->radius[0] = paint->radius;
		frag->feather[0] = paint->feather;
		nvgTransformInverse(invxform, paint->xform);
	}

	D3Dnvg__xformToMat3x3(paintMat, invxform);
	D3Dnvg_copyMatrix3to4(frag->paintMat, paintMat);

	//D3Dnvg_updateShaders(D3D);

	return 1;
}

static int forwardnvg_renderCreate(void* uptr) {
	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;
	gl->viewport[0] = 0.0f;
	gl->viewport[1] = 0.0f;
	gl->textures.clear();
	return 1;
}

static int forwardnvg_renderCreateTexture(void* uptr, int type, int w, int h, int imageFlags, const unsigned char* data) {

	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;
	const auto retID = static_cast<int>(gl->textures.size()) + 1;
	std::stringstream ss;
	ss << "nvg_tex_" << retID;
	forward::shared_ptr<forward::Texture2D> t;
	if (data) {
		std::string filePath = (const char*)data;
		auto filePathW = forward::TextHelper::ToUnicode(filePath);
		t = forward::make_shared<forward::Texture2D>(ss.str(), filePathW);
	}
	else {
		t = forward::make_shared<forward::Texture2D>(ss.str(), 
			type == 1 ? forward::DF_R8_UNORM : forward::DF_R8G8B8A8_UNORM, w, h, forward::TBP_Shader);
	}
	ForwardNVGcontext::TextureInfo tex = {
		.tex = t,
		.nvgId = retID,
		.nvgType = type,
		.nvgFlags = imageFlags,
		.w = w,
		.h = h
	};
	gl->textures.push_back(tex);
	return retID;
}

static int forwardnvg_renderDeleteTexture(void* uptr, int image) {

	std::cout << "forwardnvg_renderDeleteTexture" << std::endl;

	return 0;
}

static int forwardnvg_renderUpdateTexture(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data) {

	const auto idx = image - 1;
	assert(idx >= 0);
	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;
	const auto& tex = gl->textures[idx];

	const auto left = x;
	const auto right = (x + w);
	const auto top = y;
	const auto bottom = (y + h);

	const unsigned int pixelWidthBytes = tex.nvgType == NVG_TEXTURE_RGBA ? 4 : 1;
	const auto offset = y * (tex.w * pixelWidthBytes) + x * pixelWidthBytes;
	auto pData = (unsigned char*)data + offset;
	auto pDst = tex.tex->GetData() + offset;

	forward::Resource::CopyPitched2(h, tex.w * pixelWidthBytes, pData, tex.tex->GetWidth() * pixelWidthBytes, pDst);

	return 1;
}

static int forwardnvg_renderGetTextureSize(void* uptr, int image, int* w, int* h) {
	const auto idx = image - 1;
	assert(idx >= 0);
	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;
	const auto& tex = gl->textures[idx];
	*w = tex.w;
	*h = tex.h;
	return 1;
}

static void forwardnvg_renderViewport(void* uptr, float width, float height, float devicePixelRatio) {
	NVG_NOTUSED(devicePixelRatio);
	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;
	gl->viewport[0] = width;
	gl->viewport[1] = height;
}

static void forwardnvg_renderCancel(void* uptr) {

	std::cout << "forwardnvg_renderCancel" << std::endl;
}

static void forwardnvg_renderFlush(void* uptr) {

	//std::cout << "forwardnvg_renderFlush" << std::endl;
	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;
}

static void forwardnvg_renderFill(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
	const float* bounds, const NVGpath* paths, int npaths) {

	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;

	auto fromFan2List = [](std::vector<int>& vIndex, int i) {
		if (i < 2)
			return;
		vIndex.push_back(0);
		vIndex.push_back(i - 1);
		vIndex.push_back(i);
		};
	auto fromStrip2List = [](std::vector<int>& vIndex, int i) {
		if (i < 2)
			return;
		vIndex.push_back(i - 2);
		vIndex.push_back(i - 1);
		vIndex.push_back(i);
		};

	auto drawFills = [&](NVGvertex* fills, int fillCount, auto toListFunc, RenderItem& renderItem) {
		if (fillCount > 0) {
			if (fillCount >= 3) {
				std::vector<nvg_vertex> vertex(fillCount);
				std::vector<int> vIndex;
				for (int i = 0; i < fillCount; ++i) {
					vertex[i].position = { fills[i].x, fills[i].y, 0.0f };
					vertex[i].color = forward::float4(paint->innerColor.r, paint->innerColor.g, 
						paint->innerColor.b, paint->innerColor.a);
					vertex[i].tex_coord = { fills[i].u, fills[i].v };
					toListFunc(vIndex, i);
				}

				memset(&renderItem.constant_buffer, 0, sizeof(renderItem.constant_buffer));
				// Fill shader
				D3Dnvg__convertPaint(gl, &renderItem.constant_buffer, paint, scissor, fringe, fringe, -1.0f);
				renderItem.vertex_buffer = vertex;
				renderItem.index_buffer = vIndex;
			}
			else {
				assert(false);
			}
		}
		};

	if (npaths == 1 && paths[0].convex) {
		for (int i = 0; i < npaths; ++i) {
			auto& path = paths[i];
			RenderItem renderItem;
			drawFills(path.fill, path.nfill, fromFan2List, renderItem);
			renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
			renderItem.SetupBlenderStateBlend();
			renderItem.SetupDepthStencilDefault();
			if (forward_nvg_fill_callback) {
				forward_nvg_fill_callback(renderItem);
			}
		}

		for (int i = 0; i < npaths; ++i) {
			auto& path = paths[i];
			RenderItem renderItem;
			drawFills(path.stroke, path.nstroke, fromStrip2List, renderItem);
			renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
			renderItem.SetupBlenderStateBlend();
			renderItem.SetupDepthStencilDefault();
			if (forward_nvg_fill_callback) {
				forward_nvg_fill_callback(renderItem);
			}
		}
	}
	else {

		for (int i = 0; i < npaths; ++i) {
			auto& path = paths[i];
			RenderItem renderItem;
			drawFills(path.fill, path.nfill, fromFan2List, renderItem);
			renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
			renderItem.SetupBlenderStateNoWrite();
			renderItem.SetupDepthStencilDrawShapes();
			memset(&renderItem.constant_buffer, 0, sizeof(renderItem.constant_buffer));
			renderItem.constant_buffer.strokeMult[1] = -1.0f;
			renderItem.constant_buffer.type = NSVG_SHADER_SIMPLE;
			if (forward_nvg_fill_callback) {
				forward_nvg_fill_callback(renderItem);
			}
		}

		for (int i = 0; i < npaths; ++i) {
			auto& path = paths[i];
			RenderItem renderItem;
			drawFills(path.stroke, path.nstroke, fromStrip2List, renderItem);
			renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
			renderItem.SetupBlenderStateBlend();
			renderItem.SetupDepthStencilDrawAA();
			if (forward_nvg_fill_callback) {
				forward_nvg_fill_callback(renderItem);
			}
		}

		NVGvertex quad[4];
		D3Dnvg__vset(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
		D3Dnvg__vset(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
		D3Dnvg__vset(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
		D3Dnvg__vset(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);
		std::vector<int> vIndex = { 0, 1, 2, 2, 1, 3 };
		RenderItem renderItem;
		std::vector<nvg_vertex> vertex(4);
		for (int i = 0; i < 4; ++i) {
			vertex[i].position = { quad[i].x, quad[i].y, 0.0f };
			vertex[i].color = forward::float4(paint->innerColor.r, paint->innerColor.g,
				paint->innerColor.b, paint->innerColor.a);
			vertex[i].tex_coord = { quad[i].u, quad[i].v };
		}

		memset(&renderItem.constant_buffer, 0, sizeof(renderItem.constant_buffer));
		// Fill shader
		D3Dnvg__convertPaint(gl, &renderItem.constant_buffer, paint, scissor, fringe, fringe, -1.0f);
		renderItem.vertex_buffer = vertex;
		renderItem.index_buffer = vIndex;
		renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
		renderItem.SetupBlenderStateBlend();
		renderItem.SetupDepthStencilFill();
		if (forward_nvg_fill_callback)
			forward_nvg_fill_callback(renderItem);
	}
}

static void forwardnvg_renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
	float strokeWidth, const NVGpath* paths, int npaths) {

	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;

	auto fromStrip2List = [](std::vector<int>& vIndex, int i) {
		if (i < 2)
			return;
		vIndex.push_back(i - 2);
		vIndex.push_back(i - 1);
		vIndex.push_back(i);
		};

	auto drawFills = [&](NVGvertex* fills, int fillCount, auto toListFunc, RenderItem& renderItem) {
		if (fillCount > 0) {
			if (fillCount >= 3) {
				std::vector<nvg_vertex> vertex(fillCount);
				std::vector<int> vIndex;
				for (int i = 0; i < fillCount; ++i) {
					vertex[i].position = { fills[i].x, fills[i].y, 0.0f };
					vertex[i].color = forward::float4(paint->innerColor.r, paint->innerColor.g,
						paint->innerColor.b, paint->innerColor.a);
					vertex[i].tex_coord = { fills[i].u, fills[i].v };
					toListFunc(vIndex, i);
				}

				renderItem.vertex_buffer = vertex;
				renderItem.index_buffer = vIndex;
			}
			else {
				assert(false);
			}
		}
		};


	for (int i = 0; i < npaths; ++i) {
		auto& path = paths[i];
		{
			RenderItem renderItem;
			drawFills(path.stroke, path.nstroke, fromStrip2List, renderItem);
			memset(&renderItem.constant_buffer, 0, sizeof(renderItem.constant_buffer));
			// Fill shader
			D3Dnvg__convertPaint(gl, &renderItem.constant_buffer, paint, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);
			renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
			renderItem.SetupBlenderStateBlend();
			renderItem.SetupDepthStencilDefault();
			if (forward_nvg_fill_callback) {
				forward_nvg_fill_callback(renderItem);
			}
		}

		{
			RenderItem renderItem;
			drawFills(path.stroke, path.nstroke, fromStrip2List, renderItem);
			memset(&renderItem.constant_buffer, 0, sizeof(renderItem.constant_buffer));
			D3Dnvg__convertPaint(gl, &renderItem.constant_buffer, paint, scissor, strokeWidth, fringe, -1.0f);
			renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
			renderItem.SetupBlenderStateBlend();
			renderItem.SetupDepthStencilDrawAA();
			if (forward_nvg_fill_callback) {
				forward_nvg_fill_callback(renderItem);
			}
		}

		{
			RenderItem renderItem;
			drawFills(path.stroke, path.nstroke, fromStrip2List, renderItem);
			memset(&renderItem.constant_buffer, 0, sizeof(renderItem.constant_buffer));
			D3Dnvg__convertPaint(gl, &renderItem.constant_buffer, paint, scissor, strokeWidth, fringe, -1.0f);
			renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
			renderItem.SetupBlenderStateBlend();
			renderItem.SetupDepthStencilFill();
			if (forward_nvg_fill_callback) {
				forward_nvg_fill_callback(renderItem);
			}
		}
	}
}

static void forwardnvg_renderTriangles(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor,
	const NVGvertex* verts, int nverts, float fringe) {

	if (nverts == 0)
		return;

	ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;

	RenderItem renderItem;
	std::vector<nvg_vertex> vertex(nverts);
	std::vector<int> vIndex(nverts);
	for (int i = 0; i < nverts; ++i) {
		vertex[i].position = { verts[i].x, verts[i].y, 0.0f };
		vertex[i].color = forward::float4(0, 0, 0, 0);
		vertex[i].tex_coord = { verts[i].u, verts[i].v };
		vIndex[i] = i;
	}
	renderItem.vertex_buffer = vertex;
	renderItem.index_buffer = vIndex;

	memset(&renderItem.constant_buffer, 0, sizeof(renderItem.constant_buffer));
	D3Dnvg__convertPaint(gl, &renderItem.constant_buffer, paint, scissor, 1.0f, fringe, -1.0f);
	renderItem.constant_buffer.type = NSVG_SHADER_IMG;
	renderItem.cull_mode = forward::RasterizerState::CULL_NONE;
	renderItem.SetupBlenderStateBlend();
	renderItem.SetupDepthStencilDefault();
	assert(paint->image == 1);
	renderItem.tex = gl->textures[paint->image - 1].tex;
	if (forward_nvg_fill_callback) {
		forward_nvg_fill_callback(renderItem);
	}
}

static void forwardnvg_renderDelete(void* uptr) {

	std::cout << "forwardnvg_renderDelete" << std::endl;
}

NVGcontext* nvgCreateForward(int flags)
{
	NVGparams params;
	NVGcontext* ctx = NULL;
	ForwardNVGcontext* gl = new ForwardNVGcontext;

	memset(&params, 0, sizeof(params));
	params.renderCreate = forwardnvg_renderCreate;
	params.renderCreateTexture = forwardnvg_renderCreateTexture;
	params.renderDeleteTexture = forwardnvg_renderDeleteTexture;
	params.renderUpdateTexture = forwardnvg_renderUpdateTexture;
	params.renderGetTextureSize = forwardnvg_renderGetTextureSize;
	params.renderViewport = forwardnvg_renderViewport;
	params.renderCancel = forwardnvg_renderCancel;
	params.renderFlush = forwardnvg_renderFlush;
	params.renderFill = forwardnvg_renderFill;
	params.renderStroke = forwardnvg_renderStroke;
	params.renderTriangles = forwardnvg_renderTriangles;
	params.renderDelete = forwardnvg_renderDelete;
	params.userPtr = gl;
	params.edgeAntiAlias = flags & NVG_ANTIALIAS ? 1 : 0;

	ctx = nvgCreateInternal(&params);
	if (ctx == NULL) goto error;

	return ctx;

error:
	// 'gl' is freed by nvgDeleteInternal.
	if (ctx != NULL) nvgDeleteInternal(ctx);
	return NULL;
}


void nvgDeleteForward(NVGcontext* ctx)
{
	nvgDeleteInternal(ctx);
}

#endif