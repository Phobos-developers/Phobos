#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

#include <HouseClass.h>
#include <ScenarioClass.h>
#include <ParticleSystemClass.h>

/*
DEFINE_HOOK(0x471A8D, CaptureManagerClass_Overload_Count_A, 0x6)
{
	GET(CaptureManagerClass*, pThis, ESI);
	GET(int, nControlNodeCount, EDX);
	R->ECX(CaptureExt::ExtMap.Find(pThis)->OverloadCount.Items);
	return 0x471A93;
}

// replacing whole loop
// using outside vector causing item-1 , which i dunno why
DEFINE_HOOK(0x471A82, CaptureManagerClass_Overload_Count,0x6)
{
	GET(CaptureManagerClass*, pThis, ESI);

	auto const &OverloadCount = CaptureExt::ExtMap.Find(pThis)->OverloadCount;
	int nCurIdx = 0;
	int nOvcIdx = 1;

	if (pThis->ControlNodes.Count > OverloadCount[0])
	{
		do
		{
			if (nCurIdx >= (OverloadCount.Count - 1) || nOvcIdx >= (OverloadCount.Count - 1))
				break;

			++nCurIdx;
		} while (pThis->ControlNodes.Count > OverloadCount[nOvcIdx++]);
	}

	R->EAX(nCurIdx);
	return 0x471AB1;
}


DEFINE_HOOK(0x471A9B, CeptureManagerClass_Overload_Count_B, 0x6)
{
	GET(CaptureManagerClass*, pThis, ESI);
	R->EDI((CaptureExt::ExtMap.Find(pThis)->OverloadCount.Count - 1));
	return 0x471AA2;
}

DEFINE_HOOK(0x471AB5, CeptureManagerClass_Overload_Frames, 0x6)
{
	GET(RulesClass*, pRules, EBX);
	GET(CaptureManagerClass*, pThis, ESI);
	GET(int, nIdx, EAX);
	auto const pManagerOwnerExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	R->EDX(pManagerOwnerExt->Overload_Frames.GetElements(pRules->OverloadFrames)[nIdx]);
	return 0x471ABE;
}

DEFINE_HOOK(0x471AC7, CeptureManagerClass_Overload_Damage, 0x6)
{
	GET(RulesClass*, pRules, ECX);
	GET(CaptureManagerClass*, pThis, ESI);
	GET(int, nIdx, EAX);
	auto const pManagerOwnerExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	R->EAX(pManagerOwnerExt->Overload_Damage.GetElements(pRules->OverloadDamage)[nIdx]);
	return 0x471AD0;
}

DEFINE_HOOK(0x471B39, CaptureManagerClass_Overload_DeathSound, 0x6)
{
	GET(RulesClass*, pRules, ECX);
	GET(CaptureManagerClass*, pThis, ESI);
	auto const pManagerOwnerExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	auto const nSound = pManagerOwnerExt->Overload_DeathSound.Get(pRules->MasterMindOverloadDeathSound);
	R->ECX(nSound);
	return 0x471B3F;
}

DEFINE_HOOK(0x471C0C, CaptureManagerClass_Overload_ParticleSys, 0x6)
{
	GET(RulesClass*, pRules, EDX);
	GET(CaptureManagerClass*, pThis, ESI);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	R->ECX(pExt->Overload_ParticleSys.Get(pRules->DefaultSparkSystem));
	return 0x471C12;
}*/

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


void __fastcall CaptureManagerClass_Overload_AI(CaptureManagerClass *pThis , void*_)
{

	if (pThis->InfiniteMindControl)
	{
		auto const pOwner = pThis->Owner;
		auto const pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
		auto const pRules = RulesClass::Instance();

		if (pThis->OverloadPipState > 0)
			--pThis->OverloadPipState;

		if (pThis->OverloadDamageDelay <= 0)
		{
			auto const &OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(pRules->OverloadCount);
			int nCurIdx = 0;
			int const nNodeCount = pThis->ControlNodes.Count;

			if (nNodeCount > OverloadCount[0])
			{
				for (int i = 1; nNodeCount > OverloadCount[i]; ++i)
				{
					if (nCurIdx >= (int)(OverloadCount.size() - 1) || i >= (int)(OverloadCount.size() - 1))
						break;

					++nCurIdx;
				}
			}

			pThis->OverloadDamageDelay = pOwnerTypeExt->Overload_Frames.GetElements(pRules->OverloadFrames)[nCurIdx];
			auto nDamage = pOwnerTypeExt->Overload_Damage.GetElements(pRules->OverloadDamage)[nCurIdx];

			if (nDamage <= 0)
			{
				pThis->OverloadDeathSoundPlayed = false;
			}
			else
			{
				pThis->OverloadPipState = 10;
				pOwner->ReceiveDamage(&nDamage, 0, pRules->C4Warhead, 0, 0, 0, 0);

				if (!pThis->OverloadDeathSoundPlayed)
				{
					VocClass::PlayAt(pOwnerTypeExt->Overload_DeathSound.Get(pRules->MasterMindOverloadDeathSound), pOwner->Location, 0);
					pThis->OverloadDeathSoundPlayed = true;
				}

				for (int i = pOwnerTypeExt->Overload_ParticleSysCount.Get(5); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					CoordStruct nParticleCoord{ pOwner->Location.X + nRamdomX, nRandomY + pOwner->Location.Y, pOwner->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pOwnerTypeExt->Overload_ParticleSys.Get(pRules->DefaultSparkSystem), nParticleCoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
				}

				if (nCurIdx > 0 && pOwner->IsAlive)
				{
					double const nBase = (nCurIdx != 1) ?  0.015 : 0.029999999;
					double const nCopied_base = (ScenarioClass::Instance->Random.RandomRanged(0, 100) < 50) ? -nBase : nBase;
					pOwner->RockingSidewaysPerFrame = (float)nCopied_base;
				}
			}
		}
		else
		{
			--pThis->OverloadDamageDelay;
		}
	}
}

DEFINE_POINTER_CALL(0x6FA730, &CaptureManagerClass_Overload_AI);