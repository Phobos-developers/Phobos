#pragma once

#include <cstdint>
#include <vector>

#include "Stream.h"
#include "Debug.h"

class AbstractClass;
class AbstractExt;

// Slot-generation table that backs every GameHandle<T>. Owned by ScenarioExt as
// a member field (not a singleton, not serialized): on savegame load the table
// is rebuilt from each AbstractExt's persisted SlotIndex. Each AbstractExt-based
// extension reserves a slot on construction (storing the owner pointer and a
// generation counter) and releases it on destruction, bumping the generation so
// that handles created before the release compare unequal and resolve to null.
class GameHandleTable
{
public:
	static constexpr uint32_t INVALID_SLOT = 0xFFFFFFFFu;
	// 4x the previous uint16 limit of 65535. A real Mental Omega session peaks
	// above 65535 live extensions, so the slot index had to be widened to
	// uint32_t. 262140 is a power-of-two-ish ceiling with headroom; bump if
	// a mod exceeds it.
	static constexpr uint32_t MAX_SLOTS = 262140u; // exclusive upper bound

	// Returns the active table owned by ScenarioExt, or null when ScenarioExt
	// has not been constructed yet (extensions created before scenario load get
	// INVALID_SLOT and are simply skipped). Implemented in GameHandle.cpp.
	static GameHandleTable* TryInstance();

	// Called from AbstractExt's constructor: reserves a slot, records the owner
	// pointer and returns its index. Returns INVALID_SLOT when the table is full.
	uint32_t Allocate(AbstractClass* ptr);

	// Called from AbstractExt's destructor: clears the pointer, bumps the
	// generation (invalidating every handle that still holds the old one) and
	// pushes the slot back onto the free list for reuse.
	void Release(uint32_t slotIndex) noexcept;

	// Reads the current generation of a slot, used when constructing a handle.
	uint32_t GetGeneration(uint32_t slotIndex) const noexcept;

	// O(1) lookup used by GameHandle<T>::Get(); returns null when the slot is
	// empty or the stored generation does not match the handle's.
	AbstractClass* Query(uint32_t slotIndex, uint32_t generation) const noexcept;

	// Post-swizzle rebuild: walks every loaded extension and writes its owner
	// pointer back into the slot the extension carries. The table is not
	// serialized, so on load it starts empty and is fully rebuilt here from the
	// persisted SlotIndex values. Defined in Phobos.Ext.cpp because it needs
	// PhobosTypeRegistry, which is only visible there.
	void RebuildPointers();

private:
	struct Entry
	{
		AbstractClass* Pointer { nullptr };
		uint32_t Generation { 0 };
	};

	std::vector<Entry> Entries;        // dense array, indexed by SlotIndex
	std::vector<uint32_t> FreeSlots;   // reusable slot stack
};

// Safe drop-in replacement for a raw AbstractClass-derived pointer reference.
// Stores a (SlotIndex, Generation) pair; resolves through GameHandleTable so
// that destruction of the pointee automatically invalidates every outstanding
// handle, preventing use-after-free. Only SlotIndex is persisted; Generation is
// reset to 0 on load and matched against a freshly rebuilt table.
template <typename T>
class GameHandle
{
public:
	GameHandle() noexcept : Slot(GameHandleTable::INVALID_SLOT), Generation(0) { }

	explicit GameHandle(T* ptr);
	GameHandle(const GameHandle&) = default;
	GameHandle& operator=(const GameHandle&) = default;
	GameHandle& operator=(T* ptr);

	T* Get() const noexcept;
	T* operator->() const noexcept { return this->Get(); }
	explicit operator bool() const noexcept { return this->Get() != nullptr; }

	// Comparisons against raw pointers, used by Hooks.Production.cpp.
	bool operator==(T* ptr) const noexcept { return this->Get() == ptr; }
	bool operator!=(T* ptr) const noexcept { return this->Get() != ptr; }

	void Reset() noexcept
	{
		this->Slot = GameHandleTable::INVALID_SLOT;
		this->Generation = 0;
	}

	// ImplementsUpperCaseSaveLoad-compatible members: Stm.Process(handle)
	// dispatches into these automatically. Only Slot is persisted; Generation is
	// not (the table is rebuilt from SlotIndex on load, with all generations at
	// 0, so a freshly loaded handle matches by also starting at 0).
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Stm.Load(this->Slot);
		this->Generation = 0;
		return true;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		Stm.Save(this->Slot);
		return true;
	}

private:
	uint32_t Slot;
	uint32_t Generation;
};

template <typename T>
GameHandle<T>::GameHandle(T* ptr) : Slot(GameHandleTable::INVALID_SLOT), Generation(0)
{
	if (!ptr)
		return;

	auto* const pExt = AbstractExt::TryFetch(ptr);

	if (!pExt || pExt->GetSlotIndex() == GameHandleTable::INVALID_SLOT)
		return; // no extension attached or slot not allocated, handle stays empty

	auto* const table = GameHandleTable::TryInstance();

	if (!table)
		return; // ScenarioExt not yet constructed, cannot resolve generation

	this->Slot = pExt->GetSlotIndex();
	this->Generation = table->GetGeneration(this->Slot);

	Debug::Log("[GameHandle] Construct slot=%u gen=%u ptr=%p\n",
		static_cast<unsigned>(this->Slot),
		static_cast<unsigned>(this->Generation),
		static_cast<void*>(ptr));
}

template <typename T>
T* GameHandle<T>::Get() const noexcept
{
	if (this->Slot == GameHandleTable::INVALID_SLOT)
		return nullptr;

	auto* const table = GameHandleTable::TryInstance();

	if (!table)
		return nullptr;

	return static_cast<T*>(table->Query(this->Slot, this->Generation));
}

template <typename T>
GameHandle<T>& GameHandle<T>::operator=(T* ptr)
{
	*this = GameHandle<T>(ptr);
	return *this;
}
