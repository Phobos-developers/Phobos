#pragma once
#include <TiberiumClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TiberiumExt
{
public:
	using base_type = TiberiumClass;

	static constexpr DWORD Canary = 0xAABBCCDD;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public AbstractTypeClassExtension
	{
	public:
		// typed owner accessor
		TiberiumClass* OwnerObject() const
		{
			return static_cast<TiberiumClass*>(this->GetAttachedObject());
		}

		Nullable<ColorStruct> MinimapColor;

		ExtData(TiberiumClass* OwnerObject) : AbstractTypeClassExtension(OwnerObject)
			, MinimapColor {}
		{ }

		virtual ~ExtData() = default;

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

// top-level name for the TiberiumExt extension
using TiberiumClassExtension = TiberiumExt::ExtData;
