// TensorFlow-Unreal tests. Run headless:
//   UnrealEditor-Cmd.exe <Project>.uproject -ExecCmds="Automation RunTests TensorFlow; Quit" -unattended -nullrhi -nopause
// TensorFlow.Mnist.WithCamera needs a real RHI (-RenderOffscreen instead of -nullrhi). The tests that run
// Python need the TensorFlow (CPU) profile installed: the first run downloads tensorflow-cpu (~335 MB) and
// MNIST (11 MB).

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "TensorFlow.h"
#include "TensorFlowBlueprintLibrary.h"
#include "TensorFlowComponent.h"
#include "TensorFlowSettings.h"
#include "TensorFlowTestReceiver.h"
#include "PythonBlueprintLibrary.h"
#include "PythonEnvironment.h"
#include "PythonRenderTargetStreamComponent.h"
#include "PythonSubsystem.h"
#include "CanvasTypes.h"
#include "DynamicRHI.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/Paths.h"

namespace TensorFlowTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter;

	struct FRig
	{
		UGameInstance* GameInstance = nullptr;
		UWorld* World = nullptr;
		AActor* Actor = nullptr;
		UTensorFlowComponent* TF = nullptr;
		UTensorFlowTestReceiver* R = nullptr;

		void Destroy()
		{
			if (Actor)
			{
				Actor->Destroy();
			}
			GameInstance->Shutdown();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
			GameInstance->RemoveFromRoot();
			R->RemoveFromRoot();
		}
	};

	/** A world with an actor holding a TensorFlow component; Configure runs before BeginPlay. */
	static TSharedRef<FRig> MakeRig(const FString& Script, const TArray<FString>& Args, TFunction<void(FRig&)> Configure = nullptr)
	{
		TSharedRef<FRig> Rig = MakeShared<FRig>();
		Rig->GameInstance = NewObject<UGameInstance>(GEngine);
		Rig->GameInstance->AddToRoot();
		Rig->GameInstance->InitializeStandalone();
		Rig->World = Rig->GameInstance->GetWorld();
		Rig->World->InitializeActorsForPlay(FURL());
		Rig->R = NewObject<UTensorFlowTestReceiver>();
		Rig->R->AddToRoot();
		Rig->Actor = Rig->World->SpawnActor<AActor>();
		UTensorFlowComponent* C = NewObject<UTensorFlowComponent>(Rig->Actor);
		C->ScriptParams.Script = Script;
		C->ScriptParams.Args = Args;
		C->OnScriptStarted.AddDynamic(Rig->R, &UTensorFlowTestReceiver::OnStarted);
		C->OnScriptError.AddDynamic(Rig->R, &UTensorFlowTestReceiver::OnError);
		C->OnSetupComplete.AddDynamic(Rig->R, &UTensorFlowTestReceiver::OnSetupComplete);
		C->OnInputResults.AddDynamic(Rig->R, &UTensorFlowTestReceiver::OnInputResults);
		C->OnTrainingProgress.AddDynamic(Rig->R, &UTensorFlowTestReceiver::OnTrainingProgress);
		C->OnTrainingComplete.AddDynamic(Rig->R, &UTensorFlowTestReceiver::OnTrainingComplete);
		C->OnCustomEvent.AddDynamic(Rig->R, &UTensorFlowTestReceiver::OnCustomEvent);
		Rig->TF = C;
		if (Configure)
		{
			Configure(*Rig);
		}
		C->RegisterComponent();
		Rig->Actor->DispatchBeginPlay();
		return Rig;
	}

	static void Step(TFunction<void()> Fn)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Fn]() { Fn(); return true; }));
	}

	static void WaitUntil(FAutomationTestBase* Test, const FString& Label, TFunction<bool()> Condition, double Timeout)
	{
		const TSharedRef<double> Deadline = MakeShared<double>(0.0);
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Test, Label, Condition, Deadline, Timeout]() -> bool
		{
			if (*Deadline == 0.0)
			{
				*Deadline = FPlatformTime::Seconds() + Timeout;
			}
			if (Condition())
			{
				return true;
			}
			if (FPlatformTime::Seconds() > *Deadline)
			{
				Test->AddError(TEXT("Timed out waiting for ") + Label);
				return true;
			}
			return false;
		}));
	}

	/** The environment (with TensorFlow) is ready; the first run installs it. */
	static void WaitForEnvironment(FAutomationTestBase* Test)
	{
		Step([] { if (UPythonSubsystem* S = UPythonSubsystem::Get()) { S->SetupEnvironment(); } });
		WaitUntil(Test, TEXT("Python environment with TensorFlow"), []
		{
			UPythonSubsystem* S = UPythonSubsystem::Get();
			return S && S->GetEnvironmentState() != EPythonEnvironmentState::SettingUp;
		}, 900.0);
	}

	static double Number(const FString& Json, const FString& Path)
	{
		double Value = -1.0;
		UPythonBlueprintLibrary::GetJsonNumber(Json, Path, Value);
		return Value;
	}

	static bool Near(float A, float B)
	{
		return FMath::Abs(A - B) <= 1.f / 255.f + 1e-4f;
	}

	static const TArray<FString> QuickMnist = { TEXT("samples=10000"), TEXT("epochs=2"), TEXT("model=mnist_test.keras") };
}

