#include "PrismRelay.h"

#include <Ext/Bullet/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <New/Entity/AttachEffectClass.h>
#include <New/Type/AttachEffectTypeClass.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>

#include <algorithm>
#include <unordered_map>

AttachEffectTypeClass* PrismRelay::GetRelayType(TechnoClass* pTechno)
{
	if (!pTechno)
		return nullptr;

	auto const pExt = TechnoExt::ExtMap.TryFind(pTechno);
	if (!pExt)
		return nullptr;

	for (auto const& pAE : pExt->AttachedEffects)
	{
		if (!pAE)
			continue;

		auto const pType = pAE->GetType();

		if (!pType || !pType->PrismRelay)
			continue;

		if (pType->PrismRelay_SupportWeapon)
			return pType;
	}

	return nullptr;
}

AttachEffectTypeClass* PrismRelay::GetRelayTypeForNetwork(TechnoClass* pTechno, int networkId)
{
	if (!pTechno)
		return nullptr;

	auto const pExt = TechnoExt::ExtMap.TryFind(pTechno);
	if (!pExt)
		return nullptr;

	for (auto const& pAE : pExt->AttachedEffects)
	{
		if (!pAE)
			continue;

		auto const pType = pAE->GetType();

		if (!pType || !pType->PrismRelay)
			continue;

		if (pType->PrismRelay_NetworkID != networkId)
			continue;

		if (pType->PrismRelay_SupportWeapon)
			return pType;
	}

	return nullptr;
}

std::vector<TechnoClass*> PrismRelay::BuildDamageProvidersFromEdges(const std::vector<PrismRelaySupportEdge>& edges)
{
	std::unordered_map<TechnoClass*, int> providerLayer;

	for (auto const& edge : edges)
	{
		if (!edge.From)
			continue;

		auto const it = providerLayer.find(edge.From);

		if (it == providerLayer.end())
			providerLayer.emplace(edge.From, edge.Layer);
		else if (edge.Layer > it->second)
			it->second = edge.Layer;
	}

	std::vector<std::pair<TechnoClass*, int>> sorted;
	sorted.reserve(providerLayer.size());

	for (auto const& entry : providerLayer)
		sorted.emplace_back(entry.first, entry.second);

	std::sort(sorted.begin(), sorted.end(), [](auto const& a, auto const& b)
	{
		return a.second > b.second;
	});

	std::vector<TechnoClass*> providers;
	providers.reserve(sorted.size());

	for (auto const& entry : sorted)
		providers.push_back(entry.first);

	return providers;
}

namespace
{
	constexpr int DefaultSupportBulletWaitFrames = 900;

	int GetGlobalRelayLockoutFrames()
	{
		if (auto const pRulesExt = RulesExt::Global())
		{
			if (pRulesExt->PrismRelay_SupportTimeout > 0)
				return pRulesExt->PrismRelay_SupportTimeout;
		}

		return RulesClass::Instance->PrismSupportDelay;
	}
}

int PrismRelay::GetRelayLockoutFrames(AttachEffectTypeClass* pType)
{
	if (!pType)
		return GetGlobalRelayLockoutFrames();

	if (pType->PrismRelay_SupportTimeout > 0)
		return pType->PrismRelay_SupportTimeout;

	if (pType->PrismRelay_SupportTimeout == 0)
		return 0;

	return GetGlobalRelayLockoutFrames();
}

static AttachEffectTypeClass* ResolveRelayTypeForCooldown(TechnoClass* pTechno, int networkId)
{
	if (!pTechno)
		return nullptr;

	if (auto const pType = PrismRelay::GetRelayTypeForNetwork(pTechno, networkId))
		return pType;

	return PrismRelay::GetRelayType(pTechno);
}

static void StartPassCooldown(TechnoClass* pTechno, int networkId)
{
	if (!pTechno)
		return;

	auto const pRelayType = ResolveRelayTypeForCooldown(pTechno, networkId);
	const int cooldownFrames = PrismRelay::GetRelayLockoutFrames(pRelayType);

	if (cooldownFrames <= 0)
		return;

	if (auto const pExt = TechnoExt::ExtMap.TryFind(pTechno))
		pExt->PrismRelayCooldown.Start(cooldownFrames);
}

