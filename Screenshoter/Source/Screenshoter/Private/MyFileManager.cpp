// Fill out your copyright notice in the Description page of Project Settings.

#include "MyFileManager.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/UMG/Public/Components/ScrollBox.h"
#include "Runtime/UMG/Public/Components/Button.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"
#include "Runtime/UMG/Public/Components/HorizontalBox.h"
#include "Runtime/UMG/Public/Components/CheckBox.h"

#include "FMItemWidget.h"
#include "SSGameInstance.h"
#include "Runtime/UMG/Public/Components/Border.h"


/*UMyFileManager::UMyFileManager(const FObjectInitializer & ObjectInitializer) : Super(ObjectInitializer)
{

}*/

void UMyFileManager::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonExit && !(ButtonExit->OnReleased.IsBound()))
	{
		ButtonExit->OnReleased.AddDynamic(this, &UMyFileManager::Hide);
	}
	if (ButtonSend && !(ButtonSend->OnReleased.IsBound()))
	{
		ButtonSend->OnReleased.AddDynamic(this, &UMyFileManager::AddItem);
	}

	if (TextBlockButtonExit) TextBlockButtonExit->SetText(FText::FromString(NameTextBlockButtonExit));
	if (TextBlockButtonMode) TextBlockButtonMode->SetText(FText::FromString(NameTextBlockButtonMode));
	if (TextBlockButtonAddFolder) TextBlockButtonAddFolder->SetText(FText::FromString(NameTextBlockButtonAddFolder));
	if (TextBlockButtonSend) TextBlockButtonSend->SetText(FText::FromString(NameTextBlockButtonSend));
}

TSharedRef<SWidget> UMyFileManager::RebuildWidget()
{
	TSharedRef<SWidget> Widget = Super::RebuildWidget();

	RootLocation = GetRootLocationFromGameInstance();
	CurrentLocation = RootLocation;
	RebuildItems();
	
	return Widget;	
}

void UMyFileManager::RebuildItems()
{
	ClearItems();

	UE_LOG(LogTemp, Warning, TEXT("Path:%s"), *CurrentLocation)
	ScanDirectory(CurrentLocation);

	//	Add ItemUp widget
	if (CurrentLocation != RootLocation)
	{
		CreateItem(this, NameItemUp, NameFunctionChangeLocation);
	}

	//	Add Directory widgets
	if (FoundDirectories.Num() != 0)
	{
		for (FString Directory : FoundDirectories)
		{
			CreateItem(this, Directory, NameFunctionChangeLocation);
		}
	}

	//	Add File widgets
	if (FoundFiles.Num() != 0)
	{
		for (FString FileName : FoundFiles)
		{
			CreateItem(this, FileName, NameFunctionOpenPicture, FLinearColor::Green);
		}
	}
}

void UMyFileManager::CreateItem(UMyFileManager* Parent, FString& Header, FString& NameFunctionDoubleClick, FSlateColor Color)
{
	UFMItemWidget* Item = NewObject<UFMItemWidget>(this, UFMItemWidget::StaticClass());

	if (ScrollBoxFilesField)
	{
		ScrollBoxFilesField->AddChild(Item);
		Item->SetParent(Parent);
		Item->SetText(Header);
		//Item->Border->OnMouseButtonDownEvent.BindUFunction(this, FName(*NameFunctionButtonClick));
		//	Item->Border->OnMouseButtonDownEvent.BindDynamic()
		Item->Border->OnMouseDoubleClickEvent.BindUFunction(this, FName(*NameFunctionDoubleClick));
		Item->TextBlock->SetColorAndOpacity(Color);
	}
	Items.Emplace(Item);
}

void UMyFileManager::ClearItems()
{
	for (UFMItemWidget* Item : Items)
	{
		ScrollBoxFilesField->RemoveChild(Item);
	}
	Items.Empty();
}

void UMyFileManager::ScanDirectory(FString& CurrentDirectory)
{
	FoundDirectories.Empty();
	FoundFiles.Empty();
	IFileManager& FileMgr = IFileManager::Get();
	if (!FileMgr.DirectoryExists(*CurrentDirectory))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attention! Path is not exist - %s"), *CurrentDirectory)
		return;
	}

	FString Path = CurrentDirectory + "*";
	FileMgr.FindFiles(FoundDirectories, *Path, false, true);
	FileMgr.FindFiles(FoundFiles, *Path, true, false);
}

void UMyFileManager::AddItem()
{
	if (ScrollBoxFilesField)
	{
		UFMItemWidget* FMItemWidget = NewObject<UFMItemWidget>(this, UFMItemWidget::StaticClass());

		ScrollBoxFilesField->AddChild(FMItemWidget);

		UE_LOG(LogTemp, Warning, TEXT("FMItemWidget is created"))
		/*UHorizontalBox *HorizontalBox = NewObject<UHorizontalBox>(this, UHorizontalBox::StaticClass());
		UCheckBox* CheckBox = NewObject<UCheckBox>(this, UCheckBox::StaticClass());

		UTextBlock *tstText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
		tstText->SetText(FText::FromString(TEXT("Test Button")));
		UButton *tstBtn = NewObject<UButton>(this, UButton::StaticClass());
		tstBtn->AddChild(tstText);

		HorizontalBox->AddChild(CheckBox);
		HorizontalBox->AddChild(tstBtn);
		ScrollBoxFilesField->AddChild(HorizontalBox);*/
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NO ScrollBoxFilesField"))
	}

}

void UMyFileManager::ChangeLocation()
{
	if (ToFile == NameItemUp)		//	if put UP 
	{
		if (Path.Num())
		{
			Path.Pop();
		}
	}
	else if (ToFile == "")
	{
		UE_LOG(LogTemp, Warning, TEXT("Attention: ToFile is Empty. Nowhere to move"))
		return;
	}
	else
	{
		Path.Emplace(ToFile);
	}

	SetCurrentLocation();
	SetCurrentLocationOnGameInstance();

	RebuildItems();
}

void UMyFileManager::SetCurrentLocation()
{
	CurrentLocation = RootLocation;
	for (FString Dir : Path)
	{
		CurrentLocation = CurrentLocation + Dir + "/";
	}
}

void UMyFileManager::SetCurrentLocationOnGameInstance()
{
	USSGameInstance* SSGI = Cast<USSGameInstance>(GetWorld()->GetGameInstance());
	if (SSGI)
	{
		SSGI->SetPathToScreenshot(CurrentLocation);
	}
}

FString UMyFileManager::GetRootLocationFromGameInstance()
{
	USSGameInstance* SSGI = Cast<USSGameInstance>(GetWorld()->GetGameInstance());
	if (SSGI)
	{
		return SSGI->GetRootLocation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SS Game Instance isn't available"))
		return "";
	}
}

void UMyFileManager::OpenPicture()
{
	UE_LOG(LogTemp, Warning, TEXT("Open File %s"), *ToFile)
}

void UMyFileManager::SetToFile(FString& ToNewFile)
{
	ToFile = ToNewFile;
}

void UMyFileManager::Show()
{
	AddToViewport();
}
void UMyFileManager::Hide()
{
	RemoveFromViewport();
}
