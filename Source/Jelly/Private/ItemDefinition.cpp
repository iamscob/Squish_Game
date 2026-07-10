// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinition.h"

#include "IDetailTreeNode.h"

UItemDefinition* UItemDefinition::CreateItemCopy(UObject* Outer) const
{
	if (!Outer)
	{
		Outer = GetTransientPackage();
	}
	return DuplicateObject<UItemDefinition>(this, Outer);
}
