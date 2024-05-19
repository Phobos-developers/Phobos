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
	void SetAnimationVisibility(bool visible);
	AttachEffectTypeClass* GetType() const;
	void RefreshDuration(int durationOverride = 0);
	bool ResetIfRecreatable();
	bool IsSelfOwned() const;
	bool HasExpired() const;
	bool AllowedToBeActive() const;
	bool IsActive() const;
	bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const;

	void ExpireWeapon() const;

	static void PointerGotInvalid(void* ptr, bool removed);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static bool Attach(AttachEffectTypeClass* pType, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, int durationOverride = 0, int delay = 0, int initialDelay = 0, int recreationDelay = -1);

	static int Attach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, std::vector<int>& durationOverrides, std::vector<int> const& delays, std::vector<int> const& initialDelays, std::vector<int> const& recreationDelays);

	static int Detach(AttachEffectTypeClass* pType, TechnoClass* pTarget,int minCount = -1, int maxCount = -1);
	static int Detach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts);
	static int DetachByGroups(std::vector<const char*> const& groups, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts);
	static void TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget);

private:
	void OnlineCheck();
	void CloakCheck();
	void CreateAnim();

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
	bool IsUnderTemporal;
	bool IsOnline;
	bool IsCloaked;
	bool HasInitialized;
	bool NeedsDurationRefresh;

public:
	bool IsFirstCumulativeInstance;
};

