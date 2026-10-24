#pragma once
#include <CellClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class CellExt final : public AbstractExt
{
public:
	using base_type = CellClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = CellExt;

	static constexpr DWORD Canary = 0x13371337;

	// cell extensions live in the cell's own savegame block, not in the centralized
	// extension stream: while a savegame is loading they are created by LoadInline
	// rather than by the cell constructor, which the game re-runs in place
	static constexpr bool SavedInline = true;

	struct RadLevel
	{
		RadSiteClass* Rad { nullptr };
		int Level { 0 };

		RadLevel() = default;
		RadLevel(RadSiteClass* pRad, int level) : Rad(pRad), Level(level)
		{ }

		bool Load(PhobosStreamReader& stm, bool registerForChange);
		bool Save(PhobosStreamWriter& stm) const;

	private:
		template <typename T>
		bool Serialize(T& stm);
	};

public:
	// typed owner accessor
	CellClass* OwnerObject() const
	{
		return static_cast<CellClass*>(this->GetAttachedObject());
	}

	std::vector<RadSiteClass*> RadSites {};
	std::vector<RadLevel> RadLevels { };
	int InfantryCount{ 0 };
	UnitClass* IncomingUnit {};
	UnitClass* IncomingUnitAlt {};
	// true while this cell is in the list of cells with a tracked incoming unit (see TrackIncoming)
	bool IncomingTracked { false };

	CellExt(CellClass* OwnerObject) : AbstractExt(OwnerObject)
	{ }

	virtual ~CellExt();

	void InvalidatePointer(void* ptr, bool bRemoved);

	// Incoming unit pointers are cleared when the unit is removed from the game: the old per-extension
	// InvalidatePointer is no longer called by the extension system, so this keeps a list of the cells
	// holding one and a detach listener (Cell/Body.cpp) clears them.
	void TrackIncoming();
	static void ClearIncomingUnit(UnitClass* pUnit);

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<CellExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		// cell extension data is persisted inline within each cell's own savegame
		// block instead of the centralized extension stream: the owner is known at
		// load time, so no owner remapping is needed
		bool SaveAllToStream(IStream*) { return true; }
		bool LoadAllFromStream(IStream*) { return true; }

		void SaveInline(CellClass* pCell, IStream* pStm);
		void LoadInline(CellClass* pCell, IStream* pStm);

		// cells the game omits from the savegame need extensions allocated on load
		void RelinkExtensionPointers();
	};

	static ExtContainer ExtMap;

	static CellExt* Fetch(const CellClass* pThis)
	{
		return AbstractExt::Fetch<CellExt>(pThis);
	}

	static CellExt* TryFetch(const CellClass* pThis)
	{
		return AbstractExt::TryFetch<CellExt>(pThis);
	}
};

