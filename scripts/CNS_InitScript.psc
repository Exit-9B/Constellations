Scriptname CNS_InitScript extends ReferenceAlias

Perk Property CNS_AlchemyEffects Auto
Perk Property CNS_EnchantingEffects Auto
Perk Property CNS_H2H_AutoPerk Auto
Perk Property CNS_BlackBookSeekerPerks Auto

int Property CurrentVersion = 1 AutoReadOnly
int KnownVersion = 0

Event OnInit()
	Update()
EndEvent

Event OnPlayerLoadGame()
	if KnownVersion != CurrentVersion
		Update()
	endif
EndEvent

Function Update()
	Actor actorRef = self.GetActorReference()
	if KnownVersion < 1
		actorRef.AddPerk(CNS_AlchemyEffects)
		actorRef.AddPerk(CNS_EnchantingEffects)
		actorRef.AddPerk(CNS_H2H_AutoPerk)
		actorRef.AddPerk(CNS_BlackBookSeekerPerks)
	endif

	KnownVersion = CurrentVersion
EndFunction
