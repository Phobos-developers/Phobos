#include "Body.h"
#include <EBolt.h>
#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>

PhobosMap<EBolt*, WeaponTypeExt::EBoltWeaponStruct> WeaponTypeExt::BoltWeaponMap;
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
	}

	return 0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt*, pBolt, ECX);

	WeaponTypeExt::BoltWeaponMap.erase(pBolt);

	return 0;
}

DEFINE_HOOK(0x4C20BC, EBolt_DrawArcs, 0xB)
{
	enum { DoLoop = 0x4C20C7, Break = 0x4C2400 };

	GET_STACK(EBolt*, pBolt, 0x40);
	WeaponTypeExt::BoltWeaponType = WeaponTypeExt::BoltWeaponMap.get_or_default(pBolt).Weapon;

	GET_STACK(int, plotIndex, STACK_OFFSET(0x408, -0x3E0));

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
