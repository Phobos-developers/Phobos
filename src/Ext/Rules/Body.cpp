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
	const char* sectionAudioVisual = "AudioVisual";
	const char* sectionShowValue = "ShowValue";

	INI_EX exINI(pINI);

	this->Storage_TiberiumIndex.Read(exINI, GENERAL_SECTION, "Storage.TiberiumIndex");
	this->RadApplicationDelay_Building.Read(exINI, "Radiation", "RadApplicationDelay.Building");
	this->Pips_Shield.Read(exINI, "AudioVisual", "Pips.Shield");
	this->Pips_Shield_Buildings.Read(exINI, "AudioVisual", "Pips.Shield.Building");
	this->MissingCameo.Read(pINI, "AudioVisual", "MissingCameo");
	this->JumpjetAllowLayerDeviation.Read(exINI, "JumpjetControls", "AllowLayerDeviation");

	this->Buildings_ShowHP.Read(exINI, sectionShowValue, "Buildings.ShowHP");
	this->Buildings_ShowShield.Read(exINI, sectionShowValue, "Buildings.ShowShield");
	this->Buildings_ShowColorHPHigh.Read(exINI, sectionShowValue, "Buildings.ShowColorHPHigh");
	this->Buildings_ShowColorHPMid.Read(exINI, sectionShowValue, "Buildings.ShowColorHPMid");
	this->Buildings_ShowColorHPLow.Read(exINI, sectionShowValue, "Buildings.ShowColorHPLow");
	this->Buildings_ShowColorShieldHigh.Read(exINI, sectionShowValue, "Buildings.ShowColorShieldHigh");
	this->Buildings_ShowColorShieldMid.Read(exINI, sectionShowValue, "Buildings.ShowColorShieldMid");
	this->Buildings_ShowColorShieldLow.Read(exINI, sectionShowValue, "Buildings.ShowColorShieldLow");
	this->Buildings_ShowHPOffset.Read(exINI, sectionShowValue, "Buildings.ShowHPOffset");
	this->Buildings_ShowShieldOffset.Read(exINI, sectionShowValue, "Buildings.ShowShieldOffset");
	this->Buildings_ShowBackground.Read(exINI, sectionShowValue, "Buildings.ShowBackground");
	this->Buildings_ShowHPOffset_WithoutShield.Read(exINI, sectionShowValue, "Buildings.ShowHPOffset.WithoutShield");
	this->Units_ShowHP.Read(exINI, sectionShowValue, "Units.ShowHP");
	this->Units_ShowShield.Read(exINI, sectionShowValue, "Units.ShowShield");
	this->Units_ShowColorHPHigh.Read(exINI, sectionShowValue, "Units.ShowColorHPHigh");
	this->Units_ShowColorHPMid.Read(exINI, sectionShowValue, "Units.ShowColorHPMid");
	this->Units_ShowColorHPLow.Read(exINI, sectionShowValue, "Units.ShowColorHPLow");
	this->Units_ShowColorShieldHigh.Read(exINI, sectionShowValue, "Units.ShowColorShieldHigh");
	this->Units_ShowColorShieldMid.Read(exINI, sectionShowValue, "Units.ShowColorShieldMid");
	this->Units_ShowColorShieldLow.Read(exINI, sectionShowValue, "Units.ShowColorShieldLow");
	this->Units_ShowHPOffset.Read(exINI, sectionShowValue, "Units.ShowHPOffset");
	this->Units_ShowShieldOffset.Read(exINI, sectionShowValue, "Units.ShowShieldOffset");
	this->Units_ShowBackground.Read(exINI, sectionShowValue, "Units.ShowBackground");
	this->Units_ShowHPOffset_WithoutShield.Read(exINI, sectionShowValue, "Units.ShowHPOffset.WithoutShield");
	this->Buildings_UseSHPShowHP.Read(exINI, sectionShowValue, "Buildings.UseSHPShowHP");
	this->Buildings_UseSHPShowShield.Read(exINI, sectionShowValue, "Buildings.UseSHPShowShield");
	this->Buildings_HPNumberSHP.Read(pINI, sectionShowValue, "Buildings.HPNumberSHP");
	this->Buildings_HPNumberPAL.Read(pINI, sectionShowValue, "Buildings.HPNumberPAL");
	this->Buildings_ShieldNumberSHP.Read(pINI, sectionShowValue, "Buildings.ShieldNumberSHP");
	this->Buildings_ShieldNumberPAL.Read(pINI, sectionShowValue, "Buildings.ShieldNumberPAL");
	this->Buildings_HPNumberInterval.Read(exINI, sectionShowValue, "Buildings.HPNumberInterval");
	this->Buildings_ShieldNumberInterval.Read(exINI, sectionShowValue, "Buildings,ShieldNumberInterval");
	this->Units_UseSHPShowHP.Read(exINI, sectionShowValue, "Units.UseSHPShowHP");
	this->Units_UseSHPShowShield.Read(exINI, sectionShowValue, "Units.UseSHPShowShield");
	this->Units_HPNumberSHP.Read(pINI, sectionShowValue, "Units.HPNumberSHP");
	this->Units_HPNumberPAL.Read(pINI, sectionShowValue, "Units.HPNumberPAL");
	this->Units_ShieldNumberSHP.Read(pINI, sectionShowValue, "Units.ShieldNumberSHP");
	this->Units_ShieldNumberPAL.Read(pINI, sectionShowValue, "Units.ShieldNumberPAL");
	this->Units_HPNumberInterval.Read(exINI, sectionShowValue, "Units.HPNumberInterval");
	this->Units_ShieldNumberInterval.Read(exINI, sectionShowValue, "Units.ShieldNumberInterval");

	// Section AITargetTypes
	int itemsCount = pINI->GetKeyCount(sectionAITargetTypes);
	for (int i = 0; i < itemsCount; ++i)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList;
		char* context = nullptr;
		pINI->ReadString(sectionAITargetTypes, pINI->GetKeyName(sectionAITargetTypes, i), "", Phobos::readBuffer);

		for (char *cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
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

		for (char *cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
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

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Buildings)
		.Process(this->RadApplicationDelay_Building)
		.Process(this->MissingCameo)
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->Buildings_ShowHP)
		.Process(this->Buildings_ShowShield)
		.Process(this->Buildings_ShowColorHPHigh)
		.Process(this->Buildings_ShowColorHPMid)
		.Process(this->Buildings_ShowColorHPLow)
		.Process(this->Buildings_ShowColorShieldHigh)
		.Process(this->Buildings_ShowColorShieldMid)
		.Process(this->Buildings_ShowColorShieldLow)
		.Process(this->Buildings_ShowHPOffset)
		.Process(this->Buildings_ShowShieldOffset)
		.Process(this->Buildings_ShowBackground)
		.Process(this->Buildings_ShowHPOffset_WithoutShield)
		.Process(this->Units_ShowHP)
		.Process(this->Units_ShowShield)
		.Process(this->Units_ShowColorHPHigh)
		.Process(this->Units_ShowColorHPMid)
		.Process(this->Units_ShowColorHPLow)
		.Process(this->Units_ShowColorShieldHigh)
		.Process(this->Units_ShowColorShieldMid)
		.Process(this->Units_ShowColorShieldLow)
		.Process(this->Units_ShowHPOffset)
		.Process(this->Units_ShowShieldOffset)
		.Process(this->Units_ShowBackground)
		.Process(this->Units_ShowHPOffset_WithoutShield)
		.Process(this->Buildings_UseSHPShowHP)
		.Process(this->Buildings_UseSHPShowShield)
		.Process(this->Buildings_HPNumberSHP)
		.Process(this->Buildings_HPNumberPAL)
		.Process(this->Buildings_ShieldNumberSHP)
		.Process(this->Buildings_ShieldNumberPAL)
		.Process(this->Buildings_HPNumberInterval)
		.Process(this->Buildings_ShieldNumberInterval)
		.Process(this->Units_UseSHPShowHP)
		.Process(this->Units_UseSHPShowShield)
		.Process(this->Units_HPNumberSHP)
		.Process(this->Units_HPNumberPAL)
		.Process(this->Units_ShieldNumberSHP)
		.Process(this->Units_ShieldNumberPAL)
		.Process(this->Units_HPNumberInterval)
		.Process(this->Units_ShieldNumberInterval)
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
