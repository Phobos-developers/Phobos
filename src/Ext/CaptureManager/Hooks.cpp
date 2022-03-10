#include "Body.h"
#include <Ext/TechnoType/Body.h>

#include <HouseClass.h>

DEFINE_HOOK(0x471A8D, CaptureManagerClass_Overload_Count_A, 0x6)
{
	GET(CaptureManagerClass*, pThis, ESI);
	R->ECX(CaptureExt::ExtMap.Find(pThis)->OverloadCount.Items);
	return 0x471A93;
}

DEFINE_HOOK(0x471A9B, CeptureManagerClass_Overload_Count_B, 0x6)
{
	GET(CaptureManagerClass*, pThis, ESI);
	R->EDI((CaptureExt::ExtMap.Find(pThis)->OverloadCount.Count - 1));
	return 0x471AA2;
}

DEFINE_HOOK(0x471AB5, CeptureManagerClass_Overload_Frames, 0x6)
{
	//GET(RulesClass*, pRules, EBX);
	GET(CaptureManagerClass*, pThis, ESI);
	GET(int, nIdx, EAX);
	R->EDX(CaptureExt::ExtMap.Find(pThis)->OverloadFrames[nIdx]);
	return 0x471ABE;
}

DEFINE_HOOK(0x471AC7, CeptureManagerClass_Overload_Damage, 0x6)
{
	//GET(RulesClass*, pRules, ECX);
	GET(CaptureManagerClass*, pThis, ESI);
	GET(int, nIdx, EAX);
	R->EAX(CaptureExt::ExtMap.Find(pThis)->OverloadDamage[nIdx]);
	return 0x471AD0;
}

DEFINE_HOOK(0x471B39, CaptureManagerClass_Overload_DeathSound, 0x6)
{
	//GET(RulesClass*, pRules, ECX);
	GET(CaptureManagerClass*, pThis, ESI);
	R->ECX(CaptureExt::ExtMap.Find(pThis)->OverloadDeathSound);
	return 0x471B3F;
}

DEFINE_HOOK(0x471C0C, CaptureManagerClass_Overload_ParticleSys, 0x6)
{
	GET(RulesClass*, pRules, EDX);
	GET(CaptureManagerClass*, pThis, ESI);

	auto const& pExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	R->ECX(pExt->Overload_ParticleSys.Get(pRules->DefaultSparkSystem));

	return 0x471C12;
}

static void __stdcall DrawALinkTo(CoordStruct nFrom, CoordStruct nTo, ColorStruct color)
{
	JMP_STD(0x704E40);
}

DEFINE_HOOK(0x4721E6, CaptureManagerClass_DrawLinkToVictim, 0x6)
{
	GET(CaptureManagerClass*, pThis, EDI);
	GET(TechnoClass*, pVictim, ECX);
	GET_STACK(int, nNodeCount, STACK_OFFS(0x30, 0x1C));

	auto const pAttacker = pThis->Owner;
	const auto pExt = TechnoTypeExt::ExtMap.Find(pAttacker->GetTechnoType());

	if (pExt->Draw_MindControlLink.Get())
	{
		auto nVictimCoord = pVictim->Location;
		nVictimCoord.Z += pVictim->GetTechnoType()->LeptonMindControlOffset;
		auto nFLH = pAttacker->GetFLH(-1 - nNodeCount % 5, CoordStruct::Empty);
		DrawALinkTo(nFLH, nVictimCoord, pAttacker->Owner->Color);
	}

	R->EBP(nNodeCount);
	return 0x472287;
}