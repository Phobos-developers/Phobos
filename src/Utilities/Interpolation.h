#pragma once

#include <YRPPCore.h>

enum class InterpolationMode : BYTE
{
	None = 0,
	Linear = 1
};

namespace detail
{
	template <typename T>
	inline T interpolate(T first, T second, double percentage, InterpolationMode mode)
	{
		return first;
	}

	template <>
	inline double interpolate<double>(double first, double second, double percentage, InterpolationMode mode)
	{
		double result = first;

		switch (mode)
		{
		case InterpolationMode::Linear:
			result = first + ((second - first) * percentage);
			break;
		default:
			break;
		}

		return result;
	}

	template <>
	inline int interpolate<int>(int first, int second, double percentage, InterpolationMode mode)
	{
		return static_cast<int>(interpolate(static_cast<double>(first), static_cast<double>(second), percentage, mode));
	}

	template <>
	inline ColorStruct interpolate<ColorStruct>(ColorStruct first, ColorStruct second, double percentage, InterpolationMode mode)
	{
		ColorStruct result = first;

		switch (mode)
		{
		case InterpolationMode::Linear:
		{
			BYTE r = static_cast<BYTE>(first.R + ((second.R - first.R) * percentage));
			BYTE g = static_cast<BYTE>(first.G + ((second.G - first.G) * percentage));
			BYTE b = static_cast<BYTE>(first.B + ((second.B - first.B) * percentage));
			result = ColorStruct { r, g, b };
			break;
		}
		default:
			break;
		}

		return result;
	}
}
