#include "HandToHandPerks.h"

#include "Data/Lookup.h"
#include "Data/ModObjectManager.h"
#include "RE/Offset.h"

namespace Hooks
{
	void HandToHandPerks::WriteHooks()
	{
		WeaponSpeedMultPatch();
		CombatHitSpellPatch();
	}

	[[nodiscard]] static auto GetEquippedHands(const RE::Actor* a_actor)
		-> std::pair<const RE::TESObjectARMO*, const RE::ExtraDataList*>
	{
		const auto invChanges = const_cast<RE::Actor*>(a_actor)->GetInventoryChanges();
		if (invChanges && invChanges->entryList) {
			for (const auto& entry : *invChanges->entryList) {
				if (entry && entry->object && entry->extraLists) {
					const auto armor = entry->object->As<RE::TESObjectARMO>();
					if (armor &&
						armor->HasPartOf(RE::BGSBipedObjectForm::BipedObjectSlot::kHands)) {
						for (const auto& xList : *entry->extraLists) {
							if (xList && xList->HasType<RE::ExtraWorn>()) {
								return { armor, xList };
							}
						}
					}
				}
			}
		}
		return {};
	}

	[[nodiscard]] static float GetItemHealthDamageBonus(float a_health)
	{
		float healthMin = 1.0f;
		float healthRange = 1.0f;
		const auto fHealthDataValue1 = "fHealthDataValue1"_gs;
		const auto fHealthDataValue6 = "fHealthDataValue6"_gs;
		if (fHealthDataValue1 && fHealthDataValue6) {
			healthMin = *fHealthDataValue1;
			healthRange = *fHealthDataValue6 - healthMin;
		}

		const float healthRatio = (a_health - healthMin) / healthRange;

		// it could be argued that gauntlets should use fSmithingArmorMax, but our intent is to
		// balance unarmed against other weapons
		const float smithingMax = "fSmithingWeaponMax"_gs.value_or(10.0f);
		static constexpr float smithingMin = 1.0f;

		return (std::max)((healthRatio * (smithingMax - smithingMin)) + smithingMin, 0.0f);
	}

	void HandToHandPerks::HandleBaseDamage(const RE::Actor* a_actor, float& a_damage)
	{
		if (const auto perk = Data::ModObject<RE::BGSPerk>("UnarmedTemper"sv);
			perk && a_actor->HasPerk(perk)) {
			if (const auto& [armor, xList] = GetEquippedHands(a_actor); armor && xList) {
				if (const auto xHealth = xList->GetByType<RE::ExtraHealth>()) {
					const auto bonus = GetItemHealthDamageBonus(xHealth->health);
					a_damage += bonus * "UnarmedTemperMult"_gv.value_or(1.0f);
				}
			}
		}
	}

	[[nodiscard]] static RE::TESObjectWEAP* GetUnarmedWeapon()
	{
		static constinit RE::TESObjectWEAP* UnarmedWeapon = nullptr;
		if (!UnarmedWeapon) {
			UnarmedWeapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(0x1F4);
		}
		return UnarmedWeapon;
	}

	using GetCurrentWeapon_t = RE::TESObjectWEAP*(const RE::Actor*, bool);
	template <std::uint64_t ID, std::ptrdiff_t Off>
	struct CallHookImpl<struct WeaponSpeedTag, GetCurrentWeapon_t, ID, Off>
	{
		static RE::TESObjectWEAP* func(const RE::Actor* a_actor, bool a_leftHand)
		{
			auto weapon = util::call_original<&func>(a_actor, a_leftHand);
			if (!weapon) {
				const auto process = a_actor->currentProcess;
				const auto middleHigh = process ? process->middleHigh : nullptr;
				if (middleHigh) {
					const auto object = !a_leftHand ? middleHigh->rightHand : middleHigh->leftHand;
					if (!object) {
						weapon = GetUnarmedWeapon();
					}
				}
			}
			else if (weapon->IsStaff()) {
				// workaround issue where the left punch uses the speed of the staff
				weapon = nullptr;
			}
			return weapon;
		}
	};

	void HandToHandPerks::WeaponSpeedMultPatch()
	{
		// TRAMPOLINE: 28
		util::CallHook<
			struct WeaponSpeedTag,
			GetCurrentWeapon_t,
			RE::Offset::HandleWeaponSpeedChannel.id(),
			0xE>::write5();

		util::CallHook<
			struct WeaponSpeedTag,
			GetCurrentWeapon_t,
			RE::Offset::HandleLeftWeaponSpeedChannel.id(),
			0xE>::write5();
	}

	void HandToHandPerks::CombatHitSpellPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Actor::CombatHit, 0x194);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_ApplyHitSpells = trampoline.write_call<5>(
			hook.address(),
			&HandToHandPerks::ApplyHitSpells);
	}

	void HandToHandPerks::ApplyHitSpells(
		RE::Actor* a_attacker,
		RE::InventoryEntryData* a_weapon,
		bool a_leftHand,
		RE::Actor* a_target)
	{
		RE::InventoryEntryData* tempInstance = nullptr;
		if (!a_weapon &&
			a_attacker->actorState1.meleeAttackState != RE::ATTACK_STATE_ENUM::kBash) {
			tempInstance = new RE::InventoryEntryData(GetUnarmedWeapon(), 0);
			a_weapon = tempInstance;
		}
		_ApplyHitSpells(a_attacker, a_weapon, a_leftHand, a_target);
		delete tempInstance;
	}
}
