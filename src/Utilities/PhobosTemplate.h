#pragma once

template<typename _Ty>
struct string_formatter
{
	using value_type = const char*;

	static constexpr value_type value = "%*";

	constexpr operator value_type() const noexcept
	{	// return stored value
		return (value);
	}

	[[nodiscard]] constexpr value_type operator()() const noexcept
	{	// return stored value
		return (value);
	}
};

template<typename T>
struct string_formatter<const T>
{
	static constexpr const char* value = string_formatter<T>::value;
};

template<>
struct string_formatter<int>
{
	static constexpr const char* value = "%d";
};

template<>
struct string_formatter<float>
{
	static constexpr const char* value = "%f";
};

template<>
struct string_formatter<double>
{
	static constexpr const char* value = "%lf";
};

template<>
struct string_formatter<char*>
{
	static constexpr const char* value = "%s";
};

template<>
struct string_formatter<wchar_t*>
{
	static constexpr const char* value = "%ls";
};

template<typename T>
struct string_formatter<T*>
{
	static constexpr const char* value = "%p";
};
