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

	auto sourcePos = pThis->GetCoords();
	auto targetPos = pThis->TargetCoords;

	if (pThis->Type->Inaccurate)
	{
		auto pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

		int nMin = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
		int nMax = pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance()->BallisticScatter));

		double random = ScenarioClass::Instance()->Random.RandomRanged(nMin, nMax);
		double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;
		
		CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};
		targetPos += offset;
	}

	auto nZDiff = targetPos.Z - sourcePos.Z;
	targetPos.Z = 0;
	sourcePos.Z = 0;
	auto const nDistance = targetPos.DistanceFrom(sourcePos);

	if (pThis->WeaponType && pThis->WeaponType->Lobber)
		pThis->Speed /= 2;

	auto const nSpeed = pThis->Speed;

	pThis->Velocity.X = targetPos.X - sourcePos.X;
	pThis->Velocity.Y = targetPos.Y - sourcePos.Y;
	pThis->Velocity *= nSpeed / nDistance;
	pThis->Velocity.Z = nZDiff * nSpeed / nDistance + 0.5 * RulesClass::Instance()->Gravity * nDistance / nSpeed;
}

// =============================
// load / save


template <typename T>
void BulletExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Intercepted)
		.Process(this->ShouldIntercept)
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

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.FindOrAllocate(pItem);
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