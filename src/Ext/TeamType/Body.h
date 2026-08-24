#pragma once
#include <TeamTypeClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TeamTypeExt final : public AbstractTypeExt
{
public:
	using base_type = TeamTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = TeamTypeExt;

	static constexpr DWORD Canary = 0xABCDEF01;

public:
	// typed owner accessor
	TeamTypeClass* OwnerObject() const
	{
		return static_cast<TeamTypeClass*>(this->GetAttachedObject());
	}

	Nullable<int> SetRecruitableOnLiberate;
	Valueable<AircraftTypeClass*> ParaDropAircraft;

	TeamTypeExt(TeamTypeClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, SetRecruitableOnLiberate {}
		, ParaDropAircraft { nullptr }
	{ }

	virtual ~TeamTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	// virtual void Initialize() override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<TeamTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TeamTypeExt* Fetch(const TeamTypeClass* pThis)
	{
		return AbstractExt::Fetch<TeamTypeExt>(pThis);
	}

	static TeamTypeExt* TryFetch(const TeamTypeClass* pThis)
	{
		return AbstractExt::TryFetch<TeamTypeExt>(pThis);
	}
};

