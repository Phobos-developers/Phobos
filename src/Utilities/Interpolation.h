#pragma once

#include <YRPPCore.h>
#include "Enum.h"

namespace detail
{
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
		double firstValue = first;
		double secondValue = second;
		return (int)interpolate(firstValue, secondValue, percentage, mode);
	}

	template <>
	inline BYTE interpolate<BYTE>(BYTE& first, BYTE& second, double percentage, InterpolationMode mode)
	{
		double firstValue = first;
		double secondValue = second;
		return (BYTE)interpolate(firstValue, secondValue, percentage, mode);
	}

	template <>
	inline ColorStruct interpolate<ColorStruct>(ColorStruct& first, ColorStruct& second, double percentage, InterpolationMode mode)
	{
		BYTE r = interpolate(first.R, second.R, percentage, mode);
		BYTE g = interpolate(first.G, second.G, percentage, mode);
		BYTE b = interpolate(first.B, second.B, percentage, mode);
		return ColorStruct { r, g, b };
	}

	template <>
	inline TranslucencyLevel interpolate<TranslucencyLevel>(TranslucencyLevel& first, TranslucencyLevel& second, double percentage, InterpolationMode mode)
	{
		double firstValue = first.GetIntValue();
		double secondValue = second.GetIntValue();
		int value = (int)interpolate(firstValue, secondValue, percentage, mode);
		return TranslucencyLevel(value, true);
	}
}
