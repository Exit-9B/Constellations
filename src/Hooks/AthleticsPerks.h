#pragma once

namespace Hooks
{
	class AthleticsPerks final
	{
	public:
		static void WriteHooks();

		static void HandleSprintingCost(const RE::Actor* a_actor, float& a_cost);

	private:
		// Use jumping bonus for jump height
		static void JumpHeightPatch();

		// Evade arrows while sprinting
		static void ArrowEvadePatch();

		static float GetScale(RE::Actor* a_actor);

		static void CombatHit(
			RE::Actor* a_aggressor,
			RE::Actor* a_target,
			RE::Projectile* a_projectile,
			bool a_leftHand);

		static bool IsValidImpact(
			RE::TESObjectREFR* a_target,
			RE::hkpCollidable* a_collidable,
			RE::Projectile* a_projectile,
			RE::BGSProjectile* a_base);

		inline static REL::Relocation<decltype(&GetScale)> _GetScale;
		inline static REL::Relocation<decltype(&CombatHit)> _CombatHit;
		inline static REL::Relocation<decltype(&IsValidImpact)> _IsValidImpact;
	};
}
