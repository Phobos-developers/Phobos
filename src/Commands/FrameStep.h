#pragma once

#include "Commands.h"

#include "FrameByFrame.h"

template<size_t Frame>
class FrameStepCommandClass : public CommandClass
{
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

template<size_t Frame>
inline const char* FrameStepCommandClass<Frame>::GetName() const
{
	// "StepXXFrames"
	class to_string_t
	{
	public:
		char buffer[Frame >= 10 ? 13 : 12];

	public:
		constexpr to_string_t() noexcept
			: buffer { "Step" }
		{
			size_t idx = 4;
			if constexpr (Frame < 10)
			{
				buffer[idx++] = Frame + '0';
				buffer[idx++] = 'F';
				buffer[idx++] = 'r';
				buffer[idx++] = 'a';
				buffer[idx++] = 'm';
				buffer[idx++] = 'e';
				buffer[idx++] = 's';
				buffer[idx++] = '\0';
			}
			else
			{
				buffer[idx++] = Frame / 10 + '0';
				buffer[idx++] = Frame % 10 + '0';
				buffer[idx++] = 'F';
				buffer[idx++] = 'r';
				buffer[idx++] = 'a';
				buffer[idx++] = 'm';
				buffer[idx++] = 'e';
				buffer[idx++] = 's';
				buffer[idx++] = '\0';
			}
		}

		constexpr operator char* () noexcept { return buffer; }
	};
	static to_string_t ret;
	return ret;
}

template<size_t Frame>
inline const wchar_t* FrameStepCommandClass<Frame>::GetUIName() const
{
	// L"Step forward XX frames."
	class to_string_t
	{
	public:
		wchar_t buffer[Frame >= 10 ? 23 : 22];

	public:
		constexpr to_string_t() noexcept
			: buffer { L"Step forward " }
		{
			size_t idx = 13;
			if constexpr (Frame < 10)
			{
				buffer[idx++] = Frame + '0';
				buffer[idx++] = ' ';
				buffer[idx++] = 'f';
				buffer[idx++] = 'r';
				buffer[idx++] = 'a';
				buffer[idx++] = 'm';
				buffer[idx++] = 'e';
				buffer[idx++] = 's';
				buffer[idx++] = '\0';
			}
			else
			{
				buffer[idx++] = Frame / 10 + '0';
				buffer[idx++] = Frame % 10 + '0';
				buffer[idx++] = ' ';
				buffer[idx++] = 'f';
				buffer[idx++] = 'r';
				buffer[idx++] = 'a';
				buffer[idx++] = 'm';
				buffer[idx++] = 'e';
				buffer[idx++] = 's';
				buffer[idx++] = '\0';
			}
		}

		constexpr operator wchar_t* () noexcept { return buffer; }
	};
	static to_string_t ret;
	return StringTable::TryFetchString("TXT_STEP_XX_FORWARD", ret);
}

template<size_t Frame>
inline const wchar_t* FrameStepCommandClass<Frame>::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

template<size_t Frame>
inline const wchar_t* FrameStepCommandClass<Frame>::GetUIDescription() const
{
	// L"Frame Step Only: Step forward XX frames."
	class to_string_t
	{
	public:
		wchar_t buffer[Frame >= 10 ? 41 : 40];

	public:
		constexpr to_string_t() noexcept
			: buffer { L"Frame Step Only: Step forward " }
		{
			size_t idx = 30;
			if constexpr (Frame < 10)
			{
				buffer[idx++] = Frame + '0';
				buffer[idx++] = ' ';
				buffer[idx++] = 'f';
				buffer[idx++] = 'r';
				buffer[idx++] = 'a';
				buffer[idx++] = 'm';
				buffer[idx++] = 'e';
				buffer[idx++] = 's';
				buffer[idx++] = '\0';
			}
			else
			{
				buffer[idx++] = Frame / 10 + '0';
				buffer[idx++] = Frame % 10 + '0';
				buffer[idx++] = ' ';
				buffer[idx++] = 'f';
				buffer[idx++] = 'r';
				buffer[idx++] = 'a';
				buffer[idx++] = 'm';
				buffer[idx++] = 'e';
				buffer[idx++] = 's';
				buffer[idx++] = '\0';
			}
		}

		constexpr operator wchar_t* () noexcept { return buffer; }
	};
	static to_string_t ret;
	return StringTable::TryFetchString("TXT_STEP_XX_FORWARD_DESC", ret);
}

template<size_t Frame>
inline void FrameStepCommandClass<Frame>::Execute(WWKey eInput) const
{
	if (!FrameByFrameCommandClass::FrameStep)
		return;

	Debug::LogAndMessage("Stepping %d frames...\n", Frame);

	FrameByFrameCommandClass::FrameStepCount = Frame;
}
