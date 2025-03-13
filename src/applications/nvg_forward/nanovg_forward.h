#pragma once

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

	//ForwardNVGcontext* gl = (ForwardNVGcontext*)uptr;

	//auto fromFan2List = [](std::vector<int>& vIndex, int i) {
	//	if (i < 2)
	//		return;
	//	vIndex.push_back(0);
	//	vIndex.push_back(i - 1);
	//	vIndex.push_back(i);
	//	};
	//auto fromStrip2List = [](std::vector<int>& vIndex, int i) {
	//	if (i < 2)
	//		return;
	//	vIndex.push_back(i - 2);
	//	vIndex.push_back(i - 1);
	//	vIndex.push_back(i);
	//	};

	//auto drawFills = [&](NVGvertex* fills, int fillCount, auto toListFunc) {
	//	if (fillCount > 0) {
	//		if (fillCount >= 3) {
	//			std::vector<SDL_Vertex> vertex(fillCount);
	//			std::vector<int> vIndex;
	//			for (int i = 0; i < fillCount; ++i) {
	//				vertex[i].position = { fills[i].x, fills[i].y };
	//				vertex[i].color = FromFloatColor2IntColor<SDL_Color>(paint->innerColor);
	//				vertex[i].tex_coord = { fills[i].u, fills[i].v };
	//				toListFunc(vIndex, i);
	//			}
	//			SDL_RenderGeometry(gl->render, NULL, vertex.data(), vertex.size(), vIndex.data(), vIndex.size());
	//		}
	//		else {
	//			assert(false);
	//		}
	//	}
	//	};

	//for (int i = 0; i < npaths; ++i) {
	//	auto& path = paths[i];
	//	drawFills(path.fill, path.nfill, fromFan2List);
	//	drawFills(path.stroke, path.nstroke, fromStrip2List);
	//}
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