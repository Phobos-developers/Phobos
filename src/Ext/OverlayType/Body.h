#pragma once
#include <OverlayTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class OverlayTypeExt final : public ObjectTypeExt
{
public:
	using base_type = OverlayTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = OverlayTypeExt;

	static constexpr DWORD Canary = 0xADF48498;

public:
	// typed owner accessor
	OverlayTypeClass* OwnerObject() const
	{
		return static_cast<OverlayTypeClass*>(this->GetAttachedObject());
	}

	Nullable<bool> CanBeBuiltOn;
	Nullable<bool> CanBeBuiltOn_Remove;
	Valueable<int> ZAdjust;
	PhobosFixedString<32u> PaletteFile;
	DynamicVectorClass<ColorScheme*>* Palette; // Intentionally not serialized - rebuilt from the palette file on load.

	OverlayTypeExt(OverlayTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, CanBeBuiltOn {}
		, CanBeBuiltOn_Remove {}
		, ZAdjust { 0 }
		, PaletteFile {}
		, Palette {}
	{ }

	virtual ~OverlayTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<OverlayTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static OverlayTypeExt* Fetch(const OverlayTypeClass* pThis)
	{
		return AbstractExt::Fetch<OverlayTypeExt>(pThis);
	}

	static OverlayTypeExt* TryFetch(const OverlayTypeClass* pThis)
	{
		return AbstractExt::TryFetch<OverlayTypeExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool CanPlaceBuildingOnOverlay(int overlayTypeIndex, BuildingTypeClass* pBuildingType, bool requireToBeRemovable);
	static void RemoveOverlayFromCell(int overlayTypeIndex, CellClass* pCell, HouseClass* pSource);
};

