#include "Body.h"
#include <New/Type/RadTypeClass.h>
#include <Ext/WarheadType/Body.h>
#include <LightSourceClass.h>
#include <Notifications.h>

RadSiteExt::ExtContainer RadSiteExt::ExtMap;

void RadSiteExt::ExtData::Initialize()
{
	this->Type = RadTypeClass::FindOrAllocate(GameStrings::Radiation);
}

bool RadSiteExt::ExtData::ApplyRadiationDamage(TechnoClass* pTarget, int& damage, int distance)
{
	auto const pWarhead = this->Type->GetWarhead();

	if (!this->Type->GetWarheadDetonate())
	{
		if (pTarget->ReceiveDamage(&damage, distance, pWarhead, this->RadInvoker, false, true, this->RadHouse) == DamageState::NowDead)
			return false;
	}
	else
	{
		if (this->Type->GetWarheadDetonateFull())
		{
			WarheadTypeExt::DetonateAt(pWarhead, pTarget, this->RadInvoker, damage, this->RadHouse);
		}
		else
		{
			auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
			pWHExt->DamageAreaWithTarget(pTarget->GetCoords(), damage, this->RadInvoker, pWarhead, true, this->RadHouse, pTarget);
		}

		if (!pTarget->IsAlive)
			return false;
	}

	return true;
}


void RadSiteExt::CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, HouseClass* const pOwner, TechnoClass* const pInvoker)
{
	// use real ctor
	auto const pRadSite = GameCreate<RadSiteClass>();
	auto pRadExt = RadSiteExt::ExtMap.Find(pRadSite);

	//Adding Owner to RadSite, from bullet
	if (pWeaponExt->RadType->GetHasOwner() && pRadExt->RadHouse != pOwner)
		pRadExt->RadHouse = pOwner;

	if (pWeaponExt->RadType->GetHasInvoker() && pRadExt->RadInvoker != pInvoker)
		pRadExt->RadInvoker = pInvoker;

	pRadExt->LastUpdateFrame = Unsorted::CurrentFrame;
	pRadExt->Weapon = pWeaponExt->OwnerObject();
	pRadExt->Type = pWeaponExt->RadType;
	pRadSite->SetBaseCell(&location);
	pRadSite->SetSpread(spread);
	pRadExt->SetRadLevel(amount);
	pRadExt->CreateLight();
}

//RadSiteClass Activate , Rewritten
void RadSiteExt::ExtData::CreateLight()
{
	const auto pThis = this->OwnerObject();
	auto nLevelDelay = this->Type->GetLevelDelay();
	auto nLightDelay = this->Type->GetLightDelay();

	pThis->RadLevelTimer.Start(nLevelDelay);
	pThis->RadLightTimer.Start(nLightDelay);

	auto nLightFactor = pThis->RadLevel * this->Type->GetLightFactor();
	nLightFactor = Math::min(nLightFactor, 2000.0);
	auto nDuration = pThis->RadDuration;

	pThis->Intensity = Game::F2I(nLightFactor);
	pThis->LevelSteps = nDuration / nLevelDelay;
	pThis->IntensitySteps = nDuration / nLightDelay;
	pThis->IntensityDecrement = Game::F2I(nLightFactor) / (nDuration / nLightDelay);

	auto nRadcolor = this->Type->GetColor();
	auto nTintFactor = this->Type->GetTintFactor();

	//=========Red
	auto red = ((1000 * nRadcolor.R) / 255) * nTintFactor;
	red = Math::min(red, 2000.0);
	//=========Green
	auto green = ((1000 * nRadcolor.G) / 255) * nTintFactor;
	green = Math::min(green, 2000.0);
	//=========Blue
	auto blue = ((1000 * nRadcolor.B) / 255) * nTintFactor;
	blue = Math::min(blue, 2000.0);;

	TintStruct nTintBuffer { Game::F2I(red) ,Game::F2I(green) ,Game::F2I(blue) };
	pThis->Tint = nTintBuffer;
	bool update = false;

	if (pThis->LightSource)
	{
		pThis->LightSource->ChangeLevels(Game::F2I(nLightFactor), nTintBuffer, update);
		pThis->Radiate();
	}
	else
	{
		auto const pCell = MapClass::Instance->TryGetCellAt(pThis->BaseCell);
		auto const pLight = GameCreate<LightSourceClass>(pCell->GetCoords(), pThis->SpreadInLeptons, Game::F2I(nLightFactor), nTintBuffer);
		pThis->LightSource = pLight;
		pLight->DetailLevel = 0;
		pLight->Activate(update);
		pThis->Radiate();
	}
}

