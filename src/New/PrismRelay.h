#pragma once

#include <vector>

#include <GeneralDefinitions.h>
#include <Utilities/TemplateDef.h>

class AttachEffectClass;
class AttachEffectTypeClass;
class BulletClass;
class TechnoClass;
class WeaponTypeClass;

enum class PrismRelayPhase : unsigned char
{
	Idle = 0,
	Supporting,
	FiringMaster
};

struct PrismRelaySupportEdge
{
	TechnoClass* From { nullptr };
	TechnoClass* To { nullptr };
	int Layer { 0 };

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->From)
			.Process(this->To)
			.Process(this->Layer)
			;
	}
};

struct TechnoPrismRelaySession
{
	PrismRelayPhase Phase { PrismRelayPhase::Idle };
	AbstractClass* EnemyTarget { nullptr };
	WeaponTypeClass* MasterWeapon { nullptr };
	int WeaponIndex { -1 };
	int SupportCount { 0 };
	int PendingBullets { 0 };
	CDTimerClass Timeout {};
	CDTimerClass SupportFireTimer {};
	int ActiveSupportLayer { 0 };
	int PendingNextLayer { 0 };
	std::vector<TechnoClass*> NetworkNodes {};
	std::vector<PrismRelaySupportEdge> SupportEdges {};
	AttachEffectTypeClass* RelayType { nullptr };

	void Reset()
	{
		this->Phase = PrismRelayPhase::Idle;
		this->EnemyTarget = nullptr;
		this->MasterWeapon = nullptr;
		this->WeaponIndex = -1;
		this->SupportCount = 0;
		this->PendingBullets = 0;
		this->Timeout.Stop();
		this->SupportFireTimer.Stop();
		this->ActiveSupportLayer = 0;
		this->PendingNextLayer = 0;
		this->NetworkNodes.clear();
		this->SupportEdges.clear();
		this->RelayType = nullptr;
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->Phase)
			.Process(this->EnemyTarget)
			.Process(this->MasterWeapon)
			.Process(this->WeaponIndex)
			.Process(this->SupportCount)
			.Process(this->PendingBullets)
			.Process(this->Timeout)
			.Process(this->SupportFireTimer)
			.Process(this->ActiveSupportLayer)
			.Process(this->PendingNextLayer)
			.Process(this->NetworkNodes)
			.Process(this->SupportEdges)
			.Process(this->RelayType)
			;

		if constexpr (Stm.IsReading())
		{
			if (this->Phase != PrismRelayPhase::Idle)
				this->Reset();
		}
	}
};

class PrismRelay
{
public:
	static AttachEffectTypeClass* GetRelayType(TechnoClass* pTechno);
	static AttachEffectTypeClass* GetRelayTypeForNetwork(TechnoClass* pTechno, int networkId);
	static int GetRelayLockoutFrames(AttachEffectTypeClass* pType);
	static std::vector<TechnoClass*> BuildDamageProvidersFromEdges(const std::vector<PrismRelaySupportEdge>& edges);

	static bool CanProvide(TechnoClass* pTechno, AttachEffectTypeClass* pType);
	static bool CanReceive(TechnoClass* pTechno, AttachEffectTypeClass* pType);
	static bool IsOnCooldown(TechnoClass* pTechno);
	static bool IsWeaponRelayAllowed(AttachEffectTypeClass* pType, WeaponTypeClass* pWeapon);
	static int ResolveMasterFireWeaponIndex(TechnoClass* pMaster, AttachEffectTypeClass* pType, AbstractClass* pTarget, int initiatingWeaponIndex);

	static int ApplyDamageBonus(int damage, const std::vector<TechnoClass*>& providers, int networkId);

	static bool TryHandleFireAt(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon, int weaponIndex);
	static void NotifyBulletDestroyed(BulletClass* pBullet);
	static void UpdateSessionTimeouts(TechnoClass* pThis);
};
