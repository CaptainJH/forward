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

std::function<void(std::vector<nvg_vertex>& v, std::vector<int>& idx, D3DNVGfragUniforms& fragUniform)> forward_nvg_fill_callback = nullptr;

#if defined NANOVG_FORWARD_IMPLEMENTATION

struct ForwardNVGcontext {

	float viewport[2];

	struct TextureInfo {

		int nvgFormat;
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

static int D3Dnvg__convertPaint(struct D3DNVGcontext* D3D, struct D3DNVGfragUniforms* frag,
	struct NVGpaint* paint, struct NVGscissor* scissor,
	float width, float fringe, float strokeThr)
{
	struct D3DNVGtexture* tex = NULL;
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

	assert(paint->image == 0);
//	if (paint->image != 0)
//	{
//		tex = D3Dnvg__findTexture(D3D, paint->image);
//		if (tex == NULL)
//		{
//			return 0;
//		}
//
//		if ((tex->flags & NVG_IMAGE_FLIPY) != 0)
//		{
//			float m1[6], m2[6];
//			nvgTransformTranslate(m1, 0.0f, frag->extent[1] * 0.5f);
//			nvgTransformMultiply(m1, paint->xform);
//			nvgTransformScale(m2, 1.0f, -1.0f);
//			nvgTransformMultiply(m2, m1);
//			nvgTransformTranslate(m1, 0.0f, -frag->extent[1] * 0.5f);
//			nvgTransformMultiply(m1, m2);
//			nvgTransformInverse(invxform, m1);
//		}
//		else
//		{
//			nvgTransformInverse(invxform, paint->xform);
//		}
//		frag->type = NSVG_SHADER_FILLIMG;
//
//#if NANOVG_GL_USE_UNIFORMBUFFER
//		if (tex->type == NVG_TEXTURE_RGBA)
//		{
//			frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0 : 1;
//		}
//		else
//		{
//			frag->texType = 2;
//		}
//#else
//		if (tex->type == NVG_TEXTURE_RGBA)
//			frag->texType = (tex->flags & NVG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
//		else
//			frag->texType = 2.0f;
//#endif
//	}
//	else
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
	static int c = 1;
	return c++;
}

static int forwardnvg_renderDeleteTexture(void* uptr, int image) {

	std::cout << "forwardnvg_renderDeleteTexture" << std::endl;

	return 0;
}

static int forwardnvg_renderUpdateTexture(void* uptr, int image, int x, int y, int w, int h, const unsigned char* data) {


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

	auto drawFills = [&](NVGvertex* fills, int fillCount, auto toListFunc) {
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

				D3DNVGfragUniforms fragUniform;
				memset(&fragUniform, 0, sizeof(fragUniform));
				fragUniform.strokeMult[1] = -1.0f;
				fragUniform.type = NSVG_SHADER_SIMPLE;
				// Fill shader
				D3Dnvg__convertPaint(nullptr, &fragUniform, paint, scissor, fringe, fringe, -1.0f);
				if (forward_nvg_fill_callback)
					forward_nvg_fill_callback(vertex, vIndex, fragUniform);
			}
			else {
				assert(false);
			}
		}
		};

	for (int i = 0; i < npaths; ++i) {
		auto& path = paths[i];
		drawFills(path.fill, path.nfill, fromFan2List);
		drawFills(path.stroke, path.nstroke, fromStrip2List);
	}
}

static void forwardnvg_renderStroke(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
	float strokeWidth, const NVGpath* paths, int npaths) {

	//std::cout << "forwardnvg_renderStroke" << std::endl;
}

static void forwardnvg_renderTriangles(void* uptr, NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor,
	const NVGvertex* verts, int nverts, float fringe) {

	//std::cout << "forwardnvg_renderTriangles" << std::endl;
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