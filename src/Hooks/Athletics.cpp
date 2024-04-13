#include "Athletics.h"
#include "AthleticsPerks.h"

#include "Data/Lookup.h"
#include "Data/Skills.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace Hooks
{
	Athletics::SkillIncreaseHandler* Athletics::SkillIncreaseHandler::Instance()
	{
		static SkillIncreaseHandler instance{};
		return &instance;
	}

	void Athletics::WriteHooks()
	{
		ExperiencePatch();
		MoveSpeedPatch();
		SprintingCostPatch();
		SpeedMultModifiedPatch();
	}

	void Athletics::ExperiencePatch()
	{
		auto vtbl = REL::Relocation<std::uintptr_t>(RE::Offset::PlayerCharacter::Vtbl);
		_Update = vtbl.write_vfunc(173, &Athletics::Update);
	}

	void Athletics::MoveSpeedPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Actor::ComputeMovementType, 0x51);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetScale = trampoline.write_call<5>(hook.address(), &Athletics::GetSpeedMult);
	}

	void Athletics::SprintingCostPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Actor::UpdateSprinting, 0xCE);
		REL::make_pattern<"0F 57 05">().match_or_fail(hook.address());

		// TRAMPOLINE: 17
		struct Patch : Xbyak::CodeGenerator
		{
			Patch() : Xbyak::CodeGenerator(17, SKSE::GetTrampoline().allocate(17))
			{
				mov(rdx, rdi);
				jmp(ptr[rip]);
				dq(std::bit_cast<std::uintptr_t>(&Athletics::CalcSprintingStaminaMod));
			}
		};

		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x7);
		trampoline.write_call<5>(hook.address(), Patch().getCode());
	}

	void Athletics::SpeedMultModifiedPatch()
	{
		auto callbacks = REL::Relocation<std::uintptr_t>(
			RE::Offset::Actor::ActorValueModifiedCallbacks);

		_SpeedMultModifiedCallback = callbacks.write_vfunc(
			static_cast<std::size_t>(RE::ActorValue::kSpeedMult),
			&SpeedMultModifiedCallback);
	}

	static void ForceUpdateCachedMovementType(RE::Actor* a_actor)
	{
		using func_t = decltype(&ForceUpdateCachedMovementType);
		REL::Relocation<func_t> func{ RE::Offset::Actor::ForceUpdateCachedMovementType };
		return func(a_actor);
	}

	static void SpeedAffectingValueModified(RE::Actor* a_actor)
	{
		const auto saveLoadGame = RE::BGSSaveLoadGame::GetSingleton();
		if (!saveLoadGame->GetSaveGameLoading()) {
			ForceUpdateCachedMovementType(a_actor);
		}
	}

	RE::BSEventNotifyControl Athletics::SkillIncreaseHandler::ProcessEvent(
		const Event* a_event,
		[[maybe_unused]] RE::BSTEventSource<Event>* a_eventSource)
	{
		using enum RE::BSEventNotifyControl;

		if (a_event && std::strcmp(a_event->skillID, "Athletics") == 0) {
			SpeedAffectingValueModified(RE::PlayerCharacter::GetSingleton());
		}

		return kContinue;
	}

	void Athletics::Update(RE::PlayerCharacter* a_player, float a_delta)
	{
		_Update(a_player, a_delta);
		if (a_player->IsSprinting()) {
			float skillUse = a_delta;
			if (const auto customSkills = util::GetCustomSkillsInterface()) {
				customSkills->AdvanceSkill("Athletics", skillUse);
			}
		}
	}

	float Athletics::GetSpeedMult(const RE::Actor* a_actor)
	{
		float scale = _GetScale(a_actor);
		if (a_actor->IsPlayerRef()) {
			const float skill = (std::max)(Data::Athletics::GetLevel(), 0.0f);
			const float maxMult = "MovePCSkillMax"_gv.value_or(1.068f);
			const float minMult = "MovePCSkillMin"_gv.value_or(0.988f);

			const float skillMult = std::lerp(minMult, maxMult, skill / 100.0f);
			scale *= skillMult;
		}
		return scale;
	}

	float Athletics::CalcSprintingStaminaMod(float a_cost, const RE::Actor* a_actor)
	{
		float cost = a_cost;
		AthleticsPerks::HandleSprintingCost(a_actor, cost);

		if (a_actor->IsPlayerRef()) {
			const float skill = Data::Athletics::GetLevel();
			const float minMult = "SprintCostMin"_gv.value_or(1.0f);
			static constexpr float maxMult = 1.0f;

			if (minMult < maxMult) {
				cost *= std::lerp(maxMult, minMult, skill / 100.0f);
				cost = (std::max)(cost, 0.0f);
			}
		}

		// xorps xmm0, -1.0
		return -cost;
	}

	void Athletics::SpeedMultModifiedCallback(
		RE::Actor* a_actor,
		RE::ActorValue a_actorValue,
		float a_oldValue,
		float a_delta,
		RE::TESObjectREFR* a_cause)
	{
		SpeedAffectingValueModified(a_actor);

		if (_SpeedMultModifiedCallback.address()) {
			_SpeedMultModifiedCallback(a_actor, a_actorValue, a_oldValue, a_delta, a_cause);
		}
	}
}
