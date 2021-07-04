#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>

template<> const DWORD Extension<BulletClass>::Canary = 0x2A2A2A2A;
BulletExt::ExtContainer BulletExt::ExtMap;

void BulletExt::ExtData::ApplyRadiationToCell(CellStruct Cell, int Spread, int RadLevel)
{
	auto pThis = this->OwnerObject();
	auto const& Instances = RadSiteExt::Array;
	auto const pWeapon = pThis->GetWeaponType();
	auto const pWeaponExt = WeaponTypeExt::ExtMap.FindOrAllocate(pWeapon);
	auto const pRadType = pWeaponExt->RadType;
	auto const pThisHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;

	if (Instances.Count > 0)
	{
		auto const it = std::find_if(Instances.begin(), Instances.end(),
			[=](RadSiteExt::ExtData* const pSite) // Lambda
			{// find 
				return pSite->Type == pRadType &&
					pSite->OwnerObject()->BaseCell == Cell &&
					Spread == pSite->OwnerObject()->Spread;
			});

		if (it == Instances.end())
		{
			RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThisHouse);
		}
		else
		{
			auto pRadExt = *it;
			auto pRadSite = pRadExt->OwnerObject();

			if (pRadSite->GetRadLevel() + RadLevel > pRadType->GetLevelMax())
			{
				RadLevel = pRadType->GetLevelMax() - pRadSite->GetRadLevel();
			}

			// Handle It 
			pRadExt->Add(RadLevel);
		}
	}
	else
	{
		RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThisHouse);
	}
}

void BulletExt::ExtData::ApplyArcingFix()
{
	auto pThis = this->OwnerObject();

	this->ArcingFixed = true;

	auto sourcePos = pThis->GetCoords();
	auto targetPos = pThis->TargetCoords;

	if (pThis->Type->FlakScatter)
	{
		auto pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

		int min = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
		int max = pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance()->BallisticScatter));

		double random = ScenarioClass::Instance()->Random.RandomRanged(min, max);
		double theta = ScenarioClass::Instance()->Random.RandomDouble() * 2 * Math::Pi;
		
		CoordStruct offset = { random * Math::cos(theta),random * Math::sin(theta),0 };
		targetPos += offset;
	}

	auto nZDiff = targetPos.Z - sourcePos.Z;
	targetPos.Z = 0;
	sourcePos.Z = 0;
	auto nDistance = targetPos.DistanceFrom(sourcePos);

	if (pThis->WeaponType && pThis->WeaponType->Lobber)
		pThis->Speed /= 2;

	auto nSpeed = pThis->Speed;
	auto& pVelocity = pThis->Velocity;

	pVelocity.X = targetPos.X - sourcePos.X;
	pVelocity.Y = targetPos.Y - sourcePos.Y;
	pVelocity *= nSpeed / nDistance;
	pVelocity.Z = nZDiff * nSpeed / nDistance + 0.5 * RulesClass::Instance()->Gravity * nDistance / nSpeed;

}

// =============================
// load / save


template <typename T>
void BulletExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Intercepted)
		.Process(this->ShouldIntercept)
		.Process(this->ArcingFixed)
		;
}

void BulletExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<BulletClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<BulletClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") {
}

BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(4664BA, BulletClass_CTOR, 5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(4665E9, BulletClass_DTOR, A)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(46AFB0, BulletClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(46AE70, BulletClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(46AF97, BulletClass_Load_Suffix, 7)
DEFINE_HOOK(46AF9E, BulletClass_Load_Suffix, 7)
{
	BulletExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(46AFC4, BulletClass_Save_Suffix, 3)
{
	BulletExt::ExtMap.SaveStatic();
	return 0;
}