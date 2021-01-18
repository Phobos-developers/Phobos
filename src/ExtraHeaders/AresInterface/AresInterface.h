#pragma once
#include <HouseClass.h>

struct AresBuildTimeInfo {
	long Version;
	long BuildTime;
};

__interface __declspec(uuid("6B6A4584-0803-4E5B-ABD1-D461C580F5D2"))
	IAresBuildTime : IUnknown
{
	virtual HRESULT __stdcall GetBuildTime(
		unsigned int abs, int idxType, HouseClass const* pOwner,
		AresBuildTimeInfo* out) const = 0;
};

class AresInterface
{
private:
	static IUnknown* Instance;
	static bool isInit;

	static YRComPtr<IAresBuildTime> BuildTimeInterface;

public:
	static void Initialize();

	static long GetBuildTime(unsigned int abs, int idxType, HouseClass const* pOwner);
};