bool PrismRelay::IsOnCooldown(TechnoClass* pTechno)
{
	auto const pExt = TechnoExt::ExtMap.TryFind(pTechno);
	return pExt && pExt->PrismRelayCooldown.InProgress();
}

static bool IsRelayTechnoEligible(TechnoClass* pTechno)
{
	if (!pTechno)
		return false;

	if (pTechno->InLimbo || !pTechno->IsAlive || pTechno->Health <= 0)
		return false;

	if (!pTechno->IsInPlayfield || !pTechno->IsOnMap)
		return false;

	if (PrismRelay::IsOnCooldown(pTechno))
		return false;

	return true;
}

static bool HasActiveRelaySession(TechnoClass* pTechno)
{
	if (!pTechno)
		return false;

	auto const pExt = TechnoExt::ExtMap.TryFind(pTechno);

	return pExt && pExt->PrismRelay.Phase != PrismRelayPhase::Idle;
}

static bool HasEnemyCombatTarget(TechnoClass* pTechno)
{
	auto const pTarget = pTechno ? pTechno->Target : nullptr;

	if (!pTarget)
		return false;

	if (auto const pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
		return pTargetTechno->IsAlive && !pTechno->Owner->IsAlliedWith(pTargetTechno->Owner);

	return abstract_cast<CellClass*>(pTarget) != nullptr;
}

static bool IsTechnoEngagedInOwnAttack(TechnoClass* pTechno)
{
	if (!pTechno)
		return false;

	if (HasActiveRelaySession(pTechno))
		return true;

	if (pTechno->CurrentBurstIndex > 0)
		return true;

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building:
	{
		auto const pBuilding = abstract_cast<BuildingClass*>(pTechno);

		if (!pBuilding)
			break;

		if (pBuilding->DelayBeforeFiring > 0)
			return true;

		if (auto const pBldExt = BuildingExt::ExtMap.TryFind(pBuilding))
		{
			if (pBldExt->IsFiringNow)
				return true;
		}

		if (pBuilding->PrismStage == PrismChargeState::Master && HasEnemyCombatTarget(pTechno))
			return true;

		break;
	}
	case AbstractType::Infantry:
	case AbstractType::Unit:
	{
		auto const pFoot = abstract_cast<FootClass*>(pTechno);

		if (pFoot && pFoot->IsFiring)
			return true;

		if (auto const pUnit = abstract_cast<UnitClass*>(pTechno))
		{
			if (pUnit->CurrentFiringFrame >= 0)
				return true;
		}

		break;
	}
	case AbstractType::Aircraft:
	{
		auto const pAircraft = abstract_cast<AircraftClass*>(pTechno);

		if (pAircraft)
		{
			auto const status = static_cast<AirAttackStatus>(pAircraft->MissionStatus);

			if (status >= AirAttackStatus::FireAtTarget && status <= AirAttackStatus::FireAtTarget5_Strafe)
				return true;
		}

		break;
	}
	default:
		break;
	}

	if (!HasEnemyCombatTarget(pTechno))
		return false;

	return pTechno->SelectWeapon(pTechno->Target) >= 0;
}

bool PrismRelay::CanProvide(TechnoClass* pTechno, AttachEffectTypeClass* pType)
{
	if (!pType || !pType->PrismRelay || !pType->PrismRelay_Provider)
		return false;

	if (!IsRelayTechnoEligible(pTechno))
		return false;

	if (IsTechnoEngagedInOwnAttack(pTechno))
		return false;

	auto const pProviderType = GetRelayTypeForNetwork(pTechno, pType->PrismRelay_NetworkID);

	if (!pProviderType || !pProviderType->PrismRelay_Provider)
		return false;

	return pProviderType->PrismRelay_SupportWeapon != nullptr;
}

bool PrismRelay::CanReceive(TechnoClass* pTechno, AttachEffectTypeClass* pType)
{
	if (!pType || !pType->PrismRelay || !pType->PrismRelay_Receiver)
		return false;

	if (!IsRelayTechnoEligible(pTechno))
		return false;

	auto const pReceiverType = GetRelayTypeForNetwork(pTechno, pType->PrismRelay_NetworkID);

	return pReceiverType && pReceiverType->PrismRelay_Receiver;
}

