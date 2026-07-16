#pragma once
#include <VoxelAnimTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>

class VoxelAnimTypeExt final : public ObjectTypeExt
{
public:
	using base_type = VoxelAnimTypeClass;
	using ExtData = VoxelAnimTypeExt;

	static constexpr DWORD Canary = 0xAAAEEEEE;
	static constexpr size_t ExtPointerOffset = 0x18;

public:
	// typed owner accessor
	VoxelAnimTypeClass* OwnerObject() const
	{
		return static_cast<VoxelAnimTypeClass*>(this->GetAttachedObject());
	}


	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
	Valueable<bool> ExplodeOnWater;
	Valueable<bool> Warhead_Detonate;
	ValueableVector<AnimTypeClass*> WakeAnim;
	NullableVector<AnimTypeClass*> SplashAnims;
	Valueable<bool> SplashAnims_PickRandom;
	Valueable<int> Trailer_SpawnDelay;

	VoxelAnimTypeExt(VoxelAnimTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, LaserTrail_Types()
		, ExplodeOnWater { false }
		, Warhead_Detonate { false }
		, WakeAnim {}
		, SplashAnims {}
		, SplashAnims_PickRandom { false }
		, Trailer_SpawnDelay { 2 }
	{ }

	virtual ~VoxelAnimTypeExt() = default;
	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void Initialize() override;
	virtual void LoadFromStream(PhobosStreamReader& Stm)override;
	virtual void SaveToStream(PhobosStreamWriter& Stm)override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<VoxelAnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static VoxelAnimTypeExt* Fetch(const VoxelAnimTypeClass* pThis)
	{
		return ExtMap.Find(pThis);
	}

	static VoxelAnimTypeExt* TryFetch(const VoxelAnimTypeClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

