#pragma once
#include <TiberiumClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TiberiumExt final : public AbstractTypeExt
{
public:
	using base_type = TiberiumClass;
	using ExtData = TiberiumExt;

	static constexpr DWORD Canary = 0xAABBCCDD;
	static constexpr size_t ExtPointerOffset = 0x18;

public:
	// typed owner accessor
	TiberiumClass* OwnerObject() const
	{
		return static_cast<TiberiumClass*>(this->GetAttachedObject());
	}

	Nullable<ColorStruct> MinimapColor;

	TiberiumExt(TiberiumClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, MinimapColor {}
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
		return ExtMap.Find(pThis);
	}

	static TiberiumExt* TryFetch(const TiberiumClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

