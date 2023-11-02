#pragma once

#include <map>

#include <IsometricTileTypeClass.h>

#include <Utilities/Constructs.h>
#include <Utilities/Container.h>
#include <Utilities/Template.h>

class IsometricTileTypeExt
{
public:
	using base_type = IsometricTileTypeClass;
	static constexpr DWORD Canary = 0x91577125;

	class ExtData final : public Extension<IsometricTileTypeClass>
	{
	public:

		static int CurrentTileset;
		static std::map<std::string, std::map<TintStruct, LightConvertClass*>> LightConvertEntities;

		Valueable<int> Tileset;
		CustomPalette Palette;

		ExtData(IsometricTileTypeClass* OwnerObject);

		virtual ~ExtData() = default;

		LightConvertClass* GetLightConvert(int r, int g, int b);

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& stm) override;
		virtual void SaveToStream(PhobosStreamWriter& stm) override;

	private:
		template <typename T>
		void Serialize(T& stm);
	};

	class ExtContainer final : public Container<IsometricTileTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);
	static void Clear();
};
