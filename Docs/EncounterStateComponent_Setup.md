# EncounterStateComponent 설정 가이드

목표: `BP_EncounterZone`의 `Box Collision` 하나를 전투 구역으로 사용한다. 박스 안에 있는 적들이 `Encounter Enemies`로 자동 등록되고, 플레이어가 그 박스에 들어오면 전투가 시작된다. 적이 모두 처치되면 `On Encounter Cleared`가 발생한다.

## 핵심 규칙

전투 구역의 진짜 범위는 `BP_EncounterZone` 안의 `Box Collision`이다. 별도의 폴링 박스, 임시 범위, 마커 범위는 사용하지 않는다.

- `Box Collision` 크기: `BP_EncounterZone` 인스턴스의 위치/스케일 또는 Box 컴포넌트 크기로 조절
- 적 자동 등록: `Box Collision` 안에 있는 적 액터만 등록
- 전투 시작: 플레이어가 `Box Collision`에 Overlap하면 시작
- 전투 종료: 등록된 적이 모두 처치되면 `On Encounter Cleared`

## BP_EncounterZone 기본 설정

1. `BP_EncounterZone`을 연다.
2. `Components`에 `Box Collision`이 있는지 확인한다.
3. `Box Collision`의 `Generate Overlap Events`를 켠다.
4. `Box Collision`의 Collision Preset은 `Trigger` 또는 Pawn을 Overlap하는 설정으로 둔다.
5. `EncounterStateComponent`를 선택한다.
6. `Auto Start On Player Overlap`을 켠다.
7. `Collect Enemies From Zone On Start`를 켠다.
8. `Enemy Actor Class`를 실제 적의 공통 부모 블루프린트로 지정한다. 예: `BP_Enemy_Base`
9. `Log Encounter State`를 켠다.
10. `Compile` 후 `Save`한다.

## 레벨 배치 순서

1. 레벨에 `BP_EncounterZone`을 놓는다.
2. `BP_EncounterZone`의 위치와 스케일을 조절해서 전투 방을 덮는다.
3. 적 액터들을 그 Box Collision 안에 배치한다.
4. 플레이어 시작점은 Box Collision 밖에 둔다.
5. Play 후 플레이어가 Box Collision 안으로 들어가게 한다.

## 블루프린트 연결

`EncounterStateComponent`는 상태만 관리한다. 적 AI 활성화, 문 닫기/열기, UI 표시 같은 실제 반응은 `BP_EncounterZone` 블루프린트에서 이벤트에 연결한다.

- `On Encounter Started`: 등록된 `Encounter Enemies`를 활성화
- `On Enemy Defeated`: 남은 적 수 표시 또는 중간 반응
- `On Encounter Cleared`: 문 열기, 다음 구역 활성화, 보상 생성

`BP_EncounterZone`에 별도 `Encounter Enemies` 배열 변수를 만들 필요는 없다. 적 목록의 원본은 `EncounterStateComponent` 안에 있다.

`On Encounter Started`에서 적을 활성화하려면 아래 순서로 연결한다.

1. `EncounterStateComponent` 참조를 가져온다.
2. `Get Encounter Enemies` 노드를 호출한다.
3. 반환된 배열을 `For Each Loop`로 돈다.
4. 각 적에게 `Set Active Enemy` 같은 기존 블루프린트 처리를 호출한다.

## 로그 확인

Output Log에서 `[EncounterState:`로 검색한다.

정상 흐름은 대략 아래와 같다.

```text
BeginPlay. EnemyClass=BP_Enemy_Base_C.
Bound zone box overlap: Box
Registered enemy: BP_Enemy_Base_C_1
Registered enemy: BP_Enemy_Ranged_C_1
Zone box collection checked 10 enemy candidates and registered 2 enemies.
Encounter started with 2 tracked enemies.
Encounter cleared.
```

자주 보는 문제는 아래처럼 판단한다.

- `No Box Collision component found`: `BP_EncounterZone`에 Box Collision이 없다.
- `Generate Overlap Events disabled`: Box Collision의 Overlap 이벤트가 꺼져 있다.
- `registered 0 enemies`: 적이 Box Collision 밖에 있거나 `Enemy Actor Class`가 맞지 않다.
- `Encounter started`가 안 뜸: 플레이어가 Box Collision과 Overlap하지 못하고 있다.

## 처치 감지

컴포넌트는 아래 신호를 적 처치로 본다.

- 적 액터가 Destroy됨
- 적 액터에 `Dead`, `Defeated`, `Executed` 태그가 있음
- 적 블루프린트에 `IsDead`, `bIsDead`, `IsDefeated`, `bIsDefeated`, `IsExecuted` 같은 bool 변수가 있고 값이 true임

가장 확실한 방식은 적이 죽는 블루프린트 흐름에서 `Notify Enemy Defeated`를 호출하고, `Enemy` 입력에 `Self`를 넣는 것이다.
