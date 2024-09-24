#pragma once
#include <TiberiumClass.h>
#include <OverlayTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

class TiberiumExt
{
public:
	using base_type = TiberiumClass;

	static constexpr DWORD Canary = 0xAABBCCDD;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<TiberiumClass>
	{
	public:
		Nullable<ColorStruct> MinimapColor;

		Valueable<OverlayTypeClass*> Overlay;
		Valueable<bool> Overlays_UseSlopes;

		ExtData(TiberiumClass* OwnerObject) : Extension<TiberiumClass>(OwnerObject)
			, MinimapColor {}
			, Overlay {}
			, Overlays_UseSlopes {}
		{ }

		virtual ~ExtData() override = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TiberiumExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
