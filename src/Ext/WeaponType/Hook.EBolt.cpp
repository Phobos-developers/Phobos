#include "Body.h"
#include <EBolt.h>

#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/AresHelper.h>
#include <Helpers/Macro.h>

PhobosMap<EBolt*, WeaponTypeExt::EBoltWeaponStruct> WeaponTypeExt::BoltWeaponMap;
int* WeaponTypeExt::BoltColor1 = nullptr;
int* WeaponTypeExt::BoltColor2 = nullptr;
int* WeaponTypeExt::BoltColor3 = nullptr;
const WeaponTypeExt::ExtData* WeaponTypeExt::BoltWeaponType = nullptr;

DEFINE_HOOK(0x6FD494, TechnoClass_FireEBolt_SetExtMap_AfterAres, 0x7)
{
	GET(TechnoClass*, pThis, EDI);
	GET(EBolt*, pBolt, EAX);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x30, 0x8));

	if (pWeapon)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		auto& weaponStruct = WeaponTypeExt::BoltWeaponMap[pBolt];
		weaponStruct.Weapon = pWeaponExt;
		weaponStruct.BurstIndex = pThis->CurrentBurstIndex;
		pBolt->Lifetime = 1 << (Math::clamp(pWeaponExt->Bolt_Duration, 1, 31) - 1);
	}

	return 0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt*, pBolt, ECX);

	WeaponTypeExt::BoltWeaponMap.erase(pBolt);

	return 0;
}

static COLORREF __forceinline EBolt_GetDefaultColor_Int(ConvertClass* pConvert, int idx)
{
	if (pConvert->BytesPerPixel == 1)
		return reinterpret_cast<uint8_t*>(pConvert->PaletteData)[idx];
	else
		return reinterpret_cast<uint16_t*>(pConvert->PaletteData)[idx];
}

DWORD _cdecl WeaponTypeExt::_EBolt_Draw_Colors(REGISTERS* R)
{
	enum { SkipGameCode = 0x4C1F66 };

	GET(EBolt*, pThis, ECX);
	int* boltColor1 = nullptr;
	int* boltColor2 = nullptr;
	int* boltColor3 = nullptr;

	if (AresHelper::CanUseAres)
	{
		GET_BASE(int, colorIdx, 0x20);
		boltColor1 = WeaponTypeExt::BoltColor1;
		boltColor2 = WeaponTypeExt::BoltColor2;
		boltColor3 = WeaponTypeExt::BoltColor3;

		const COLORREF defaultAlternate = EBolt_GetDefaultColor_Int(FileSystem::PALETTE_PAL, colorIdx);
		const COLORREF defaultWhite = EBolt_GetDefaultColor_Int(FileSystem::PALETTE_PAL, 15);
		*boltColor1 = *boltColor2 = defaultAlternate;
		*boltColor3 = defaultWhite;
	}

	if (const auto pWeaponExt = WeaponTypeExt::BoltWeaponMap.get_or_default(pThis).Weapon)
	{
		WeaponTypeExt::BoltWeaponType = pWeaponExt;

		if (AresHelper::CanUseAres)
		{
			if (pWeaponExt->Bolt_Color1.isset())
				*boltColor1 = Drawing::RGB_To_Int(pWeaponExt->Bolt_Color1.Get());

			if (pWeaponExt->Bolt_Color2.isset())
				*boltColor2 = Drawing::RGB_To_Int(pWeaponExt->Bolt_Color2.Get());

			if (pWeaponExt->Bolt_Color3.isset())
				*boltColor3 = Drawing::RGB_To_Int(pWeaponExt->Bolt_Color3.Get());
		}
	}
	else
	{
		WeaponTypeExt::BoltWeaponType = nullptr;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x4C20BC, EBolt_DrawArcs, 0xB)
{
	enum { DoLoop = 0x4C20C7, Break = 0x4C2400 };

	GET_STACK(EBolt*, pBolt, 0x40);
	GET_STACK(int, plotIndex, STACK_OFFSET(0x408, -0x3E0));

	if (!AresHelper::CanUseAres)
		WeaponTypeExt::BoltWeaponType = WeaponTypeExt::BoltWeaponMap.get_or_default(pBolt).Weapon;

	int arcCount = WeaponTypeExt::BoltWeaponType ? WeaponTypeExt::BoltWeaponType->Bolt_Arcs : 8;

	return plotIndex < arcCount ? DoLoop : Break;
}

DEFINE_HOOK(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	GET_STACK(EBolt*, pBolt, 0x40);
	WeaponTypeExt::BoltWeaponType = WeaponTypeExt::BoltWeaponMap.get_or_default(pBolt).Weapon;

	return (WeaponTypeExt::BoltWeaponType && WeaponTypeExt::BoltWeaponType->Bolt_Disable1) ? 0x4C2515 : 0;
}

DEFINE_HOOK(0x4C25FD, Ebolt_DrawSecond_Disable, 0xA)
{
	return (WeaponTypeExt::BoltWeaponType && WeaponTypeExt::BoltWeaponType->Bolt_Disable2) ? 0x4C262A : 0;
}

DEFINE_HOOK(0x4C26EE, Ebolt_DrawThird_Disable, 0x8)
{
	return (WeaponTypeExt::BoltWeaponType && WeaponTypeExt::BoltWeaponType->Bolt_Disable3) ? 0x4C2710 : 0;
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
	pTechno->CurrentBurstIndex = WeaponTypeExt::BoltWeaponMap[pThis].BurstIndex;
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
