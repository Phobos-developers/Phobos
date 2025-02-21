#include "Body.h"
#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;

bool WeaponTypeExt::ExtData::HasRequiredAttachedEffects(TechnoClass* pTarget, TechnoClass* pFirer) const
{
	bool hasRequiredTypes = this->AttachEffect_RequiredTypes.size() > 0;
	bool hasDisallowedTypes = this->AttachEffect_DisallowedTypes.size() > 0;
	bool hasRequiredGroups = this->AttachEffect_RequiredGroups.size() > 0;
	bool hasDisallowedGroups = this->AttachEffect_DisallowedGroups.size() > 0;

	if (hasRequiredTypes || hasDisallowedTypes || hasRequiredGroups || hasDisallowedGroups)
	{
		auto pTechno = pTarget;

		if (this->AttachEffect_CheckOnFirer && pFirer)
			pTechno = pFirer;

		if (!pTechno)
			return true;

		auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

		if (hasDisallowedTypes && pTechnoExt->HasAttachedEffects(this->AttachEffect_DisallowedTypes, false, this->AttachEffect_IgnoreFromSameSource, pFirer, this->OwnerObject()->Warhead, &this->AttachEffect_DisallowedMinCounts, &this->AttachEffect_DisallowedMaxCounts))
			return false;

		if (hasDisallowedGroups && pTechnoExt->HasAttachedEffects(AttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_DisallowedGroups), false, this->AttachEffect_IgnoreFromSameSource, pFirer, this->OwnerObject()->Warhead, &this->AttachEffect_DisallowedMinCounts, &this->AttachEffect_DisallowedMaxCounts))
			return false;

		if (hasRequiredTypes && !pTechnoExt->HasAttachedEffects(this->AttachEffect_RequiredTypes, true, this->AttachEffect_IgnoreFromSameSource, pFirer, this->OwnerObject()->Warhead, &this->AttachEffect_RequiredMinCounts, &this->AttachEffect_RequiredMaxCounts))
			return false;

		if (hasRequiredGroups &&
			!pTechnoExt->HasAttachedEffects(AttachEffectTypeClass::GetTypesFromGroups(this->AttachEffect_RequiredGroups), true, this->AttachEffect_IgnoreFromSameSource, pFirer, this->OwnerObject()->Warhead, &this->AttachEffect_RequiredMinCounts, &this->AttachEffect_RequiredMaxCounts))
			return false;
	}

	return true;
}

void WeaponTypeExt::ExtData::Initialize()
{
	this->RadType = RadTypeClass::FindOrAllocate(GameStrings::Radiation);
}

int WeaponTypeExt::ExtData::GetBurstDelay(int burstIndex) const
{
	int burstDelay = -1;

	if (burstIndex == 0)
		return 0;
	else if (this->Burst_Delays.size() > (unsigned)burstIndex)
		burstDelay = this->Burst_Delays[burstIndex - 1];
	else if (this->Burst_Delays.size() > 0)
		burstDelay = this->Burst_Delays[this->Burst_Delays.size() - 1];

	return burstDelay;
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
	this->ProjectileRange.Read(exINI, pSection, "ProjectileRange");

	this->Bolt_Disable1.Read(exINI, pSection, "Bolt.Disable1");
	this->Bolt_Disable2.Read(exINI, pSection, "Bolt.Disable2");
	this->Bolt_Disable3.Read(exINI, pSection, "Bolt.Disable3");

	this->Bolt_Arcs.Read(exINI, pSection, "Bolt.Arcs");

	this->RadType.Read<true>(exINI, pSection, "RadType");

	this->Strafing.Read(exINI, pSection, "Strafing");
	this->Strafing_Shots.Read(exINI, pSection, "Strafing.Shots");
	this->Strafing_SimulateBurst.Read(exINI, pSection, "Strafing.SimulateBurst");
	this->Strafing_UseAmmoPerShot.Read(exINI, pSection, "Strafing.UseAmmoPerShot");
	this->CanTarget.Read(exINI, pSection, "CanTarget");
	this->CanTargetHouses.Read(exINI, pSection, "CanTargetHouses");
	this->Burst_Delays.Read(exINI, pSection, "Burst.Delays");
	this->Burst_FireWithinSequence.Read(exINI, pSection, "Burst.FireWithinSequence");
	this->AreaFire_Target.Read(exINI, pSection, "AreaFire.Target");
	this->FeedbackWeapon.Read<true>(exINI, pSection, "FeedbackWeapon");
	this->Laser_IsSingleColor.Read(exINI, pSection, "IsSingleColor");
	this->ROF_RandomDelay.Read(exINI, pSection, "ROF.RandomDelay");
	this->ChargeTurret_Delays.Read(exINI, pSection, "ChargeTurret.Delays");
	this->OmniFire_TurnToTarget.Read(exINI, pSection, "OmniFire.TurnToTarget");
	this->FireOnce_ResetSequence.Read(exINI, pSection, "FireOnce.ResetSequence");
	this->ExtraWarheads.Read(exINI, pSection, "ExtraWarheads");
	this->ExtraWarheads_DamageOverrides.Read(exINI, pSection, "ExtraWarheads.DamageOverrides");
	this->ExtraWarheads_DetonationChances.Read(exINI, pSection, "ExtraWarheads.DetonationChances");
	this->ExtraWarheads_FullDetonation.Read(exINI, pSection, "ExtraWarheads.FullDetonation");
	this->AmbientDamage_Warhead.Read<true>(exINI, pSection, "AmbientDamage.Warhead");
	this->AmbientDamage_IgnoreTarget.Read(exINI, pSection, "AmbientDamage.IgnoreTarget");
	this->AttachEffect_RequiredTypes.Read(exINI, pSection, "AttachEffect.RequiredTypes");
	this->AttachEffect_DisallowedTypes.Read(exINI, pSection, "AttachEffect.DisallowedTypes");
	exINI.ParseStringList(this->AttachEffect_RequiredGroups, pSection, "AttachEffect.RequiredGroups");
	exINI.ParseStringList(this->AttachEffect_DisallowedGroups, pSection, "AttachEffect.DisallowedGroups");
	this->AttachEffect_RequiredMinCounts.Read(exINI, pSection, "AttachEffect.RequiredMinCounts");
	this->AttachEffect_RequiredMaxCounts.Read(exINI, pSection, "AttachEffect.RequiredMaxCounts");
	this->AttachEffect_DisallowedMinCounts.Read(exINI, pSection, "AttachEffect.DisallowedMinCounts");
	this->AttachEffect_DisallowedMaxCounts.Read(exINI, pSection, "AttachEffect.DisallowedMaxCounts");
	this->AttachEffect_CheckOnFirer.Read(exINI, pSection, "AttachEffect.CheckOnFirer");
	this->AttachEffect_IgnoreFromSameSource.Read(exINI, pSection, "AttachEffect.IgnoreFromSameSource");
	this->KickOutPassengers.Read(exINI, pSection, "KickOutPassengers");
}

