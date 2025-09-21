#include "Body.h"
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/AresHelper.h>
#include <Helpers/Macro.h>
#include <Misc/FogOfWar.h>

#define FOW_DEBUG 0


namespace BoltTemp
{
	EBoltExt::ExtData* ExtData = nullptr;
	int Color[3];
	bool FogHidden = false; // Flag to track if current bolt should be hidden due to fog
}

// Skip create particlesystem in EBolt::Fire
// We will create after this function
DEFINE_JUMP(LJMP, 0x4C2AFF, 0x4C2B0E)
DEFINE_JUMP(LJMP, 0x4C2B0F, 0x4C2B35)

DEFINE_HOOK(0x6FD494, TechnoClass_FireEBolt_SetExtMap_AfterAres, 0x7)
{
	GET(TechnoClass*, pThis, EDI);
	GET(EBolt*, pBolt, EAX);
	const auto pBoltExt = EBoltExt::ExtMap.Find(pBolt);
	pBoltExt->BurstIndex = pThis->CurrentBurstIndex;

	return 0;
}

DEFINE_HOOK(0x6FD55F, TechnoClass_FireEBolt_ParticleSystem, 0x5)
{
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x30, 0x8));
	GET_STACK(EBolt*, pBolt, STACK_OFFSET(0x30, -0x20));

	if (const auto particle = WeaponTypeExt::ExtMap.Find(pWeapon)->Bolt_ParticleSystem.Get(RulesClass::Instance->DefaultSparkSystem))
		GameCreate<ParticleSystemClass>(particle, pBolt->Point2, nullptr, nullptr, CoordStruct::Empty, nullptr);

	return 0;
}

DWORD _cdecl EBoltExt::_EBolt_Draw_Colors(REGISTERS* R)
{
	enum { SkipGameCode = 0x4C1F66 };


	GET(EBolt*, pThis, ECX);
	if (!pThis || !*(void**)pThis) return SkipGameCode;

	auto* pExt = EBoltExt::ExtMap.Find(pThis);
	if (!pExt) return SkipGameCode;

	BoltTemp::ExtData = pExt;
	BoltTemp::FogHidden = false;

	// DESYNC SAFETY: Only apply fog gating for local player's view
	// This is purely visual and must not affect game simulation or synchronized state
	if (ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar) {
		if (auto* me = HouseClass::CurrentPlayer; me && !me->SpySatActive) {
			// Owner-only fog gate: if you can see the shooter, you can see their bolt
			if (pThis->Owner) {
				// Convert to cell → back to the cell center; clamp Z to 0 so height can't bite us
				CoordStruct ownerWorld = pThis->Owner->GetCoords();
				const CellStruct ownerCell = CellClass::Coord2Cell(ownerWorld);
				ownerWorld = CellClass::Cell2Coord(ownerCell);
				ownerWorld.Z = 0;

				#if FOW_DEBUG
				static int debugCount = 0;
				if (debugCount++ < 20) {
					CoordStruct originalOwner = pThis->Owner->GetCoords();
					Debug::Log("[FOW] EBolt coord compare: original(%d,%d,%d) cell(%d,%d) cellCenter(%d,%d,0) fogged=%d\n",
						originalOwner.X, originalOwner.Y, originalOwner.Z,
						ownerCell.X, ownerCell.Y, ownerWorld.X, ownerWorld.Y,
						Fog::IsFogged(ownerWorld) ? 1 : 0);

					// Also test the original coordinates for comparison
					Debug::Log("[FOW] Original coord fogged=%d, cell center fogged=%d\n",
						Fog::IsFogged(originalOwner) ? 1 : 0, Fog::IsFogged(ownerWorld) ? 1 : 0);
				}
				#endif

				bool foggedByCell = false;

				// Try direct cell fog check
				auto* cellObj = MapClass::Instance.GetCellAt(ownerCell);
				if (cellObj) {
					foggedByCell = cellObj->IsFogged();
				}

				#if FOW_DEBUG
				if (debugCount <= 20) {
					Debug::Log("[FOW] Fog methods: coord=%d cell=%d\n", foggedByCoord ? 1 : 0, foggedByCell ? 1 : 0);
				}
				#endif

				// Use cell-based method since coord-based is giving wrong results
				if (foggedByCell) {
					BoltTemp::FogHidden = true;
				}
			}
		}
	}

	// Always populate colors; some code expects them even if we end up hidden
	const auto& color = pExt->Color;
	for (int i = 0; i < 3; ++i)
		BoltTemp::Color[i] = Drawing::RGB_To_Int(color[i]);

	return SkipGameCode;
}

DEFINE_HOOK(0x4C1F33, EBolt_Draw_Colors, 0x7)
{
	return EBoltExt::_EBolt_Draw_Colors(R);
}

