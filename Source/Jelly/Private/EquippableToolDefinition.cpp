// Fill out your copyright notice in the Description page of Project Settings.


#include "EquippableToolDefinition.h"

#include "Tools/UEdMode.h"

UEquippableToolDefinition* UEquippableToolDefinition::CreateItemCopy(UObject* Outer) const
{
	if (!Outer)
	{
		Outer = GetTransientPackage();
	}
	return DuplicateObject<UEquippableToolDefinition>(this, Outer);
}
