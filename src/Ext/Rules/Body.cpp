#include "Body.h"
#include <Ext/Side/Body.h>
#include <Utilities/TemplateDef.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>

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
	DigitalDisplayTypeClass::LoadFromINIList(pINI);
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

	this->Storage_TiberiumIndex.Read(exINI, GameStrings::General, "Storage.TiberiumIndex");
	this->InfantryGainSelfHealCap.Read(exINI, GameStrings::General, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GameStrings::General, "UnitsGainSelfHealCap");
	this->EnemyInsignia.Read(exINI, GameStrings::General, "EnemyInsignia");
	this->DisguiseBlinkingVisibility.Read(exINI, GameStrings::General, "DisguiseBlinkingVisibility");
	this->ChronoSparkleDisplayDelay.Read(exINI, GameStrings::General, "ChronoSparkleDisplayDelay");
	this->ChronoSparkleBuildingDisplayPositions.Read(exINI, GameStrings::General, "ChronoSparkleBuildingDisplayPositions");
	this->UseGlobalRadApplicationDelay.Read(exINI, GameStrings::Radiation, "UseGlobalRadApplicationDelay");
	this->RadApplicationDelay_Building.Read(exINI, GameStrings::Radiation, "RadApplicationDelay.Building");
	this->RadWarhead_Detonate.Read(exINI, GameStrings::Radiation, "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, GameStrings::Radiation, "RadHasOwner");
	this->RadHasInvoker.Read(exINI, GameStrings::Radiation, "RadHasInvoker");
	this->MissingCameo.Read(pINI, GameStrings::AudioVisual, "MissingCameo");

	this->PlacementPreview.Read(exINI, GameStrings::AudioVisual, "PlacementPreview");
	this->PlacementPreview_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementPreview.Translucency");
	this->PlacementGrid_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.Translucency");
	this->Pips_Shield.Read(exINI, GameStrings::AudioVisual, "Pips.Shield");
	this->Pips_Shield_Background.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Building.Empty");
	this->Pips_SelfHeal_Infantry.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Buildings");
	this->Pips_SelfHeal_Infantry_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Infantry.Offset");
	this->Pips_SelfHeal_Units_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Units.Offset");
	this->Pips_SelfHeal_Buildings_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Buildings.Offset");
	this->Pips_Generic_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Size");
	this->Pips_Generic_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Buildings.Size");
	this->Pips_Ammo_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Size");
	this->Pips_Ammo_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Buildings.Size");
	this->Pips_Tiberiums_Frames.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.Frames");
	this->Pips_Tiberiums_DisplayOrder.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.DisplayOrder");
	this->ToolTip_Background_Color.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.BlurSize");
	this->RadialIndicatorVisibility.Read(exINI, GameStrings::AudioVisual, "RadialIndicatorVisibility");
	this->DrawTurretShadow.Read(exINI, GameStrings::AudioVisual, "DrawTurretShadow");
	this->AnimRemapDefaultColorScheme.Read(exINI, GameStrings::AudioVisual, "AnimRemapDefaultColorScheme");
	this->TimerBlinkColorScheme.Read(exINI, GameStrings::AudioVisual, "TimerBlinkColorScheme");
	this->ShowDesignatorRange.Read(exINI, GameStrings::AudioVisual, "ShowDesignatorRange");

	this->AllowParallelAIQueues.Read(exINI, "GlobalControls", "AllowParallelAIQueues");
	this->ForbidParallelAIQueues_Aircraft.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Building.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Building");
	this->ForbidParallelAIQueues_Infantry.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Navy.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Navy");
	this->ForbidParallelAIQueues_Vehicle.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Vehicle");

	this->IronCurtain_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage, "IronCurtain.KeptOnDeploy");
	this->IronCurtain_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "IronCurtain.EffectOnOrganics");
	this->IronCurtain_KillOrganicsWarhead.Read(exINI, GameStrings::CombatDamage, "IronCurtain.KillOrganicsWarhead");

	this->CrateOnlyOnLand.Read(exINI, GameStrings::CrateRules, "CrateOnlyOnLand");

	this->ROF_RandomDelay.Read(exINI, GameStrings::CombatDamage, "ROF.RandomDelay");

	this->DisplayIncome.Read(exINI, GameStrings::AudioVisual, "DisplayIncome");
	this->DisplayIncome_Houses.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.Houses");
	this->DisplayIncome_AllowAI.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.AllowAI");

	this->IsVoiceCreatedGlobal.Read(exINI, GameStrings::AudioVisual, "IsVoiceCreatedGlobal");

	this->Buildings_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Buildings.DefaultDigitalDisplayTypes");
	this->Infantry_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Infantry.DefaultDigitalDisplayTypes");
	this->Vehicles_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Vehicles.DefaultDigitalDisplayTypes");
	this->Aircraft_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Aircraft.DefaultDigitalDisplayTypes");

	if (HugeBar_Config.empty())
	{
		this->HugeBar_Config.emplace_back(new HugeBarData(DisplayInfoType::Health));
		this->HugeBar_Config.emplace_back(new HugeBarData(DisplayInfoType::Shield));
	}

	this->HugeBar_Config[0]->LoadFromINI(pINI);
	this->HugeBar_Config[1]->LoadFromINI(pINI);

	// Section AITargetTypes
	int itemsCount = pINI->GetKeyCount(sectionAITargetTypes);
	for (int i = 0; i < itemsCount; ++i)
	{
		std::vector<TechnoTypeClass*> objectsList;
		char* context = nullptr;
		pINI->ReadString(sectionAITargetTypes, pINI->GetKeyName(sectionAITargetTypes, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			TechnoTypeClass* buffer;
			if (Parser<TechnoTypeClass*>::TryParse(cur, &buffer))
				objectsList.emplace_back(buffer);
			else
				Debug::Log("[Developer warning] AITargetTypes (Count: %d): Error parsing [%s]\n", this->AITargetTypesLists.size(), cur);
		}

		this->AITargetTypesLists.emplace_back(std::move(objectsList));
	}

	// Section AIScriptsList
	int scriptitemsCount = pINI->GetKeyCount(sectionAIScriptsList);
	for (int i = 0; i < scriptitemsCount; ++i)
	{
		std::vector<ScriptTypeClass*> objectsList;

		char* context = nullptr;
		pINI->ReadString(sectionAIScriptsList, pINI->GetKeyName(sectionAIScriptsList, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			ScriptTypeClass* pNewScript = ScriptTypeClass::FindOrAllocate(cur);
			objectsList.emplace_back(pNewScript);
		}

		this->AIScriptsLists.emplace_back(std::move(objectsList));
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


RulesExt::ExtData::HugeBarData::HugeBarData(DisplayInfoType infoType)
	: HugeBar_RectWidthPercentage(0.82)
	, HugeBar_RectWH({ -1, 30 })
	, HugeBar_Frame(-1)
	, HugeBar_Pips_Frame(-1)
	, HugeBar_Pips_Num(100)
	, Value_Shape_Spacing(8)
	, Value_Num_BaseFrame(0)
	, Value_Sign_BaseFrame(30)
	, DisplayValue(true)
	, Anchor(HorizontalPosition::Center, VerticalPosition::Top)
	, InfoType(infoType)
	, VisibleToHouses(AffectedHouse::All)
	, VisibleToHouses_Observer(true)
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
	this->HugeBar_Pips_Spacing.Read(exINI, section, "HugeBar.Pips.Spacing");

	this->HugeBar_Offset.Read(exINI, section, "HugeBar.Offset");
	this->HugeBar_Pips_Offset.Read(exINI, section, "HugeBar.Pips.Offset");
	this->HugeBar_Pips_Num.Read(exINI, section, "HugeBar.Pips.Num");

	this->Value_Text_Color.Read(exINI, section, "Value.Text.Color.%s");

	this->Value_Shape.Read(exINI, section, "Value.Shape");
	this->Value_Palette.LoadFromINI(pINI, section, "Value.Palette");
	this->Value_Num_BaseFrame.Read(exINI, section, "Value.Num.BaseFrame");
	this->Value_Sign_BaseFrame.Read(exINI, section, "Value.Sign.BaseFrame");
	this->Value_Shape_Spacing.Read(exINI, section, "Value.Shape.Spacing");

	this->DisplayValue.Read(exINI, section, "DisplayValue");
	this->Value_Offset.Read(exINI, section, "Value.Offset");
	this->Value_Percentage.Read(exINI, section, "Value.Percentage");
	this->Anchor.Read(exINI, section, "Anchor.%s");

	this->VisibleToHouses.Read(exINI, section, "VisibleToHouses");
	this->VisibleToHouses_Observer.Read(exINI, section, "VisibleToHouses.Observer");
}

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->HarvesterTypes)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->EnemyInsignia)
		.Process(this->DisguiseBlinkingVisibility)
		.Process(this->ChronoSparkleDisplayDelay)
		.Process(this->ChronoSparkleBuildingDisplayPositions)
		.Process(this->UseGlobalRadApplicationDelay)
		.Process(this->RadApplicationDelay_Building)
		.Process(this->RadWarhead_Detonate)
		.Process(this->RadHasOwner)
		.Process(this->RadHasInvoker)
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->MissingCameo)
		.Process(this->PlacementGrid_Translucency)
		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Translucency)
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
		.Process(this->Pips_Generic_Size)
		.Process(this->Pips_Generic_Buildings_Size)
		.Process(this->Pips_Ammo_Size)
		.Process(this->Pips_Ammo_Buildings_Size)
		.Process(this->Pips_Tiberiums_Frames)
		.Process(this->Pips_Tiberiums_DisplayOrder)
		.Process(this->AllowParallelAIQueues)
		.Process(this->ForbidParallelAIQueues_Aircraft)
		.Process(this->ForbidParallelAIQueues_Building)
		.Process(this->ForbidParallelAIQueues_Infantry)
		.Process(this->ForbidParallelAIQueues_Navy)
		.Process(this->ForbidParallelAIQueues_Vehicle)
		.Process(this->IronCurtain_EffectOnOrganics)
		.Process(this->IronCurtain_KillOrganicsWarhead)
		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->ROF_RandomDelay)
		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)
		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_AllowAI)
		.Process(this->DisplayIncome_Houses)
		.Process(this->CrateOnlyOnLand)
		.Process(this->RadialIndicatorVisibility)
		.Process(this->DrawTurretShadow)
		.Process(this->IsVoiceCreatedGlobal)
		.Process(this->AnimRemapDefaultColorScheme)
		.Process(this->TimerBlinkColorScheme)
		.Process(this->Buildings_DefaultDigitalDisplayTypes)
		.Process(this->Infantry_DefaultDigitalDisplayTypes)
		.Process(this->Vehicles_DefaultDigitalDisplayTypes)
		.Process(this->Aircraft_DefaultDigitalDisplayTypes)
		.Process(this->HugeBar_Config)
		.Process(this->ShowDesignatorRange)
		;
}

template <typename T>
bool RulesExt::ExtData::HugeBarData::Serialize(T& stm)
{
	return stm
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
		.Process(this->HugeBar_Pips_Spacing)

		.Process(this->HugeBar_Offset)
		.Process(this->HugeBar_Pips_Offset)
		.Process(this->HugeBar_Pips_Num)

		.Process(this->Value_Text_Color)

		.Process(this->Value_Shape)
		.Process(this->Value_Palette)
		.Process(this->Value_Num_BaseFrame)
		.Process(this->Value_Sign_BaseFrame)
		.Process(this->Value_Shape_Spacing)

		.Process(this->DisplayValue)
		.Process(this->Value_Offset)
		.Process(this->Value_Percentage)
		.Process(this->Anchor)
		.Process(this->InfoType)

		.Process(this->VisibleToHouses)
		.Process(this->VisibleToHouses_Observer)

		.Success()
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

bool RulesExt::ExtData::HugeBarData::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool RulesExt::ExtData::HugeBarData::Save(PhobosStreamWriter& stm) const
{
	return const_cast<HugeBarData*>(this)->Serialize(stm);
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

		if (Reader.Expect(RulesExt::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(RulesExt::Canary);
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
