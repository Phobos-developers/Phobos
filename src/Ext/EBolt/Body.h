#pragma once
#include <EBolt.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class EBoltExt
{
public:
	using base_type = EBolt;

	static constexpr DWORD Canary = 0x06C28114;

	class ExtData final : public Extension<EBolt>
	{
	public:
		ColorStruct Color[3] {};
		bool Disable[3] { false };
		int Arcs { 8 };
		int BurstIndex { 0 };

		ExtData(EBolt* OwnerObject) : Extension<EBolt>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void Initialize() override { };

		virtual void InvalidatePointer(void* ptr, bool removed) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<EBoltExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int __forceinline GetDefaultColor_Int(ConvertClass* pConvert, int idx)
	{
		if (pConvert->BytesPerPixel == 1)
			return reinterpret_cast<uint8_t*>(pConvert->PaletteData)[idx];
		else
			return reinterpret_cast<uint16_t*>(pConvert->PaletteData)[idx];
	}

	static EBolt* CreateEBolt(WeaponTypeClass* pWeapon);
	static DWORD _cdecl _EBolt_Draw_Colors(REGISTERS* R);
};
