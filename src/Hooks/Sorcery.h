#pragma once

namespace Hooks
{
	class Sorcery final
	{
	public:
		static void WriteHooks();

	private:
		// Skill use for fire and forget spells
		static void FireAndForgetXPPatch();

		// Skill use for concentration (non-aimed) spells
		static void ConcentrationXPPatch();

		// Skill use for spells that don't hit a target
		static void NoTargetXPPatch();

		// Skill use for standard custom archetypes
		static void StandardCustomXPPatch();

		// Skill use for bound weapon spells
		static void BoundWeaponXPPatch();

		// Skill use for summon/reanimate spells
		static void CommandActorXPPatch();

		// Modify spell power
		static void PowerPatch();

		// Modify enchantment cost for manual cost calc
		static void ManualCalcCostPatch();

		// Modify enchantment cost for auto cost calc
		static void AutoCalcCostPatch();

		static bool SkillUseFireForget(
			RE::MagicTarget* a_magicTarget,
			const RE::MagicTarget::AddTargetData& a_addTargetData);

		static void SkillUseConcentration(
			RE::Actor* a_caster,
			RE::MagicItem* a_magicItem,
			float a_delta);

		static void SkillUseWithoutTarget(RE::Actor* a_caster, RE::MagicItem* a_magicItem);

		static void SkillUseCustomDelta(RE::ActiveEffect* a_activeEffect, float a_delta);

		static void SkillUseCustom(RE::ActiveEffect* a_activeEffect);

		static void AdjustCost(
			const RE::EnchantmentItem* a_enchantment,
			float& a_cost,
			RE::Actor* a_caster);

		static bool GetEnchantmentCost(
			const RE::MagicItem* a_magicItem,
			float& a_cost,
			RE::Actor* a_caster);

		inline static REL::Relocation<decltype(&SkillUseFireForget)> _CheckAddEffect;
		inline static REL::Relocation<decltype(&AdjustCost)> _AdjustCost;
	};
}
