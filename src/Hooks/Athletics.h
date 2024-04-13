#pragma once

namespace Hooks
{
	class Athletics final
	{
	public:
		static void WriteHooks();

		class SkillIncreaseHandler final :
			public RE::BSTEventSink<CustomSkills::SkillIncreaseEvent>
		{
		public:
			using Event = CustomSkills::SkillIncreaseEvent;

			static SkillIncreaseHandler* Instance();

			RE::BSEventNotifyControl ProcessEvent(
				const Event* a_event,
				RE::BSTEventSource<Event>* a_eventSource) override;
		};

	private:
		// Skill use for sprinting
		static void ExperiencePatch();

		// Modify move speed
		static void MoveSpeedPatch();

		// Modify sprinting cost
		static void SprintingCostPatch();

		// Update cached movement when speed mult is modified (for enchantments/potions/etc)
		static void SpeedMultModifiedPatch();

		static void Update(RE::PlayerCharacter* a_player, float a_delta);

		static float GetSpeedMult(const RE::Actor* a_actor);

		static float CalcSprintingStaminaMod(float a_cost, const RE::Actor* a_actor);

		static void SpeedMultModifiedCallback(
			RE::Actor* a_actor,
			RE::ActorValue a_actorValue,
			float a_originalValue,
			float a_modValue,
			RE::TESObjectREFR* a_cause);

		inline static REL::Relocation<decltype(&Update)> _Update;
		inline static REL::Relocation<decltype(&GetSpeedMult)> _GetScale;
		inline static REL::Relocation<decltype(&SpeedMultModifiedCallback)>
			_SpeedMultModifiedCallback;
	};
}