template <typename T>
void WeaponTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DiskLaser_Radius)
		.Process(this->ProjectileRange)
		.Process(this->Bolt_Disable1)
		.Process(this->Bolt_Disable2)
		.Process(this->Bolt_Disable3)
		.Process(this->Bolt_Arcs)
		.Process(this->Strafing)
		.Process(this->Strafing_Shots)
		.Process(this->Strafing_SimulateBurst)
		.Process(this->Strafing_UseAmmoPerShot)
		.Process(this->CanTarget)
		.Process(this->CanTargetHouses)
		.Process(this->RadType)
		.Process(this->Burst_Delays)
		.Process(this->Burst_FireWithinSequence)
		.Process(this->AreaFire_Target)
		.Process(this->FeedbackWeapon)
		.Process(this->Laser_IsSingleColor)
		.Process(this->ROF_RandomDelay)
		.Process(this->ChargeTurret_Delays)
		.Process(this->OmniFire_TurnToTarget)
		.Process(this->FireOnce_ResetSequence)
		.Process(this->ExtraWarheads)
		.Process(this->ExtraWarheads_DamageOverrides)
		.Process(this->ExtraWarheads_DetonationChances)
		.Process(this->ExtraWarheads_FullDetonation)
		.Process(this->AmbientDamage_Warhead)
		.Process(this->AmbientDamage_IgnoreTarget)
		.Process(this->AttachEffect_RequiredTypes)
		.Process(this->AttachEffect_DisallowedTypes)
		.Process(this->AttachEffect_RequiredGroups)
		.Process(this->AttachEffect_DisallowedGroups)
		.Process(this->AttachEffect_RequiredMinCounts)
		.Process(this->AttachEffect_RequiredMaxCounts)
		.Process(this->AttachEffect_DisallowedMinCounts)
		.Process(this->AttachEffect_DisallowedMaxCounts)
		.Process(this->AttachEffect_CheckOnFirer)
		.Process(this->AttachEffect_IgnoreFromSameSource)
		.Process(this->KickOutPassengers)
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

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget->GetCoords(), pOwner, pThis->Damage, pFiringHouse, pTarget);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget->GetCoords(), pOwner, damage, pFiringHouse, pTarget);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, HouseClass* pFiringHouse, AbstractClass* pTarget)
{
	WeaponTypeExt::DetonateAt(pThis, coords, pOwner, pThis->Damage, pFiringHouse, pTarget);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget)
{
	if (BulletClass* pBullet = pThis->Projectile->CreateBullet(pTarget, pOwner,
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

int WeaponTypeExt::GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer)
{
	int range = 0;

	if (!pThis && !pFirer)
		return range;
	else if (pFirer && pFirer->CanOccupyFire())
		range = RulesClass::Instance->OccupyWeaponRange * Unsorted::LeptonsPerCell;
	else if (pThis && pFirer)
		range = pThis->Range;
	else
		return range;

	if (range == -512)
		return range;

	return WeaponTypeExt::GetRangeWithModifiers(pThis, pFirer, range);
}

int WeaponTypeExt::GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, int range)
{
	auto pTechno = pFirer;

	if (pTechno->Transporter && pTechno->Transporter->GetTechnoType()->OpenTopped)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->Transporter->GetTechnoType());

		if (pTypeExt->OpenTopped_UseTransportRangeModifiers)
			pTechno = pTechno->Transporter;
	}

	auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (!pTechnoExt->AE.HasRangeModifier)
		return range;

	int extraRange = 0;

	for (auto const& attachEffect : pTechnoExt->AttachedEffects)
	{
		if (!attachEffect->IsActive())
			continue;

		auto const type = attachEffect->GetType();

		if (type->WeaponRange_Multiplier == 1.0 && type->WeaponRange_ExtraRange == 0.0)
			continue;

		if (type->WeaponRange_AllowWeapons.size() > 0 && pThis && !type->WeaponRange_AllowWeapons.Contains(pThis))
			continue;

		if (type->WeaponRange_DisallowWeapons.size() > 0 && pThis && type->WeaponRange_DisallowWeapons.Contains(pThis))
			continue;

		range = static_cast<int>(range * Math::max(type->WeaponRange_Multiplier, 0.0));
		extraRange += static_cast<int>(type->WeaponRange_ExtraRange * Unsorted::LeptonsPerCell);
	}

	range += extraRange;

	return Math::max(range, 0);
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

	WeaponTypeExt::ExtMap.TryAllocate(pItem);

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
