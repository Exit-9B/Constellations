#include "Sorcery.h"

#include "Data/Lookup.h"
#include "Data/Skills.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace Hooks
{
	void Sorcery::WriteHooks()
	{
		FireAndForgetXPPatch();
		ConcentrationXPPatch();
		NoTargetXPPatch();
		StandardCustomXPPatch();
		BoundWeaponXPPatch();
		CommandActorXPPatch();
#if 0
		PowerPatch();
#else
		ManualCalcCostPatch();
		AutoCalcCostPatch();
#endif
	}

	void Sorcery::FireAndForgetXPPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::MagicTarget::AddTarget, 0x20B);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_CheckAddEffect = trampoline.write_call<5>(hook.address(), &Sorcery::SkillUseFireForget);
	}

	void Sorcery::ConcentrationXPPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::ActorMagicCaster::Update, 0xE8);
		REL::make_pattern<"4C 89 64 24 30">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label funcLbl;

				movss(xmm2, xmm8);
				mov(rdx, rdi);
				mov(rcx, ptr[rbx + 0xB8]);
				call(ptr[rip + funcLbl]);
				mov(ptr[rsp + 0x30], r12);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x5);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&Sorcery::SkillUseConcentration));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<5>(hook.address(), patch->getCode());
	}

	void Sorcery::NoTargetXPPatch()
	{
		// Light
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::MagicCaster::FindTargets, 0x524);
		REL::make_pattern<"48 8B 45 A8 48 89 44 24 48">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label funcLbl;

				mov(rdx, rsi);
				mov(rcx, ptr[rsp + 0x40]);
				call(ptr[rip + funcLbl]);
				mov(rax, ptr[rbp - 0x58]);
				mov(ptr[rsp + 0x48], rax);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x9);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&Sorcery::SkillUseWithoutTarget));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x9);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Sorcery::StandardCustomXPPatch()
	{
		// Value Modifier family, Detect Life, Telekinesis
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::ActiveEffect::DoStandardCustomSkillUsage,
			0xF5);
		REL::make_pattern<"48 8B 47 48 48 89 44 24 40">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label done;

				movaps(xmm1, xmm6);
				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				// ignore instant source because we don't want reflected spells to gain normal xp
				cmp(dword[rdi + offsetof(RE::ActiveEffect, castingSource)],
					static_cast<std::uint32_t>(RE::MagicSystem::CastingSource::kInstant));
				jz(done, T_SHORT);
				mov(rax, ptr[rdi + 0x48]);
				mov(ptr[rsp + 0x40], rax);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x9);

				L(done);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0xF9);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&Sorcery::SkillUseCustomDelta));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x9);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Sorcery::BoundWeaponXPPatch()
	{
		// Bound Weapon
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::BoundItemEffect::Update, 0x221);
		REL::make_pattern<"48 8B 47 48 48 89 44 24 38">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label funcLbl;

				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				mov(rcx, ptr[rdi + 0x40]);
				mov(rax, ptr[rdi + 0x48]);
				mov(ptr[rsp + 0x38], rax);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x9);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&Sorcery::SkillUseCustom));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x9);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Sorcery::CommandActorXPPatch()
	{
		// Summon Creature, Reanimate
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::Actor::UpdateCommandedActor,
			0x196);
		REL::make_pattern<"48 8B 47 48 48 89 44 24 40">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label funcLbl;

				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				mov(rcx, ptr[rdi + 0x40]);
				mov(rax, ptr[rdi + 0x48]);
				mov(ptr[rsp + 0x40], rax);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x9);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&SkillUseCustom));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x9);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	using EffectFlag = RE::EffectSetting::EffectSettingData::Flag;

	constexpr auto PowerPatches = std::to_array<std::tuple<REL::ID, std::ptrdiff_t, EffectFlag>>({
		// Spell duration
		{ RE::Offset::ActiveEffect::AdjustForPerks, 0x43, EffectFlag::kPowerAffectsDuration },
		// Spell magnitude
		{ RE::Offset::ActiveEffect::AdjustForPerks, 0x60, EffectFlag::kPowerAffectsMagnitude },
		// Target level (illusion, command, etc.) magnitude
		{ RE::Offset::ActiveEffectFactory::CheckTargetLevelMagnitude,
		  0x48,
		  EffectFlag::kPowerAffectsMagnitude },
		// Item card duration
		{ RE::Offset::ItemCard::GetMagicItemDescription,
		  0x31B,
		  EffectFlag::kPowerAffectsDuration },
		// Item card magnitude
		{ RE::Offset::ItemCard::GetMagicItemDescription,
		  0x33D,
		  EffectFlag::kPowerAffectsMagnitude },
		// Effect description magnitude
		{ RE::Offset::EffectItemReplaceTagsFunc::Invoke,
		  0x196,
		  EffectFlag::kPowerAffectsMagnitude },
		// Effect description duration
		{ RE::Offset::EffectItemReplaceTagsFunc::Invoke,
		  0x253,
		  EffectFlag::kPowerAffectsDuration },
	});

	static consteval EffectFlag PowerFlag(std::uint64_t a_id, std::ptrdiff_t a_offset)
	{
		return std::get<2>(*std::ranges::find_if(
			PowerPatches,
			[&](auto&& desc)
			{
				return std::get<0>(desc).id() == a_id && std::get<1>(desc) == a_offset;
			}));
	}

	using ModSpellPower_t =
		void(RE::BGSEntryPoint::ENTRY_POINT, RE::Actor*, RE::MagicItem*, RE::Actor*, float&);

	template <std::uint64_t ID, std::ptrdiff_t Off>
	struct CallHookImpl<struct SpellPowerTag, ModSpellPower_t, ID, Off>
	{
		static void func(
			RE::BGSEntryPoint::ENTRY_POINT a_entryPoint,
			RE::Actor* a_perkOwner,
			RE::MagicItem* a_spell,
			RE::Actor* a_target,
			float& a_power)
		{
			util::call_original<&func>(a_entryPoint, a_perkOwner, a_spell, a_target, a_power);

			if (a_perkOwner && a_perkOwner->IsPlayerRef()) {
				const auto spellType = a_spell->GetSpellType();
				const bool hasPowerFlag = std::ranges::any_of(
					a_spell->effects,
					[](auto&& effect)
					{
						return effect->baseEffect->data.flags.all(PowerFlag(ID, Off));
					});

				if (hasPowerFlag &&
					(spellType == RE::MagicSystem::SpellType::kStaffEnchantment ||
					 spellType == RE::MagicSystem::SpellType::kScroll)) {

					const float skill = (std::max)(Data::Sorcery::GetLevel(), 0.0f);

					const float maxMult = "MagicPowerPCSkillMax"_gv.value_or(1.5f);
					const float minMult = "MagicPowerPCSkillMin"_gv.value_or(1.0f);
					const float skillMult = std::lerp(minMult, maxMult, skill / 100.0f);

					a_power *= skillMult;
				}
			}
		}
	};

	void Sorcery::PowerPatch()
	{
		// TRAMPOLINE: 98
		[]<std::size_t... I>(std::index_sequence<I...>)
		{
			(util::CallHook<
				 struct SpellPowerTag,
				 ModSpellPower_t,
				 std::get<0>(PowerPatches[I]).id(),
				 std::get<1>(PowerPatches[I])>::write5(),
			 ...);
		}(std::make_index_sequence<PowerPatches.size()>());
	}

	void Sorcery::ManualCalcCostPatch()
	{
		auto vtbl = REL::Relocation<std::uintptr_t>(RE::Offset::EnchantmentItem::Vtbl);
		_AdjustCost = vtbl.write_vfunc(99, &Sorcery::AdjustCost);
	}

	void Sorcery::AutoCalcCostPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::MagicItem::GetCost, 0x88);
		REL::make_pattern<"0F 1F 84 00 00 00 00 00">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				Xbyak::Label calcDone;
				Xbyak::Label funcLbl;

				mov(r8, rsi);
				lea(rdx, ptr[rsp + 0x40]);
				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				cmp(al, 0);
				jnz(calcDone, T_SHORT);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x8);

				L(calcDone);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x3B);

				L(funcLbl);
				dq(std::bit_cast<std::uintptr_t>(&Sorcery::GetEnchantmentCost));
			}
		};

		auto patch = new Patch(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x8);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	static float GetSorcerySkillUsage(RE::MagicItem* a_spell, RE::Effect* a_effect = nullptr)
	{
		const auto spellType = a_spell->GetSpellType();
		if (spellType != RE::MagicSystem::SpellType::kStaffEnchantment &&
			spellType != RE::MagicSystem::SpellType::kScroll) {
			return 0.0f;
		}

		auto effect = a_effect;
		if (!effect) {
			effect = a_spell->GetCostliestEffectItem();
			if (!effect)
				return 0.0f;
		}

		auto cost = effect->cost;
		if (!a_spell->IsAutoCalc()) {
			auto costOverride = static_cast<float>(a_spell->GetData1()->costOverride);
			if (cost >= costOverride) {
				cost = costOverride;
			}
		}

		const auto baseEffect = effect->baseEffect;
		const auto archetype = baseEffect->GetArchetype();
		const bool customSkillUse = RE::EffectArchetypes::IsFlagSet(
			archetype,
			RE::EffectArchetypes::Flags::kCustomSkillUse);

		if ((a_effect != nullptr) ^ customSkillUse) {
			return 0.0f;
		}

		return cost * effect->baseEffect->data.skillUsageMult;
	}

	bool Sorcery::SkillUseFireForget(
		RE::MagicTarget* a_magicTarget,
		const RE::MagicTarget::AddTargetData& a_addTargetData)
	{
		const bool result = _CheckAddEffect(a_magicTarget, a_addTargetData);
		if (!result)
			return result;

		const auto& caster = a_addTargetData.caster;
		const auto& spell = a_addTargetData.magicItem;

		if (!caster || !caster->IsPlayerRef())
			return result;

		if (spell->GetCastingType() != RE::MagicSystem::CastingType::kFireAndForget)
			return result;

		const float skillUse = GetSorcerySkillUsage(spell);
		if (skillUse > 0.0f) {
			if (auto customSkills = util::GetCustomSkillsInterface()) {
				customSkills->AdvanceSkill("Sorcery", skillUse);
			}
		}
		return result;
	}

	void Sorcery::SkillUseConcentration(RE::Actor* a_caster, RE::MagicItem* a_spell, float a_delta)
	{
		if (!a_caster->IsPlayerRef())
			return;

		const float skillUse = GetSorcerySkillUsage(a_spell) * a_delta;
		if (skillUse > 0.0f) {
			if (auto customSkills = util::GetCustomSkillsInterface()) {
				customSkills->AdvanceSkill("Sorcery", skillUse);
			}
		}
	}

	void Sorcery::SkillUseWithoutTarget(RE::Actor* a_caster, RE::MagicItem* a_spell)
	{
		if (!a_caster->IsPlayerRef())
			return;

		const float skillUse = GetSorcerySkillUsage(a_spell);
		if (skillUse > 0.0f) {
			if (auto customSkills = util::GetCustomSkillsInterface()) {
				customSkills->AdvanceSkill("Sorcery", skillUse);
			}
		}
	}

	void Sorcery::SkillUseCustomDelta(RE::ActiveEffect* a_activeEffect, float a_delta)
	{
		const auto caster = a_activeEffect->caster.get();
		if (!caster || !caster->IsPlayerRef())
			return;

		const auto& spell = a_activeEffect->spell;
		const auto& effect = a_activeEffect->effect;

		float skillUse = GetSorcerySkillUsage(spell, effect);
		if (skillUse > 0.0f) {
			if (spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) {
				skillUse *= a_delta;
			}

			skillUse *= a_activeEffect->GetCustomSkillUseMagnitudeMultiplier(a_delta);

			if (auto customSkills = util::GetCustomSkillsInterface()) {
				customSkills->AdvanceSkill("Sorcery", skillUse);
			}

			a_activeEffect->flags.set(RE::ActiveEffect::Flag::kCustomSkillUse);
		}
	}

	void Sorcery::SkillUseCustom(RE::ActiveEffect* a_activeEffect)
	{
		const auto caster = a_activeEffect->caster.get();
		if (!caster || !caster->IsPlayerRef())
			return;

		const auto& spell = a_activeEffect->spell;
		const auto& effect = a_activeEffect->effect;
		const float skillUse = GetSorcerySkillUsage(spell, effect);
		if (skillUse > 0.0f) {
			if (auto customSkills = util::GetCustomSkillsInterface()) {
				customSkills->AdvanceSkill("Sorcery", skillUse);
			}
		}
	}

	static float AdjustMagicSpellCost(
		RE::ActorValueOwner* a_caster,
		float a_cost,
		std::int32_t a_skill)
	{
		using func_t = decltype(&AdjustMagicSpellCost);
		static REL::Relocation<func_t> func{ RE::Offset::MagicFormulas::AdjustMagicSpellCost };
		return func(a_caster, a_cost, a_skill);
	}

	void Sorcery::AdjustCost(
		const RE::EnchantmentItem* a_enchantment,
		float& a_cost,
		RE::Actor* a_caster)
	{
		if (!a_caster || !a_caster->IsPlayerRef()) {
			return _AdjustCost(a_enchantment, a_cost, a_caster);
		}

		const float skill = (std::max)(Data::Sorcery::GetLevel(), 0.0f);
		a_cost = AdjustMagicSpellCost(a_caster, a_cost, static_cast<std::int32_t>(skill));
	}

	bool Sorcery::GetEnchantmentCost(
		const RE::MagicItem* a_spell,
		float& a_cost,
		RE::Actor* a_caster)
	{
		if (a_spell->GetSpellType() != RE::MagicSystem::SpellType::kStaffEnchantment ||
			!a_caster || !a_caster->IsPlayerRef()) {
			return false;
		}

		float cost = 0.0f;
		for (const auto& effect : a_spell->effects) {
			cost += effect->cost;
		}

		a_spell->AdjustCost(cost, a_caster);

		a_cost = cost;
		return true;
	}
}
