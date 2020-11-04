#include <Helpers/Macro.h>
#include <AnimClass.h>
#include <TechnoClass.h>
#include <FootClass.h>

//Replace: checking of HasExtras = > checking of (HasExtras && Shadow)
DEFINE_HOOK(423365, replace_shp_shadow_check, 8)
{
	GET(AnimClass*, anim, ESI);
	return (anim->Type->Shadow && anim->HasExtras) ?
		0x42336D :
		0x4233EE;
}
