#include "Body.h"

#include <New/Type/RadTypeClass.h>
#include <LightSourceClass.h>

template<> const DWORD Extension<RadSiteClass>::Canary = 0x87654321;
RadSiteExt::ExtContainer RadSiteExt::ExtMap;

DynamicVectorClass<RadSiteExt::ExtData*> RadSiteExt::Array;

void RadSiteExt::CreateInstance(CellStruct location, int spread, int amount, WeaponTypeExt::ExtData* pWeaponExt, HouseClass* const pOwner)
{
	// use real ctor
	auto const pRadSite = GameCreate<RadSiteClass>();
	auto pRadExt = RadSiteExt::ExtMap.FindOrAllocate(pRadSite);

	//Adding Owner to RadSite, from bullet
	if (!pWeaponExt->Rad_NoOwner && pRadExt->RadHouse != pOwner)
		pRadExt->RadHouse = pOwner;

	pRadExt->Weapon = pWeaponExt->OwnerObject();
	pRadExt->Type = pWeaponExt->RadType;
	pRadSite->SetBaseCell(&location);
	pRadSite->SetSpread(spread);
	RadSiteExt::SetRadLevel(pRadSite,amount);
	RadSiteExt::CreateLight(pRadSite);

	Array.AddUnique(pRadExt);
}

//RadSiteClass Activate , Rewritten
void RadSiteExt::CreateLight(RadSiteClass* pThis)
{
	Debug::Log("[" __FUNCTION__ "] Here Iam\n");

	auto const RadExt = RadSiteExt::ExtMap.Find(pThis);
	auto leveldelay = RadExt->Type->GetLevelDelay();
	auto lightdelay = RadExt->Type->GetLightDelay();
	pThis->RadLevelTimer.StartTime = Unsorted::CurrentFrame;
	pThis->RadLevelTimer.TimeLeft = leveldelay;
	pThis->RadLightTimer.StartTime = Unsorted::CurrentFrame;
	pThis->RadLightTimer.TimeLeft = lightdelay;
	auto color = RadExt->Type->GetColor();

	//=========Level
	auto LightFactor = pThis->RadLevel * RadExt->Type->GetLightFactor();
	LightFactor = LightFactor > 2000.0 ? 2000.0 : LightFactor;

	auto tintfactor = RadExt->Type->GetTintFactor();
	auto duration = pThis->RadDuration;
	pThis->Intensity = static_cast<int>(LightFactor);
	pThis->LevelSteps = duration / leveldelay;
	pThis->IntensitySteps = duration / lightdelay;
	pThis->IntensityDecrement = static_cast<int>(LightFactor) / (duration / lightdelay);

	//=========Red
	auto Red = 1000 * color.R / 255 * tintfactor;
	Red = Red > 2000.0 ? 2000.0 : Red;
	//=========Green
	auto Green = 1000 * color.G / 255 * tintfactor;
	Green = Green > 2000.0 ? 2000.0 : Green;
	//=========Blue 
	auto Blue = 1000 * color.B / 255 * tintfactor;
	Blue = Blue > 2000.0 ? 2000.0 : Blue;

	TintStruct nColorBuffer{ static_cast<int>(Red) ,static_cast<int>(Green) ,static_cast<int>(Blue) };
	pThis->Tint = nColorBuffer;
	bool update = false;

	if (pThis->LightSource)
	{
		pThis->LightSource->ChangeLevels(static_cast<int>(LightFactor), nColorBuffer, update);
		pThis->Radiate();
	}
	else
	{
		auto const pos = MapClass::Instance->TryGetCellAt(pThis->BaseCell);
		if (auto const pLight = GameCreate<LightSourceClass>(pos->GetCoords(), pThis->SpreadInLeptons, static_cast<int>(LightFactor), nColorBuffer))
		{
			pThis->LightSource = pLight;
			pLight->DetailLevel = 0;
			pLight->Activate(update);
			pThis->Radiate();
		}
	}
}

// Rewrite because of crashing craziness
void RadSiteExt::Add(RadSiteClass* pThis,int amount)
{
	auto const RadExt = RadSiteExt::ExtMap.Find(pThis);
	int value = pThis->RadLevel * pThis->RadTimeLeft / pThis->RadDuration;
	pThis->Deactivate();
	pThis->RadLevel = value + amount;
	pThis->RadDuration = pThis->RadLevel * RadExt->Type->GetDurationMultiple();
	pThis->RadTimeLeft = pThis->RadDuration;
	RadSiteExt::CreateLight(pThis);
}

void RadSiteExt::SetRadLevel(RadSiteClass* pThis,int amount)
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

DEFINE_HOOK(65B28D, RadSiteClass_CTOR, 6)
{
	GET(RadSiteClass*, pThis, ESI);
	auto pRadSiteExt = RadSiteExt::ExtMap.FindOrAllocate(pThis);

	RadSiteExt::Array.AddUnique(pRadSiteExt);

	return 0;
}

DEFINE_HOOK(65B2F4, RadSiteClass_DTOR, 5)
{
	GET(RadSiteClass*, pThis, ECX);
	auto pRadExt = RadSiteExt::ExtMap.Find(pThis);

	RadSiteExt::ExtMap.Remove(pThis);
	RadSiteExt::Array.Remove(pRadExt);

	return 0;
}

DEFINE_HOOK_AGAIN(65B3D0, RadSiteClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(65B450, RadSiteClass_SaveLoad_Prefix, 8)
{
	GET_STACK(RadSiteClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	RadSiteExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(65B43F, RadSiteClass_Load_Suffix, 7)
{
	RadSiteExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(65B464, RadSiteClass_Save_Suffix, 5)
{
	RadSiteExt::ExtMap.SaveStatic();

	return 0;
}