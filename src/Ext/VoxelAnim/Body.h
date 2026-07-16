#pragma once

#include <VoxelAnimClass.h>

#include <Ext/VoxelAnimType/Body.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/Object/Body.h>

class VoxelAnimExt
{
public:
	using base_type = VoxelAnimClass;

	static constexpr DWORD Canary = 0xAAAAAACC;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public ObjectClassExtension
	{
	public:
		// typed owner accessor
		VoxelAnimClass* OwnerObject() const
		{
			return static_cast<VoxelAnimClass*>(this->GetAttachedObject());
		}


		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		CDTimerClass TrailerSpawnTimer;

		ExtData(VoxelAnimClass* OwnerObject) : ObjectClassExtension(OwnerObject)
			, LaserTrails()
			, TrailerSpawnTimer()
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;
		virtual void Initialize() override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<VoxelAnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static void InitializeLaserTrails(VoxelAnimClass* pThis);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

// top-level name for the VoxelAnimExt extension
using VoxelAnimClassExtension = VoxelAnimExt::ExtData;