bool PrismRelay::IsWeaponRelayAllowed(AttachEffectTypeClass* pType, WeaponTypeClass* pWeapon)
{
	if (!pType || !pWeapon)
		return false;

	if (pType->PrismRelay_AllowWeapons.size() > 0 && !pType->PrismRelay_AllowWeapons.Contains(pWeapon))
		return false;

	if (pType->PrismRelay_DisallowWeapons.size() > 0 && pType->PrismRelay_DisallowWeapons.Contains(pWeapon))
		return false;

	return true;
}

namespace
{
	bool IsWeaponIndexUsable(TechnoClass* pTechno, int weaponIndex)
	{
		if (!pTechno || weaponIndex < 0)
			return false;

		auto const pWeaponStruct = pTechno->GetWeapon(weaponIndex);

		return pWeaponStruct && pWeaponStruct->WeaponType;
	}

	int NormalizeWeaponIndex(TechnoClass* pTechno, int weaponIndex, int fallback)
	{
		return IsWeaponIndexUsable(pTechno, weaponIndex) ? weaponIndex : fallback;
	}
}

int PrismRelay::ResolveMasterFireWeaponIndex(TechnoClass* pMaster, AttachEffectTypeClass* pType, AbstractClass* pTarget, int initiatingWeaponIndex)
{
	if (!pMaster || !pType)
		return initiatingWeaponIndex;

	const int fallback = NormalizeWeaponIndex(pMaster, initiatingWeaponIndex, -1);

	if (pType->PrismRelay_MasterWeaponUseMultiWeaponSelection && pTarget)
	{
		auto const pMasterExt = TechnoExt::ExtMap.TryFind(pMaster);

		if (pMasterExt && pMasterExt->TypeExtData)
		{
			auto const pTypeExt = pMasterExt->TypeExtData;
			int selectedIndex = pTypeExt->SelectForceWeapon(pMaster, pTarget);

			if (selectedIndex < 0)
				selectedIndex = pTypeExt->SelectMultiWeapon(pMaster, pTarget);

			if (selectedIndex >= 0)
				return NormalizeWeaponIndex(pMaster, selectedIndex, fallback);
		}
	}

	if (pType->PrismRelay_MasterWeaponIndex >= 0)
		return NormalizeWeaponIndex(pMaster, pType->PrismRelay_MasterWeaponIndex, fallback);

	return fallback;
}

int PrismRelay::ApplyDamageBonus(int damage, const std::vector<TechnoClass*>& providers, int networkId)
{
	if (damage == 0 || providers.empty())
		return damage;

	for (auto const pProvider : providers)
	{
		if (!pProvider)
			continue;

		auto const pType = GetRelayTypeForNetwork(pProvider, networkId);

		if (!pType)
			continue;

		const double mult = pType->PrismRelay_SupportMultiplier;

		if (mult != 1.0)
			damage = static_cast<int>(damage * mult);

		const int add = pType->PrismRelay_DamageAdd;

		if (add != 0)
			damage += add;
	}

	return damage;
}

static void CompleteSupportPhase(TechnoExt::ExtData* pMasterExt);
static void OnSupportBulletEnded(TechnoExt::ExtData* pMasterExt);
static void OnSupportLayerWaveComplete(TechnoExt::ExtData* pMasterExt);
static void BeginSupportLayerFire(TechnoExt::ExtData* pMasterExt, int layer);
static void ScheduleSupportLayerFire(TechnoExt::ExtData* pMasterExt, int layer);
static void AbortSupportPhase(TechnoExt::ExtData* pMasterExt);
static void ProcessSupportFireSchedule(TechnoExt::ExtData* pMasterExt);

struct RelayNetworkResult
{
	std::vector<TechnoClass*> Nodes;
	std::vector<PrismRelaySupportEdge> Edges;
};

static bool ResolveSameFrameMaster(TechnoClass* pCandidate, WeaponTypeClass* /*pWeapon*/, int weaponIndex)
{
	auto const pScenExt = ScenarioExt::Global();
	const int frame = Unsorted::CurrentFrame;

	if (pScenExt->PrismRelayClaimFrame != frame)
	{
		pScenExt->PrismRelayClaimFrame = frame;
		pScenExt->PrismRelayClaimMaster = pCandidate;
		pScenExt->PrismRelayClaimWeaponIndex = weaponIndex;
		return true;
	}

	if (pScenExt->PrismRelayClaimMaster == pCandidate)
		return true;

	const int existingIndex = pScenExt->PrismRelayClaimWeaponIndex;

	if (weaponIndex != existingIndex)
		return weaponIndex < existingIndex;

	return ScenarioClass::Instance->Random.RandomRanged(0, 1) == 0;
}

