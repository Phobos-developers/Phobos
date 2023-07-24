#include "Body.h"
#include <GameStrings.h>
#include <New/Type/RadTypeClass.h>
#include <Ext/WarheadType/Body.h>
#include <LightSourceClass.h>

template<> const DWORD Extension<RadSiteClass>::Canary = 0x87654321;
RadSiteExt::ExtContainer RadSiteExt::ExtMap;

DynamicVectorClass<RadSiteExt::ExtData*> RadSiteExt::Array;

void RadSiteExt::ExtData::Initialize()
{
	this->Type = RadTypeClass::FindOrAllocate(GameStrings::Radiation);
}

bool RadSiteExt::ExtData::ApplyRadiationDamage(TechnoClass* pTarget, int& damage, int distance)
{
	auto pWarhead = this->Type->GetWarhead();

	if (!this->Type->GetWarheadDetonate())
	{
		if (pTarget->ReceiveDamage(&damage, distance, pWarhead, this->RadInvoker, false, true, this->RadHouse) == DamageState::NowDead)
			return false;
	}
	else
	{
		WarheadTypeExt::DetonateAt(pWarhead, pTarget, this->RadInvoker, damage);

		if (!pTarget->IsAlive)
			return false;
	}

	return true;
}


void RadSiteExt::CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, HouseClass* const pOwner, TechnoClass* const pInvoker)
{
	// use real ctor
	auto const pRadSite = GameCreate<RadSiteClass>();
	auto pRadExt = RadSiteExt::ExtMap.FindOrAllocate(pRadSite);

	//Adding Owner to RadSite, from bullet
	if (pWeaponExt->RadType->GetHasOwner() && pRadExt->RadHouse != pOwner)
		pRadExt->RadHouse = pOwner;

	if (pWeaponExt->RadType->GetHasInvoker() && pRadExt->RadInvoker != pInvoker)
		pRadExt->RadInvoker = pInvoker;

	pRadExt->Weapon = pWeaponExt->OwnerObject();
	pRadExt->Type = pWeaponExt->RadType;
	pRadSite->SetBaseCell(&location);
	pRadSite->SetSpread(spread);
	RadSiteExt::SetRadLevel(pRadSite, amount);
	RadSiteExt::CreateLight(pRadSite);

	Array.AddUnique(pRadExt);
}

//RadSiteClass Activate , Rewritten
void RadSiteExt::CreateLight(RadSiteClass* pThis)
{
	auto pRadExt = RadSiteExt::ExtMap.Find(pThis);
	auto nLevelDelay = pRadExt->Type->GetLevelDelay();
	auto nLightDelay = pRadExt->Type->GetLightDelay();

	pThis->RadLevelTimer.StartTime = Unsorted::CurrentFrame;
	pThis->RadLevelTimer.TimeLeft = nLevelDelay;
	pThis->RadLightTimer.StartTime = Unsorted::CurrentFrame;
	pThis->RadLightTimer.TimeLeft = nLightDelay;

	auto nLightFactor = pThis->RadLevel * pRadExt->Type->GetLightFactor();
	nLightFactor = Math::min(nLightFactor, 2000.0);
	auto nDuration = pThis->RadDuration;

	pThis->Intensity = Game::F2I(nLightFactor);
	pThis->LevelSteps = nDuration / nLevelDelay;
	pThis->IntensitySteps = nDuration / nLightDelay;
	pThis->IntensityDecrement = Game::F2I(nLightFactor) / (nDuration / nLightDelay);

	auto nRadcolor = pRadExt->Type->GetColor();
	auto nTintFactor = pRadExt->Type->GetTintFactor();

	//=========Red
	auto red = ((1000 * nRadcolor.R) / 255)* nTintFactor;
	red = Math::min(red, 2000.0);
	//=========Green
	auto green = ((1000 * nRadcolor.G) / 255) * nTintFactor;
	green = Math::min(green, 2000.0);
	//=========Blue
	auto blue = ((1000 * nRadcolor.B) / 255) * nTintFactor;
	blue = Math::min(blue, 2000.0);;

	TintStruct nTintBuffer{ Game::F2I(red) ,Game::F2I(green) ,Game::F2I(blue) };
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
		if (auto const pLight = GameCreate<LightSourceClass>(pCell->GetCoords(), pThis->SpreadInLeptons, Game::F2I(nLightFactor), nTintBuffer))
		{
			pThis->LightSource = pLight;
			pLight->DetailLevel = 0;
			pLight->Activate(update);
			pThis->Radiate();
		}
	}
}

// Rewrite because of crashing craziness
void RadSiteExt::Add(RadSiteClass* pThis, int amount)
{
	auto const RadExt = RadSiteExt::ExtMap.Find(pThis);
	int value = pThis->RadLevel * pThis->RadTimeLeft / pThis->RadDuration;
	pThis->Deactivate();
	pThis->RadLevel = value + amount;
	pThis->RadDuration = pThis->RadLevel * RadExt->Type->GetDurationMultiple();
	pThis->RadTimeLeft = pThis->RadDuration;
	RadSiteExt::CreateLight(pThis);
}

void RadSiteExt::SetRadLevel(RadSiteClass* pThis, int amount)
{
	auto const RadExt = RadSiteExt::ExtMap.Find(pThis);
	const int mult = RadExt->Type->GetDurationMultiple();
	pThis->RadLevel = amount;
	pThis->RadDuration = mult * amount;
	pThis->RadTimeLeft = mult * amount;
}

// helper function provided by AlexB
const double RadSiteExt::GetRadLevelAt(RadSiteClass* pThis, CellStruct const& cell)
{
	const auto base = MapClass::Instance->GetCellAt(pThis->BaseCell)->GetCoords();
	const auto coords = MapClass::Instance->GetCellAt(cell)->GetCoords();
	const auto max = static_cast<double>(pThis->SpreadInLeptons);
	const auto dist = coords.DistanceFrom(base);
	return (dist > max) ? 0.0 : (max - dist) / max * pThis->RadLevel;
}

// =============================
// load / save

template <typename T>
void RadSiteExt::ExtData::Serialize(T& Stm)
{
	Stm
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
	auto pRadSiteExt = RadSiteExt::ExtMap.FindOrAllocate(pThis);

	RadSiteExt::Array.AddUnique(pRadSiteExt);

	return 0;
}

DEFINE_HOOK(0x65B2F4, RadSiteClass_DTOR, 0x5)
{
	GET(RadSiteClass*, pThis, ECX);
	auto pRadExt = RadSiteExt::ExtMap.Find(pThis);

	RadSiteExt::ExtMap.Remove(pThis);
	RadSiteExt::Array.Remove(pRadExt);

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