using namespace TensorFlowTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorFlowProfileTest, "TensorFlow.Settings.Profile", TensorFlowTests::Flags)
bool FTensorFlowProfileTest::RunTest(const FString&)
{
	TestTrue(TEXT("Content/Scripts is a PythonCLI script folder"), FPythonEnvironment::GetScriptDirectories().Contains(FTensorFlowModule::ScriptsDir()));
	const UTensorFlowSettings* Settings = GetDefault<UTensorFlowSettings>();
	const FString File = UTensorFlowSettings::RequirementsFile(Settings->Profile);
	AddInfo(FString::Printf(TEXT("profile %s: %s"), *UEnum::GetValueAsString(Settings->Profile), *File));
	TestTrue(TEXT("the profile's requirements file exists"), Settings->Profile == ETensorFlowProfile::None || FPaths::FileExists(File));
	TestEqual(TEXT("TF_CPP_MIN_LOG_LEVEL for Python processes"), FPythonEnvironment::GetEnvironmentVariables().FindRef(TEXT("TF_CPP_MIN_LOG_LEVEL")), FString::FromInt(Settings->TensorFlowLogLevel));
	return true;
}

// Image <-> float array / tensor conversions on transient textures (no Python).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorFlowImagesTest, "TensorFlow.BlueprintLibrary.Images", TensorFlowTests::Flags)
bool FTensorFlowImagesTest::RunTest(const FString&)
{
	const TArray<float> Gray = { 0.f, 0.25f, 0.5f, 1.f };
	UTexture2D* GrayTex = UTensorFlowBlueprintLibrary::Conv_GrayScaleFloatArrayToTexture2D(Gray);
	const TArray<float> GrayBack = UTensorFlowBlueprintLibrary::Conv_GreyScaleTexture2DToFloatArray(GrayTex);
	TestTrue(TEXT("grayscale round trip (2x2)"), GrayBack.Num() == 4 && Near(GrayBack[1], 0.25f) && Near(GrayBack[3], 1.f));

	//2x2 RGBA: red, green, blue, half gray with half alpha
	const TArray<float> Rgba = { 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0.5f, 0.5f, 0.5f, 0.5f };
	UTexture2D* ColorTex = UTensorFlowBlueprintLibrary::Conv_FloatArrayToTexture2D(Rgba);
	const TArray<float> RgbaBack = UTensorFlowBlueprintLibrary::Conv_Texture2DToFloatArray(ColorTex);
	bool bSame = RgbaBack.Num() == Rgba.Num();
	for (int32 i = 0; bSame && i < Rgba.Num(); ++i)
	{
		bSame = Near(RgbaBack[i], Rgba[i]);
	}
	TestTrue(TEXT("RGBA round trip in RGBA order"), bSame);

	FTensorFlowImageOptions Options;
	FPyTensor T = UTensorFlowBlueprintLibrary::TextureToTensor(ColorTex, Options);
	TestTrue(TEXT("RGB tensor (2, 2, 3)"), T.Shape == TArray<int64>({ 2, 2, 3 }) && Near(T.ToFloats()[0], 1.f) && Near(T.ToFloats()[4], 1.f) && Near(T.ToFloats()[8], 1.f));
	Options.bChannelsFirst = true;
	T = UTensorFlowBlueprintLibrary::TextureToTensor(ColorTex, Options);
	TestTrue(TEXT("channels first (3, 2, 2): the red plane"), T.Shape == TArray<int64>({ 3, 2, 2 }) && Near(T.ToFloats()[0], 1.f) && Near(T.ToFloats()[1], 0.f));
	Options = FTensorFlowImageOptions();
	Options.Channels = ETensorFlowChannels::Grayscale;
	Options.Size = FIntPoint(1, 1);
	Options.bInvert = true;
	Options.Normalization = ETensorFlowNormalization::MinusOneToOne;
	T = UTensorFlowBlueprintLibrary::TextureToTensor(ColorTex, Options);
	//Gray of the four pixels averages (1/3 * 3 + 0.5) / 4 = 0.375; inverted 0.625; scaled to -1..1: 0.25
	TestTrue(TEXT("resize, grayscale, invert, -1..1"), T.Shape == TArray<int64>({ 1, 1, 1 }) && FMath::IsNearlyEqual(T.ToFloats()[0], 0.25f, 0.02f));

	const FPyTensor Image = FPyTensor::FromFloats({ 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1 }, { 2, 2, 3 });
	UTexture2D* FromTensor = UTensorFlowBlueprintLibrary::TensorToTexture(Image);
	const FPyTensor Back = UTensorFlowBlueprintLibrary::TextureToTensor(FromTensor, FTensorFlowImageOptions());
	TestTrue(TEXT("tensor -> texture -> tensor"), Back.Shape == Image.Shape && Back.ToFloats() == Image.ToFloats());

	TestTrue(TEXT("InvertFloatArray"), UTensorFlowBlueprintLibrary::InvertFloatArray({ 0.f, 0.25f }) == TArray<float>({ 1.f, 0.75f }));
	TestTrue(TEXT("ToFloatArray (bytes)"), UTensorFlowBlueprintLibrary::Conv_ByteToFloatArray({ 0, 255 }, 1.f / 255.f) == TArray<float>({ 0.f, 1.f }));
	AddExpectedMessage(TEXT("don't make a"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1, false);
	TestNull(TEXT("non-square array without a size"), UTensorFlowBlueprintLibrary::Conv_GrayScaleFloatArrayToTexture2D({ 0.f, 1.f, 0.f }));
	return true;
}

// The bundled add example: TensorFlow imports and runs in the Python process.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorFlowAddExampleTest, "TensorFlow.Component.AddExample", TensorFlowTests::Flags)
bool FTensorFlowAddExampleTest::RunTest(const FString&)
{
	WaitForEnvironment(this);
	TSharedRef<TSharedPtr<FRig>> Rig = MakeShared<TSharedPtr<FRig>>();
	Step([Rig]
	{
		*Rig = MakeRig(TEXT("examples/add_example"), {});
		(*Rig)->TF->SendJsonInput(TEXT("{\"a\": [1, 2], \"b\": [3, 4]}"), FString());
		(*Rig)->TF->CallCustomFunction(TEXT("change_operation"), TEXT("\"-\""));
		(*Rig)->TF->SendJsonInput(TEXT("{\"a\": [1, 2], \"b\": [3, 4]}"), FString());
	});
	WaitUntil(this, TEXT("results"), [Rig] { return (*Rig)->R->Results.Num() >= 3 || (*Rig)->R->Errors.Num() > 0; }, 120.0);
	Step([this, Rig]
	{
		UTensorFlowTestReceiver* R = (*Rig)->R;
		if (!TestEqual(TEXT("errors"), R->Errors.Num(), 0))
		{
			AddError(R->Errors[0]);
		}
		else
		{
			TestEqual(TEXT("1 + 3, 2 + 4"), R->Results[0].Value, FString(TEXT("{\"c\":[4,6]}")));
			TestEqual(TEXT("1 - 3, 2 - 4"), R->Results[2].Value, FString(TEXT("{\"c\":[-2,-2]}")));
		}
		(*Rig)->Destroy();
	});
	return true;
}

// MNIST: train on a subset with progress and events, predict a test digit, then stop a training early.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorFlowMnistTrainingTest, "TensorFlow.Mnist.Training", TensorFlowTests::Flags)
bool FTensorFlowMnistTrainingTest::RunTest(const FString&)
{
	WaitForEnvironment(this);
	TSharedRef<TSharedPtr<FRig>> Rig = MakeShared<TSharedPtr<FRig>>();
	TSharedRef<FPythonCallResult> Sample = MakeShared<FPythonCallResult>();
	TSharedRef<bool> bSample = MakeShared<bool>(false);
	Step([Rig] { *Rig = MakeRig(TEXT("examples/mnist_keras_cnn"), QuickMnist); });
	WaitUntil(this, TEXT("setup"), [Rig] { return (*Rig)->R->Setups == 1 || (*Rig)->R->Errors.Num() > 0; }, 120.0);
	Step([Rig] { (*Rig)->TF->BeginTraining(); });
	WaitUntil(this, TEXT("training"), [Rig] { return (*Rig)->R->Completions.Num() == 1 || (*Rig)->R->Errors.Num() > 0; }, 600.0);
	Step([this, Rig, Sample, bSample]
	{
		UTensorFlowTestReceiver* R = (*Rig)->R;
		if (R->Errors.Num() > 0)
		{
			AddError(R->Errors[0]);
			return;
		}
		const FString& Summary = R->Completions[0].Key;
		AddInfo(FString::Printf(TEXT("MNIST, 10000 images, 2 epochs: %s; %d progress events"), *Summary, R->Progress.Num()));
		TestTrue(TEXT("test accuracy > 0.85"), Number(Summary, TEXT("test_accuracy")) > 0.85);
		TestTrue(TEXT("progress events"), R->Progress.Num() >= 2 && Number(R->Progress[0], TEXT("loss")) > 0.0);
		const TPair<FString, FString>* TrainingSample = R->FindEvent(TEXT("TrainingSample"));
		TestTrue(TEXT("TrainingSample event with 784 pixels"), TrainingSample != nullptr && Number(TrainingSample->Value, TEXT("pixels.783")) >= 0.0);
		(*Rig)->TF->CallML(TEXT("custom"), TEXT("test_sample"), MakeShared<FJsonValueNumber>(0), {}, [Sample, bSample](const FPythonCallResult& Result) { *Sample = Result; *bSample = true; });
	});
	WaitUntil(this, TEXT("test sample"), [bSample] { return *bSample; }, 60.0);
	Step([this, Rig, Sample]
	{
		if (!TestTrue(TEXT("test_sample returned an image tensor"), Sample->bSuccess && Sample->Tensors.Num() == 1))
		{
			return;
		}
		AddInfo(FString::Printf(TEXT("test image 0: label %d, shape %s"), (int32)Number(Sample->JsonResult, TEXT("label")), *FString::JoinBy(Sample->Tensors[0].Shape, TEXT("x"), [](int64 D) { return FString::Printf(TEXT("%lld"), D); })));
		(*Rig)->TF->SendTensorInput(Sample->Tensors[0], FString());
	});
	WaitUntil(this, TEXT("prediction"), [Rig] { return (*Rig)->R->Results.ContainsByPredicate([](const TPair<FString, FString>& P) { return P.Key == TEXT("on_tensor_input"); }); }, 60.0);
	Step([this, Rig, Sample]
	{
		const TPair<FString, FString>* P = (*Rig)->R->Results.FindByPredicate([](const TPair<FString, FString>& X) { return X.Key == TEXT("on_tensor_input"); });
		AddInfo(TEXT("prediction: ") + P->Value.Left(120));
		TestEqual(TEXT("the trained model reads the test digit"), (int32)Number(P->Value, TEXT("prediction")), (int32)Number(Sample->JsonResult, TEXT("label")));
		(*Rig)->TF->BeginTraining();
	});
	WaitUntil(this, TEXT("second training's progress"), [Rig] { return (*Rig)->R->Progress.Num() >= 1 && (*Rig)->TF->bIsTraining; }, 120.0);
	TSharedRef<double> StopAt = MakeShared<double>(0.0);
	Step([Rig, StopAt]
	{
		(*Rig)->R->Progress.Reset();
		*StopAt = FPlatformTime::Seconds();
		(*Rig)->TF->StopTraining();
	});
	WaitUntil(this, TEXT("stopped training"), [Rig] { return (*Rig)->R->Completions.Num() == 2; }, 120.0);
	Step([this, Rig, StopAt]
	{
		AddInfo(FString::Printf(TEXT("StopTraining took %.2f s to end fit() (including the test evaluation)"), FPlatformTime::Seconds() - *StopAt));
		TestTrue(TEXT("stopped early"), (*Rig)->R->Completions[1].Value);
		FString Kept;
		TestTrue(TEXT("a stopped training keeps the finished model"), UPythonBlueprintLibrary::GetJsonString((*Rig)->R->Completions[1].Key, TEXT("kept_previous_model"), Kept) && Kept == TEXT("true"));
		(*Rig)->Destroy();
	});
	return true;
}

// The camera stream: a test digit drawn into a render target, streamed to the classifier, recognized.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTensorFlowMnistCameraTest, "TensorFlow.Mnist.WithCamera", TensorFlowTests::Flags)
bool FTensorFlowMnistCameraTest::RunTest(const FString&)
{
	if (!FApp::CanEverRender() || !GDynamicRHI || FCString::Strcmp(GDynamicRHI->GetName(), TEXT("Null")) == 0)
	{
		AddWarning(TEXT("needs a real RHI (run with -RenderOffscreen instead of -nullrhi): skipped"));
		return true;
	}
	WaitForEnvironment(this);
	struct FState
	{
		TSharedPtr<FRig> Classifier;
		TSharedPtr<FRig> Mnist;
		UTextureRenderTarget2D* RT = nullptr;
		UPythonRenderTargetStreamComponent* Camera = nullptr;
		FPythonCallResult Sample;
		bool bSample = false;
		int32 Captures = 0;
	};
	TSharedRef<FState> S = MakeShared<FState>();
	Step([S]
	{
		//The trained model from TensorFlow.Mnist.Training; the MNIST example also provides the test digit
		S->Mnist = MakeRig(TEXT("examples/mnist_keras_cnn"), QuickMnist);
		S->Mnist->TF->CallML(TEXT("custom"), TEXT("test_sample"), MakeShared<FJsonValueNumber>(1), {}, [S](const FPythonCallResult& R) { S->Sample = R; S->bSample = true; });
		S->Classifier = MakeRig(TEXT("examples/camera_stream_classify"), { TEXT("model=mnist_test.keras"), TEXT("interval=0.1") }, [S](FRig& Rig)
		{
			S->RT = NewObject<UTextureRenderTarget2D>(Rig.Actor);
			S->RT->RenderTargetFormat = RTF_RGBA8;
			S->RT->ClearColor = FLinearColor::Black;
			S->RT->InitAutoFormat(256, 256);
			S->RT->UpdateResourceImmediate(true);
			S->Camera = NewObject<UPythonRenderTargetStreamComponent>(Rig.Actor);
			S->Camera->RenderTarget = S->RT;
			S->Camera->bAutoCapture = false;
			S->Camera->PythonComponent = Rig.TF;
			S->Camera->RegisterComponent();
		});
	});
	WaitUntil(this, TEXT("test digit"), [S] { return S->bSample; }, 120.0);
	Step([this, S]
	{
		if (!TestTrue(TEXT("test digit"), S->Sample.bSuccess && S->Sample.Tensors.Num() == 1))
		{
			return;
		}
		//Draw it scaled up into the render target, as a drawing or a scene capture would
		UTexture2D* Digit = UTensorFlowBlueprintLibrary::TensorToTexture(S->Sample.Tensors[0]);
		FlushRenderingCommands(); //The texture's GPU resource exists before the canvas draws it
		UCanvas* Canvas = nullptr;
		FVector2D Size;
		FDrawToRenderTargetContext Context;
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(S->Classifier->World, S->RT, Canvas, Size, Context);
		Canvas->K2_DrawTexture(Digit, FVector2D::ZeroVector, Size, FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::White, BLEND_Opaque);
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(S->Classifier->World, Context);
		FlushRenderingCommands();
		FTensorFlowImageOptions Gray;
		Gray.Channels = ETensorFlowChannels::Grayscale;
		const TArray<float> Drawn = UTensorFlowBlueprintLibrary::RenderTargetToTensor(S->RT, Gray).ToFloats();
		const TArray<float> Source = UTensorFlowBlueprintLibrary::TextureToTensor(Digit, Gray).ToFloats();
		double DrawnSum = 0, SourceSum = 0;
		for (float V : Drawn) { DrawnSum += V; }
		for (float V : Source) { SourceSum += V; }
		AddInfo(FString::Printf(TEXT("digit texture mean %.3f (%d px, canvas %p, size %.0fx%.0f); render target mean after drawing %.3f (%d px)"),
			Source.Num() ? SourceSum / Source.Num() : -1.0, Source.Num(), Canvas, Size.X, Size.Y, Drawn.Num() ? DrawnSum / Drawn.Num() : -1.0, Drawn.Num()));
	});
	WaitUntil(this, TEXT("classified camera frame"), [S]
	{
		if (S->Camera && S->Classifier->R->Setups == 1 && S->Captures < 200 && S->Camera->Capture())
		{
			++S->Captures;
		}
		if (S->Camera)
		{
			S->Camera->PollReadbacks();
		}
		//Predictions are reported when they change: wait for the drawn digit's (or give up after 100 frames)
		const int32 Label = (int32)Number(S->Sample.JsonResult, TEXT("label"));
		const bool bSeen = S->Classifier->R->CustomEvents.ContainsByPredicate([Label](const TPair<FString, FString>& E)
		{
			return E.Key == TEXT("Prediction") && (int32)Number(E.Value, TEXT("prediction")) == Label;
		});
		return bSeen || S->Captures >= 100 || S->Classifier->R->Errors.Num() > 0;
	}, 120.0);
	Step([this, S]
	{
		if (S->Classifier->R->Errors.Num() > 0)
		{
			AddError(S->Classifier->R->Errors[0]);
		}
		const TPair<FString, FString>* P = S->Classifier->R->CustomEvents.FindLastByPredicate([](const TPair<FString, FString>& E) { return E.Key == TEXT("Prediction"); }) != INDEX_NONE
			? &S->Classifier->R->CustomEvents[S->Classifier->R->CustomEvents.FindLastByPredicate([](const TPair<FString, FString>& E) { return E.Key == TEXT("Prediction"); })] : nullptr;
		TestNotNull(TEXT("a Prediction event"), P);
		if (P)
		{
			AddInfo(FString::Printf(TEXT("camera: %s (label %d) after %d captures"), *P->Value, (int32)Number(S->Sample.JsonResult, TEXT("label")), S->Captures));
			TestEqual(TEXT("the streamed render target is classified as the drawn digit"), (int32)Number(P->Value, TEXT("prediction")), (int32)Number(S->Sample.JsonResult, TEXT("label")));
		}
		S->Mnist->Destroy();
		S->Classifier->Destroy();
	});
	return true;
}

#endif
