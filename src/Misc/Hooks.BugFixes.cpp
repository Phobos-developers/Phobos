#include <Helpers/Macro.h>
#include <AnimClass.h>
#include <TechnoClass.h>
#include <FootClass.h>

//Replace: checking of HasExtras = > checking of (HasExtras && Shadow)
DEFINE_HOOK(423365, Phobos_BugFixes_SHPShadowCheck, 8)
{
	GET(AnimClass*, pAnim, ESI);
	return (pAnim->Type->Shadow && pAnim->HasExtras) ?
		0x42336D :
		0x4233EE;
}
