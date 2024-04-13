#pragma once

#include "ModObjectManager.h"

namespace Data
{
	namespace HandToHand
	{
		[[nodiscard]] inline float GetLevel()
		{
			const auto level = ModObject<RE::TESGlobal>("SkillHandToHandLevel");
			return level ? level->value : 0.0f;
		}
	}

	namespace Athletics
	{
		[[nodiscard]] inline float GetLevel()
		{
			const auto level = ModObject<RE::TESGlobal>("SkillAthleticsLevel");
			return level ? level->value : 0.0f;
		}
	}

	namespace Sorcery
	{
		[[nodiscard]] inline float GetLevel()
		{
			const auto level = ModObject<RE::TESGlobal>("SkillSorceryLevel");
			return level ? level->value : 0.0f;
		}
	}
}
