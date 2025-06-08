#include "Body.h"
#include <ParticleSystemClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/AresHelper.h>
#include <Helpers/Macro.h>

int* EBoltExt::AresBoltColor1 = nullptr;
int* EBoltExt::AresBoltColor2 = nullptr;
int* EBoltExt::AresBoltColor3 = nullptr;

namespace BoltTemp
{
	EBoltExt::ExtData* ExtData = nullptr;
	int Color1;
	int Color2;
	int Color3;
}

// Skip create particlesystem in EBolt::Fire
// We will create after this function
DEFINE_JUMP(LJMP, 0x4C2AFF, 0x4C2B0E)
DEFINE_JUMP(LJMP, 0x4C2B0F, 0x4C2B35)

DWORD _cdecl EBoltExt::_TechnoClass_FireEBolt(REGISTERS* R)
{
	enum { SkipGameCode = 0x6FD480 };

	GET(TechnoClass*, pThis, EDI);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x30, 0x8));

	const auto pBolt = GameCreate<EBolt>();
	const auto pBoltExt = EBoltExt::ExtMap.Allocate(pBolt);

	const int alternateIdx = pWeapon->IsAlternateColor ? 5 : 10;
	const COLORREF defaultAlternate = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, alternateIdx);
	const COLORREF defaultWhite = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, 15);
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	for (int idx = 0; idx < 3; ++idx)
	{
		if (pWeaponExt->Bolt_Disable[idx])
			pBoltExt->Disable[idx] = true;
		else if (pWeaponExt->Bolt_Color[idx].isset())
			pBoltExt->Color[idx] = pWeaponExt->Bolt_Color[idx].Get();
		else
			pBoltExt->Color[idx] = Drawing::Int_To_RGB(idx < 2 ? defaultAlternate : defaultWhite);
	}

	pBoltExt->Arcs = pWeaponExt->Bolt_Arcs;
	pBoltExt->BurstIndex = pThis->CurrentBurstIndex;
	pBolt->Lifetime = 1 << (std::clamp(pWeaponExt->Bolt_Duration.Get(), 1, 31) - 1);
	return SkipGameCode;
}

DEFINE_HOOK(0x6FD469, TechnoClass_FireEBolt, 0x9)
{
	return EBoltExt::_TechnoClass_FireEBolt(R);
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
	int& boltColor1 = AresHelper::CanUseAres ? *EBoltExt::AresBoltColor1 : BoltTemp::Color1;
	int& boltColor2 = AresHelper::CanUseAres ? *EBoltExt::AresBoltColor2 : BoltTemp::Color2;
	int& boltColor3 = AresHelper::CanUseAres ? *EBoltExt::AresBoltColor3 : BoltTemp::Color3;

	BoltTemp::ExtData = EBoltExt::ExtMap.Find(pThis);

	if (const auto pExt = BoltTemp::ExtData)
	{
		boltColor1 = Drawing::RGB_To_Int(pExt->Color[0]);
		boltColor2 = Drawing::RGB_To_Int(pExt->Color[1]);
		boltColor3 = Drawing::RGB_To_Int(pExt->Color[2]);
	}
	else
	{
		GET_BASE(int, colorIdx, 0x20);
		const COLORREF defaultAlternate = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, colorIdx);
		const COLORREF defaultWhite = EBoltExt::GetDefaultColor_Int(FileSystem::PALETTE_PAL, 15);
		boltColor1 = boltColor2 = defaultAlternate;
		boltColor3 = defaultWhite;
	}

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
	const auto pExt = BoltTemp::ExtData;
	const int arcCount = pExt ? pExt->Arcs : 8;

	return plotIndex < arcCount ? DoLoop : Break;
}

DEFINE_HOOK(0x4C24BE, EBolt_DrawFirst_Color, 0x5)
{
	R->EAX(BoltTemp::Color1);
	return 0x4C24E4;
}

DEFINE_HOOK(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	return BoltTemp::ExtData && BoltTemp::ExtData->Disable[0] ? 0x4C2515 : 0;
}

DEFINE_HOOK(0x4C24BE, EBolt_DrawSecond_Color, 0x5)
{
	R->EAX(BoltTemp::Color2);
	return 0x4C25FD;
}

DEFINE_HOOK(0x4C25FD, Ebolt_DrawSecond_Disable, 0xA)
{
	return BoltTemp::ExtData && BoltTemp::ExtData->Disable[1] ? 0x4C262A : 0;
}

DEFINE_HOOK(0x4C26CF, EBolt_DrawThird_Color, 0x5)
{
	R->EAX(BoltTemp::Color3);
	return 0x4C26EE;
}

DEFINE_HOOK(0x4C26EE, Ebolt_DrawThird_Disable, 0x8)
{
	return BoltTemp::ExtData && BoltTemp::ExtData->Disable[2] ? 0x4C2710 : 0;
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
	GET(int, weaponIndex, EBX);

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