static bool IsRelayCandidateEligible(TechnoClass* pProvider, TechnoClass* pHub, TechnoClass* pMaster,
	AttachEffectTypeClass* pType, WeaponTypeClass* pSupportWeapon, const std::vector<TechnoClass*>& marked)
{
	if (!pProvider || !pHub || !pType || !pSupportWeapon)
		return false;

	if (pProvider == pHub || pProvider == pMaster)
		return false;

	if (std::find(marked.begin(), marked.end(), pProvider) != marked.end())
		return false;

	if (!PrismRelay::CanProvide(pProvider, pType))
		return false;

	if (!pType->PrismRelay_ToAllies && pProvider->Owner != pMaster->Owner)
		return false;

	if (pType->PrismRelay_ToAllies && !EnumFunctions::CanTargetHouse(AffectedHouse::All, pMaster->Owner, pProvider->Owner))
		return false;

	const int searchRange = WeaponTypeExt::GetRangeWithModifiers(pSupportWeapon, pProvider);

	if (!searchRange)
		return false;

	const int dist = pProvider->DistanceFrom(pHub);

	if (searchRange != -512 && dist > searchRange)
		return false;

	return WeaponTypeExt::IsTargetInWeaponRange(pProvider, pHub, pSupportWeapon);
}

static std::vector<TechnoClass*> FindLinkCandidates(TechnoClass* pHub, TechnoClass* pMaster, AttachEffectTypeClass* pType,
	WeaponTypeClass* pSupportWeapon, const std::vector<TechnoClass*>& marked, int maxCount)
{
	std::vector<TechnoClass*> result;

	if (!pHub || !pMaster || !pType || !pSupportWeapon || maxCount <= 0)
		return result;

	const int searchRange = WeaponTypeExt::GetRangeWithModifiers(pSupportWeapon, pHub);
	std::vector<std::pair<TechnoClass*, int>> scored;

	const auto tryCandidate = [&](TechnoClass* pProvider)
	{
		if (!IsRelayCandidateEligible(pProvider, pHub, pMaster, pType, pSupportWeapon, marked))
			return;

		scored.emplace_back(pProvider, pProvider->DistanceFrom(pHub));
	};

	if (searchRange == -512)
	{
		for (auto const pProvider : TechnoClass::Array)
			tryCandidate(pProvider);
	}
	else
	{
		const double cellSpread = static_cast<double>(searchRange) / Unsorted::LeptonsPerCell;
		auto const& technos = Helpers::Alex::getCellSpreadItems(pHub->Location, cellSpread, true);

		for (auto const pProvider : technos)
			tryCandidate(pProvider);
	}

	std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b)
	{
		return a.second < b.second;
	});

	result.reserve(static_cast<size_t>(maxCount));

	for (auto const& entry : scored)
	{
		result.push_back(entry.first);

		if (static_cast<int>(result.size()) >= maxCount)
			break;
	}

	return result;
}

