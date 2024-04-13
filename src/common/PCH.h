#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;
using namespace RE::literals;

namespace logger = SKSE::log;

#include "CustomSkills/Interfaces.h"

class PluginAPIStorage
{
public:
	static PluginAPIStorage& get()
	{
		static PluginAPIStorage singleton{};
		return singleton;
	}

	CustomSkills::CustomSkillsInterface* customSkills;
};

template <typename Tag, typename, std::uint64_t ID, std::ptrdiff_t Off>
struct CallHookImpl;

template <typename Tag, typename R, typename... Args, std::uint64_t ID, std::ptrdiff_t Off>
struct CallHookImpl<Tag, R(Args...), ID, Off>
{
	static R func(Args...);
};

namespace util
{
	using SKSE::stl::report_and_fail;

	inline CustomSkills::CustomSkillsInterface* GetCustomSkillsInterface()
	{
		return PluginAPIStorage::get().customSkills;
	}

	template <auto F>
	struct original_func
	{
		inline static REL::Relocation<decltype(F)> value;
	};

	template <auto F>
	inline static const auto& call_original = original_func<F>::value;

	template <typename Tag, typename F, std::uint64_t ID, std::ptrdiff_t Off>
	struct CallHook
	{
		template <std::size_t N, bool TAIL = false>
		static bool write()
		{
			auto hook = REL::Relocation<std::uintptr_t>(REL::ID(ID), Off);
			[]()
			{
				if constexpr (N == 5)
					if constexpr (!TAIL)
						return REL::make_pattern<"E8">();
					else
						return REL::make_pattern<"E9">();
				else if constexpr (N == 6)
					if constexpr (!TAIL)
						return REL::make_pattern<"FF 15">();
					else
						return REL::make_pattern<"FF 25">();
			}()
				.match_or_fail(hook.address());

			std::uintptr_t addr =
				(SKSE::GetTrampoline().*
				 []()
				 {
					 if constexpr (!TAIL)
						 return &SKSE::Trampoline::write_call<N, F>;
					 else
						 return &SKSE::Trampoline::write_branch<N, F>;
				 }())(hook.address(), &CallHookImpl<Tag, F, ID, Off>::func);

			if constexpr (N == 6)
				addr = *reinterpret_cast<std::uintptr_t*>(addr);

			original_func<&CallHookImpl<Tag, F, ID, Off>::func>::value = addr;

			return true;
		}

		static bool write5() { return write<5, false>(); }
		static bool write5_tail() { return write<5, true>(); }
		static bool write6() { return write<6, false>(); }
		static bool write6_tail() { return write<6, true>(); }
	};

	[[nodiscard]] constexpr int ascii_tolower(int ch) noexcept
	{
		if (ch >= 'A' && ch <= 'Z')
			ch += 'a' - 'A';
		return ch;
	}

	struct iless
	{
		using is_transparent = int;

		template <std::ranges::contiguous_range S1, std::ranges::contiguous_range S2>
			requires(
				std::is_same_v<std::ranges::range_value_t<S1>, char> &&
				std::is_same_v<std::ranges::range_value_t<S2>, char>)
		constexpr bool operator()(S1&& a_str1, S2&& a_str2) const
		{
			std::size_t count = std::ranges::size(a_str2);
			const std::size_t len1 = std::ranges::size(a_str1);
			const bool shorter = len1 < count;
			if (shorter)
				count = len1;

			if (count) {
				const char* p1 = std::ranges::data(a_str1);
				const char* p2 = std::ranges::data(a_str2);

				do {
					const int ch1 = ascii_tolower(*p1++);
					const int ch2 = ascii_tolower(*p2++);
					if (ch1 != ch2)
						return ch1 < ch2;
				} while (--count);
			}

			return shorter;
		}
	};

	template <class T>
	using istring_map = std::map<std::string, T, iless>;
}

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"
