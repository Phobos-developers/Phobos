#include "AresFunctions.h"
#include "AresHelper.h"
#include "Patch.h"
#include "Template.h"

#include <AlphaShapeClass.h>
#include <HouseClass.h>
#include <HouseTypeClass.h>
#include <InfantryTypeClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>

#define NOTE_ARES_FUN(name,reladdr) AresFunctions::name = reinterpret_cast<decltype(AresFunctions::name)>(AresHelper::AresBaseAddress + reladdr)

decltype(AresFunctions::ConvertTypeTo) AresFunctions::ConvertTypeTo = nullptr;
decltype(AresFunctions::CreateAresEBolt) AresFunctions::CreateAresEBolt = nullptr;
decltype(AresFunctions::SpawnSurvivors) AresFunctions::SpawnSurvivors = nullptr;
decltype(AresFunctions::ReverseEngineer) AresFunctions::ReverseEngineer = nullptr;
decltype(AresFunctions::IsTargetConstraintsEligible) AresFunctions::IsTargetConstraintsEligible = nullptr;
decltype(AresFunctions::UnitDeliveryStateMachine_Update) AresFunctions::UnitDeliveryStateMachine_Update = nullptr;
decltype(AresFunctions::ApplyPermaMC) AresFunctions::ApplyPermaMC = nullptr;
decltype(AresFunctions::DetailsCurrentlyEnabled) AresFunctions::DetailsCurrentlyEnabled = nullptr;
decltype(AresFunctions::SendPDPlane) AresFunctions::SendPDPlane = nullptr;
std::function<AresSWTypeExtData* (SuperWeaponTypeClass*)> AresFunctions::SWTypeExtMap_Find;
PhobosMap<ObjectClass*, AlphaShapeClass*>* AresFunctions::AlphaExtMap = nullptr;

decltype(AresFunctions::GetTunnel) AresFunctions::GetTunnel = nullptr;
decltype(AresFunctions::AddPassengerFromTunnel) AresFunctions::AddPassengerFromTunnel = nullptr;

decltype(AresFunctions::FindEVAIndex) AresFunctions::FindEVAIndex = nullptr;

void* AresFunctions::_SWTypeExtMap = nullptr;
decltype(AresFunctions::_SWTypeExtMapFind) AresFunctions::_SWTypeExtMapFind = nullptr;

decltype(AresFunctions::FindAlphaShape) AresFunctions::FindAlphaShape = nullptr;
decltype(AresFunctions::GetDisableWeaponTimer) AresFunctions::GetDisableWeaponTimer = nullptr;
decltype(AresFunctions::GetDriverKilled) AresFunctions::GetDriverKilled = nullptr;
decltype(AresFunctions::IsPsionicsImmune) AresFunctions::IsPsionicsImmune = nullptr;
decltype(AresFunctions::IsVeteranBuilding) AresFunctions::IsVeteranBuilding = nullptr;
decltype(AresFunctions::GetInfiltrated) AresFunctions::GetInfiltrated = nullptr;
decltype(AresFunctions::GetOperators) AresFunctions::GetOperators = nullptr;

// The Ares extension layouts, in one place instead of scattered across call sites
// as DummyExtHere structs. Only the Ares backend may use these -- Antares is a
// different compile and answers the same questions through its table.
namespace AresLayout
{
	struct TechnoExtData
	{
		char _pad0[0x50];
		CDTimerClass DisableWeaponsTimer;
		char _pad1[0x40];
		bool DriverKilled;
	};

	struct TechnoTypeExtData
	{
		char _pad0[0xF4];
		ValueableVector<TechnoTypeClass*> Operators;
		bool Operator_Any;
		char _pad1[0x131 - 0xF4 - sizeof(ValueableVector<TechnoTypeClass*>) - 1];
		bool Vet_PsionicsImmune;
		char _pad2[0x6];
		bool Elite_PsionicsImmune;
	};

	struct HouseExtData
	{
		char _pad0[0x48];
		bool ShipYardInfiltrated;
		bool AirFieldInfiltrated;
		bool ConstructionYardInfiltrated;
	};

	struct HouseTypeExtData
	{
		char _pad0[0x15C];
		ValueableVector<BuildingTypeClass*> VeteranBuildings;
	};

	//! The extension pointer Ares stores on the object, as a typed view.
	template <typename T>
	T* Ext(uintptr_t stored)
	{
		return reinterpret_cast<T*>(stored);
	}

	//! Same, for classes whose slot is not a reachable member.
	template <typename T>
	T* ExtAt(void const* pObject, size_t offset)
	{
		return reinterpret_cast<T*>(*reinterpret_cast<uintptr_t const*>(
			reinterpret_cast<char const*>(pObject) + offset));
	}

	AlphaShapeClass* __stdcall FindAlphaShape(ObjectClass* pObject)
	{
		auto const pMap = AresFunctions::AlphaExtMap;
		return pMap ? pMap->get_or_default(pObject) : nullptr;
	}

