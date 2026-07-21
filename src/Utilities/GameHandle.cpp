#include "GameHandle.h"

#include <Utilities/Debug.h>
#include <Ext/Scenario/Body.h>

// The table lives inside ScenarioExt as a member field. ScenarioExt::Global()
// is null before the scenario is constructed (during early game init), so
// extensions created in that window get INVALID_SLOT and are simply skipped -
// those objects are never referenced by GameHandle anyway.
GameHandleTable* GameHandleTable::TryInstance()
{
	auto* const scenario = ScenarioExt::Global();

	return scenario ? &scenario->SlotTable : nullptr;
}

uint32_t GameHandleTable::Allocate(AbstractClass* ptr)
{
	uint32_t slot;

	if (!this->FreeSlots.empty())
	{
		slot = this->FreeSlots.back();
		this->FreeSlots.pop_back();
	}
	else
	{
		slot = static_cast<uint32_t>(this->Entries.size());

		if (slot >= MAX_SLOTS)
		{
			Debug::Log("[GameHandle] Allocate FAILED: table full (%u slots)\n", this->Entries.size());
			return INVALID_SLOT;
		}

		this->Entries.push_back({});
	}

	this->Entries[slot].Pointer = ptr;
	// Generation is preserved across reuse (Release already bumped it), which
	// is what prevents stale handles from resolving to the new occupant.

	const size_t used = this->Entries.size() - this->FreeSlots.size();
	Debug::Log("[GameHandle] Allocate slot=%u ptr=%p entries=%u free=%u used=%u\n",
		static_cast<unsigned>(slot), static_cast<void*>(ptr),
		static_cast<unsigned>(this->Entries.size()),
		static_cast<unsigned>(this->FreeSlots.size()),
		static_cast<unsigned>(used));

	// Early warning so we notice capacity pressure long before exhaustion
	if (this->Entries.size() > 240000u && this->Entries.size() % 10000u == 0u)
		Debug::Log("[GameHandle] WARNING: entries=%u approaching MAX_SLOTS=%u\n",
			static_cast<unsigned>(this->Entries.size()),
			static_cast<unsigned>(MAX_SLOTS));

	return slot;
}

void GameHandleTable::Release(uint32_t slotIndex) noexcept
{
	if (slotIndex >= this->Entries.size())
		return;

	auto& entry = this->Entries[slotIndex];
	entry.Pointer = nullptr;
	++entry.Generation;
	this->FreeSlots.push_back(slotIndex);

	Debug::Log("[GameHandle] Release slot=%u gen=%u free=%u\n",
		static_cast<unsigned>(slotIndex),
		static_cast<unsigned>(entry.Generation),
		static_cast<unsigned>(this->FreeSlots.size()));
}

uint32_t GameHandleTable::GetGeneration(uint32_t slotIndex) const noexcept
{
	if (slotIndex >= this->Entries.size())
		return 0;

	return this->Entries[slotIndex].Generation;
}

AbstractClass* GameHandleTable::Query(uint32_t slotIndex, uint32_t generation) const noexcept
{
	if (slotIndex >= this->Entries.size())
		return nullptr;

	const auto& entry = this->Entries[slotIndex];

	if (entry.Pointer == nullptr || entry.Generation != generation)
		return nullptr;

	return entry.Pointer;
}

// RebuildPointers() is defined in Phobos.Ext.cpp: it needs PhobosTypeRegistry,
// which is declared inside that translation unit and is not visible here.
