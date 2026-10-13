// Plays the example map (/TensorFlow/Examples/MnistExample) in PIE: the model trains (or loads), test digits are
// drawn into the canvas, the camera stream classifies them, and the N key asks for the next digit. Needs a real
// RHI (-RenderOffscreen) and the editor; a screenshot lands in Saved/Screenshots.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "TensorFlowComponent.h"
#include "Components/TextRenderComponent.h"
#include "DynamicRHI.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "UnrealClient.h"

namespace TensorFlowExampleMapTests
{
	struct FState
	{
		double Start = 0.0;
		double Deadline = 0.0;
		TWeakObjectPtr<AActor> Example;
		FString LastStatus;
		TArray<FString> StatusLines;
		FString Matched;
		int32 Phase = 0;
		double PhaseStart = 0.0;
		FString DigitBeforeKey;
	};

	static FString TextOf(AActor* Actor, const TCHAR* ComponentName)
	{
		for (UActorComponent* C : Actor->GetComponents())
		{
			if (UTextRenderComponent* T = Cast<UTextRenderComponent>(C); T && T->GetName() == ComponentName)
			{
				return T->Text.ToString();
			}
		}
		return FString();
	}

	/** "Drawn: test image 123, a 7" -> 7; "Camera sees: 7 (98%)" -> 7. */
	static int32 DigitIn(const FString& Text, const TCHAR* Marker)
	{
		const int32 At = Text.Find(Marker);
		if (At == INDEX_NONE || At + FCString::Strlen(Marker) >= Text.Len())
		{
			return -1;
		}
		const TCHAR C = Text[At + FCString::Strlen(Marker)];
		return FChar::IsDigit(C) ? C - TEXT('0') : -1;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorFlowExampleMapTest, "TensorFlow.Examples.MnistMap", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTensorFlowExampleMapTest::RunTest(const FString&)
{
	using namespace TensorFlowExampleMapTests;
	if (!FApp::CanEverRender() || !GDynamicRHI || FCString::Strcmp(GDynamicRHI->GetName(), TEXT("Null")) == 0)
	{
		AddWarning(TEXT("needs a real RHI (run with -RenderOffscreen instead of -nullrhi): skipped"));
		return true;
	}
	TSharedRef<FState> S = MakeShared<FState>();
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/TensorFlow/Examples/MnistExample")));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, S]() -> bool
	{
		const double Now = FPlatformTime::Seconds();
		if (S->Start == 0.0)
		{
			S->Start = Now;
			S->Deadline = Now + 600.0; //First run: Python environment, MNIST download, two epochs of training
		}
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		if (!World)
		{
			return Now > S->Deadline;
		}
		if (!S->Example.IsValid())
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (It->GetComponentByClass<UTensorFlowComponent>())
				{
					S->Example = *It;
				}
			}
			return false;
		}
		AActor* Example = S->Example.Get();
		const FString Status = TextOf(Example, TEXT("StatusText"));
		if (Status != S->LastStatus)
		{
			S->LastStatus = Status;
			S->StatusLines.Add(Status);
			UE_LOG(LogTemp, Display, TEXT("MnistMap status: %s"), *Status);
		}
		const FString Digit = TextOf(Example, TEXT("DigitText"));
		const FString Seen = TextOf(Example, TEXT("PredictionText"));
		const int32 Drawn = DigitIn(Digit, TEXT(", a "));
		const int32 Read = DigitIn(Seen, TEXT("Camera sees: "));

		if (S->Phase == 0 && Drawn >= 0 && Drawn == Read && !Status.StartsWith(TEXT("Training")))
		{
			//The trained model reads the digit the actor drew; now press N for the next one, and screenshot
			S->Matched = FString::Printf(TEXT("%s | %s"), *Digit, *Seen);
			S->DigitBeforeKey = Digit;
			FScreenshotRequest::RequestScreenshot(TEXT("TensorFlow_MnistExample.png"), false, false);
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				const FInputDeviceId Device = FInputDeviceId::CreateFromInternalId(0);
				PC->InputKey(FInputKeyEventArgs(nullptr, Device, EKeys::N, IE_Pressed, FPlatformTime::Cycles64()));
				PC->InputKey(FInputKeyEventArgs(nullptr, Device, EKeys::N, IE_Released, FPlatformTime::Cycles64()));
			}
			S->Phase = 1;
			S->PhaseStart = Now;
			return false;
		}
		if (S->Phase == 1)
		{
			//N asks Python for another digit (well before the 3 s timer would)
			if (Digit != S->DigitBeforeKey || Now - S->PhaseStart > 2.5)
			{
				TestTrue(TEXT("N drew another digit within 2.5 s"), Digit != S->DigitBeforeKey);
				S->Phase = 2;
			}
			return false;
		}
		if (S->Phase == 2)
		{
			AddInfo(FString::Printf(TEXT("after %.0f s: %s"), Now - S->Start, *S->Matched));
			AddInfo(TEXT("status lines: ") + FString::Join(S->StatusLines, TEXT(" / ")));
			return true;
		}
		if (Now > S->Deadline)
		{
			AddError(FString::Printf(TEXT("the map never read a drawn digit; status: %s; digit: %s; prediction: %s"), *Status, *Digit, *Seen));
			return true;
		}
		return false;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