	CDTimerClass* __stdcall GetDisableWeaponTimer(TechnoClass* pThis)
	{
		return pThis ? &Ext<TechnoExtData>(pThis->align_154)->DisableWeaponsTimer : nullptr;
	}

	bool* __stdcall GetDriverKilled(TechnoClass* pThis)
	{
		return pThis ? &Ext<TechnoExtData>(pThis->align_154)->DriverKilled : nullptr;
	}

	bool __stdcall IsPsionicsImmune(TechnoTypeClass* pType, VeterancyStruct const* pVeterancy)
	{
		if (!pType || !pVeterancy)
			return false;

		if (pType->ImmuneToPsionics)
			return true;

		auto const pExt = Ext<TechnoTypeExtData>(pType->align_2FC);

		// Ranks accumulate: an elite unit still has whatever the veteran set granted,
		// so this falls through deliberately. Antares answers the same way, through
		// its ability set.
		switch (pVeterancy->GetRemainingLevel())
		{
		case Rank::Elite:
			if (pExt->Elite_PsionicsImmune)
				return true;
			[[fallthrough]];

		case Rank::Veteran:
			if (pExt->Vet_PsionicsImmune)
				return true;
			break;

		default:
			break;
		}

		return false;
	}

	bool __stdcall IsVeteranBuilding(HouseTypeClass* pCountry, BuildingTypeClass* pType)
	{
		// HouseTypeClass' slot is protected, so it is reached by offset.
		return pCountry && pType
			&& ExtAt<HouseTypeExtData>(pCountry, 0xC4)->VeteranBuildings.Contains(pType);
	}

	bool* __stdcall GetInfiltrated(HouseClass* pHouse, AntaresFactory factory)
	{
		if (!pHouse)
			return nullptr;

		auto const pExt = Ext<HouseExtData>(pHouse->unknown_16084);

		switch (factory)
		{
		case AntaresFactory::WarFactory:       return &pHouse->WarFactoryInfiltrated;
		case AntaresFactory::Barracks:         return &pHouse->BarracksInfiltrated;
		case AntaresFactory::NavalYard:        return &pExt->ShipYardInfiltrated;
		case AntaresFactory::AircraftFactory:  return &pExt->AirFieldInfiltrated;
		case AntaresFactory::ConstructionYard: return &pExt->ConstructionYardInfiltrated;
		}

		return nullptr;
	}

	bool __stdcall GetOperators(TechnoTypeClass* pType, InfantryTypeClass* const** ppItems,
		int* pCount, bool* pAnyAllowed)
	{
		if (!pType)
			return false;

		auto const pExt = Ext<TechnoTypeExtData>(pType->align_2FC);

		// Ares types this as TechnoTypeClass*, Antares as InfantryTypeClass*; only
		// infantry can ever be an operator, so the reinterpret is safe either way.
		if (ppItems)
			*ppItems = reinterpret_cast<InfantryTypeClass* const*>(pExt->Operators.data());
		if (pCount)
			*pCount = static_cast<int>(pExt->Operators.size());
		if (pAnyAllowed)
			*pAnyAllowed = pExt->Operator_Any;

		return true;
	}

	//! Point the shared accessors at the Ares implementations above.
	void BindAccessors()
	{
		AresFunctions::FindAlphaShape = &FindAlphaShape;
		AresFunctions::GetDisableWeaponTimer = &GetDisableWeaponTimer;
		AresFunctions::GetDriverKilled = &GetDriverKilled;
		AresFunctions::IsPsionicsImmune = &IsPsionicsImmune;
		AresFunctions::IsVeteranBuilding = &IsVeteranBuilding;
		AresFunctions::GetInfiltrated = &GetInfiltrated;
		AresFunctions::GetOperators = &GetOperators;
	}
}

void Apply_Ares3_0_Patches();
void Apply_Ares3_0p1_Patches();

void AresFunctions::InitAres3_0()
{
	NOTE_ARES_FUN(ConvertTypeTo, 0x43650);

	NOTE_ARES_FUN(CreateAresEBolt, 0x550F0);

	// an issue occured with this fix enabled: sometimes survivor will be spawned at coordinate 0,0
	if constexpr (AresFunctions::AresWasWrongAboutSpawnSurvivors)
	{
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4C0EB, { 0x5C });
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x48C69, { 0x30 });
	}
	else
	{
		NOTE_ARES_FUN(SpawnSurvivors, 0x464C0);
	}

	NOTE_ARES_FUN(ReverseEngineer, 0x022360);

	NOTE_ARES_FUN(IsTargetConstraintsEligible, 0x032110);

	NOTE_ARES_FUN(UnitDeliveryStateMachine_Update, 0x075DE0);

	NOTE_ARES_FUN(ApplyPermaMC, 0x052CD0);

	NOTE_ARES_FUN(DetailsCurrentlyEnabled, 0x02A6C0);

	NOTE_ARES_FUN(SendPDPlane, 0x0741A0);

	NOTE_ARES_FUN(_SWTypeExtMapFind, 0x57C70);
	NOTE_ARES_FUN(_SWTypeExtMap, 0xC1C54);
	SWTypeExtMap_Find = [](SuperWeaponTypeClass* swt) { return _SWTypeExtMapFind(_SWTypeExtMap, swt); };

	AresLayout::BindAccessors();

	NOTE_ARES_FUN(AlphaExtMap, 0xC1924);

	// BuildingTypeExt
	NOTE_ARES_FUN(AresFunctions::GetTunnel, 0x0D740);
	NOTE_ARES_FUN(AresFunctions::AddPassengerFromTunnel, 0x09000);

	// VoxClass
	NOTE_ARES_FUN(AresFunctions::FindEVAIndex, 0x063560);

