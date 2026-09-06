// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetControllers/OverlayWidgetController.h"

#include "IPGameplayTags.h"
#include "AbilitySystem/IPAbilitySystemComponent.h"
#include "AbilitySystem/IPAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/IPPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetIPAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetIPAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetIPAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetIPAS()->GetMaxMana());
	OnCharacterNameInit.Broadcast(GetIPPS()->GetCharacterName());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	//XP
	GetIPPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);

	//Level변경
	GetIPPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);

	//Player이름
	GetIPPS()->OnPlayerNameInitDelegate.AddLambda(
		[this](const FString& CharacterName)
		{
			OnCharacterNameInit.Broadcast(CharacterName);
		}
	);
	
	//HP
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetIPAS()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
	);

	//MaxHP
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetIPAS()->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
	);

	//MP
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetIPAS()->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
	);

	//MaxMP
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetIPAS()->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
	);
	//
	// if (GetIPASC())
	// {
	// 	GetIPASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
	// 	if (GetIPASC()->bStartupAbilitiesGiven)
	// 	{
	// 		BroadcastAbilityInfo();
	// 	}
	// 	else
	// 	{
	// 		GetIPASC()->AbilitiesGivenDelegate.AddUObject(this, &UIPWidgetController::BroadcastAbilityInfo);
	// 	}
	//
	// 	GetIPASC()->EffectAssetTags.AddLambda(
	// 		[this](const FGameplayTagContainer& AssetTags)
	// 		{
	// 			for (const FGameplayTag& Tag : AssetTags)
	// 			{
	// 				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
	// 				if (Tag.MatchesTag(MessageTag)) //Message라는 태그가 있다면
	// 				{
	// 					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
	// 					MessageWidgetRowDelegate.Broadcast(*Row); //Message출력
	// 				}
	// 			}
	// 		}
	// 	);
	// }
	
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetIPPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));
	
	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();
	
	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
	
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;
	
		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);
	
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

// void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
// 	const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
// {
// 	const FIPGameplayTags& GameplayTags = FIPGameplayTags::Get();
//
// 	FIPAbilityInfo LastSlotInfo;
// 	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
//
// 	AbilityInfoDelegate.Broadcast(LastSlotInfo, PreviousSlot, GameplayTags.Abilities_Status_Unlocked); //None상태의 Info구조체로 브로드캐스트 
//
// 	FIPAbilityInfo CurrentInfo = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
// 	AbilityInfoDelegate.Broadcast(CurrentInfo, Slot, Status);
// }
