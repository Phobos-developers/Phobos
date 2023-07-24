#include "Body.h"
#include <GameStrings.h>
#include <Ext/Bullet/Body.h>

template<> const DWORD Extension<WeaponTypeClass>::Canary = 0x22222222;
WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;

void WeaponTypeExt::ExtData::Initialize()
{
	this->RadType = RadTypeClass::FindOrAllocate(GameStrings::Radiation);
}

// =============================
// load / save

void WeaponTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->DiskLaser_Radius.Read(exINI, pSection, "DiskLaser.Radius");

	this->Bolt_Disable1.Read(exINI, pSection, "Bolt.Disable1");
	this->Bolt_Disable2.Read(exINI, pSection, "Bolt.Disable2");
	this->Bolt_Disable3.Read(exINI, pSection, "Bolt.Disable3");

	this->RadType.Read(exINI, pSection, "RadType", true);

	this->Strafing_Shots.Read(exINI, pSection, "Strafing.Shots");
	this->Strafing_SimulateBurst.Read(exINI, pSection, "Strafing.SimulateBurst");
	this->CanTarget.Read(exINI, pSection, "CanTarget");
	this->CanTargetHouses.Read(exINI, pSection, "CanTargetHouses");
	this->Burst_Delays.Read(exINI, pSection, "Burst.Delays");
	this->AreaFire_Target.Read(exINI, pSection, "AreaFire.Target");
	this->FeedbackWeapon.Read(exINI, pSection, "FeedbackWeapon", true);
	this->Laser_IsSingleColor.Read(exINI, pSection, "IsSingleColor");
}

template <typename T>
void WeaponTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DiskLaser_Radius)
		.Process(this->Bolt_Disable1)
		.Process(this->Bolt_Disable2)
		.Process(this->Bolt_Disable3)
		.Process(this->Strafing_Shots)
		.Process(this->Strafing_SimulateBurst)
		.Process(this->CanTarget)
		.Process(this->CanTargetHouses)
		.Process(this->RadType)
		.Process(this->Burst_Delays)
		.Process(this->AreaFire_Target)
		.Process(this->FeedbackWeapon)
		.Process(this->Laser_IsSingleColor)
		;
};

void WeaponTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WeaponTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);

}

void WeaponTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WeaponTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WeaponTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(OldRadius)
		.Success();
}

bool WeaponTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(OldRadius)
		.Success();
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget, pOwner, pThis->Damage, pFiringHouse);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse)
{
	if (BulletClass* pBullet = pThis->Projectile->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, 0, pThis->Bright))
	{
		const CoordStruct& coords = pTarget->GetCoords();

		if (pFiringHouse)
		{
			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			pBulletExt->FirerHouse = pFiringHouse;
		}

		pBullet->SetWeaponType(pThis);
		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, coords, pOwner, pThis->Damage, pFiringHouse);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse)
{
	if (BulletClass* pBullet = pThis->Projectile->CreateBullet(nullptr, pOwner,
		damage, pThis->Warhead, 0, pThis->Bright))
	{
		if (pFiringHouse)
		{
			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			pBulletExt->FirerHouse = pFiringHouse;
		}

		pBullet->SetWeaponType(pThis);
		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

// =============================
// container

WeaponTypeExt::ExtContainer::ExtContainer() : Container("WeaponTypeClass") { }

WeaponTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x771EE9, WeaponTypeClass_CTOR, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x77311D, WeaponTypeClass_SDDTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x772EB0, WeaponTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x772CD0, WeaponTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x772EA6, WeaponTypeClass_Load_Suffix, 0x6)
{
	WeaponTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x772F8C, WeaponTypeClass_Save, 0x5)
{
	WeaponTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x7729C7, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x7729D6, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x7729B0, WeaponTypeClass_LoadFromINI, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
