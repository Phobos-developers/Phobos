#include <Utilities/Macro.h>
#include <AnimClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
//Replace: checking of HasExtras = > checking of (HasExtras && Shadow)
DEFINE_HOOK(423365, Phobos_BugFixes_SHPShadowCheck, 8)
{
	GET(AnimClass*, pAnim, ESI);
	return (pAnim->Type->Shadow && pAnim->HasExtras) ?
		0x42336D :
		0x4233EE;
}

/* 
	Allow usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable

	When TileSet number crosses 255 in theater INI files, the NE-SW broken bridges
	become non-repairable. The reason is that the NonMarbleMadnessTile array of size 256
	overflows when filled and affects the variables like BridgeTopRight1 and BridgeBottomLeft2
	that come after it. This patch removes the filling of the unused NonMarbleMadnessTile array
	and its references.

	Author : E1 Elite
*/
DEFINE_LJMP(0x545CE2, 0x545CE9) //Phobos_BugFixes_Tileset255_RemoveNonMMArrayFill
DEFINE_LJMP(0x546C23, 0x546C8B) //Phobos_BugFixes_Tileset255_RefNonMMArray


// WWP's shit code! Wrong check.
// To avoid units dying when they are already dead.
DEFINE_HOOK(5F53AA, ObjectClass_ReceiveDamage_DyingFix, 6)
{
	GET(int, health, EAX);
	GET(ObjectClass*, pThis, ESI);

	if (health <= 0 || !pThis->IsAlive)
		return 0x5F583E; // return DamageState::PostMortem

	return 0x5F53B0; //continue vanilla check
}

DEFINE_HOOK(4D7431, FootClass_ReceiveDamage_DyingFix, 5)
{
	GET(FootClass*, pThis, ESI);
	GET(DamageState, result, EAX);

	if (result != DamageState::PostMortem && (pThis->IsSinking /*|| pThis->IsCrashing*/))
		R->EAX(DamageState::PostMortem);

	return 0;
}

DEFINE_HOOK(737D57, UnitClass_ReceiveDamage_DyingFix, 7)
{
	GET(UnitClass*, pThis, ESI);
	GET(DamageState, result, EAX);

	if (result != DamageState::PostMortem && pThis->DeathFrameCounter > 0)
		R->EAX(DamageState::PostMortem);

	return 0;
}
