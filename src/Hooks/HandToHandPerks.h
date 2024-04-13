#pragma once

namespace Hooks
{
	class HandToHandPerks final
	{
	public:
		static void WriteHooks();

		static void HandleBaseDamage(const RE::Actor* a_actor, float& a_damage);

	private:
		// Use unarmed weapon as equipped weapon for speed mult
		static void WeaponSpeedMultPatch();

		// Apply combat hit spell for unarmed attacks
		static void CombatHitSpellPatch();

		static void ApplyHitSpells(
			RE::Actor* a_attacker,
			RE::InventoryEntryData* a_weapon,
			bool a_leftHand,
			RE::Actor* a_target);

		inline static REL::Relocation<decltype(&ApplyHitSpells)> _ApplyHitSpells;
	};
}
