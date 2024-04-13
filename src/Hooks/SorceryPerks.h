#pragma once

namespace Hooks
{
	class SorceryPerks final
	{
	public:
		static void WriteHooks();

	private:
		// Check for staff enchantments with the IsInList function
		static void IsInListPatch();

		// Use weapon speed mult for staff charge time
		static void ChargeTimePatch();

		// Absorb hostile spells to staff charge
		static void AbsorbChargePatch();

		// Reflect spells with absorb chance when resources are full
		static void ReflectSpellPatch();

		// Restore absorb chance after processing effects
		static void ReflectSpellPatch2();

		static std::int32_t GetItemIndex(const RE::BGSListForm* a_list, const RE::TESForm* a_item);
		static void SetCastingTimerForCharge(RE::ActorMagicCaster* a_caster);
		static void AbsorbMagicka(
			RE::ActorValueOwner* a_owner,
			RE::ACTOR_VALUE_MODIFIER a_modifier,
			int,
			float a_value);
		static bool CheckReflect(RE::Projectile* a_projectile, RE::Actor* a_actor);
		static void RestoreAbsorb(RE::Actor* a_actor);

		inline static REL::Relocation<decltype(&GetItemIndex)> _GetItemIndex;
		inline static REL::Relocation<decltype(&SetCastingTimerForCharge)> _SetCastingTimerForCharge;
	};
}
