#pragma once
#include <IsometricTileTypeClass.h>
#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <set>

class IsometricTileTypeExt
{
public:
	using base_type = IsometricTileTypeClass;

	class ExtData final : public Extension<IsometricTileTypeClass>
	{
	public:
		Valueable<int> Tileset;
		ExtData(IsometricTileTypeClass* OwnerObject) : Extension<IsometricTileTypeClass>(OwnerObject)
			, Tileset { -1 }
		{
		}

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static int CurrentTileset;
	static std::map<std::string, int> PalettesInitHelper;
	static std::map<int, int> LoadedPalettesLookUp;
	static std::vector<std::map<TintStruct, LightConvertClass*>> LoadedPalettes;
	static std::vector<CustomPalette> CustomPalettes;

	static LightConvertClass* IsometricTileTypeExt::InitDrawer(int nLookUpIdx, int red, int green, int blue);
	static void LoadPaletteFromName(int nTileset, std::string PaletteName);

	class ExtContainer final : public Container<IsometricTileTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};