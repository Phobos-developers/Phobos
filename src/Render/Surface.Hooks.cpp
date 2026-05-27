#include "Surface.h"

#include <Utilities/Macro.h>

DEFINE_FUNCTION_JUMP(LJMP, 0x4BA770, DXSurface::CreatePrimary);

DEFINE_JUMP(LJMP, 0x77747A, 0x777575); // Skip Restore_Check.

static bool __fastcall DSurface_IsSurfaceLost(DSurface*, void*) // Always return false to prevent surface lost state
{
	return false;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x4BAFE0, DSurface_IsSurfaceLost);

static DXSurface* __fastcall DXSurfaceCtor(DXSurface* pSurface, void*, int width, int height, bool systemMem, bool enable3D) {
	return new(pSurface) DXSurface(width, height);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x4BA5A0, DXSurfaceCtor);

static int __stdcall BinkDDSurfaceType(void*) {
	return 10; // BINKSURFACE565
}
DEFINE_PATCH_TYPED(void*, 0x7E15A8, BinkDDSurfaceType);