static RelayNetworkResult BuildNetwork(TechnoClass* pMaster, AttachEffectTypeClass* pType)
{
	RelayNetworkResult network;

	if (!pMaster || !pType)
		return network;

	if (!PrismRelay::CanReceive(pMaster, pType))
		return network;

	auto const pSupportWeapon = pType->PrismRelay_SupportWeapon;

	if (!pSupportWeapon)
		return network;

	std::vector<TechnoClass*> marked;
	std::vector<TechnoClass*> frontier;

	network.Nodes.push_back(pMaster);
	marked.push_back(pMaster);
	frontier.push_back(pMaster);

	const int maxReceiveLinks = pType->PrismRelay_MaxReceiveLinks;
	const int maxNodeLinks = pType->PrismRelay_MaxNodeLinks;
	int providerCount = 0;

	while (!frontier.empty())
	{
		if (maxReceiveLinks >= 0 && providerCount >= maxReceiveLinks)
			break;

		std::vector<TechnoClass*> nextFrontier;

		for (auto const pHub : frontier)
		{
			if (maxReceiveLinks >= 0 && providerCount >= maxReceiveLinks)
				break;

			const int remainingTotal = maxReceiveLinks >= 0 ? maxReceiveLinks - providerCount : INT_MAX;
			int perNodeLimit = remainingTotal;

			if (maxNodeLinks >= 0)
				perNodeLimit = std::min(maxNodeLinks, remainingTotal);

			if (perNodeLimit <= 0)
				continue;

			auto const candidates = FindLinkCandidates(pHub, pMaster, pType, pSupportWeapon, marked, perNodeLimit);

			for (auto const pProvider : candidates)
			{
				network.Edges.push_back({ pProvider, pHub, 0 });
				network.Nodes.push_back(pProvider);
				marked.push_back(pProvider);
				nextFrontier.push_back(pProvider);
				providerCount++;

				if (maxReceiveLinks >= 0 && providerCount >= maxReceiveLinks)
					break;
			}
		}

		if (nextFrontier.empty())
			break;

		frontier = std::move(nextFrontier);
	}

	// Fix layer assignment in a second pass for clarity.
	std::unordered_map<TechnoClass*, int> depth;
	depth[pMaster] = 0;

	for (auto& edge : network.Edges)
	{
		const int parentDepth = depth[edge.To];
		edge.Layer = parentDepth + 1;
		depth[edge.From] = edge.Layer;
	}

	std::sort(network.Edges.begin(), network.Edges.end(), [](const PrismRelaySupportEdge& a, const PrismRelaySupportEdge& b)
	{
		if (a.Layer != b.Layer)
			return a.Layer > b.Layer;

		return a.From < b.From;
	});

	return network;
}

static bool FireSupportBullet(TechnoClass* pFrom, TechnoClass* pTo, WeaponTypeClass* pWeapon, TechnoClass* pMaster);

static int GetMaxSupportLayer(const std::vector<PrismRelaySupportEdge>& edges)
{
	int maxLayer = 0;

	for (auto const& edge : edges)
		maxLayer = std::max(maxLayer, edge.Layer);

	return maxLayer;
}

static void FireSupportLayer(TechnoExt::ExtData* pMasterExt, int layer)
{
	if (!pMasterExt || layer <= 0)
		return;

	auto& sess = pMasterExt->PrismRelay;
	auto const pSupportWeapon = sess.RelayType ? sess.RelayType->PrismRelay_SupportWeapon : nullptr;
	auto* const pMaster = pMasterExt->OwnerObject();
	const int networkId = sess.RelayType ? sess.RelayType->PrismRelay_NetworkID : 0;

	if (!pSupportWeapon || !pMaster)
		return;

	for (auto const& edge : sess.SupportEdges)
	{
		if (edge.Layer != layer)
			continue;

		if (FireSupportBullet(edge.From, edge.To, pSupportWeapon, pMaster))
		{
			sess.PendingBullets++;
			StartPassCooldown(edge.From, networkId);
		}
	}
}

static bool FireSupportBullet(TechnoClass* pFrom, TechnoClass* pTo, WeaponTypeClass* pWeapon, TechnoClass* pMaster)
{
	if (!pFrom || !pTo || !pWeapon || !pMaster)
		return false;

	auto const pBulletType = pWeapon->Projectile;

	if (!pBulletType)
		return false;

	BulletClass* const pBullet = pBulletType->CreateBullet(
		pTo, pFrom, 0, pWeapon->Warhead,
		static_cast<int>(pWeapon->Speed), pWeapon->Bright);

	if (!pBullet)
		return false;

	pBullet->Owner = pFrom;

	auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
	pBulletExt->FirerHouse = pFrom->Owner;
	pBulletExt->PrismRelayMaster = pMaster;
	pBulletExt->PrismRelaySupportBullet = true;

	const auto firePos = pFrom->Location;
	BulletExt::SimulatedFiringUnlimbo(pBullet, pFrom->Owner, pWeapon, firePos, false);
	BulletExt::SimulatedFiringEffects(pBullet, pFrom->Owner, pFrom, true, true);

	return true;
}

static void BeginSupportLayerFire(TechnoExt::ExtData* pMasterExt, int layer)
{
	if (!pMasterExt || layer <= 0)
		return;

	auto& sess = pMasterExt->PrismRelay;
	sess.ActiveSupportLayer = layer;
	FireSupportLayer(pMasterExt, layer);

	if (sess.PendingBullets <= 0)
		OnSupportLayerWaveComplete(pMasterExt);
}

