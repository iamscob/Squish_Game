// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinition.h"

#include "IDetailTreeNode.h"

UItemDefinition* UItemDefinition::CreateItemCopy(UObject* Outer) const
{
	UItemDefinition* ItemCopy = NewObject<UItemDefinition>(Outer); 
	ItemCopy->ID = this->ID;
	ItemCopy->ItemType = this->ItemType;
	ItemCopy->ItemText = this->ItemText;
	ItemCopy->WorldMesh = this->WorldMesh;
	return ItemCopy;
}
