#include "Body.h"

#include <memory>

#include <MapClass.h>

CellExt::ExtContainer CellExt::ExtMap;

// =============================
// load / save

template <typename T>
void CellExt::Serialize(T& Stm)
{
	Stm
		.Process(this->RadSites)
		.Process(this->RadLevels)
		.Process(this->InfantryCount)
		;
}

void CellExt::LoadFromStream(PhobosStreamReader& Stm)
{
	AbstractExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void CellExt::SaveToStream(PhobosStreamWriter& Stm)
{
	AbstractExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool CellExt::RadLevel::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool CellExt::RadLevel::Save(PhobosStreamWriter& stm) const
{
	return const_cast<CellExt::RadLevel*>(this)->Serialize(stm);
}

template <typename T>
bool CellExt::RadLevel::Serialize(T& stm)
{
	return stm
		.Process(this->Rad)
		.Process(this->Level)
		.Success();
}

// =============================
// container

CellExt::ExtContainer::ExtContainer() : Container("CellClass") { }
CellExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

// The static working cell (MapClass::InvalidCell) is re-initialized in place at the
// end of every MapClass::SetMapDimensions call and never enters the game's savegame
// stream (only cells installed in the map array do). It keeps a single untracked,
// process-lifetime extension instead of a container-managed one.
static std::unique_ptr<CellExt> InvalidCellExt;

DEFINE_HOOK(0x47BDA1, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, ESI);

	if (pItem == &MapClass::InvalidCell)
	{
		// recreated even during savegame load: this extension is never in the
		// stream, the live one is the real thing
		InvalidCellExt = std::make_unique<CellExt>(pItem);
		InvalidCellExt->EnsureConstanted();
		AbstractExt::Attach(pItem, InvalidCellExt.get());
	}
	else
	{
		CellExt::ExtMap.Allocate(pItem);
	}

	return 0;
}

DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
{
	GET(CellClass*, pItem, ECX);

	if (pItem == &MapClass::InvalidCell)
		InvalidCellExt.reset();
	else
		CellExt::ExtMap.Remove(pItem);

	return 0;
}

// MapClass::InitCells re-runs the cell constructor in place on every allocated cell
// (the AbstractClass constructor zeroes the extension slot in the process); drop the
// tracked extensions up front so the constructor hook can allocate fresh ones.
DEFINE_HOOK(0x565BC0, MapClass_InitCells, 0x6)
{
	CellExt::ExtMap.RemoveAll();

	return 0;
}

// MapClass::SetMapDimensions re-runs the cell constructor in place on cells that
// already exist at the target map slot; drop the extension first, then make the
// displaced constructor call ourselves.
DEFINE_HOOK(0x5663FC, MapClass_SetMapDimensions_ReinitCell, 0x5)
{
	GET(CellClass*, pItem, ECX);

	CellExt::ExtMap.Remove(pItem);

	R->EAX(reinterpret_cast<CellClass*(__thiscall*)(CellClass*)>(0x47BBF0)(pItem));

	return 0x566401;
}

