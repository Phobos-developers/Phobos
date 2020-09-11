#include <Helpers/Macro.h>
#include <Surface.h>

DEFINE_HOOK(6B9D9C, RGB_PCX_Loader, 7)
{
	GET(BSurface*, pSurf, EDI);
	if (pSurf->BytesPerPixel == 2) {
		return 0x6B9EE7;
	}
	return 0;
}
