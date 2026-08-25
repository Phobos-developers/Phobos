#pragma once

#include <VoxelAnimClass.h>

#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/Object/Body.h>

class VoxelAnimExt final : public ObjectExt
{
public:
	using base_type = VoxelAnimClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = VoxelAnimExt;

	static constexpr DWORD Canary = 0xAAAAAACC;

public:
	// typed owner accessor
	VoxelAnimClass* OwnerObject() const
	{
		return static_cast<VoxelAnimClass*>(this->GetAttachedObject());
	}


	std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
	CDTimerClass TrailerSpawnTimer;

	VoxelAnimExt(VoxelAnimClass* OwnerObject) : ObjectExt(OwnerObject)
		, LaserTrails()
		, TrailerSpawnTimer()
	{ }

	virtual ~VoxelAnimExt() = default;
	virtual void LoadFromStream(PhobosStreamReader& Stm)override;
	virtual void SaveToStream(PhobosStreamWriter& Stm)override;
	virtual void Initialize() override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<VoxelAnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static VoxelAnimExt* Fetch(const VoxelAnimClass* pThis)
	{
		return AbstractExt::Fetch<VoxelAnimExt>(pThis);
	}

	static VoxelAnimExt* TryFetch(const VoxelAnimClass* pThis)
	{
		return AbstractExt::TryFetch<VoxelAnimExt>(pThis);
	}
	static void InitializeLaserTrails(VoxelAnimClass* pThis);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

