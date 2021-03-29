#include "Body.h"
#include "../Side/Body.h"
#include "../../Enum/RadTypes.h"
#include "../../Utilities/TemplateDef.h"
#include <FPSCounter.h>
#include <GameOptionsClass.h>

template<> const DWORD Extension<RulesClass>::Canary = 0x12341234;
std::unique_ptr<RulesExt::ExtData> RulesExt::Data = nullptr;

void RulesExt::Allocate(RulesClass* pThis) {
	Data = std::make_unique<RulesExt::ExtData>(pThis);
}

void RulesExt::Remove(RulesClass* pThis) {
	Data = nullptr;
}

void RulesExt::LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI) {
	Data->LoadFromINI(pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI) {

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI) {
	if (pINI == CCINIClass::INI_Rules) {
		Data->InitializeAfterTypeData(pThis);
	}
	Data->LoadAfterTypeData(pThis, pINI);
}

void RulesExt::ExtData::InitializeConstants() {

}

void RulesExt::ExtData::LoadFromINIFile(CCINIClass* pINI) {
	// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI) {

	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData) {
		return;
	}

	INI_EX exINI(pINI);

	this->Shield_Pip.Read(exINI, "AudioVisual", "Shield.Pip");
}

// this runs between the before and after type data loading methods for rules ini
void RulesExt::ExtData::InitializeAfterTypeData(RulesClass* const pThis) {
	
}

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI) {
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData) {
		return;
	}

	INI_EX exINI(pINI);

}

bool RulesExt::DetailsCurrentlyEnabled()
{
	// not only checks for the min frame rate from the rules, but also whether
	// the low frame rate is actually desired. in that case, don't reduce.
	auto const current = FPSCounter::CurrentFrameRate;
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
void RulesExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Shield_Pip)
		;
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void RulesExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<RulesClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool RulesExt::LoadGlobals(PhobosStreamReader& Stm) {


	return Stm.Success();
}

bool RulesExt::SaveGlobals(PhobosStreamWriter& Stm) {


	return Stm.Success();
}

// =============================
// container hooks

DEFINE_HOOK(667A1D, RulesClass_CTOR, 5) {
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);
	return 0;
}

DEFINE_HOOK(667A30, RulesClass_DTOR, 5) {
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);
	return 0;
}

IStream* g_pStm = nullptr;

DEFINE_HOOK_AGAIN(674730, RulesClass_SaveLoad_Prefix, 6)
DEFINE_HOOK(675210, RulesClass_SaveLoad_Prefix, 5)
{
	//GET(RulesClass*, pItem, ECX);
	GET_STACK(IStream*, pStm, 0x4);

	g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(678841, RulesClass_Load_Suffix, 7)
{
	auto buffer = RulesExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(g_pStm)) {
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(RulesExt::ExtData::Canary) && Reader.RegisterChange(buffer)) {
			buffer->LoadFromStream(Reader);
		}
	}

	return 0;
}

DEFINE_HOOK(675205, RulesClass_Save_Suffix, 8)
{
	auto buffer = RulesExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(RulesExt::ExtData::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(g_pStm);

	return 0;
}
