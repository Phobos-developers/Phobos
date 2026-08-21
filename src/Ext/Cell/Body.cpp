#include "Body.h"

#include <algorithm>
#include <memory>
#include <FootClass.h>
#include <Ext/Techno/Body.h>
#include <CellSpread.h>
#include <MapClass.h>
#include <Unsorted.h>
#include <JumpjetLocomotionClass.h>

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
		.Process(this->InAirJumpjets)
		.Process(this->Jumpjet_LastScatterAffectedFrame)
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

	pExt->InAirJumpjets.erase(std::remove(pExt->InAirJumpjets.begin(), pExt->InAirJumpjets.end(), nullptr), pExt->InAirJumpjets.end());

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
// =============================
// ExtendedJumpjetHovering

static int GetJumpjetCollisionRange(FootClass* pFoot)
{
	return pFoot->WhatAmI() == AbstractType::Infantry ? 64 : 128;
}

static bool IsCloseEnoughToScatter(FootClass* p1, FootClass* p2)
{
	int minDist = GetJumpjetCollisionRange(p1) + GetJumpjetCollisionRange(p2);
	CoordStruct c1 = p1->GetCoords();
	c1.Z = 0;
	CoordStruct c2 = p2->GetCoords();
	c2.Z = 0;
	return c1.DistanceFrom(c2) < minDist;
}

static inline bool IsJumpjet(FootClass* pFoot)
{
	return locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor) != nullptr;
}

int CellExt::GetWeightedJumpjetCount(FootClass* pHov, bool* shouldScatter)
{
	if (!shouldScatter)
		return 0;

	bool scatter = *shouldScatter;
	int count = 0;

	for (size_t i = 0; i < this->InAirJumpjets.size(); )
	{
		FootClass* pFoot = this->InAirJumpjets[i];

		if (!pFoot)
		{
			// null entries can remain after savegame swizzling - drop them
			this->InAirJumpjets.erase(this->InAirJumpjets.begin() + i);
			continue;
		}

		if (pFoot == pHov || pFoot->InLimbo || !IsJumpjet(pFoot))
		{
			++i;
			continue;
		}

		count += (pFoot->WhatAmI() == AbstractType::Infantry ? 1 : 4);
		if (!scatter && IsCloseEnoughToScatter(pFoot, pHov))
			scatter = true;
		if (count > 11)
			break;
		++i;
	}

	*shouldScatter = scatter;
	return count;
}

// ExtendedJumpjetHovering - per-cell pointer invalidation. The Detach funnel notifies every
// cell extension when a foot is removed from the game; each cell cleans its own
// InAirJumpjets list, so a destroyed unit never leaves a dangling pointer behind.
//
// TEMPORARY: this OnDetach cleanup is the active crash-safe path while the dtor
// cleanup in TechnoExt::~TechnoExt is commented out. The dtor cleanup only looks
// at Jumpjet_LastCell, so if the foot was registered in a cell that is NOT its
// LastCell, the dtor would have missed it - report that case with LogAndMessage.
void CellExt::OnDetach(FootClass* pFoot, bool removed)
{
	if (!pFoot || !removed)
		return;

	auto& vec = this->InAirJumpjets;
	auto const it = std::find(vec.begin(), vec.end(), pFoot);
	if (it == vec.end())
		return;

	vec.erase(it);

	// Diagnostic: the foot was registered in THIS cell, so its Jumpjet_LastCell is
	// expected to point here. If it does not, the dtor-based cleanup would have
	// missed this entry -> log it so the mismatch can be tracked down.
	CellClass* pLastCell = nullptr;
	const char* typeId = "?";
	if (auto pTechnoExt = TechnoExt::TryFetch(pFoot))
	{
		pLastCell = pTechnoExt->Jumpjet_LastCell;
		if (pTechnoExt->TypeExtData)
			typeId = pTechnoExt->TypeExtData->OwnerObject()->ID;
	}

	if (pLastCell != this->OwnerObject())
	{
		CellStruct thisCoords = this->OwnerObject()->MapCoords;
		CellStruct lastCoords { -1, -1 };
		if (pLastCell)
			lastCoords = pLastCell->MapCoords;
		Debug::LogAndMessage("[ExtendedJumpjetHovering] MISMATCH: '%s'(0x%08X) was in cell(%d,%d) but LastCell=%s(%d,%d)\n",
			typeId, pFoot, thisCoords.X, thisCoords.Y,
			pLastCell ? "cell" : "NULL", lastCoords.X, lastCoords.Y);
	}
}

