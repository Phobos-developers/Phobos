#include "Body.h"
#include <Ext/Side/Body.h>
#include <Utilities/TemplateDef.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>

template<> const DWORD Extension<RulesClass>::Canary = 0x12341234;
std::unique_ptr<RulesExt::ExtData> RulesExt::Data = nullptr;

void RulesExt::Allocate(RulesClass* pThis)
{
	Data = std::make_unique<RulesExt::ExtData>(pThis);
}

void RulesExt::Remove(RulesClass* pThis)
{
	Data = nullptr;
}

void RulesExt::LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINI(pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RadTypeClass::LoadFromINIList(pINI);
	ShieldTypeClass::LoadFromINIList(pINI);
	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	if (pINI == CCINIClass::INI_Rules)
		Data->InitializeAfterTypeData(pThis);

	Data->LoadAfterTypeData(pThis, pINI);
}

void RulesExt::ExtData::InitializeConstants()
{

}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{

}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	const char* sectionAITargetTypes = "AITargetTypes";
	const char* sectionAIScriptsList = "AIScriptsList";

	INI_EX exINI(pINI);

	this->Storage_TiberiumIndex.Read(exINI, GENERAL_SECTION, "Storage.TiberiumIndex");
	this->InfantryGainSelfHealCap.Read(exINI, GENERAL_SECTION, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GENERAL_SECTION, "UnitsGainSelfHealCap");
	this->JumpjetAllowLayerDeviation.Read(exINI, "JumpjetControls", "AllowLayerDeviation");
	this->RadApplicationDelay_Building.Read(exINI, "Radiation", "RadApplicationDelay.Building");
	this->RadWarhead_Detonate.Read(exINI, "Radiation", "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, "Radiation", "RadHasOwner");
	this->RadHasInvoker.Read(exINI, "Radiation", "RadHasInvoker");
	this->MissingCameo.Read(pINI, "AudioVisual", "MissingCameo");
	this->JumpjetTurnToTarget.Read(exINI, "JumpjetControls", "TurnToTarget");
	this->PlacementGrid_TranslucentLevel.Read(exINI, "AudioVisual", "BuildingPlacementGrid.TranslucentLevel");
	this->BuildingPlacementPreview_TranslucentLevel.Read(exINI, "AudioVisual", "BuildingPlacementPreview.DefaultTranslucentLevel");
	this->Pips_Shield.Read(exINI, "AudioVisual", "Pips.Shield");
	this->Pips_Shield_Background.Read(exINI, "AudioVisual", "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, "AudioVisual", "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, "AudioVisual", "Pips.Shield.Building.Empty");
	this->Pips_SelfHeal_Infantry.Read(exINI, "AudioVisual", "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, "AudioVisual", "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, "AudioVisual", "Pips.SelfHeal.Buildings");
	this->Pips_SelfHeal_Infantry_Offset.Read(exINI, "AudioVisual", "Pips.SelfHeal.Infantry.Offset");
	this->Pips_SelfHeal_Units_Offset.Read(exINI, "AudioVisual", "Pips.SelfHeal.Units.Offset");
	this->Pips_SelfHeal_Buildings_Offset.Read(exINI, "AudioVisual", "Pips.SelfHeal.Buildings.Offset");

	this->IronCurtain_KeptOnDeploy.Read(exINI, "CombatDamage", "IronCurtain.KeptOnDeploy");

	if (HugeBar_Config.empty())
	{
		this->HugeBar_Config.emplace_back(new HugeBarData(DisplayInfoType::Health));
		this->HugeBar_Config[0]->LoadFromINI(pINI);

		this->HugeBar_Config.emplace_back(new HugeBarData(DisplayInfoType::Shield));
		this->HugeBar_Config[1]->LoadFromINI(pINI);
	}

	// Section AITargetTypes
	int itemsCount = pINI->GetKeyCount(sectionAITargetTypes);
	for (int i = 0; i < itemsCount; ++i)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList;
		char* context = nullptr;
		pINI->ReadString(sectionAITargetTypes, pINI->GetKeyName(sectionAITargetTypes, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			TechnoTypeClass* buffer;
			if (Parser<TechnoTypeClass*>::TryParse(cur, &buffer))
				objectsList.AddItem(buffer);
			else
				Debug::Log("DEBUG: [AITargetTypes][%d]: Error parsing [%s]\n", AITargetTypesLists.Count, cur);
		}

		AITargetTypesLists.AddItem(objectsList);
		objectsList.Clear();
	}

	// Section AIScriptsList
	int scriptitemsCount = pINI->GetKeyCount(sectionAIScriptsList);
	for (int i = 0; i < scriptitemsCount; ++i)
	{
		DynamicVectorClass<ScriptTypeClass*> objectsList;

		char* context = nullptr;
		pINI->ReadString(sectionAIScriptsList, pINI->GetKeyName(sectionAIScriptsList, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			ScriptTypeClass* pNewScript = GameCreate<ScriptTypeClass>(cur);
			objectsList.AddItem(pNewScript);
		}

		AIScriptsLists.AddItem(objectsList);
		objectsList.Clear();
	}
}

// this runs between the before and after type data loading methods for rules ini
void RulesExt::ExtData::InitializeAfterTypeData(RulesClass* const pThis)
{

}

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	INI_EX exINI(pINI);
}