// Rewrite because of crashing craziness
void RadSiteExt::ExtData::Add(int amount)
{
	const auto pThis = this->OwnerObject();
	auto const RadExt = RadSiteExt::ExtMap.Find(pThis);
	int value = pThis->RadLevel * pThis->RadTimeLeft / pThis->RadDuration;
	pThis->Deactivate();
	pThis->RadLevel = value + amount;
	pThis->RadDuration = pThis->RadLevel * RadExt->Type->GetDurationMultiple();
	pThis->RadTimeLeft = pThis->RadDuration;
	this->CreateLight();
	this->LastUpdateFrame = Unsorted::CurrentFrame;
}

void RadSiteExt::ExtData::SetRadLevel(int amount)
{
	const auto pThis = this->OwnerObject();
	const int mult = this->Type->GetDurationMultiple();
	pThis->RadLevel = amount;
	pThis->RadDuration = mult * amount;
	pThis->RadTimeLeft = mult * amount;
}

// helper function provided by AlexB
double RadSiteExt::ExtData::GetRadLevelAt(CellStruct const& cell) const
{
	const auto pThis = this->OwnerObject();
	const auto base = MapClass::Instance->GetCellAt(pThis->BaseCell)->GetCoords();
	const auto coords = MapClass::Instance->GetCellAt(cell)->GetCoords();
	const auto max = static_cast<double>(pThis->SpreadInLeptons);
	const auto dist = coords.DistanceFrom(base);
	double radLevel = pThis->RadLevel;

	//  will produce `-nan(ind)` result if both dist and max is zero
	// and used on formula below this check
	// ,.. -Otamaa
	if (dist && max)
		radLevel = (dist > max) ? 0.0 : (max - dist) / max * pThis->RadLevel;

	// Vanilla YR stores & updates the decremented RadLevel on CellClass.
	// Because we're not storing multiple radiation site data on CellClass (yet?)
	// we need to fully recalculate this stuff every time we need the radiation level for a cell coord - Starkku
	int stepCount = (Unsorted::CurrentFrame - this->LastUpdateFrame) / this->Type->GetLevelDelay();
	radLevel -= (radLevel / pThis->LevelSteps) * stepCount;

	return radLevel;
}

// =============================
// load / save

template <typename T>
void RadSiteExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->LastUpdateFrame)
		.Process(this->Weapon)
		.Process(this->RadHouse)
		.Process(this->RadInvoker)
		.Process(this->Type)
		;
}

void RadSiteExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RadSiteClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void RadSiteExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<RadSiteClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

RadSiteExt::ExtContainer::ExtContainer() : Container("RadSiteClass") { };
RadSiteExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x65B28D, RadSiteClass_CTOR, 0x6)
{
	GET(RadSiteClass*, pThis, ESI);

	RadSiteExt::ExtMap.TryAllocate(pThis, pThis->WhatAmI() == AbstractType::RadSite, "Attempted to allocate RadSiteExt from unknown pointer!");
	PointerExpiredNotification::NotifyInvalidObject->Add(pThis);

	return 0;
}

DEFINE_HOOK(0x65B2F4, RadSiteClass_DTOR, 0x5)
{
	GET(RadSiteClass*, pThis, ECX);

	RadSiteExt::ExtMap.Remove(pThis);
	PointerExpiredNotification::NotifyInvalidObject->Remove(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x65B3D0, RadSiteClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x65B450, RadSiteClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(RadSiteClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	RadSiteExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x65B43F, RadSiteClass_Load_Suffix, 0x7)
{
	RadSiteExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x65B464, RadSiteClass_Save_Suffix, 0x5)
{
	RadSiteExt::ExtMap.SaveStatic();

	return 0;
}
