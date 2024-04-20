Scriptname CNS_InitScript extends ReferenceAlias

Perk Property CNS_AlchemyEffects Auto
Perk Property CNS_EnchantingEffects Auto
Perk Property CNS_H2H_AutoPerk Auto
Perk Property CNS_BlackBookSeekerPerks Auto

int Property CurrentVersion = 2 AutoReadOnly
int KnownVersion = 0

Event OnInit()
	RegisterForSingleUpdate(1)
EndEvent

Event OnPlayerLoadGame()
	if KnownVersion != CurrentVersion
		RegisterForSingleUpdate(0)
	endif
EndEvent

Event OnUpdate()
	Actor actorRef = self.GetActorReference()
	if KnownVersion < 2
		actorRef.AddPerk(CNS_AlchemyEffects)
		actorRef.AddPerk(CNS_EnchantingEffects)
		actorRef.AddPerk(CNS_H2H_AutoPerk)
		actorRef.AddPerk(CNS_BlackBookSeekerPerks)
	endif

	KnownVersion = CurrentVersion
EndEvent