bool RulesExt::DetailsCurrentlyEnabled()
{
	// not only checks for the min frame rate from the rules, but also whether
	// the low frame rate is actually desired. in that case, don't reduce.
	auto const current = FPSCounter::CurrentFrameRate();
	auto const wanted = static_cast<unsigned int>(
		60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

	return current >= wanted || current >= Detail::GetMinFrameRate();
}

bool RulesExt::DetailsCurrentlyEnabled(int const minDetailLevel)
{
	return GameOptionsClass::Instance->DetailLevel >= minDetailLevel
		&& DetailsCurrentlyEnabled();
}

RulesExt::ExtData::HugeBarData::HugeBarData(DisplayInfoType infoType)
	: HugeBar_RectWidthPercentage(0.82)
	, HugeBar_RectWH({ -1, 30 })
	, HugeBar_Frame(-1)
	, HugeBar_Pips_Frame(-1)
	, HugeBar_Pips_Num(100)
	, Value_Shape_Interval(8)
	, Value_Num_BaseFrame(0)
	, Value_Sign_BaseFrame(30)
	, DisplayValue(true)
	, Anchor(HorizontalPosition::Center, VerticalPosition::Top)
	, InfoType(infoType)
{
	switch (infoType)
	{
	case DisplayInfoType::Health:
		HugeBar_Pips_Color1 = Damageable<ColorStruct>({ 0, 255, 0 }, { 255, 255, 0 }, { 255, 0, 0 });
		HugeBar_Pips_Color2 = Damageable<ColorStruct>({ 0, 216, 0 }, { 255, 180, 0 }, { 216, 0, 0 });
		Value_Text_Color = Damageable<ColorStruct>({ 0, 255, 0 }, { 255, 180, 0 }, { 255, 0, 0 });
		break;
	case DisplayInfoType::Shield:
		HugeBar_Pips_Color1 = Damageable<ColorStruct>({ 0, 0, 255 });
		HugeBar_Pips_Color2 = Damageable<ColorStruct>({ 0, 0, 216 });
		Value_Text_Color = Damageable<ColorStruct>({ 0, 0, 216 });
		break;
	default:
		break;
	}
}

// =============================
// load / save


template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->RadApplicationDelay_Building)
		.Process(this->RadWarhead_Detonate)
		.Process(this->RadHasOwner)
		.Process(this->RadHasInvoker)
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)
		.Process(this->PlacementGrid_TranslucentLevel)
		.Process(this->BuildingPlacementPreview_TranslucentLevel)
		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Background)
		.Process(this->Pips_Shield_Building)
		.Process(this->Pips_Shield_Building_Empty)
		.Process(this->Pips_SelfHeal_Infantry)
		.Process(this->Pips_SelfHeal_Units)
		.Process(this->Pips_SelfHeal_Buildings)
		.Process(this->Pips_SelfHeal_Infantry_Offset)
		.Process(this->Pips_SelfHeal_Units_Offset)
		.Process(this->Pips_SelfHeal_Buildings_Offset)
		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->HugeBar_Config)
		;
}

