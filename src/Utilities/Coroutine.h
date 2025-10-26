// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
