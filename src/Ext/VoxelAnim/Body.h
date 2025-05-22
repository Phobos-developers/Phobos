#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>

#include <Helpers/Macro.h>

#include <New/Entity/LaserTrailClass.h>

class VoxelAnimExt
{
public:
	using base_type = VoxelAnimClass;

	static constexpr DWORD Canary = 0xAAAAAACC;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<VoxelAnimClass>
	{
	public:

		std::vector<LaserTrailClass> LaserTrails;

		ExtData(VoxelAnimClass* OwnerObject) : Extension<VoxelAnimClass>(OwnerObject)
			, LaserTrails()
		{ }

		virtual ~ExtData() = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
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
