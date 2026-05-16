#include "Surface.h"

#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

DEFINE_FUNCTION_JUMP(LJMP, 0x4BA770, DXSurface::CreatePrimary);

DEFINE_JUMP(LJMP, 0x77747A, 0x777575); // Skip Restore_Check

static DXSurface* __fastcall _DXSurface_CTOR(DXSurface* surface, void*, int width, int height, bool system_mem, bool enable_3d) {
	return new(surface) DXSurface(width, height);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x4BA5A0, _DXSurface_CTOR);

static int __stdcall _BinkDDSurfaceType(void*)
{
	return 10; // BINKSURFACE565
}
DEFINE_PATCH_TYPED(void*, 0x7E15A8, _BinkDDSurfaceType);

static int __stdcall _BinkCopyToBuffer(void* bnk, void* dest, int destpitch, unsigned int destheight, unsigned int destx, unsigned int desty, unsigned int flags)
{
	// Skip the bink movie render for now to avoid the crash. The movie will be rendered as black screen, but at least it won't crash.
	// So we can test the other features without worrying about the movie rendering. We will try to implement the movie rendering later.
	return 0;
}
DEFINE_PATCH_TYPED(void*, 0x7E15B8, _BinkCopyToBuffer);
