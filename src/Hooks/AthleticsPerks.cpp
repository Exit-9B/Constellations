#include "AthleticsPerks.h"

#include "Data/Lookup.h"
#include "Data/ModObjectManager.h"
#include "PerkUtils/PerkUtils.h"

namespace Hooks
{
	void AthleticsPerks::WriteHooks()
	{
		JumpHeightPatch();
		ArrowEvadePatch();
	}

	void AthleticsPerks::HandleSprintingCost(const RE::Actor* a_actor, float& a_cost)
	{
		if (const auto perk = Data::ModObject<RE::BGSPerk>("SprintReduceCost"sv);
			perk && a_actor->HasPerk(perk)) {
			float costMult = "SprintPerkCostMult"_gv.value_or(0.75f);

			float actorRank = 1.0f;
			float totalRank = 1.0f;
			for (auto nextPerk = perk->nextPerk; nextPerk; nextPerk = nextPerk->nextPerk) {
				++totalRank;
				if (a_actor->HasPerk(nextPerk)) {
					++actorRank;
				}
			}

			const float ratio = actorRank / totalRank;
			costMult = (std::max)(1.0f - (1.0f - costMult) * ratio, 0.0f);

			a_cost *= costMult;
		}
	}

	void AthleticsPerks::JumpHeightPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Actor::Jump, 0x17F);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetScale = trampoline.write_call<5>(hook.address(), &AthleticsPerks::GetScale);
	}

	void AthleticsPerks::ArrowEvadePatch()
	{
		auto hook1 = REL::Relocation<std::uintptr_t>(
			RE::Offset::MissileProjectile::AddImpact,
			0x73);
		auto hook2 = REL::Relocation<std::uintptr_t>(RE::Offset::Projectile::CombatHit, 0x90);

		REL::make_pattern<"E8">().match_or_fail(hook1.address());
		REL::make_pattern<"E8">().match_or_fail(hook2.address());

		// TRAMPOLINE: 28
		auto& trampoline = SKSE::GetTrampoline();
		_IsValidImpact = trampoline.write_call<5>(hook1.address(), &AthleticsPerks::IsValidImpact);
		_CombatHit = trampoline.write_call<5>(hook2.address(), &AthleticsPerks::CombatHit);
	}

	float AthleticsPerks::GetScale(RE::Actor* a_actor)
	{
		float result = _GetScale(a_actor);

		const float jumpingBonus = a_actor->GetActorValue(RE::ActorValue::kJumpingBonus);
		result *= (1.0f + jumpingBonus / 100.0f);

		return result;
	}

	[[nodiscard]] static bool ShouldEvade(RE::Actor* a_actor, RE::Projectile* a_projectile)
	{
		if (a_projectile->spell)
			return false;

		if (const auto perk = Data::ModObject<RE::BGSPerk>("SprintEvade"sv);
			perk && a_actor->HasPerk(perk)) {
			if (a_actor->IsSprinting()) {
				const std::int32_t evadeChance = static_cast<std::int32_t>(
					"SprintPerkEvadeChance"_gv.value_or(50.0f));

				if (PerkUtils::GetRandomPercent(a_projectile) < evadeChance) {
					return true;
				}
			}
		}

		return false;
	}

	void AthleticsPerks::CombatHit(
		RE::Actor* a_aggressor,
		RE::Actor* a_target,
		RE::Projectile* a_projectile,
		bool a_leftHand)
	{
		if (a_projectile && ShouldEvade(a_target, a_projectile)) {
			return;
		}

		return _CombatHit(a_aggressor, a_target, a_projectile, a_leftHand);
	}

	bool AthleticsPerks::IsValidImpact(
		RE::TESObjectREFR* a_target,
		RE::hkpCollidable* a_collidable,
		RE::Projectile* a_projectile,
		RE::BGSProjectile* a_base)
	{
		const auto actor = a_target->As<RE::Actor>();
		if (actor && ShouldEvade(actor, a_projectile)) {
			return false;
		}

		return _IsValidImpact(a_target, a_collidable, a_projectile, a_base);
	}
}
