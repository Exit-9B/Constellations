Scriptname CNS_MagicEnsureInFaction extends ActiveMagicEffect

Faction Property FactionToAdd Auto

Event OnEffectStart(Actor akTarget, Actor akCaster)
	akTarget.AddToFaction(FactionToAdd)
EndEvent
