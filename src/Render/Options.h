#pragma once

struct RenderOptions {
	static RenderOptions& Config() {
		static RenderOptions instance;
		return instance;
	}

	bool PreserveAspectRatio { true };
	bool WindowedBorder { true };
	bool StartFullscreen { true };
	bool PauseGameWhenLoseFocus { true };
};
