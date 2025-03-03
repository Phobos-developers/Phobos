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
	bool ShouldBeDiscardedNow() const;
	bool IsActive() const;
	bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const;

	static void PointerGotInvalid(void* ptr, bool removed);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static int Attach(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachInfoTypeClass const& attachEffectInfo);
	static int Detach(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo);
	static int DetachByGroups(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo);
	static void TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget);

private:
	void OnlineCheck();
	void CloakCheck();
	void AnimCheck();

	static AttachEffectClass* CreateAndAttach(AttachEffectTypeClass* pType, TechnoClass* pTarget, std::vector<std::unique_ptr<AttachEffectClass>>& targetAEs, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, AEAttachParams const& attachInfo);

	static int DetachTypes(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo, std::vector<AttachEffectTypeClass*> const& types);
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
	bool ShouldBeDiscarded;
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
	bool HasOnFireDiscardables;
	bool HasRestrictedArmorMultipliers;

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
		, HasOnFireDiscardables { false }
		, HasRestrictedArmorMultipliers { false }
	{ }
};
