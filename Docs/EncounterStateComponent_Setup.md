# EncounterStateComponent 설정 가이드

목표: `BP_EncounterZone`에 C++ 컴포넌트를 붙여서 전투 상태가 `Dormant` → `Active` → `Cleared`로 넘어가게 만든다. 플레이어가 구역에 들어오면 전투가 시작되고, 추적 중인 적이 모두 처치되면 `On Encounter Cleared` 이벤트가 발생한다.

## 처음 한 번만 필요한 C++ 빌드

이 프로젝트는 원래 블루프린트 중심이었고, 이번에 `Source/` 폴더와 C++ 컴포넌트가 추가됐다. 그래서 에디터가 새 C++ 클래스를 인식하려면 한 번 빌드가 필요하다.

1. Unreal Editor가 열려 있으면 저장 후 닫는다.
2. `MyActionGame.uproject`를 다시 연다.
3. `Missing modules` 또는 `rebuild` 관련 창이 뜨면 `Yes`를 누른다.
4. 에디터가 열리면 `BP_EncounterZone`에서 컴포넌트를 추가할 수 있다.

빌드가 Live Coding 때문에 막히면 에디터에서 `Ctrl+Alt+F11`을 누르거나 에디터를 완전히 닫고 다시 시도한다.

## BP_EncounterZone에 컴포넌트 붙이기

1. Content Browser에서 `Content/_Game/Systems/Stage/BP_EncounterZone`을 연다.
2. 왼쪽 `Components` 패널에서 `Add`를 누른다.
3. 검색창에 `Encounter State` 또는 `EncounterStateComponent`를 입력한다.
4. `Encounter State Component`를 선택해서 추가한다.
5. 컴포넌트 이름은 기본값 그대로 둬도 된다. 필요하면 `EncounterState` 정도로 바꾼다.
6. `Compile`을 누르고 `Save`한다.

## 기본 값 설정

`EncounterStateComponent`를 선택한 상태에서 Details 패널을 설정한다.

1. `Auto Start On Player Overlap`은 켜둔다.
2. `Enemy Actor Class`는 가능하면 `BP_Enemy_Base`로 지정한다.
3. 적들이 구역 안에 배치되어 있으면 `Collect Overlapping Enemies On Start`를 켜둔다.
4. 적들이 구역 밖에 있거나 자동 감지가 안 되면 `Encounter Enemies` 배열에 해당 적 액터들을 직접 넣는다.
5. 테스트 중에는 `Log Encounter State`를 켜둔다.

구역의 Box/Trigger 컴포넌트는 플레이어와 적을 Overlap할 수 있어야 한다. 플레이어가 들어와도 시작 로그가 안 뜨면 먼저 Trigger의 collision/overlap 설정을 확인한다.

## 적 처치 감지 방식

컴포넌트는 아래 신호를 자동으로 감지한다.

- 적 액터가 Destroy됨
- 적 액터에 `Dead`, `Defeated`, `Executed` 태그가 있음
- 적 블루프린트에 `IsDead`, `bIsDead`, `IsDefeated`, `bIsDefeated`, `IsExecuted` 같은 bool 변수가 있고 값이 true임

현재 `BP_Enemy_Base`의 사망 상태 변수 이름이 다르면 `Defeated Bool Names` 배열에 그 변수 이름을 추가한다.

자동 감지가 불안하면 적의 죽는 흐름에서 직접 알려주는 방식이 가장 확실하다.

1. 적이 죽는 블루프린트 그래프 위치로 간다.
2. 현재 레벨의 `BP_EncounterZone` 또는 그 안의 `EncounterStateComponent` 참조를 가져온다.
3. `Notify Enemy Defeated`를 호출한다.
4. `Enemy` 입력에는 죽은 적 자신, 즉 `Self`를 넣는다.

## 클리어 이벤트 연결

전투 종료 후 뭔가를 하고 싶으면 `BP_EncounterZone`에서 `EncounterStateComponent`를 선택한 뒤 이벤트를 추가한다.

1. `EncounterStateComponent` 선택
2. Details 패널의 Events 영역에서 `On Encounter Cleared` 추가
3. 이벤트 뒤에 원하는 처리를 연결

처음에는 아래 중 하나만 붙이는 것을 추천한다.

- `Print String`: `Encounter Cleared`
- 문 열기
- 다음 구역 Trigger 활성화
- 보상 액터 Spawn

## 빠른 테스트

1. Play를 누른다.
2. 플레이어로 Encounter Zone에 들어간다.
3. Output Log에 `Encounter started`가 찍히는지 확인한다.
4. 등록된 적을 모두 처치한다.
5. Output Log에 `Encounter cleared`가 찍히는지 확인한다.
6. `On Encounter Cleared`에 붙인 블루프린트 처리가 실행되는지 확인한다.

## 지금 단계에서 추천하는 다음 연결

먼저 `On Encounter Cleared`에 `Print String`만 붙여서 종료 판정이 맞는지 확인한다. 그 다음에 문 열기, 다음 구역 활성화, 보상 같은 실제 게임 진행 요소를 하나씩 붙이면 된다.