static void ScheduleSupportLayerFire(TechnoExt::ExtData* pMasterExt, int layer)
{
	if (!pMasterExt || layer <= 0)
		return;

	auto& sess = pMasterExt->PrismRelay;
	const int fireDelay = sess.RelayType ? sess.RelayType->PrismRelay_SupportFireDelay : 0;

	if (fireDelay > 0)
	{
		sess.PendingNextLayer = layer;
		sess.SupportFireTimer.Start(fireDelay);
		return;
	}

	BeginSupportLayerFire(pMasterExt, layer);
}

static void FireSupportWave(TechnoExt::ExtData* pMasterExt)
{
	if (!pMasterExt)
		return;

	auto& sess = pMasterExt->PrismRelay;
	auto const pSupportWeapon = sess.RelayType ? sess.RelayType->PrismRelay_SupportWeapon : nullptr;

	if (!pSupportWeapon || sess.SupportEdges.empty())
		return;

	sess.PendingBullets = 0;
	sess.ActiveSupportLayer = 0;
	sess.PendingNextLayer = 0;
	sess.SupportFireTimer.Stop();
	sess.Phase = PrismRelayPhase::Supporting;

	const int maxLayer = GetMaxSupportLayer(sess.SupportEdges);
	const int fireDelay = sess.RelayType ? sess.RelayType->PrismRelay_SupportFireDelay : 0;

	if (maxLayer <= 0)
		return;

	const int waitFrames = DefaultSupportBulletWaitFrames
		+ (maxLayer + 1) * std::max(fireDelay, 0)
		+ 120;

	if (waitFrames > 0)
		sess.Timeout.Start(waitFrames);

	ScheduleSupportLayerFire(pMasterExt, maxLayer);
}

static void OnSupportLayerWaveComplete(TechnoExt::ExtData* pMasterExt)
{
	if (!pMasterExt)
		return;

	auto& sess = pMasterExt->PrismRelay;

	if (sess.Phase != PrismRelayPhase::Supporting)
		return;

	const int nextLayer = sess.ActiveSupportLayer - 1;

	if (nextLayer < 1)
	{
		CompleteSupportPhase(pMasterExt);
		return;
	}

	ScheduleSupportLayerFire(pMasterExt, nextLayer);
}

static void ProcessSupportFireSchedule(TechnoExt::ExtData* pMasterExt)
{
	if (!pMasterExt)
		return;

	auto& sess = pMasterExt->PrismRelay;

	if (sess.Phase != PrismRelayPhase::Supporting || sess.PendingNextLayer <= 0)
		return;

	if (sess.SupportFireTimer.InProgress())
		return;

	sess.ActiveSupportLayer = sess.PendingNextLayer;
	sess.PendingNextLayer = 0;
	BeginSupportLayerFire(pMasterExt, sess.ActiveSupportLayer);
}

static void ApplyCooldown(const std::vector<TechnoClass*>& chain, int networkId)
{
	for (auto const pTechno : chain)
		StartPassCooldown(pTechno, networkId);
}

static bool IsTechnoInRelayNetwork(TechnoClass* pTechno, const std::vector<TechnoClass*>& nodes)
{
	return pTechno && std::find(nodes.begin(), nodes.end(), pTechno) != nodes.end();
}

static void ClearRelayNetworkUnitState(const std::vector<TechnoClass*>& nodes, TechnoClass* pMaster)
{
	for (auto const pTechno : nodes)
	{
		if (!pTechno || pTechno == pMaster)
			continue;

		// Another tower may be firing or running its own relay session at the same time.
		if (HasActiveRelaySession(pTechno))
			continue;

		auto const pTarget = abstract_cast<TechnoClass*>(pTechno->Target);

		if (IsTechnoInRelayNetwork(pTarget, nodes))
			pTechno->SetTarget(nullptr);
	}
}

