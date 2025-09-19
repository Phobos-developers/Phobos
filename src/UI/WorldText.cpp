#include <Phobos.h>
#include <Misc/FogOfWar.h>

#include <FlyingStrings.h>
#include <ColorScheme.h>

// Producer-side fog gating for world-anchored text
// No low-level text hooks, just don't spawn text in fog

namespace WorldTextFog {

	// Floating damage numbers - hide if unit location is fogged
	void SpawnDamageTextIfVisible(int damage, const CoordStruct& at, ColorSchemeIndex color) {
		if (!Fog::ShouldShowActiveAt(at))
			return; // Don't spawn damage text in fog

		// Create floating damage text
		wchar_t damageStr[32];
		swprintf_s(damageStr, L"-%d", damage);
		FlyingStrings::AddMoneyString(damageStr, at, color, nullptr, true);
	}

	// Bounty credit display - hide if target location is fogged
	void SpawnBountyTextIfVisible(int credits, const CoordStruct& at, HouseClass* pHouse) {
		if (!Fog::ShouldShowActiveAt(at))
			return; // Don't spawn bounty text in fog

		// Create floating credit text
		wchar_t creditStr[32];
		swprintf_s(creditStr, L"+$%d", credits);
		ColorSchemeIndex color = pHouse ? pHouse->ColorSchemeIndex : ColorSchemeIndex::Yellow;
		FlyingStrings::AddMoneyString(creditStr, at, color, pHouse, true);
	}

	// ProduceCashDisplay credits - hide if building location is fogged
	void SpawnCashDisplayTextIfVisible(int credits, const CoordStruct& at, HouseClass* pHouse) {
		if (!Fog::ShouldShowActiveAt(at))
			return; // Don't spawn cash display text in fog

		// Create floating cash text
		wchar_t cashStr[32];
		swprintf_s(cashStr, L"+$%d", credits);
		ColorSchemeIndex color = pHouse ? pHouse->ColorSchemeIndex : ColorSchemeIndex::Green;
		FlyingStrings::AddMoneyString(cashStr, at, color, pHouse, true);
	}

	// Generic floating text - hide if location is fogged
	void SpawnWorldTextIfVisible(const wchar_t* text, const CoordStruct& at, ColorSchemeIndex color, HouseClass* pHouse = nullptr) {
		if (!text || !Fog::ShouldShowActiveAt(at))
			return; // Don't spawn text in fog

		// Create floating text
		FlyingStrings::AddMoneyString(text, at, color, pHouse, true);
	}

	// Experience/promotion text - hide if unit location is fogged
	void SpawnPromotionTextIfVisible(const wchar_t* rankText, const CoordStruct& at, HouseClass* pHouse) {
		if (!rankText || !Fog::ShouldShowActiveAt(at))
			return; // Don't spawn promotion text in fog

		ColorSchemeIndex color = pHouse ? pHouse->ColorSchemeIndex : ColorSchemeIndex::White;
		FlyingStrings::AddMoneyString(rankText, at, color, pHouse, true);
	}
}

// Usage examples:
// Replace direct damage text creation:
//   FlyingStrings::AddMoneyString(damageStr, at, color, house, true);
// With:
//   WorldTextFog::SpawnDamageTextIfVisible(damage, at, color);
//
// Replace direct bounty text creation:
//   FlyingStrings::AddMoneyString(creditStr, at, color, house, true);
// With:
//   WorldTextFog::SpawnBountyTextIfVisible(credits, at, house);