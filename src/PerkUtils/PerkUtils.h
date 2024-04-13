#pragma once

#include "RE/Offset.h"

#include <random>

namespace PerkUtils
{
	inline std::int32_t GetRandomPercent()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, 99);
		return distrib(gen);
	}

	inline std::int32_t GetRandomPercent(unsigned int a_seed)
	{
		std::mt19937 gen(a_seed);
		std::uniform_int_distribution<> distrib(0, 99);
		return distrib(gen);
	}

	inline std::int32_t GetRandomPercent(RE::TESObjectREFR* a_objseed)
	{
		const auto seed = a_objseed->GetHandle().native_handle();
		return GetRandomPercent(seed);
	}
}
