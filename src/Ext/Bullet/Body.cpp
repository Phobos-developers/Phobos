#include "Body.h"

#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Misc/FlyingStrings.h>

BulletExt::ExtContainer BulletExt::ExtMap;

void BulletExt::ExtData::InterceptBullet(TechnoClass* pSource, WeaponTypeClass* pWeapon)
{
	if (!pSource || !pWeapon)
		return;

	auto const pThis = this->OwnerObject();
	auto pTypeExt = this->TypeExtData;
	bool canAffect = false;
	bool isIntercepted = false;

	if (pTypeExt->Armor.isset())
	{
		double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pTypeExt->Armor.Get());

		if (versus != 0.0)
		{
			canAffect = true;

			int damage = static_cast<int>(pWeapon->Damage * versus * pSource->FirepowerMultiplier);
			this->CurrentStrength -= damage;

			if (Phobos::DisplayDamageNumbers && damage != 0)
				GeneralUtils::DisplayDamageNumberString(damage, DamageDisplayType::Intercept, this->OwnerObject()->GetRenderCoords(), this->DamageNumberOffset);

			if (this->CurrentStrength <= 0)
				isIntercepted = true;
		}
	}
	else
	{
		canAffect = true;
		isIntercepted = true;
	}

	if (canAffect)
	{
		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
		const auto pInterceptorType = pTechnoTypeExt->InterceptorType.get();
		const auto pWeaponOverride = pInterceptorType->WeaponOverride.Get(pTypeExt->Interceptable_WeaponOverride);
		bool detonate = !pInterceptorType->DeleteOnIntercept.Get(pTypeExt->Interceptable_DeleteOnIntercept);

		this->DetonateOnInterception = detonate;

		if (pWeaponOverride)
		{
			bool replaceType = pInterceptorType->WeaponReplaceProjectile;
			bool cumulative = pInterceptorType->WeaponCumulativeDamage;

			pThis->WeaponType = pWeaponOverride;
			pThis->Health = cumulative ? pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;
			pThis->WH = pWeaponOverride->Warhead;
			pThis->Bright = pWeaponOverride->Bright;

			if (replaceType && pWeaponOverride->Projectile != pThis->Type && pWeaponOverride->Projectile)
			{
				pThis->Speed = pWeaponOverride->Speed;
				pThis->Type = pWeaponOverride->Projectile;
				pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
				this->TypeExtData = pTypeExt;

				if (this->LaserTrails.size())
				{
					this->LaserTrails.clear();

					if (!pThis->Type->Inviso)
						this->InitializeLaserTrails();
				}

				// Lose target if the current bullet is no longer interceptable.
				if (!pTypeExt->Interceptable || (pTypeExt->Armor.isset() && GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pTypeExt->Armor.Get()) == 0.0))
					pSource->SetTarget(nullptr);
			}
		}

		if (isIntercepted && !pInterceptorType->KeepIntact)
			this->InterceptedStatus = InterceptedStatus::Intercepted;
	}
}

void BulletExt::ExtData::ApplyRadiationToCell(CellStruct Cell, int Spread, int RadLevel)
{
	auto const pThis = this->OwnerObject();

	auto const pWeapon = pThis->GetWeaponType();
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto const pRadType = pWeaponExt->RadType;
	auto const pThisHouse = pThis->Owner ? pThis->Owner->Owner : this->FirerHouse;

	auto const it = std::find_if(RadSiteClass::Array->begin(), RadSiteClass::Array->end(),
		[=](auto const pSite)
		{
			auto const pRadExt = RadSiteExt::ExtMap.Find(pSite);

			if (pRadExt->Type != pRadType)
				return false;

			if (MapClass::Instance->TryGetCellAt(pSite->BaseCell) != MapClass::Instance->TryGetCellAt(Cell))
				return false;

			if (Spread != pSite->Spread)
				return false;

			if (pWeapon != pRadExt->Weapon)
				return false;

			if (pRadExt->RadInvoker && pThis->Owner)
				return pRadExt->RadInvoker == pThis->Owner;

			return true;
		}
	);

	if (it != RadSiteClass::Array->end())
	{
		if ((*it)->GetRadLevel() + RadLevel >= pRadType->GetLevelMax())
		{
			RadLevel = pRadType->GetLevelMax() - (*it)->GetRadLevel();
		}

		auto const pRadExt = RadSiteExt::ExtMap.Find((*it));
		// Handle It
		pRadExt->Add(RadLevel);
		return;
	}

	RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThisHouse, pThis->Owner);
}

void BulletExt::ExtData::InitializeLaserTrails()
{
	if (this->LaserTrails.size())
		return;

	auto pThis = this->OwnerObject();

	if (auto pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
	{
		auto pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

		for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
		{
			this->LaserTrails.emplace_back(LaserTrailTypeClass::Array[idxTrail].get(), pOwner);
		}
	}
}

// =============================
// load / save

template <typename T>
void BulletExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->FirerHouse)
		.Process(this->CurrentStrength)
		.Process(this->IsInterceptor)
		.Process(this->InterceptedStatus)
		.Process(this->DetonateOnInterception)
		.Process(this->LaserTrails)
		.Process(this->SnappedToTarget)
		.Process(this->DamageNumberOffset)

		.Process(this->Trajectory) // Keep this shit at last
		;
}

void BulletExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BulletClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BulletClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") { }

BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46AFB0, BulletClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46AE70, BulletClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x46AF97, BulletClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x46AF9E, BulletClass_Load_Suffix, 0x7)
{
	BulletExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46AFC4, BulletClass_Save_Suffix, 0x3)
{
	BulletExt::ExtMap.SaveStatic();
	return 0;
}