DEFINE_HOOK(0x4C20BC, EBolt_DrawArcs, 0xB)
{
	enum { DoLoop = 0x4C20C7, Break = 0x4C2400 };


	GET_STACK(int, plotIndex, STACK_OFFSET(0x408, -0x3E0));
	const int arcCount = BoltTemp::ExtData->Arcs;

	return plotIndex < arcCount ? DoLoop : Break;
}

DEFINE_JUMP(LJMP, 0x4C24BE, 0x4C24C3)// Disable Ares's hook EBolt_Draw_Color1
DEFINE_HOOK(0x4C24C3, EBolt_DrawFirst_Color, 0x9)
{

	#if FOW_DEBUG
	static int c=0;
	if (c++ < 5) {
		Debug::Log("[FOW] EBolt_DrawFirst_Color: FogHidden=%d Disable[0]=%d\n",
			BoltTemp::FogHidden ? 1 : 0, BoltTemp::ExtData->Disable[0] ? 1 : 0);
	}
	#endif

	if (BoltTemp::FogHidden || BoltTemp::ExtData->Disable[0])
		return 0x4C2515;

	R->EAX(BoltTemp::Color[0]);
	return 0x4C24E4;
}

DEFINE_JUMP(LJMP, 0x4C25CB, 0x4C25D0)// Disable Ares's hook EBolt_Draw_Color2
DEFINE_HOOK(0x4C25D0, EBolt_DrawSecond_Color, 0x6)
{
	if (BoltTemp::FogHidden || BoltTemp::ExtData->Disable[1])
		return 0x4C262A;

	R->Stack(STACK_OFFSET(0x424, -0x40C), BoltTemp::Color[1]);
	return 0x4C25FD;
}

DEFINE_JUMP(LJMP, 0x4C26CF, 0x4C26D5)// Disable Ares's hook EBolt_Draw_Color3
DEFINE_HOOK(0x4C26D5, EBolt_DrawThird_Color, 0x6)
{
	if (BoltTemp::FogHidden || BoltTemp::ExtData->Disable[2])
		return 0x4C2710;

	R->EAX(BoltTemp::Color[2]);
	return 0x4C26EE;
}

#pragma region EBoltTrackingFixes

class EBoltFake final : public EBolt
{
public:
	void _SetOwner(TechnoClass* pTechno, int weaponIndex);
	void _RemoveFromOwner();
};

void EBoltFake::_SetOwner(TechnoClass* pTechno, int weaponIndex)
{
	if (pTechno && pTechno->IsAlive)
	{
		auto const pWeapon = pTechno->GetWeapon(weaponIndex)->WeaponType;
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if (!pWeaponExt->Bolt_FollowFLH.Get(pTechno->WhatAmI() == AbstractType::Unit))
			return;

		this->Owner = pTechno;
		this->WeaponSlot = weaponIndex;

		auto const pExt = TechnoExt::ExtMap.Find(pTechno);
		pExt->ElectricBolts.push_back(this);
	}
}

void EBoltFake::_RemoveFromOwner()
{
	auto const pExt = TechnoExt::ExtMap.Find(this->Owner);
	auto& vec = pExt->ElectricBolts;
	vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	this->Owner = nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x4C2BD0, EBoltFake::_SetOwner); // Failsafe in case called in another module

DEFINE_HOOK(0x6FD5D6, TechnoClass_InitEBolt, 0x6)
{
	enum { SkipGameCode = 0x6FD60B };

	GET(TechnoClass*, pThis, ESI);
	GET(EBolt*, pBolt, EAX);
	GET(const int, weaponIndex, EBX);

	if (pBolt)
		((EBoltFake*)pBolt)->_SetOwner(pThis, weaponIndex);

	return SkipGameCode;
}

DEFINE_HOOK(0x4C285D, EBolt_DrawAll_BurstIndex, 0x5)
{
	enum { SkipGameCode = 0x4C2882 };

	GET(TechnoClass*, pTechno, ECX);
	GET_STACK(EBolt*, pThis, STACK_OFFSET(0x34, -0x24));

	int burstIndex = pTechno->CurrentBurstIndex;
	pTechno->CurrentBurstIndex = EBoltExt::ExtMap.Find(pThis)->BurstIndex;
	auto const fireCoords = pTechno->GetFLH(pThis->WeaponSlot, CoordStruct::Empty);
	pTechno->CurrentBurstIndex = burstIndex;
	R->EAX(&fireCoords);

	return SkipGameCode;
}

DEFINE_HOOK(0x4C299F, EBolt_DrawAll_EndOfLife, 0x6)
{
	enum { SkipGameCode = 0x4C29B9 };

	GET(EBolt*, pThis, EAX);

	if (pThis->Owner)
		((EBoltFake*)pThis)->_RemoveFromOwner();

	return SkipGameCode;
}

DEFINE_HOOK(0x4C2A02, EBolt_DestroyVector, 0x6)
{
	enum { SkipGameCode = 0x4C2A08 };

	GET(EBolt*, pThis, EAX);

	((EBoltFake*)pThis)->_RemoveFromOwner();

	return SkipGameCode;
}
#pragma endregion
