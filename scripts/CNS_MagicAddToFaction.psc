Scriptname CNS_MagicAddToFaction extends ActiveMagicEffect

Faction Property FactionToAdd Auto

Event OnEffectStart(Actor akTarget, Actor akCaster)
	if !akTarget.IsInFaction(FactionToAdd)
		akTarget.AddToFaction(FactionToAdd)
	else
		akTarget.ModFactionRank(FactionToAdd, 1)
	endif
EndEvent

Event OnEffectFinish(Actor akTarget, Actor akCaster)
	if akTarget.GetFactionRank(FactionToAdd) == 0
		akTarget.RemoveFromFaction(FactionToAdd)
	else
		akTarget.ModFactionRank(FactionToAdd, -1)
	endif
EndEvent
