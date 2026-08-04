#pragma once
#include <TiberiumClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TiberiumExt final : public AbstractTypeExt
{
public:
	using base_type = TiberiumClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TiberiumExt;

	static constexpr DWORD Canary = 0xAABBCCDD;

public:
	// typed owner accessor
	TiberiumClass* OwnerObject() const
	{
		return static_cast<TiberiumClass*>(this->GetAttachedObject());
	}

	Nullable<ColorStruct> MinimapColor;

	// CustomImage extension
	PhobosFixedString<32u> CustomImageName;
	Nullable<int> CustomImageNumFrames;
	Nullable<int> CustomImageNumImages;
	Nullable<int> CustomImageNumSlopes;

	TiberiumExt(TiberiumClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, MinimapColor {}
		, CustomImageName {}
		, CustomImageNumFrames {}
		, CustomImageNumImages {}
		, CustomImageNumSlopes {}
	{ }

	virtual ~TiberiumExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<TiberiumExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TiberiumExt* Fetch(const TiberiumClass* pThis)
	{
		return AbstractExt::Fetch<TiberiumExt>(pThis);
	}

	static TiberiumExt* TryFetch(const TiberiumClass* pThis)
	{
		return AbstractExt::TryFetch<TiberiumExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

