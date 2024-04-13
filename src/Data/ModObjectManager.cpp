#include "ModObjectManager.h"

namespace Data
{
	ModObjectManager& ModObjectManager::Instance()
	{
		static ModObjectManager instance{};
		return instance;
	}

	RE::BSEventNotifyControl ModObjectManager::ProcessEvent(
		const RE::TESQuestInitEvent* a_event,
		[[maybe_unused]] RE::BSTEventSource<RE::TESQuestInitEvent>* a_eventSource)
	{
		using enum RE::BSEventNotifyControl;

		const auto quest = RE::TESForm::LookupByID<RE::TESQuest>(a_event->formID);
		if (quest && quest->formEditorID == QuestName) {
			Initialize(quest);
		}

		return kContinue;
	}

	void ModObjectManager::Reload()
	{
		const auto quest = RE::TESForm::LookupByEditorID<RE::TESQuest>(QuestName);

		if (!quest) {
			logger::warn("ModObjectManager: Failed to lookup quest: {}"sv, QuestName);
			return;
		}

		Initialize(quest);
	}

	void ModObjectManager::Initialize(RE::TESQuest* a_quest)
	{
		const auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		if (!vm) {
			logger::error("ModObjectManager: Failed to get VM"sv);
			return;
		}

		const auto bindPolicy = vm->GetObjectBindPolicy();
		const auto handlePolicy = vm->GetObjectHandlePolicy();

		if (!bindPolicy || !handlePolicy) {
			logger::error("ModObjectManager: Failed to get VM object policies"sv);
			return;
		}

		const auto handle = handlePolicy->GetHandleForObject(RE::TESQuest::FORMTYPE, a_quest);

		RE::BSTScrapHashMap<RE::BSFixedString, RE::BSScript::Variable> properties;
		std::uint32_t nonConverted;
		bindPolicy->GetInitialPropertyValues(handle, ScriptName, properties, nonConverted);

		objects.clear();

		for (const auto& [name, var] : properties) {
			if (const auto value = var.Unpack<RE::TESForm*>()) {
				objects.emplace(name, value);
			}
		}

		vm->ResetAllBoundObjects(handle);
	}

	RE::TESForm* ModObjectManager::Get(std::string_view a_key) const
	{
		if (const auto it = objects.find(a_key); it != objects.end())
			return it->second;
		return nullptr;
	}
}
