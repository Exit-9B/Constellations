Scriptname CNS_InitScript extends ReferenceAlias

Perk Property CNS_AlchemyEffects Auto
Perk Property CNS_EnchantingEffects Auto
Perk Property CNS_H2H_AutoPerk Auto
Perk Property CNS_BlackBookSeekerPerks Auto

GlobalVariable Property SkillAthleticsLevel Auto
GlobalVariable Property SkillHandToHandLevel Auto
GlobalVariable Property SkillSorceryLevel Auto

int Property CurrentVersion = 1 AutoReadOnly
int KnownVersion = 0

Event OnInit()
	RegisterForSingleUpdate(1.0)
EndEvent

Event OnPlayerLoadGame()
	if KnownVersion != CurrentVersion
		DoUpdate()
	endif
EndEvent

Event OnUpdate()
	DoUpdate()
EndEvent

Function DoUpdate()
	Actor actorRef = self.GetActorReference()
	if KnownVersion < 1
		DoNewInstall()

		actorRef.AddPerk(CNS_AlchemyEffects)
		actorRef.AddPerk(CNS_EnchantingEffects)
		actorRef.AddPerk(CNS_H2H_AutoPerk)
		actorRef.AddPerk(CNS_BlackBookSeekerPerks)
	endif

	KnownVersion = CurrentVersion
EndFunction

Function DoNewInstall()
	int startingLevel = Game.GetGameSettingInt("iAVDSkillStart")
	SkillAthleticsLevel.SetValue(startingLevel)
	SkillHandToHandLevel.SetValue(startingLevel)
	SkillSorceryLevel.SetValue(startingLevel)
EndFunction
