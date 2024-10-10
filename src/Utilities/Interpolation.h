#pragma once

#include <YRPPCore.h>
#include "Enum.h"

namespace detail
{
	#define FROM_DOUBLE(type, first, second, percentage, mode) \
		 (type)(interpolate((double&)first, (double&)second, percentage, mode));

	template <typename T>
	inline T interpolate(T& first, T& second, double percentage, InterpolationMode mode)
	{
		return first;
	}

	template <>
	inline double interpolate<double>(double& first, double& second, double percentage, InterpolationMode mode)
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
	inline int interpolate<int>(int& first, int& second, double percentage, InterpolationMode mode)
	{
		return FROM_DOUBLE(int, first, second, percentage, mode);
	}

	template <>
	inline ColorStruct interpolate<ColorStruct>(ColorStruct& first, ColorStruct& second, double percentage, InterpolationMode mode)
	{
		BYTE r = FROM_DOUBLE(BYTE, first.R, second.R, percentage, mode);
		BYTE g = FROM_DOUBLE(BYTE, first.G, second.G, percentage, mode);
		BYTE b = FROM_DOUBLE(BYTE, first.B, second.B, percentage, mode);
		return ColorStruct { r, g, b };
	}

	template <>
	inline TranslucencyLevel interpolate<TranslucencyLevel>(TranslucencyLevel& first, TranslucencyLevel& second, double percentage, InterpolationMode mode)
	{
		int firstValue = first.GetIntValue();
		int secondValue = second.GetIntValue();
		int value = FROM_DOUBLE(int, firstValue, secondValue, percentage, mode);
		return TranslucencyLevel(value, true);
	}
}
