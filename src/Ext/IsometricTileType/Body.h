#pragma once
#include <IsometricTileTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <ScenarioClass.h>

class LightConvertPalette
{
private:
	LightConvertPalette(const char* pName)
	{
		if (this->Load(pName))
			Array.AddItem(this);
	}

	~LightConvertPalette()
	{
		Array.Remove(this);
	}

	char Name[0x20];
	UniqueGamePtr<BytePalette> Palette;

	bool Load(const char* pName)
	{
		strcpy_s(this->Name, pName);
		if (auto const pPal = FileSystem::AllocatePalette(pName))
			this->Palette.reset(pPal);
		return this->Palette != nullptr;
	}
public:
	bool Loaded() const { return this->Palette != nullptr; }
	operator BytePalette* () { return Palette.get(); }

	static LightConvertPalette* FindOrAllocate(const char* pName)
	{
		int nCount = Array.Count;
		for (auto const pItem : Array)
			if (_stricmp(pName, pItem->Name) == 0)
				return pItem;

		auto const pResult = new LightConvertPalette(pName);

		return Array.Count == nCount + 1 ? pResult : nullptr;
	}

	static LightConvertPalette* FindOrAllocate(CCINIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "")
	{
		if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer))
		{
			if (auto const pSuffix = strstr(Phobos::readBuffer, "~~~"))
			{
				auto const theater = ScenarioClass::Instance->Theater;
				auto const pExtension = Theater::GetTheater(theater).Extension;
				pSuffix[0] = pExtension[0];
				pSuffix[1] = pExtension[1];
				pSuffix[2] = pExtension[2];
			}

			return FindOrAllocate(Phobos::readBuffer);
		}
		return nullptr;
	}

	static DynamicVectorClass<LightConvertPalette*> Array;
};

class IsometricTileTypeExt
{
public:
	using base_type = IsometricTileTypeClass;

	class ExtData final : public Extension<IsometricTileTypeClass>
	{
	public:
		Valueable<int> Tileset;
		LightConvertPalette* Palette;

		ExtData(IsometricTileTypeClass* OwnerObject) : Extension<IsometricTileTypeClass>(OwnerObject)
			, Tileset { -1 }
			, Palette { nullptr }
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
	static PhobosMap<LightConvertPalette*, PhobosMap<TintStruct, LightConvertClass*>> TileDrawers;
	static LightConvertClass* InitDrawer(IsometricTileTypeClass* pType, TintStruct& tint);

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