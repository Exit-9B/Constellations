#pragma once

namespace RE
{
	namespace Offset
	{
		namespace ActiveEffect
		{
			constexpr auto AdjustForPerks = REL::ID(34053);
			constexpr auto DoStandardCustomSkillUsage = REL::ID(34100);
		}

		namespace ActiveEffectFactory
		{
			constexpr auto CheckTargetLevelMagnitude = REL::ID(34048);
		}

		namespace Actor
		{
			constexpr auto ActorValueModifiedCallbacks = REL::ID(403905);
			constexpr auto CheckAbsorb = REL::ID(38741);
			constexpr auto CombatHit = REL::ID(38627);
			constexpr auto ComputeMovementType = REL::ID(37943);
			constexpr auto ForceUpdateCachedMovementType = REL::ID(37941);
			constexpr auto Jump = REL::ID(37257);
			constexpr auto UpdateCommandedActor = REL::ID(38799);
			constexpr auto UpdateSprinting = REL::ID(38022);
		}

		namespace ActorMagicCaster
		{
			constexpr auto Update = REL::ID(34143);
			constexpr auto Vtbl = REL::ID(205828);
		}

		namespace BoundItemEffect
		{
			constexpr auto Update = REL::ID(34229);
		}

		namespace EnchantmentItem
		{
			constexpr auto Vtbl = REL::ID(186389);
		}

		namespace EffectItemReplaceTagsFunc
		{
			constexpr auto Invoke = REL::ID(51905);
		}

		namespace HitData
		{
			constexpr auto InitializeHitData = REL::ID(44001);
		}

		namespace ItemCard
		{
			constexpr auto GetMagicItemDescription = REL::ID(51900);
		}

		namespace MagicCaster
		{
			constexpr auto FindTargets = REL::ID(34412);
		}

		namespace MagicFormulas
		{
			constexpr auto AdjustMagicSpellCost = REL::ID(26505);
		}

		namespace MagicItem
		{
			constexpr auto GetCost = REL::ID(11321);
		}

		namespace MagicTarget
		{
			constexpr auto AddTarget = REL::ID(34526);
		}

		namespace MissileProjectile
		{
			constexpr auto AddImpact = REL::ID(44041);
		}

		namespace Projectile
		{
			constexpr auto CombatHit = REL::ID(44218);
			constexpr auto HandleSpellCollision = REL::ID(44205);
		}

		namespace Script
		{
			constexpr auto IsInListConditionFunction = REL::ID(21539);
		}

		constexpr auto HandleWeaponSpeedChannel = REL::ID(42779);
		constexpr auto HandleLeftWeaponSpeedChannel = REL::ID(42780);
	}
}
