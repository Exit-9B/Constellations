#pragma once

namespace Hooks
{
	class HandToHand final
	{
	public:
		static void WriteHooks();

	private:
		// Skill use for unarmed attacks
		static void ExperiencePatch();

		// Modify unarmed damage
		static void SkillMultiplierPatch();

		static void ProcessHandToHandXP(const RE::HitData& a_hitData);
		static void GetUnarmedDamage(const RE::ActorValueOwner* a_avOwner, float& a_damage);

		inline static REL::Relocation<decltype(&GetUnarmedDamage)> _GetUnarmedDamage;
	};
}
