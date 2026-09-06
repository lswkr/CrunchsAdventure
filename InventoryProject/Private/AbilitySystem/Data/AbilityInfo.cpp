// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/AbilityInfo.h"

FIPAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	// for (const FIPAbilityInfo& AbilityInfo : AbilityInformation)
	// {
	// 	if (AbilityInfo.AbilityTag == AbilityTag)
	// 	{
	// 		return AbilityInfo;
	// 	}
	// }
	if (AbilityInfoMap.Contains(AbilityTag))
	{
		return AbilityInfoMap[AbilityTag];	
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find info for AbilityTag [%s]"), *AbilityTag.ToString());
	}
	return FIPAbilityInfo();
}

TSubclassOf<UGameplayAbility> UAbilityInfo::GetAbilityFromTag(const FGameplayTag& AbilityTag) const
{
	if (AbilityMap.Contains(AbilityTag))
	{
		return AbilityMap[AbilityTag];
	}
	return nullptr;
}

bool UAbilityInfo::IsPreAbilitiesAndPlayerLevelQualified(const FGameplayTag& AbilityTag, const TMap<FGameplayTag, int>& CurrentAbilitiesStatus, const int32 PlayerLevel) const
{
	// for (const FIPAbilityInfo AbilityInfo : AbilityInformation)
	// {
	// 	if (AbilityTag == AbilityInfo.AbilityTag)
	// 	{
	// 		for (const FAbilityLevelInfo LevelInfo : AbilityInfo.PreAbilitiesRequiringLevelInfo)
	// 		{
	// 			if (CurrentAbilitiesStatus[LevelInfo.AbilityTag] < LevelInfo.RequiredLevel)
	// 			{
	// 				return false;
	// 			}
	// 		}
	// 		return true;
	// 	}
	// }
	UE_LOG(LogTemp, Warning, TEXT("Is %s Qualified?"), *AbilityTag.ToString());
	
	if (AbilityInfoMap.Contains(AbilityTag))
	{
		if (AbilityInfoMap[AbilityTag].PlayerLevelRequirement>PlayerLevel)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s:  PlayerLevel is Low"), *AbilityTag.ToString());
			return false;
		}
		UE_LOG(LogTemp, Warning, TEXT("PreAbilitiesRequiringLevelInfo Num: %d"),AbilityInfoMap[AbilityTag].PreAbilitiesRequiringLevelInfo.Num());
		for (const FAbilityLevelInfo PreAbilityLevelInfo : AbilityInfoMap[AbilityTag].PreAbilitiesRequiringLevelInfo)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s's PreAbility %s Required Level %d: Current Level %d"),*AbilityTag.ToString(), *PreAbilityLevelInfo.AbilityTag.ToString(),PreAbilityLevelInfo.RequiredLevel, CurrentAbilitiesStatus[PreAbilityLevelInfo.AbilityTag]);
			if (CurrentAbilitiesStatus[PreAbilityLevelInfo.AbilityTag] < PreAbilityLevelInfo.RequiredLevel)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s's PreAbility %s Required Level %d. "),*AbilityTag.ToString(), *PreAbilityLevelInfo.AbilityTag.ToString(),PreAbilityLevelInfo.RequiredLevel);
				return false;
			}
		}
		return true;
	}
	return false;
}
