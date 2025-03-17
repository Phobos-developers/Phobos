#pragma once

// This header contains simple wraps for C++20 coroutine
// Before C++23 we probably needs it
// Author: secsome

#include <coroutine>
#include <optional>

// The simplest coroutine which return an optional value
// Can be used as a state machine
template<typename T>
struct Generator
{
	struct promise_type;

	Generator(const Generator&) = delete;
	Generator& operator=(const Generator&) = delete;

	~Generator()
	{
		if (handle_)
			handle_.destroy();
	}

	struct promise_type
	{
		T value;

		auto get_return_object()
		{
			return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
		}

		auto initial_suspend()
		{
			return std::suspend_always();
		}

		auto final_suspend()
		{
			return std::suspend_always();
		}

		void return_void() { }

		auto yield_value(T v)
		{
			value = v;
			return std::suspend_always();
		}

		void unhandled_exception()
		{
			// std::terminate();
		}
	};

	std::optional<T> next()
	{
		std::optional<T> ret;

		if (handle_)
		{
			handle_.resume();
			if (!handle_.done())
				ret.emplace(handle_.promise().value);
		}

		return ret;
	}

private:
	std::coroutine_handle<promise_type> handle_;
};

#if 0
// Sample: Fibonacci sequence: 1, 1, 2, 3, 5, 8, 13, ...
// For this show case we just let it only have the first 12 elements
Generator<int> Fibonacci()
{
	co_yield 1;
	co_yield 1;
	int a = 1, b = 1;

	int cnt = 10;
	while (cnt--)
	{
		int c = a + b;
		a = b;
		b = c;
		co_yield c;
	}
}

void Test_Fibonacci()
{
	auto f = Fibonacci();
	while (true)
	{
		auto t = f.next();
		if (!t.has_value())
			break;
		printf("%d, ", t.value());
	}
	printf("\n");
}
#endif
