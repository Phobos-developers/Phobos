#pragma once

#include <ParticleTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParticleTypeExt final : public ObjectTypeExt
{
public:
	using base_type = ParticleTypeClass;
	using ExtData = ParticleTypeExt;

	static constexpr DWORD Canary = 0xEAFEEAFE;
	static constexpr size_t ExtPointerOffset = 0x18;

public:
	// typed owner accessor
	ParticleTypeClass* OwnerObject() const
	{
		return static_cast<ParticleTypeClass*>(this->GetAttachedObject());
	}

	Valueable<int> Gas_MaxDriftSpeed;

	ParticleTypeExt(ParticleTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, Gas_MaxDriftSpeed { 2 }
	{ }

	virtual ~ParticleTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<ParticleTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static ParticleTypeExt* Fetch(const ParticleTypeClass* pThis)
	{
		return ExtMap.Find(pThis);
	}

	static ParticleTypeExt* TryFetch(const ParticleTypeClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

