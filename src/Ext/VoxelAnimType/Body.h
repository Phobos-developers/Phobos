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

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = VoxelAnimTypeExt;

	static constexpr DWORD Canary = 0xAAAEEEEE;

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
		return AbstractExt::Fetch<VoxelAnimTypeExt>(pThis);
	}

	static VoxelAnimTypeExt* TryFetch(const VoxelAnimTypeClass* pThis)
	{
		return AbstractExt::TryFetch<VoxelAnimTypeExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

