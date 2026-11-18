#pragma once
#include <TerrainTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TerrainTypeExt final : public ObjectTypeExt
{
public:
	using base_type = TerrainTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TerrainTypeExt;

	static constexpr DWORD Canary = 0xBEE78007;

public:
	// typed owner accessor
	TerrainTypeClass* OwnerObject() const
	{
		return static_cast<TerrainTypeClass*>(this->GetAttachedObject());
	}

	Valueable<int> SpawnsTiberium_Type;
	Valueable<int> SpawnsTiberium_Range;
	Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage;
	Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim;
	ValueableIdx<ParticleTypeClass> SpawnsTiberium_Particle;
	ValueableVector<AnimTypeClass*> DestroyAnim;
	ValueableIdx<VocClass> DestroySound;
	Nullable<ColorStruct> MinimapColor;
	Nullable<bool> IsPassable;
	Nullable<bool> CanBeBuiltOn;
	Valueable<bool> HasDamagedFrames;
	Valueable<bool> HasCrumblingFrames;
	ValueableIdx<VocClass> CrumblingSound;
	Nullable<int> AnimationLength;

	static constexpr CellStruct FoundationEnd { 0x7FFF, 0x7FFF };
	static constexpr int MapSide = 512;
	std::vector<CellStruct> CustomFoundation; // Empty for stock foundations, otherwise terminated by FoundationEnd.

	PhobosFixedString<32u> PaletteFile;
	DynamicVectorClass<ColorScheme*>* Palette; // Intentionally not serialized - rebuilt from the palette file on load.

	TerrainTypeExt(TerrainTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, SpawnsTiberium_Type { 0 }
		, SpawnsTiberium_Range { 1 }
		, SpawnsTiberium_GrowthStage { { 3 } }
		, SpawnsTiberium_CellsPerAnim { { 1 } }
		, SpawnsTiberium_Particle { -1 }
		, DestroyAnim {}
		, DestroySound {}
		, MinimapColor {}
		, IsPassable {}
		, CanBeBuiltOn {}
		, HasDamagedFrames { false }
		, HasCrumblingFrames { false }
		, CrumblingSound {}
		, AnimationLength {}
		, CustomFoundation {}
		, PaletteFile {}
		, Palette {}
	{ }

	virtual ~TerrainTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	virtual void PostLoad() override;

	int GetTiberiumGrowthStage();
	int GetCellsPerAnim();
	void PlayDestroyEffects(const CoordStruct& coords);
	bool HasCustomFoundation() const { return !this->CustomFoundation.empty(); }
	static TerrainClass* FindCustomFoundationTerrain(const CellClass& cell, bool onBridge);

private:
	void LoadCustomFoundation();

	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<TerrainTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TerrainTypeExt* Fetch(const TerrainTypeClass* pThis)
	{
		return AbstractExt::Fetch<TerrainTypeExt>(pThis);
	}

	static TerrainTypeExt* TryFetch(const TerrainTypeClass* pThis)
	{
		return AbstractExt::TryFetch<TerrainTypeExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void Remove(TerrainClass* pTerrain);
};

