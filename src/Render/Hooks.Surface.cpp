#include "Surface.h"

#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

DEFINE_FUNCTION_JUMP(LJMP, 0x4BA770, DXSurface::CreatePrimary);

DEFINE_JUMP(LJMP, 0x77747A, 0x777575); // Skip Restore_Check

static DXSurface* __fastcall _DXSurface_CTOR(DXSurface* surface, void*, int width, int height, bool system_mem, bool enable_3d) {
	return new(surface) DXSurface(width, height);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x4BA5A0, _DXSurface_CTOR);