#ifndef USING_MULTIFINITE_SYRINGE
	Apply_Ares3_0_Patches();
#endif
}

void AresFunctions::InitAres3_0p1()
{
	NOTE_ARES_FUN(ConvertTypeTo, 0x44130);

	NOTE_ARES_FUN(CreateAresEBolt, 0x55DA0);

	// an issue occured with this fix enabled: sometimes survivor will be spawned at coordinate 0,0
	if constexpr (AresFunctions::AresWasWrongAboutSpawnSurvivors)
	{
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4CD4B, { 0x5C });
		Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x498B9, { 0x30 });
	}
	else
	{
		NOTE_ARES_FUN(SpawnSurvivors, 0x47030);
	}

	NOTE_ARES_FUN(ReverseEngineer, 0x022DE0);

	NOTE_ARES_FUN(IsTargetConstraintsEligible, 0x032AF0);

	NOTE_ARES_FUN(UnitDeliveryStateMachine_Update, 0x076E90);

	NOTE_ARES_FUN(ApplyPermaMC, 0x053980);

	NOTE_ARES_FUN(DetailsCurrentlyEnabled, 0x02B1C0);

	NOTE_ARES_FUN(SendPDPlane, 0x075250);

	NOTE_ARES_FUN(_SWTypeExtMapFind, 0x58900);
	NOTE_ARES_FUN(_SWTypeExtMap, 0xC2C50);
	SWTypeExtMap_Find = [](SuperWeaponTypeClass* swt) { return _SWTypeExtMapFind(_SWTypeExtMap, swt); };

	AresLayout::BindAccessors();

	NOTE_ARES_FUN(AlphaExtMap, 0xC2988);

	// BuildingTypeExt
	NOTE_ARES_FUN(AresFunctions::GetTunnel, 0x0DA30);
	NOTE_ARES_FUN(AresFunctions::AddPassengerFromTunnel, 0x09040);

	// VoxClass
	NOTE_ARES_FUN(AresFunctions::FindEVAIndex, 0x0642B0);

#ifndef USING_MULTIFINITE_SYRINGE
	Apply_Ares3_0p1_Patches();
#endif
}

#undef NOTE_ARES_FUN

void AresFunctions::InitAntares()
{
	auto const api = AresHelper::Antares;

	if (!api)
		return;

	ConvertTypeTo = api->ConvertTypeTo;
	SpawnSurvivors = api->SpawnSurvivors;
	DetailsCurrentlyEnabled = api->DetailsCurrentlyEnabled;
	FindEVAIndex = api->FindEVAIndex;
	AddPassengerFromTunnel = reinterpret_cast<decltype(AddPassengerFromTunnel)>(api->AddTunnelPassenger);

	FindAlphaShape = api->FindAlphaShape;
	GetDisableWeaponTimer = api->GetDisableWeaponTimer;
	GetDriverKilled = api->GetDriverKilled;
	IsPsionicsImmune = api->IsPsionicsImmune;
	IsVeteranBuilding = api->IsVeteranBuilding;
	GetInfiltrated = api->GetInfiltrated;
	GetOperators = api->GetOperators;

	// Antares takes the superweapon type directly, so the two-step lookup Ares needs
	// collapses: hand the type through unchanged and let the constraint check take it.
	SWTypeExtMap_Find = [](SuperWeaponTypeClass* swt)
		{ return reinterpret_cast<AresSWTypeExtData*>(swt); };

	IsTargetConstraintsEligible = reinterpret_cast<decltype(IsTargetConstraintsEligible)>(
		api->MeetsAITargetingConstraints);

	// Deliberately left null: CreateAresEBolt, ReverseEngineer, ApplyPermaMC,
	// SendPDPlane, UnitDeliveryStateMachine_Update and GetTunnel either exist only to
	// serve a patch into Ares' own code, or take an Ares extension pointer that has no
	// meaning here. Their call sites go through the accessors or check CanUseAres.

	// Take over the subsystems we reimplement, so Antares stops driving them and we
	// are not both writing the same registers.
	api->DisableFeature(AntaresFeature::EBolt);
	api->DisableFeature(AntaresFeature::AlphaImage);
}

void AresFunctions::InitNoAres()
{
	Patch::Apply_RAW(0x6CDE40, { 0xC2, 0x08, 0x00, 0x90, 0x67 });
}
