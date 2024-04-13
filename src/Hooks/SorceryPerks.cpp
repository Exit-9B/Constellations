#include "SorceryPerks.h"

#include "Data/Lookup.h"
#include "Data/ModObjectManager.h"
#include "PerkUtils/PerkUtils.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

#undef GetObject

namespace Hooks
{
	void SorceryPerks::WriteHooks()
	{
		IsInListPatch();
		ChargeTimePatch();
		AbsorbChargePatch();
		ReflectSpellPatch();
		ReflectSpellPatch2();
	}

	void SorceryPerks::IsInListPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::Script::IsInListConditionFunction,
			0x46);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetItemIndex = trampoline.write_call<5>(hook.address(), &SorceryPerks::GetItemIndex);
	}

	void SorceryPerks::ChargeTimePatch()
	{
		auto vtbl = REL::Relocation<std::uintptr_t>(RE::Offset::ActorMagicCaster::Vtbl);
		_SetCastingTimerForCharge = vtbl.write_vfunc(20, &SorceryPerks::SetCastingTimerForCharge);
	}

	void SorceryPerks::AbsorbChargePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Actor::CheckAbsorb, 0x7F);
		REL::make_pattern<"44 8D 42 17 FF 53 30">().match_or_fail(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x7);
		trampoline.write_call<6>(hook.address(), &AbsorbMagicka);
	}

	void SorceryPerks::ReflectSpellPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::Projectile::HandleSpellCollision,
			0xAF);
		REL::make_pattern<"F3 0F 10 1D">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label hitDone;
				Xbyak::Label neg_1_0;

				mov(rdx, rsi);
				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				cmp(al, 0);
				jnz(hitDone);
				mov(rcx, rbx);
				movss(xmm3, ptr[rip + neg_1_0]);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x8);

				L(hitDone);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x1B6);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&SorceryPerks::CheckReflect));

				L(neg_1_0);
				dd(std::bit_cast<std::uint32_t>(-1.0f));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x8);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void SorceryPerks::ReflectSpellPatch2()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::Projectile::HandleSpellCollision,
			0x265);
		REL::make_pattern<"48 85 DB 74 1E">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label nullRef;
				Xbyak::Label funcLbl;

				mov(rcx, rsi);
				call(ptr[rip + funcLbl]);
				test(rbx, rbx);
				jz(nullRef);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x5);

				L(nullRef);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x23);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&SorceryPerks::RestoreAbsorb));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<5>(hook.address(), patch->getCode());
	}

	std::int32_t SorceryPerks::GetItemIndex(
		const RE::BGSListForm* a_list,
		const RE::TESForm* a_item)
	{
		const auto staffList = Data::ModObject<RE::BGSListForm>("StaffEnchantmentList"sv);
		if (a_list && a_list == staffList) {
			if (const auto enchantment = a_item->As<RE::EnchantmentItem>()) {
				if (enchantment->data.spellType == RE::MagicSystem::SpellType::kStaffEnchantment) {
					return 0;
				}
			}
			return -1;
		}
		else {
			return _GetItemIndex(a_list, a_item);
		}
	}

	void SorceryPerks::SetCastingTimerForCharge(RE::ActorMagicCaster* a_caster)
	{
		_SetCastingTimerForCharge(a_caster);

		const auto actor = a_caster->owner;
		if (!actor)
			return;

		const auto spellType = a_caster->currentSpell->GetSpellType();
		if (spellType == RE::MagicSystem::SpellType::kStaffEnchantment) {
#if 0
			float speedMult = 0.0f;

			switch (a_caster->actorCasterType) {
			case RE::MagicSystem::CastingSource::kLeftHand:
				speedMult = actor->GetActorValue(RE::ActorValue::kLeftWeaponSpeedMultiply);
				break;
			case RE::MagicSystem::CastingSource::kRightHand:
				speedMult = actor->GetActorValue(RE::ActorValue::kWeaponSpeedMult);
				break;
			}

			if (speedMult != 0.0f) {
				a_caster->castingTimer /= speedMult;
			}
#else
			if (const auto perk = Data::ModObject<RE::BGSPerk>("StaffSpeed"sv);
				perk && actor->HasPerk(perk)) {
				float speedMult = "StaffPerkSpeedMult"_gv.value_or(1.5f);

				float actorRank = 1.0f;
				float totalRank = 1.0f;
				for (auto nextPerk = perk->nextPerk; nextPerk; nextPerk = nextPerk->nextPerk) {
					++totalRank;
					if (actor->HasPerk(nextPerk)) {
						++actorRank;
					}
				}

				const float ratio = actorRank / totalRank;
				speedMult = 1.0f + ((speedMult - 1.0f) * ratio);
				if (speedMult != 0.0f) {
					a_caster->castingTimer /= speedMult;
				}
			}
#endif
		}
	}

	void SorceryPerks::AbsorbMagicka(
		RE::ActorValueOwner* a_owner,
		[[maybe_unused]] RE::ACTOR_VALUE_MODIFIER a_modifier,
		int,
		float a_value)
	{
		const auto actor = SKSE::stl::adjust_pointer<RE::Actor>(a_owner, -0xB8);
		const auto perk = Data::ModObject<RE::BGSPerk>("StaffAbsorb"sv);
		bool usePerk = perk && actor->HasPerk(perk);

		bool hasRightStaff = false;
		bool hasLeftStaff = false;
		if (const auto process = actor->currentProcess) {
			const auto rightObject = process->GetEquippedRightHand();
			const auto rightWeap = rightObject ? rightObject->As<RE::TESObjectWEAP>() : nullptr;
			hasRightStaff = rightWeap && rightWeap->IsStaff();

			const auto leftObject = process->GetEquippedLeftHand();
			const auto leftWeap = leftObject ? leftObject->As<RE::TESObjectWEAP>() : nullptr;
			hasLeftStaff = leftWeap && leftWeap->IsStaff();
		}

		usePerk &= hasRightStaff || hasLeftStaff;

		const float magickaDeficit = a_owner->GetPermanentActorValue(RE::ActorValue::kMagicka) -
			a_owner->GetActorValue(RE::ActorValue::kMagicka);

		if (!usePerk || a_value <= magickaDeficit) {
			a_owner->RestoreActorValue(
				RE::ACTOR_VALUE_MODIFIER::kDamage,
				RE::ActorValue::kMagicka,
				a_value);
			return;
		}

		a_owner->RestoreActorValue(
			RE::ACTOR_VALUE_MODIFIER::kDamage,
			RE::ActorValue::kMagicka,
			magickaDeficit);

		const float chargeValue = a_value - magickaDeficit;

		const float rightChargeDeficit = hasRightStaff
			? a_owner->GetPermanentActorValue(RE::ActorValue::kRightItemCharge) -
				a_owner->GetActorValue(RE::ActorValue::kRightItemCharge)
			: 0.0f;

		const float leftChargeDeficit = hasLeftStaff
			? a_owner->GetPermanentActorValue(RE::ActorValue::kLeftItemCharge) -
				a_owner->GetActorValue(RE::ActorValue::kLeftItemCharge)
			: 0.0f;

		const float totalDeficit = rightChargeDeficit + leftChargeDeficit;
		if (totalDeficit <= 0.0f) {
			return;
		}

		if (rightChargeDeficit > 0.0f) {
			a_owner->RestoreActorValue(
				RE::ACTOR_VALUE_MODIFIER::kDamage,
				RE::ActorValue::kRightItemCharge,
				chargeValue * rightChargeDeficit / totalDeficit);
		}

		if (leftChargeDeficit > 0.0f) {
			a_owner->RestoreActorValue(
				RE::ACTOR_VALUE_MODIFIER::kDamage,
				RE::ActorValue::kLeftItemCharge,
				chargeValue * leftChargeDeficit / totalDeficit);
		}
	}

	[[nodiscard]] static bool CanStaffReflect(RE::Projectile* a_projectile, RE::Actor* a_actor)
	{
		if (!a_actor || !a_projectile->spell)
			return false;

		if (a_projectile->spell->GetNoAbsorb()) {
			return false;
		}

		const auto perk = Data::ModObject<RE::BGSPerk>("StaffReflect"sv);
		if (!perk || !a_actor->HasPerk(perk)) {
			return false;
		}

		if (a_actor->GetActorValue(RE::ActorValue::kMagicka) <
			a_actor->GetPermanentActorValue(RE::ActorValue::kMagicka)) {
			return false;
		}

		const auto process = a_actor->currentProcess;
		if (!process)
			return false;

		const auto rightObject = process->GetEquippedRightHand();
		const auto rightWeap = rightObject ? rightObject->As<RE::TESObjectWEAP>() : nullptr;
		const bool hasRightStaff = rightWeap && rightWeap->IsStaff();

		const auto leftObject = process->GetEquippedLeftHand();
		const auto leftWeap = leftObject ? leftObject->As<RE::TESObjectWEAP>() : nullptr;
		const bool hasLeftStaff = leftWeap && leftWeap->IsStaff();

		if (!hasRightStaff && !hasLeftStaff) {
			return false;
		}

		if (hasRightStaff) {
			if (a_actor->GetActorValue(RE::ActorValue::kRightItemCharge) <
				a_actor->GetPermanentActorValue(RE::ActorValue::kRightItemCharge)) {
				return false;
			}
		}

		if (hasLeftStaff) {
			if (a_actor->GetActorValue(RE::ActorValue::kLeftItemCharge) <
				a_actor->GetPermanentActorValue(RE::ActorValue::kLeftItemCharge)) {
				return false;
			}
		}

		return true;
	}

	bool SorceryPerks::CheckReflect(RE::Projectile* a_projectile, RE::Actor* a_actor)
	{
		if (!CanStaffReflect(a_projectile, a_actor)) {
			return false;
		}

		const auto& spell = a_projectile->spell;
		const auto baseForm = static_cast<RE::BGSProjectile*>(a_projectile->data.objectReference);

		// Flame projectiles seem to always reorient to the caster, so skip them
		if (baseForm->data.types.any(RE::BGSProjectileData::Type::kFlamethrower)) {
			return false;
		}

		const auto reflectPercent = a_actor->GetActorValue(RE::ActorValue::kAbsorbChance);
		if (static_cast<std::int32_t>(reflectPercent) > PerkUtils::GetRandomPercent()) {
			const auto shooter = a_projectile->shooter.get();

			if (shooter) {
				RE::Projectile::LaunchData launchData;
				launchData.origin = a_projectile->GetPosition();
				launchData.projectile = baseForm;
				launchData.shooter = a_actor;
				launchData.combatController = a_actor->combatController;
				launchData.desiredTarget = shooter.get();
				launchData.parentCell = a_projectile->parentCell;
				launchData.spell = spell;
				launchData.castingSource = RE::MagicSystem::CastingSource::kInstant;
				launchData.power = a_projectile->power;
				launchData.scale = a_projectile->scale;

				const auto angle = RE::CombatUtilities::GetAngleToProjectedTarget(
					a_projectile->GetPosition(),
					shooter.get(),
					baseForm->data.speed,
					baseForm->data.gravity,
					RE::ACTOR_LOS_LOCATION::kTorso);

				launchData.heading = angle.z;
				launchData.pitch = angle.x;
				launchData.unk9E = false;  // fire at the specified angle
				launchData.useOrigin = true;
				launchData.unkA2 = true;

				RE::Projectile::Launch(launchData);
			}

			if (spell->GetCastingType() != RE::MagicSystem::CastingType::kConcentration &&
				baseForm->data.types != RE::BGSProjectileData::Type::kBeam) {

				a_projectile->Kill();
				a_projectile->explosion = nullptr;
			}

			const auto defaultObjects = RE::BGSDefaultObjectManager::GetSingleton();
			const auto absorbEffect = defaultObjects->GetObject<RE::BGSArtObject>(
				RE::DEFAULT_OBJECT::kArtObjectAbsorbEffect);

			a_actor->ApplyArtObject(
				absorbEffect,
				"fMagicAbsorbVisualTimer"_gs.value(),
				shooter.get(),
				false,
				true,
				nullptr,
				false);

			a_projectile->flags.set(RE::Projectile::Flags::kProcessedImpacts);
			return true;
		}
		else {
			a_actor->RestoreActorValue(
				RE::ACTOR_VALUE_MODIFIER::kDamage,
				RE::ActorValue::kAbsorbChance,
				reflectPercent);
			return false;
		}
	}

	void SorceryPerks::RestoreAbsorb(RE::Actor* a_actor)
	{
		if (a_actor) {
			const auto permanent = a_actor->GetPermanentActorValue(RE::ActorValue::kAbsorbChance);
			a_actor->RestoreActorValue(
				RE::ACTOR_VALUE_MODIFIER::kDamage,
				RE::ActorValue::kAbsorbChance,
				permanent);
		}
	}
}
