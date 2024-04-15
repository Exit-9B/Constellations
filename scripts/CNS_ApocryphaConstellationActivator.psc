Scriptname CNS_ApocryphaConstellationActivator extends ObjectReference

Message Property DLC2AltarNoSoulsMSG Auto
Message Property DLC2AltarNoPerksInThisSkillMSG Auto
Message Property DLC2AltarNotInCombatMSG Auto
Message Property DLC2AltarPerkPointsRefundedSingular Auto
Message Property DLC2AltarPerkPointsRefundedPlural Auto
Spell Property DLC2ApocryphaRewardSpell Auto

Message Property AltarSkillMessageSingular Auto
Message Property AltarSkillMessagePlural Auto
FormList Property Perks Auto

Event OnInit()
	;Disable follower activation (even though you shouldn't be able to have Followers here).
	Self.SetNoFavorAllowed(True)
EndEvent

Auto State Waiting
	Event OnActivate(ObjectReference akActivator)
		GotoState("Busy")
		ConstellationActivated()
		GotoState("Waiting")
	EndEvent
EndState

State Busy
EndState

Function ConstellationActivated()
	Actor player = Game.GetPlayer()
	int iPerkCount = CountPerks()
	
	;Make sure the player is not in combat, has a Dragon Soul, and has perks to recover.
	if (player.IsInCombat())
		DLC2AltarNotInCombatMSG.Show()
		Return
	ElseIf (player.GetActorValue("DragonSouls") == 0)
		DLC2AltarNoSoulsMSG.Show()
		Return
	ElseIf (iPerkCount == 0)
		DLC2AltarNoPerksInThisSkillMSG.Show()
		Return
	EndIf
	
	;Confirm that the player wants to clear their perks before doing so.
	int confirmed = 0
	if (iPerkCount == 1)
		confirmed = AltarSkillMessageSingular.Show(iPerkCount)
	Else
		confirmed = AltarSkillMessagePlural.Show(iPerkCount)
	EndIf
	
	;If the player agreed, remove the perks.
	if (confirmed == 1)
		;Play VFX
		DLC2ApocryphaRewardSpell.Cast(Player)
		;Spend Dragon Soul.
		player.ModActorValue("DragonSouls", -1)
		;Remove Perks
		int iPerkPoints = CountPerks(true)
		;Restore Perk Points
		Game.AddPerkPoints(iPerkPoints)
		;Display confirmation message.
		if (iPerkPoints > 1)
			DLC2AltarPerkPointsRefundedPlural.Show(iPerkPoints)
		Else
			DLC2AltarPerkPointsRefundedSingular.Show(iPerkPoints)
		EndIf
	EndIf
EndFunction

int function CountPerks(bool bClearPerks = false)
; 	debug.trace(self + "CountPerks for skill=" + iSkill)
	; if bClearPerks = true, also clear the perks
		; iterate through form list of perks
		; give back each perk the player has, then remove perk
	actor player = Game.GetPlayer()
	int iItem = 0
	int iPerkCount = 0 ; how many perk points to give player when done
	while iItem < Perks.GetSize()
		if player.HasPerk(Perks.GetAt(iItem) as Perk)
; 			debug.trace(self + " player has " + Perks.GetAt(iItem) as Perk)
			iPerkCount = iPerkCount + 1
		EndIf
		if bClearPerks
; 			debug.trace(self + " - removing perk")
			player.RemovePerk(Perks.GetAt(iItem) as Perk)
		endif
		iItem = iItem + 1
	endWhile
; 	debug.trace(self + " total of " + iPerkCount + " perks")
	return iPerkCount
endFunction
