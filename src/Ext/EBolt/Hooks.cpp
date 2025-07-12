#include "Body.h"
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/AresHelper.h>
#include <Helpers/Macro.h>

namespace BoltTemp
{
	EBoltExt::ExtData* ExtData = nullptr;
	int Color[3];
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
	const auto pExt = BoltTemp::ExtData = EBoltExt::ExtMap.Find(pThis);
	const auto& color = pExt->Color;

	for (int idx = 0; idx < 3; ++idx)
		BoltTemp::Color[idx] = Drawing::RGB_To_Int(color[idx]);

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
	if (BoltTemp::ExtData->Disable[0])
		return 0x4C2515;

	R->EAX(BoltTemp::Color[0]);
	return 0x4C24E4;
}

DEFINE_JUMP(LJMP, 0x4C25CB, 0x4C25D0)// Disable Ares's hook EBolt_Draw_Color2
DEFINE_HOOK(0x4C25D0, EBolt_DrawSecond_Color, 0x6)
{
	if (BoltTemp::ExtData->Disable[1])
		return 0x4C262A;

	R->Stack(STACK_OFFSET(0x424, -0x40C), BoltTemp::Color[1]);
	return 0x4C25FD;
}

DEFINE_JUMP(LJMP, 0x4C26CF, 0x4C26D5)// Disable Ares's hook EBolt_Draw_Color3
DEFINE_HOOK(0x4C26D5, EBolt_DrawThird_Color, 0x6)
{
	if (BoltTemp::ExtData->Disable[2])
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
