#pragma once

#include <array>

namespace SequenceRates
{
	struct Entry
	{
		const char* Name;
		int DefaultRate;
		bool Normalized;
	};

	inline constexpr std::array<Entry, 42> Entries = {{
		// Name               DefaultRate   Normalized
		// ---------------------------------------------
		{ "Ready",           0,          false },
		{ "Guard",           0,          false },
		{ "Prone",           6,          false },
		{ "Walk",            3,          false },
		{ "FireUp",          1,          false },
		{ "Down",            1,          false },
		{ "Crawl",           1,          false },
		{ "Up",              1,          false },
		{ "FireProne",       1,          false },
		{ "Idle1",           3,          true  }, // <-- true in vanilla
		{ "Idle2",           3,          true  }, // <-- true in vanilla
		{ "Die1",            1,          false },
		{ "Die2",            1,          false },
		{ "Die3",            1,          false },
		{ "Die4",            1,          false },
		{ "Die5",            1,          false },
		{ "Tread",           3,          false },
		{ "Swim",            1,          false },
		{ "WetIdle1",        3,          true  }, // <-- true in vanilla
		{ "WetIdle2",        3,          true  }, // <-- true in vanilla
		{ "WetDie1",         1,          false },
		{ "WetDie2",         1,          false },
		{ "WetAttack",       1,          false },
		{ "Hover",           2,          true  }, // <-- true in vanilla
		{ "Fly",             1,          false },
		{ "Tumble",          1,          false },
		{ "FireFly",         1,          false },
		{ "Deploy",          1,          false },
		{ "Deployed",        1,          false },
		{ "DeployedFire",    1,          false },
		{ "DeployedIdle",    1,          false },
		{ "Undeploy",        1,          false },
		{ "Cheer",           3,          true  }, // <-- true in vanilla
		{ "Paradrop",        1,          false },
		{ "AirDeathStart",   3,          false },
		{ "AirDeathFalling", 1,          false },
		{ "AirDeathFinish",  3,          false },
		{ "Panic",           4,          false },
		{ "Shovel",          6,          false },
		{ "Carry",           3,          false },
		{ "SecondaryFire",   1,          false },
		{ "SecondaryProne",  1,          false },
	}};
}
