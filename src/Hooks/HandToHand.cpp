#include "HandToHand.h"
#include "HandToHandPerks.h"

#include "Data/Skills.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace Hooks
{
	void HandToHand::WriteHooks()
	{
		ExperiencePatch();
		SkillMultiplierPatch();
	}

	void HandToHand::ExperiencePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Actor::CombatHit, 0x292);
		REL::make_pattern<"48 8B 8C 24 80 00 00 00">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label retn;
				Xbyak::Label funcLbl;

				lea(rcx, ptr[rsp + 0x50]);
				call(ptr[rip + funcLbl]);
				mov(rcx, ptr[rsp + 0x80]);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x8);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&HandToHand::ProcessHandToHandXP));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x8);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void HandToHand::SkillMultiplierPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::HitData::InitializeHitData, 0x24D);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetUnarmedDamage = trampoline.write_call<5>(
			hook.address(),
			&HandToHand::GetUnarmedDamage);
	}

	void HandToHand::ProcessHandToHandXP(const RE::HitData& a_hitData)
	{
		const auto aggressor = a_hitData.aggressor.get();
		if (!aggressor || !aggressor->GetIsPlayerOwner())
			return;

		if (!a_hitData.weapon || !a_hitData.weapon->IsHandToHandMelee())
			return;

		const float skillUse = aggressor->GetActorValue(RE::ActorValue::kUnarmedDamage);
		if (const auto customSkills = util::GetCustomSkillsInterface()) {
			customSkills->AdvanceSkill("HandToHand", skillUse);
		}
	}

	void HandToHand::GetUnarmedDamage(const RE::ActorValueOwner* a_avOwner, float& a_damage)
	{
		_GetUnarmedDamage(a_avOwner, a_damage);

		const auto actor = SKSE::stl::adjust_pointer<RE::Actor>(a_avOwner, -0xB8);
		HandToHandPerks::HandleBaseDamage(actor, a_damage);

		if (a_avOwner->GetIsPlayerOwner()) {
			const float skill = (std::max)(Data::HandToHand::GetLevel(), 0.0f);

			const float maxMult = "fDamagePCSkillMax"_gs.value_or(1.5f);
			const float minMult = "fDamagePCSkillMin"_gs.value_or(1.0f);
			const float skillMult = std::lerp(minMult, maxMult, skill / 100.0f);

			a_damage *= skillMult;
		}
	}
}
