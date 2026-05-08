#include "Portal.h"
#include "TemaProject03/GameInstance/MyGameInstance.h"
#include "TemaProject03/Enemy/EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

APortal::APortal()
{
    PrimaryActorTick.bCanEverTick = false;


    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = SceneRoot;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);

    // 충돌 설정 Entity Collision ~
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->SetGenerateOverlapEvents(true);


    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 델리게이트 연결
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);

    // 기본 보스맵 이름 설정
    BossLevelName = TEXT("BossMap");
}

void APortal::BeginPlay()
{
    Super::BeginPlay();

    SetPortalActive(false);

    UE_LOG(LogTemp, Log, TEXT("[Portal] %s: Setup complete. Waiting for player..."), *GetName());
}

void OpenPortal()
{

    UE_LOG(LogTemp, Warning, TEXT("[Portal] Received Open Signal from Spawner!"));
    SetPortalActive(true);

}

void APortal::SetPortalActive(bool bActive)
{
    bIsPortalOpen = bActive;

    // 포탈이 열렸을 때만 시각적으로 표시
    if (MeshComp)
    {
        MeshComp->SetVisibility(bActive);
    }

    // 포탈이 닫혀있으면 충돌도 꺼서 이벤트 발생을 방지 (방어 로직)
    if (TriggerBox)
    {
        TriggerBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Portal] %s Status -> %s"), *GetName(), bActive ? TEXT("OPEN") : TEXT("CLOSED"));
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 유효성 검사
    if (!bIsPortalOpen) return;
    if (!OtherActor || OtherActor == this) return;

    // 닿은 액터가 실제 플레이어(Character)인지 확인
    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar)
    {
        UE_LOG(LogTemp, Log, TEXT("[Portal] Ignore: %s is not a Player Character."), *OtherActor->GetName());
        return;
    }

    // 데이터 무결성 검사
    UMyGameInstance* GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("[Portal] CRITICAL: MyGameInstance is missing!"));
        return;
    }

    // 로직 1 단계 랜덤 레벨 리스트 초기화
    // 처음 포탈을 탔을 때 리스트가 비어있다면 맵 목록을 만들고 무작위로 섞음 (Shuffle)
    if (GI->CurrentStageCount == 0 && GI->RemainingLevels.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Portal] First run detected. Initializing level list..."));

        TArray<FString> LevelPool = { TEXT("Level_1"), TEXT("Level_2"), TEXT("Level_3"), TEXT("Level_4"), TEXT("Level_5") };
        for (const FString& Name : LevelPool)
        {
            GI->RemainingLevels.Add(FName(*Name));
        }

        // Fisher-Yates 알고리즘으로 랜덤 셔플
        for (int32 i = GI->RemainingLevels.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            GI->RemainingLevels.Swap(i, j);
        }
    }

    // 로직 2단계 진행 상태 갱신
    GI->CurrentStageCount++;
    UE_LOG(LogTemp, Warning, TEXT("[Portal] Progress Update: Stage %d / %d"), GI->CurrentStageCount, GI->MaxStagesBeforeBoss);

    // 로직 3단계 맵 이동 판정
    // A. 보스전 진입 판정 (클리어 횟수 초과 시)
    if (GI->CurrentStageCount > GI->MaxStagesBeforeBoss)
    {
        if (BossLevelName.IsNone())
        {
            UE_LOG(LogTemp, Error, TEXT("[Portal] Error: BossLevelName is None. Check Blueprint settings."));
            return;
        }

        UE_LOG(LogTemp, Error, TEXT("[Portal] ALL STAGES CLEARED. Moving to Boss: %s"), *BossLevelName.ToString());

        // 다음 회차를 위해 카운트 리셋
        GI->CurrentStageCount = 0;
        GI->RemainingLevels.Empty();

        UGameplayStatics::OpenLevel(this, BossLevelName);
        return;
    }

    // B. 일반 랜덤 레벨 이동 (리스트가 유효한 경우)
    if (GI->RemainingLevels.IsValidIndex(0))
    {
        FName NextLevelName = GI->RemainingLevels[0];

        if (NextLevelName.IsNone())
        {
            UE_LOG(LogTemp, Error, TEXT("[Portal] Error: Invalid Level Name found at Index 0."));
            return;
        }

        // 방문한 레벨은 리스트에서 삭제 (중복 방지)
        GI->RemainingLevels.RemoveAt(0);

        UE_LOG(LogTemp, Error, TEXT("[Portal] Teleporting to: %s (Remaining: %d)"),
            *NextLevelName.ToString(), GI->RemainingLevels.Num());

        UGameplayStatics::OpenLevel(this, NextLevelName);
    }
    else
    {
        // 예외 상황: 리스트가 비어있음
        UE_LOG(LogTemp, Error, TEXT("[Portal] Fatal Error: No levels available in RemainingLevels!"));
    }
}
