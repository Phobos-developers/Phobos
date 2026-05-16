#pragma once

struct DXRenderOptions
{
	static DXRenderOptions& Config()
	{
		static DXRenderOptions instance;
		return instance;
	}

	bool PreserveAspectRatio { true };
	bool WindowedBorder { true };
	bool StartFullscreen { true };
	bool PauseGameWhenLoseFocus { true };
};
