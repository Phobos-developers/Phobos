#pragma once

#include <VoxelAnimClass.h>

#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/Object/Body.h>

class VoxelAnimExt final : public ObjectExt
{
public:
	using base_type = VoxelAnimClass;
	using ExtData = VoxelAnimExt;

	static constexpr DWORD Canary = 0xAAAAAACC;
	static constexpr size_t ExtPointerOffset = 0x18;

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
		return ExtMap.Find(pThis);
	}

	static VoxelAnimExt* TryFetch(const VoxelAnimClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}
	static void InitializeLaserTrails(VoxelAnimClass* pThis);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