template <typename T>
void RulesExt::ExtData::HugeBarData::Serialize(T& stm)
{
	stm
		.Process(this->HugeBar_RectWidthPercentage)
		.Process(this->HugeBar_RectWH)
		.Process(this->HugeBar_Pips_Color1)
		.Process(this->HugeBar_Pips_Color2)

		.Process(this->HugeBar_Shape)
		.Process(this->HugeBar_Palette)
		.Process(this->HugeBar_Frame)
		.Process(this->HugeBar_Pips_Shape)
		.Process(this->HugeBar_Pips_Palette)
		.Process(this->HugeBar_Pips_Frame)
		.Process(this->HugeBar_Pips_Interval)

		.Process(this->HugeBar_Offset)
		.Process(this->HugeBar_Pips_Offset)
		.Process(this->HugeBar_Pips_Num)

		.Process(this->Value_Text_Color)

		.Process(this->Value_Shape)
		.Process(this->Value_Palette)
		.Process(this->Value_Num_BaseFrame)
		.Process(this->Value_Sign_BaseFrame)
		.Process(this->Value_Shape_Interval)

		.Process(this->DisplayValue)
		.Process(this->Value_Offset)
		.Process(this->Value_Percentage)
		.Process(this->Anchor)
		.Process(this->InfoType)
		;
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void RulesExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<RulesClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool RulesExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool RulesExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

void RulesExt::ExtData::HugeBarData::Load(PhobosStreamReader& stm)
{
	Serialize(stm);
}

void RulesExt::ExtData::HugeBarData::Save(PhobosStreamWriter& stm)
{
	Serialize(stm);
}

void RulesExt::ExtData::HugeBarData::LoadFromINI(CCINIClass* pINI)
{
	char typeName[0x20];

	switch (InfoType)
	{
	case DisplayInfoType::Health:
		strcpy_s(typeName, "Health");
		break;
	case DisplayInfoType::Shield:
		strcpy_s(typeName, "Shield");
		break;
	default:
		return;
	}

	char section[0x20];
	sprintf_s(section, "HugeBar_%s", typeName);
	INI_EX exINI(pINI);

	this->HugeBar_RectWidthPercentage.Read(exINI, section, "HugeBar.RectWidthPercentage");
	this->HugeBar_RectWH.Read(exINI, section, "HugeBar.RectWH");
	this->HugeBar_Pips_Color1.Read(exINI, section, "HugeBar.Pips.Color1.%s");
	this->HugeBar_Pips_Color2.Read(exINI, section, "HugeBar.Pips.Color2.%s");

	this->HugeBar_Shape.Read(exINI, section, "HugeBar.Shape");
	this->HugeBar_Palette.LoadFromINI(pINI, section, "HugeBar.Palette");
	this->HugeBar_Frame.Read(exINI, section, "HugeBar.Frame.%s");
	this->HugeBar_Pips_Shape.Read(exINI, section, "HugeBar.Pips.Shape");
	this->HugeBar_Pips_Palette.LoadFromINI(pINI, section, "HugeBar.Pips.Palette");
	this->HugeBar_Pips_Frame.Read(exINI, section, "HugeBar.Pips.Frame.%s");
	this->HugeBar_Pips_Interval.Read(exINI, section, "HugeBar.Pips.Interval");

	this->HugeBar_Offset.Read(exINI, section, "HugeBar.Offset");
	this->HugeBar_Pips_Offset.Read(exINI, section, "HugeBar.Pips.Offset");
	this->HugeBar_Pips_Num.Read(exINI, section, "HugeBar.Pips.Num");

	this->Value_Text_Color.Read(exINI, section, "Value.Text.Color.%s");

	this->Value_Shape.Read(exINI, section, "Value.Shape");
	this->Value_Palette.LoadFromINI(pINI, section, "Value.Palette");
	this->Value_Num_BaseFrame.Read(exINI, section, "Value.Num.BaseFrame");
	this->Value_Sign_BaseFrame.Read(exINI, section, "Value.Sign.BaseFrame");
	this->Value_Shape_Interval.Read(exINI, section, "Value.Shape.Interval");

	this->DisplayValue.Read(exINI, section, "DisplayValue");
	this->Value_Offset.Read(exINI, section, "Value.Offset");
	this->Value_Percentage.Read(exINI, section, "Value.Percentage");
	this->Anchor.Read(exINI, section, "Anchor.%s");
}

// =============================
// container hooks

DEFINE_HOOK(0x667A1D, RulesClass_CTOR, 0x5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x667A30, RulesClass_DTOR, 0x5)
{
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);

	return 0;
}

IStream* RulesExt::g_pStm = nullptr;

DEFINE_HOOK_AGAIN(0x674730, RulesClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x675210, RulesClass_SaveLoad_Prefix, 0x5)
{
	//GET(RulesClass*, pItem, ECX);
	GET_STACK(IStream*, pStm, 0x4);

	RulesExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x678841, RulesClass_Load_Suffix, 0x7)
{
	auto buffer = RulesExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(RulesExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(RulesExt::ExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(RulesExt::ExtData::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(RulesExt::g_pStm);

	return 0;
}

// DEFINE_HOOK(0x52D149, InitRules_PostInit, 0x5)
// {
// 	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
// 	return 0;
// }

DEFINE_HOOK(0x668BF0, RulesClass_Addition, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = false;
	RulesExt::LoadFromINIFile(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679A15, RulesData_LoadBeforeTypeData, 0x6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = true;
	RulesExt::LoadBeforeTypeData(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679CAF, RulesData_LoadAfterTypeData, 0x5)
{
	RulesClass* pItem = RulesClass::Instance();
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterTypeData(pItem, pINI);

	return 0;
}
