#include "Body.h"
#include <TechnoTypeClass.h>
#include <StringTable.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x11111111;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

// =============================
// load / save

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection)) {
		return;
	}

	INI_EX exINI(pINI);

	this->Deployed_RememberTarget.Read(exINI, pSection, "Deployed.RememberTarget");
	this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
	this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");
	this->Interceptor.Read(exINI, pSection, "Interceptor");
	this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.GuardRange");
	this->Interceptor_EliteGuardRange.Read(exINI, pSection, "Interceptor.EliteGuardRange");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	//Art tags
	INI_EX exArtINI(CCINIClass::INI_Art);

	this->TurretOffset.Read(exArtINI, pThis->ImageFile, "TurretOffset");
}

void TechnoTypeExt::ExtData::LoadFromStream(IStream* Stm) {
	this->Deployed_RememberTarget.Load(Stm);
	this->HealthBar_Hide.Load(Stm);
	this->UIDescription.Load(Stm);
	this->LowSelectionPriority.Load(Stm);
	this->MindControlRangeLimit.Load(Stm);
	this->Interceptor.Load(Stm);
	this->Interceptor_GuardRange.Load(Stm);
	this->Interceptor_EliteGuardRange.Load(Stm);
	PhobosStreamReader::Process(Stm, this->GroupAs);
	this->TurretOffset.Load(Stm);
	this->Powered_KillSpawns.Load(Stm);
}

void TechnoTypeExt::ExtData::SaveToStream(IStream* Stm) const {
	this->Deployed_RememberTarget.Save(Stm);
	this->HealthBar_Hide.Save(Stm);
	this->UIDescription.Save(Stm);
	this->LowSelectionPriority.Save(Stm);
	this->MindControlRangeLimit.Save(Stm);
	this->Interceptor.Save(Stm);
	this->Interceptor_GuardRange.Save(Stm);
	this->Interceptor_EliteGuardRange.Save(Stm);
	PhobosStreamWriter::Process(Stm, this->GroupAs);
	this->TurretOffset.Save(Stm);
	this->Powered_KillSpawns.Save(Stm);
}

// =============================
// container

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") {
}

TechnoTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(711835, TechnoTypeClass_CTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(711AE0, TechnoTypeClass_DTOR, 5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(716DC0, TechnoTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(7162F0, TechnoTypeClass_SaveLoad_Prefix, 6)
{
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(716DAC, TechnoTypeClass_Load_Suffix, A)
{
	auto pItem = TechnoTypeExt::ExtMap.Find(TechnoTypeExt::ExtMap.SavingObject);
	IStream* pStm = TechnoTypeExt::ExtMap.SavingStream;

	pItem->LoadFromStream(pStm);
	return 0;
}

DEFINE_HOOK(717094, TechnoTypeClass_Save_Suffix, 5)
{
	auto pItem = TechnoTypeExt::ExtMap.Find(TechnoTypeExt::ExtMap.SavingObject);
	IStream* pStm = TechnoTypeExt::ExtMap.SavingStream;

	pItem->SaveToStream(pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(716132, TechnoTypeClass_LoadFromINI, 5)
DEFINE_HOOK(716123, TechnoTypeClass_LoadFromINI, 5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

void TechnoTypeExt::ExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	float x = static_cast<float>(this->TurretOffset.GetEx()->X * factor);
	float y = static_cast<float>(this->TurretOffset.GetEx()->Y * factor);
	float z = static_cast<float>(this->TurretOffset.GetEx()->Z * factor);

	mtx->Translate(x, y, z);
}

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
{
	auto ext = TechnoTypeExt::ExtMap.Find(pType);

	ext->ApplyTurretOffset(mtx, factor);
}


// Ares 0.A source

const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->OwnerObject()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType))) {
		return pExt->GetSelectionGroupID();
	}

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);
	return (_strcmpi(id, pID) == 0);
}
