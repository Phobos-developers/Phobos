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

// Writes a cell's extension into the cell's own savegame block, right after the
// game's data. The block is length-prefixed and carries the extension's save-time
// address, so pointers to it could be remapped like any other.
void CellExt::ExtContainer::SaveInline(CellClass* pCell, IStream* pStm)
{
	auto const pExt = CellExt::Fetch(pCell);

	PhobosByteStream saver(sizeof(CellExt));
	PhobosStreamWriter writer(saver);

	writer.Save(CellExt::Canary);
	writer.RegisterChange(pExt);

	pExt->SaveToStream(writer);

	if (!saver.WriteBlockToStream(pStm))
		Debug::FatalErrorAndExit("SaveInline - failed to save a cell extension!\n");
}

// Recreates a cell's extension from the cell's own savegame block. Unlike the
// centralized stream, the owner is the live cell being loaded, so no owner
// remapping is needed.
void CellExt::ExtContainer::LoadInline(CellClass* pCell, IStream* pStm)
{
	PhobosByteStream loader(0);

	if (!loader.ReadBlockFromStream(pStm))
		Debug::FatalErrorAndExit("LoadInline - failed to read a cell extension block!\n");

	PhobosStreamReader reader(loader);
	void* oldPtr = nullptr;

	if (!reader.Expect(CellExt::Canary) || !reader.Load(oldPtr))
		Debug::FatalErrorAndExit("LoadInline - invalid cell extension block!\n");

	// the loaded cell carries no extension: its constructor ran with allocation
	// suppressed and the loaded image's slot was cleared
	auto const pExt = this->AllocateUnchecked(pCell);
	PhobosSwizzle::RegisterChange(oldPtr, pExt);

	pExt->LoadFromStream(reader);

	if (!reader.ExpectEndOfBlock())
		Debug::FatalErrorAndExit("LoadInline - cell extension block size mismatch!\n");
}

// The game only saves cells that its cell iterator reaches inside the map array;
// any other live cell is recreated as a default placeholder on load, while
// extension allocation is suppressed. Give those cells extensions now.
void CellExt::ExtContainer::RelinkExtensionPointers()
{
	size_t added = 0;

	for (int i = 0; i < MapClass::MaxCells; ++i)
	{
		auto const pCell = MapClass::Instance.Cells[i];

		if (pCell && !AbstractExt::Fetch(pCell))
		{
			this->AllocateUnchecked(pCell);
			++added;
		}
	}

	if (added)
		Debug::Log("CellClass - allocated %u extensions for cells absent from the savegame.\n", added);
}

// =============================
// container hooks

// The static working cell (MapClass::InvalidCell) is re-initialized in place by
// MapClass::ReadBinary and at the end of every MapClass::SetMapDimensions call, and
// never enters the game's savegame stream (only cells installed in the map array
// do). It keeps a single untracked, process-lifetime extension instead of a
// container-managed one.
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

// =============================
// inline save/load hooks

static CellClass* PersistCell = nullptr;
static IStream* PersistStream = nullptr;

DEFINE_HOOK_AGAIN(0x483C10, CellClass_SaveLoad_Prefix, 0x5) // Save
DEFINE_HOOK(0x4839F0, CellClass_SaveLoad_Prefix, 0x7)       // Load
{
	GET_STACK(CellClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	PersistCell = pItem;
	PersistStream = pStm;

	return 0;
}

DEFINE_HOOK(0x483C00, CellClass_Load_Suffix, 0x5)
{
	CellExt::ExtMap.LoadInline(PersistCell, PersistStream);

	return 0;
}

DEFINE_HOOK(0x483C79, CellClass_Save_Suffix, 0x6)
{
	CellExt::ExtMap.SaveInline(PersistCell, PersistStream);

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

// MapClass::SetMapDimensions value-copies whole cells around: it snapshots the old
// map's cells into a temporary buffer, re-initializes the cells of the new map rect,
// then copies the snapshot back into the shifted cells (or into the working cell) via
// AbstractClass::operator=, which copies the extension slot too, leaving the slots
// pointing at stale extensions. The container is the source of truth and the slots
// are only a cache: rebuild them right after the copy-back, before the boundary cells
// are deleted and their destructors read the slots again.
DEFINE_HOOK(0x566AB7, MapClass_SetMapDimensions_PostRestore, 0x6)
{
	CellExt::ExtMap.ReattachAll();

	AbstractExt::Attach(&MapClass::InvalidCell, InvalidCellExt.get());

	return 0;
}

