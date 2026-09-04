#include <Ext/LaserDraw/Body.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

std::unordered_map<ObjectClass*, std::vector<LaserDrawClass*>> LaserDrawExt::ShooterToLasers;
std::unordered_map<ObjectClass*, std::vector<LaserDrawClass*>> LaserDrawExt::TargetToLasers;

LaserDrawExt::ExtContainer LaserDrawExt::ExtMap;

void LaserDrawExt::ExtData::Initialize(TechnoClass* pShooter, AbstractClass* pTarget, int weaponIdx, PositionFollow mode,
	const CoordStruct& initialSource, const CoordStruct& localFLH, int burstIndex, bool stopOnFirerConvert)
{
	// reset any previous tracking state (an existing laser may be re-tracked)
	this->Shooter = nullptr;
	this->Target = nullptr;
	this->WeaponIndex = 0;
	this->FollowMode = PositionFollow::None;
	this->SavedOffset = CoordStruct::Empty;
	this->LocalFLH = CoordStruct::Empty;
	this->FrozenBurstIndex = 0;
	this->StopOnFirerConvert = false;
	this->OriginalType = nullptr;

	const auto pShooterBuilding = abstract_cast<BuildingClass*>(pShooter);

	if (pShooterBuilding && pShooterBuilding->Type->MaxNumberOccupants > 0)
		mode &= ~PositionFollow::Firer;

	if (pShooter && (mode & PositionFollow::Firer))
	{
		this->Shooter = pShooter;
		this->LocalFLH = localFLH;
		this->FrozenBurstIndex = burstIndex;
		this->StopOnFirerConvert = stopOnFirerConvert;

		if (stopOnFirerConvert)
			this->OriginalType = pShooter->GetTechnoType();

		const int savedBurstIndex = pShooter->CurrentBurstIndex;
		pShooter->CurrentBurstIndex = burstIndex;
		const CoordStruct worldFLH = pShooter->GetFLH(weaponIdx, CoordStruct::Empty);
		pShooter->CurrentBurstIndex = savedBurstIndex;

		this->SavedOffset = initialSource - worldFLH;
	}

	if (mode & PositionFollow::Target)
		this->Target = abstract_cast<ObjectClass*>(pTarget);

	this->WeaponIndex = weaponIdx;
	this->FollowMode = mode;
}

void LaserDrawExt::ExtData::Register()
{
	if (this->Shooter && (this->FollowMode & PositionFollow::Firer))
		LaserDrawExt::ShooterToLasers[this->Shooter].push_back(this->OwnerObject());

	if (this->Target && (this->FollowMode & PositionFollow::Target))
		LaserDrawExt::TargetToLasers[this->Target].push_back(this->OwnerObject());
}

void LaserDrawExt::ExtData::Unregister()
{
	if (this->Shooter)
	{
		auto it = LaserDrawExt::ShooterToLasers.find(this->Shooter);
		if (it != LaserDrawExt::ShooterToLasers.end())
		{
			auto& vec = it->second;
			vec.erase(std::remove(vec.begin(), vec.end(), this->OwnerObject()), vec.end());
			if (vec.empty())
				LaserDrawExt::ShooterToLasers.erase(it);
		}
	}
	if (this->Target)
	{
		auto it = LaserDrawExt::TargetToLasers.find(this->Target);
		if (it != LaserDrawExt::TargetToLasers.end())
		{
			auto& vec = it->second;
			vec.erase(std::remove(vec.begin(), vec.end(), this->OwnerObject()), vec.end());
			if (vec.empty())
				LaserDrawExt::TargetToLasers.erase(it);
		}
	}
}

// --- padding pointer ---

static uint16_t GetPadHalf(LaserDrawClass* pLaser, uintptr_t offset)
{
	return *reinterpret_cast<uint16_t*>(reinterpret_cast<char*>(pLaser) + offset);
}

static void SetPadHalf(LaserDrawClass* pLaser, uintptr_t offset, uint16_t value)
{
	*reinterpret_cast<uint16_t*>(reinterpret_cast<char*>(pLaser) + offset) = value;
}

void LaserDrawExt::ClearPointer(LaserDrawClass* pLaser)
{
	if (!pLaser)
		return;

	SetPadHalf(pLaser, LaserDrawExt::PadHighOffset, 0);
	SetPadHalf(pLaser, LaserDrawExt::PadLowOffset, 0);
}

LaserDrawExt::ExtData* LaserDrawExt::Find(LaserDrawClass* pLaser)
{
	if (!pLaser)
		return nullptr;

	if (reinterpret_cast<uintptr_t>(pLaser) < 0x10000)
		return nullptr;

	const uintptr_t address = (static_cast<uintptr_t>(GetPadHalf(pLaser, PadHighOffset)) << 16)
		| GetPadHalf(pLaser, PadLowOffset);

	return reinterpret_cast<ExtData*>(address);
}

LaserDrawExt::ExtData* LaserDrawExt::Allocate(LaserDrawClass* pLaser)
{
	auto* pExt = ExtMap.Allocate(pLaser);
	if (!pExt)
		return nullptr;

	const uintptr_t address = reinterpret_cast<uintptr_t>(pExt);
	SetPadHalf(pLaser, PadHighOffset, static_cast<uint16_t>(address >> 16));
	SetPadHalf(pLaser, PadLowOffset, static_cast<uint16_t>(address & 0xFFFF));

	return pExt;
}

void LaserDrawExt::Release(LaserDrawClass* pLaser)
{
	ClearPointer(pLaser);
	ExtMap.Remove(pLaser);
}

void LaserDrawExt::ResetPointer(LaserDrawClass* pLaser)
{
	ClearPointer(pLaser);
}

LaserDrawExt::ExtContainer::ExtContainer() : Container("LaserDraw") { }
LaserDrawExt::ExtContainer::~ExtContainer() = default;

bool LaserDrawExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool LaserDrawExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}
