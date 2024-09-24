#pragma once

#include <New/Type/AttachEffectTypeClass.h>

class AttachEffectClass
{
public:
	static std::vector<AttachEffectClass*> Array;

	AttachEffectClass();

	AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay);

	~AttachEffectClass();

	void AI();
	void AI_Temporal();
	void KillAnim();
	void CreateAnim();
	void UpdateCumulativeAnim();
	void TransferCumulativeAnim(AttachEffectClass* pSource);
	bool CanShowAnim() const;
	void SetAnimationTunnelState(bool visible);
	AttachEffectTypeClass* GetType() const { return this->Type; }
	int GetRemainingDuration() const { return this->Duration; }
	void RefreshDuration(int durationOverride = 0);
	bool ResetIfRecreatable();
	bool IsSelfOwned() const { return this->Source == this->Techno; }
	bool HasExpired() const;
	bool AllowedToBeActive() const;
	bool IsActive() const;
	bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const;

	static void PointerGotInvalid(void* ptr, bool removed);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static bool Attach(AttachEffectTypeClass* pType, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, int durationOverride = 0, int delay = 0, int initialDelay = 0, int recreationDelay = -1);

	static int Attach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, std::vector<int>& durationOverrides, std::vector<int> const& delays, std::vector<int> const& initialDelays, std::vector<int> const& recreationDelays);

	static int Detach(AttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount = -1, int maxCount = -1);
	static int Detach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts);
	static int DetachByGroups(std::vector<std::string> const& groups, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts);
	static void TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget);

	// Used for figuring out the correct values to use for a particular effect index when attaching them.
	static void SetValuesHelper(unsigned int index, std::vector<int>& durationOverrides, std::vector<int> const& delays, std::vector<int> const& initialDelays, std::vector<int> const& recreationDelays, int& durationOverride, int& delay, int& initialDelay, int& recreationDelay)
	{
		if (durationOverrides.size() > 0)
			durationOverride = durationOverrides[durationOverrides.size() > index ? index : durationOverrides.size() - 1];

		if (delays.size() > 0)
			delay = delays[delays.size() > index ? index : delays.size() - 1];

		if (initialDelays.size() > 0)
			initialDelay = initialDelays[initialDelays.size() > index ? index : initialDelays.size() - 1];

		if (recreationDelays.size() > 0)
			recreationDelay = recreationDelays[recreationDelays.size() > index ? index : recreationDelays.size() - 1];
	}

private:
	void OnlineCheck();
	void CloakCheck();
	void AnimCheck();

	static AttachEffectClass* CreateAndAttach(AttachEffectTypeClass* pType, TechnoClass* pTarget, std::vector<std::unique_ptr<AttachEffectClass>>& targetAEs,
		HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride = 0, int delay = 0, int initialDelay = 0, int recreationDelay = -1);

	static int RemoveAllOfType(AttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount);

	template <typename T>
	bool Serialize(T& Stm);

	int Duration;
	int DurationOverride;
	int Delay;
	int CurrentDelay;
	int InitialDelay;
	int RecreationDelay;
	AttachEffectTypeClass* Type;
	TechnoClass* Techno;
	HouseClass* InvokerHouse;
	TechnoClass* Invoker;
	AbstractClass* Source;
	AnimClass* Animation;
	bool IsAnimHidden;
	bool IsInTunnel;
	bool IsUnderTemporal;
	bool IsOnline;
	bool IsCloaked;
	bool HasInitialized;
	bool NeedsDurationRefresh;

public:
	bool HasCumulativeAnim;
};

// Container for TechnoClass-specific AttachEffect fields.
struct AttachEffectTechnoProperties
{
	double FirepowerMultiplier;
	double ArmorMultiplier;
	double SpeedMultiplier;
	double ROFMultiplier;
	bool Cloakable;
	bool ForceDecloak;
	bool DisableWeapons;
	bool HasRangeModifier;
	bool HasTint;
	bool ReflectDamage;

	AttachEffectTechnoProperties() :
		FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, Cloakable { false }
		, ForceDecloak { false }
		, DisableWeapons { false }
		, HasRangeModifier { false }
		, HasTint { false }
		, ReflectDamage { false }
	{}
};
