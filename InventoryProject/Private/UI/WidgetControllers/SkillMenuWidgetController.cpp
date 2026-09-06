// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetControllers/SkillMenuWidgetController.h"

#include "IPGameplayTags.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/IPAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/IPPlayerState.h"

void USkillMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	//UE_LOG(LogTemp, Log, TEXT("BroadcastInitialValues in USkillMenuWidgetController"));
	//AActor* AvatarActor = GetIPASC()->GetAvatarActor();
	
	SkillPointsChanged.Broadcast(GetIPPS()->GetSkillPoints());
}

void USkillMenuWidgetController::BindCallbacksToDependencies()
{
	//한 SkillMenuRow마다 바인드
	GetIPASC()->OnAbilityStatusChangedDelegate.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
	{
		/*
		 * Status가 Eligible이면 Button켜고 옮기기 불가.
		 * Unlock||Equipped면 Button켜고 옮기기 가능
		 */
		//UE_LOG(LogTemp, Warning, TEXT("%s StatusChanged. Status: %s, SkillPoints %d left"),*AbilityTag.ToString(), *StatusTag.ToString(), GetIPPS()->GetSkillPoints());
		
		FIPGameplayTags GameplayTags = FIPGameplayTags::Get();

		bool bSkillPointMoreThanZero = GetIPPS()->GetSkillPoints() > 0;
		
		if (!AbilityInfo)
		{
			return;
		}
		FIPAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
		
		if (StatusTag == GameplayTags.Abilities_Status_Eligible)
		{
			SkillRowDelegate.Broadcast(AbilityTag, bSkillPointMoreThanZero, false, 0);
		}
		else if (StatusTag == GameplayTags.Abilities_Status_Equipped || StatusTag == GameplayTags.Abilities_Status_Unlocked)
		{
			SkillRowDelegate.Broadcast(AbilityTag, bSkillPointMoreThanZero, true, NewLevel);
		}
		else
		{
			SkillRowDelegate.Broadcast(AbilityTag, false, false, 0);
		}
		if (AbilityInfo)
		{
			AbilityInfoDelegate.Broadcast(Info, FGameplayTag(), StatusTag); //인풋 태그는 따로 없을 수 있음
		}
	}
	);
	
	if (GetIPASC())
	{
		GetIPASC()->OnAbilityEquippedDelegate.AddUObject(this,&USkillMenuWidgetController::OnAbilityEquipped);
		
		if (GetIPASC()->bStartupAbilitiesGiven) //bStartupAbilitiesGiven는 캐릭터의 ASC에 어빌리티가 주어졌을 때 true가 됨
		{
			BroadcastAbilityInfo(); //직접 호출해서 ASC에 위젯 바인딩 시키면 됨.
		}
		else //안 주어졌을 경우 델리게이트에 바인딩하고, 그 뒤에 AddCharacterAbilities안에서 해당 델리게이트에 브로드캐스팅하면서 호출하게 됨
		{
			GetIPASC()->OnAbilitiesGivenDelegate.AddUObject(this, &USkillMenuWidgetController::BroadcastAbilityInfo);
		}

	}
	
}

void USkillMenuWidgetController::UpgradeSkill(const FGameplayTag& SkillTag)
{
	if (GetIPASC())
	{
		GetIPASC()->SpendSkillPoint(SkillTag);
		SkillPointsChanged.Broadcast(GetIPPS()->GetSkillPoints());
	}
}

void USkillMenuWidgetController::OnDroppedOnSkillQuickSlot(const FGameplayTag& AbilityTag, const FGameplayTag& QuickSlotTag)
{
	GetIPASC()->EquipAbility(AbilityTag, QuickSlotTag);
}

void USkillMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
	const FIPGameplayTags& GameplayTags = FIPGameplayTags::Get();
	
	FIPAbilityInfo LastSlotInfo;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;

	AbilityInfoDelegate.Broadcast(LastSlotInfo, PreviousSlot, GameplayTags.Abilities_Status_Unlocked ); //None상태의 Info구조체로 브로드캐스트 

	FIPAbilityInfo CurrentInfo = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	AbilityInfoDelegate.Broadcast(CurrentInfo, Slot, Status);
}

void USkillMenuWidgetController::BroadcastAbilityInfo()
{
	if (!GetIPASC()->bStartupAbilitiesGiven) return;
	FForEachAbilityDelegate BroadcastDelegate; 
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		FIPAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(IPAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		AbilityInfoDelegate.Broadcast(Info,IPAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec), IPAbilitySystemComponent->GetStatusFromSpec(AbilitySpec));
		//UE_LOG(LogTemp, Warning, TEXT("%s broadcast function binded. STATUS: %s"), *Info.AbilityTag.ToString(),*Info.StatusTag.ToString());
	});
	GetIPASC()->ForEachAbility(BroadcastDelegate);
	GetIPASC()->UpdateAbilitiesInfo();
}