static void CompleteSupportPhase(TechnoExt::ExtData* pMasterExt)
{
	if (!pMasterExt)
		return;

	auto* const pMaster = pMasterExt->OwnerObject();
	auto& sess = pMasterExt->PrismRelay;

	sess.Timeout.Stop();

	pMasterExt->PrismRelayCachedProviders = PrismRelay::BuildDamageProvidersFromEdges(sess.SupportEdges);
	pMasterExt->PrismRelayCachedNetworkId = sess.RelayType ? sess.RelayType->PrismRelay_NetworkID : 0;

	ClearRelayNetworkUnitState(sess.NetworkNodes, pMaster);
	ApplyCooldown(sess.NetworkNodes, sess.RelayType ? sess.RelayType->PrismRelay_NetworkID : 0);

	sess.Phase = PrismRelayPhase::FiringMaster;

	const int fireWeaponIndex = PrismRelay::ResolveMasterFireWeaponIndex(
		pMaster, sess.RelayType, sess.EnemyTarget, sess.WeaponIndex);

	if (sess.EnemyTarget && fireWeaponIndex >= 0)
		pMaster->Fire(sess.EnemyTarget, fireWeaponIndex);

	sess.Reset();
}

static void AbortSupportPhase(TechnoExt::ExtData* pMasterExt)
{
	if (!pMasterExt)
		return;

	ClearRelayNetworkUnitState(pMasterExt->PrismRelay.NetworkNodes, nullptr);
	pMasterExt->PrismRelay.Reset();
	pMasterExt->PrismRelayBurstChainBuilt = false;
	pMasterExt->PrismRelayCachedNetworkId = 0;
	pMasterExt->PrismRelayCachedProviders.clear();
}

static void OnSupportBulletEnded(TechnoExt::ExtData* pMasterExt)
{
	if (!pMasterExt)
		return;

	auto& sess = pMasterExt->PrismRelay;

	if (sess.Phase != PrismRelayPhase::Supporting || sess.PendingBullets <= 0)
		return;

	if (--sess.PendingBullets > 0)
		return;

	OnSupportLayerWaveComplete(pMasterExt);
}

void PrismRelay::NotifyBulletDestroyed(BulletClass* pBullet)
{
	if (!pBullet)
		return;

	auto const pBulletExt = BulletExt::ExtMap.TryFind(pBullet);

	if (!pBulletExt || !pBulletExt->PrismRelaySupportBullet || pBulletExt->PrismRelayCounted)
		return;

	pBulletExt->PrismRelayCounted = true;

	if (auto const pMaster = pBulletExt->PrismRelayMaster)
	{
		if (auto const pMasterExt = TechnoExt::ExtMap.TryFind(pMaster))
			OnSupportBulletEnded(pMasterExt);
	}
}

void PrismRelay::UpdateSessionTimeouts(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.TryFind(pThis);

	if (!pExt || pExt->PrismRelay.Phase != PrismRelayPhase::Supporting)
		return;

	ProcessSupportFireSchedule(pExt);

	if (!pExt->PrismRelay.Timeout.InProgress())
		return;

	if (!pExt->PrismRelay.Timeout.Completed())
		return;

	AbortSupportPhase(pExt);
}

bool PrismRelay::TryHandleFireAt(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon, int weaponIndex)
{
	if (!pThis || !pTarget || !pWeapon)
		return false;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto& sess = pExt->PrismRelay;

	if (sess.Phase == PrismRelayPhase::FiringMaster)
	{
		sess.Phase = PrismRelayPhase::Idle;
		return false;
	}

	if (sess.Phase == PrismRelayPhase::Supporting)
		return true;

	auto const pRelayType = GetRelayType(pThis);

	if (!pRelayType)
		return false;

	if (!CanReceive(pThis, pRelayType))
		return false;

	if (!IsWeaponRelayAllowed(pRelayType, pWeapon))
		return false;

	if (IsOnCooldown(pThis))
		return false;

	if (!ResolveSameFrameMaster(pThis, pWeapon, weaponIndex))
		return false;

	if (pThis->CurrentBurstIndex != 0)
		return false;

	if (pExt->PrismRelayBurstChainBuilt)
		return false;

	auto const network = BuildNetwork(pThis, pRelayType);

	if (network.Edges.empty())
		return false;

	sess.NetworkNodes = network.Nodes;
	sess.SupportEdges = network.Edges;
	sess.SupportCount = static_cast<int>(network.Edges.size());
	sess.EnemyTarget = pTarget;
	sess.MasterWeapon = pWeapon;
	sess.WeaponIndex = weaponIndex;
	sess.RelayType = pRelayType;

	pExt->PrismRelayBurstChainBuilt = true;

	FireSupportWave(pExt);

	return sess.Phase == PrismRelayPhase::Supporting;
}
