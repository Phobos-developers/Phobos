#include "Body.h"

#include <ParticleSystemClass.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x471D40, CaptureManagerClass_CaptureUnit, 0x7)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureManagerExt::CaptureUnit(pThis, pTechno));

	return 0x471D5A;
}

DEFINE_HOOK(0x471FF0, CaptureManagerClass_FreeUnit, 0x8)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureManagerExt::FreeUnit(pThis, pTechno));

	return 0x472006;
}

DEFINE_HOOK(0x6FCB34, TechnoClass_CanFire_CanCapture, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);

	R->AL(CaptureManagerExt::CanCapture(pThis->CaptureManager, pTarget));

	return 0x6FCB40;
}

DEFINE_HOOK(0x519F71, InfantryClass_UpdatePosition_BeforeBuildingChangeHouse, 0x6)
{
	GET(BuildingClass*, pBld, EDI);

	if (auto pBy = pBld->MindControlledBy)
		CaptureManagerExt::FreeUnit(pBy->CaptureManager, pBld);

	if (std::exchange(pBld->MindControlledByAUnit, false))
	{
		if (auto& pAnim = pBld->MindControlRingAnim)
		{
			pAnim->SetOwnerObject(nullptr);
			pAnim->UnInit();
			pAnim = nullptr;
		}
	}

	return 0;
}

static void __stdcall DrawALinkTo(CoordStruct nFrom, CoordStruct nTo, ColorStruct color)
{
	JMP_STD(0x704E40);
}

DEFINE_HOOK(0x4721E6, CaptureManagerClass_DrawLinkToVictim, 0x6)
{
	GET(CaptureManagerClass*, pThis, EDI);
	GET(TechnoClass*, pVictim, ECX);
	GET_STACK(int, nNodeCount, STACK_OFFSET(0x30, 0x1C));

	auto const pAttacker = pThis->Owner;
	const auto pExt = TechnoTypeExt::ExtMap.Find(pAttacker->GetTechnoType());

	if (pExt->MindControlDrawLink)
	{
		auto nVictimCoord = pVictim->Location;
		nVictimCoord.Z += pVictim->GetTechnoType()->LeptonMindControlOffset;
		auto nFLH = pAttacker->GetFLH(-1 - nNodeCount % 5, CoordStruct::Empty);
		DrawALinkTo(nFLH, nVictimCoord, pAttacker->Owner->Color);
	}

	R->EBP(nNodeCount);
	return 0x472287;
}

void __fastcall CaptureManagerClass_Overload_AI(CaptureManagerClass* pThis, void* _)
{
	auto const pOwner = pThis->Owner;
	auto const pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

	if (!pOwnerTypeExt) // we cant find type Ext for this , just return to original function !
	{
		pThis->HandleOverload();
		return;
	}

	if (pThis->InfiniteMindControl)
	{
		auto const pRules = RulesClass::Instance;

		if (pThis->OverloadPipState > 0)
			--pThis->OverloadPipState;

		if (pThis->OverloadDamageDelay <= 0)
		{
			auto const& OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(pRules->OverloadCount);

			if (OverloadCount.empty())
				return;

			int nCurIdx = 0;
			int const nNodeCount = pThis->ControlNodes.Count;

			for (int i = 0; i < (int)(OverloadCount.size()); ++i)
			{
				if(nNodeCount > OverloadCount[i])
					nCurIdx = i + 1; //select the index !
			}

			// prevent nCurIdx selecting out of bound index !
			constexpr auto FixIdx = [](const Iterator<int>& iter, int nInput) {
				return iter.empty() ? 0 : iter[nInput > (int)iter.size() ? (int)iter.size() : nInput];
			};

			auto const& nOverloadfr = pOwnerTypeExt->Overload_Frames.GetElements(pRules->OverloadFrames);
			pThis->OverloadDamageDelay = FixIdx(nOverloadfr, nCurIdx);

			auto const& nOverloadDmg = pOwnerTypeExt->Overload_Damage.GetElements(pRules->OverloadDamage);
			auto nDamage = FixIdx(nOverloadDmg, nCurIdx);

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

				if (auto const pParticle = pOwnerTypeExt->Overload_ParticleSys.Get(pRules->DefaultSparkSystem))
				{
					for (int i = pOwnerTypeExt->Overload_ParticleSysCount; i > 0; --i)
					{
						auto const nRandomY = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
						auto const nRamdomX = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
						CoordStruct nParticleCoord{ pOwner->Location.X + nRamdomX, nRandomY + pOwner->Location.Y, pOwner->Location.Z + 100 };
						GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
					}
				}

				if (nCurIdx > 0 && pOwner->IsAlive && pOwner->Health > 0 && !pOwner->InLimbo)
				{
					double const nBase = (nCurIdx != 1) ? 0.015 : 0.029999999;
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

DEFINE_FUNCTION_JUMP(CALL, 0x6FA730, CaptureManagerClass_Overload_AI);