void CellExt::MarkJumpjetScatterCell()
{
	if (auto pExt = CellExt::TryFetch(this->OwnerObject()))
		pExt->Jumpjet_LastScatterAffectedFrame = Unsorted::CurrentFrame;
	CellStruct pos = this->OwnerObject()->MapCoords;
	for (int dir = 0; dir < 8; ++dir)
	{
		CellStruct off = CellSpread::GetNeighbourOffset(dir);
		if (auto pCell = MapClass::Instance.GetCellAt(pos + off))
			if (auto pCellExt = CellExt::TryFetch(pCell))
				pCellExt->Jumpjet_LastScatterAffectedFrame = Unsorted::CurrentFrame;
	}
}

void CellExt::UpdateJumpjet(FootClass* pFoot, int curHeight, int oldHeight)
{
	if (this->OwnerObject()->MapCoords.X == 0 && this->OwnerObject()->MapCoords.Y == 0)
		return;
	if (oldHeight == curHeight)
		return;
	bool wasInAir = oldHeight >= Unsorted::CellHeight;
	bool nowInAir = curHeight >= Unsorted::CellHeight;
	if (wasInAir == nowInAir)
		return;
	if (wasInAir)
	{
		// symmetric removal: whatever was added to this cell's list must leave it,
		// even if the unit is no longer a jumpjet (e.g. its locomotor changed)
		for (size_t i = 0; i < this->InAirJumpjets.size(); ++i)
		{
			if (this->InAirJumpjets[i] == pFoot)
			{
				this->InAirJumpjets.erase(this->InAirJumpjets.begin() + i);
				break;
			}
		}
	}
	else if (nowInAir && IsJumpjet(pFoot))
	{
		// only in-air jumpjets are ever tracked (see AddJumpjet)
		if (std::find(this->InAirJumpjets.begin(), this->InAirJumpjets.end(), pFoot) == this->InAirJumpjets.end())
			this->InAirJumpjets.push_back(pFoot);
	}
}

void CellExt::AddJumpjet(FootClass* pFoot, int curHeight)
{
	if (!IsJumpjet(pFoot))
		return;
	if (this->OwnerObject()->MapCoords.X == 0 && this->OwnerObject()->MapCoords.Y == 0)
		return;
	this->MarkJumpjetScatterCell();
	if (curHeight >= Unsorted::CellHeight)
	{
		if (std::find(this->InAirJumpjets.begin(), this->InAirJumpjets.end(), pFoot) == this->InAirJumpjets.end())
			this->InAirJumpjets.push_back(pFoot);
	}
}

void CellExt::RemoveJumpjet(FootClass* pFoot, int oldHeight)
{
	// no IsJumpjet gate here on purpose: removal must be symmetric with AddJumpjet,
	// even if the unit's locomotor changed after it was added, otherwise stale
	// entries linger in the lists and dangle once the unit is destroyed.
	if (this->OwnerObject()->MapCoords.X == 0 && this->OwnerObject()->MapCoords.Y == 0)
		return;
	if (oldHeight < Unsorted::CellHeight)
		return;
	for (size_t i = 0; i < this->InAirJumpjets.size(); ++i)
	{
		if (this->InAirJumpjets[i] == pFoot)
		{
			this->InAirJumpjets.erase(this->InAirJumpjets.begin() + i);
			break;
		}
	}
}


