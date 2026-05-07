# PlayerFailureComponent 설정 가이드

목표: 플레이어 블루프린트의 기존 HP/Aegis/사망 판정을 단일 진실로 유지하고, `PlayerFailureComponent`는 사망 이후의 공통 이벤트 처리만 담당한다.

이 컴포넌트는 체력을 만들지 않는다. 피해를 계산하지 않는다. `Apply Damage`를 듣고 죽음을 판단하지도 않는다.

## 책임 분리

```text
플레이어 BP
  -> HP 500 관리
  -> Aegis / 방어 / 피해 감소 계산
  -> 실제 사망 조건 판단

PlayerFailureComponent
  -> 사망 판정 이후 호출됨
  -> On Player Defeated 이벤트 발생
  -> Dead / Defeated / PlayerDefeated 태그 추가
  -> 필요하면 입력/이동 정지
  -> Game Over UI, 리스타트, 실패 연출의 연결점 제공
```

## 플레이어 블루프린트 설정

1. 플레이어 캐릭터 블루프린트를 연다. 예: `BP_ThirdPersonCharacter`.
2. `Components` 패널에서 `Add`를 누르고 `PlayerFailureComponent`를 추가한다.
3. `PlayerFailureComponent`를 선택한다.
4. `Disable Input On Defeat`와 `Stop Movement On Defeat`를 켠다.
5. `Compile` 후 `Save`한다.

## 기존 사망 판정에 연결

플레이어 BP의 기존 피해 처리 흐름 끝에서 연결한다.

```text
적 피해 125
  -> 기존 HP/Aegis 계산
  -> 최종 HP가 0보다 큼
  -> 아무것도 호출하지 않음
```

죽는 경우에만 호출한다.

```text
기존 HP/Aegis 계산
  -> 최종 HP <= 0
  -> PlayerFailureComponent
  -> Trigger Player Defeated
       Damage Causer = 적 액터
```

125 피해를 받았지만 HP 500과 Aegis 때문에 살아 있어야 한다면, `Trigger Player Defeated`가 호출되면 안 된다.

## Game Over 연결

`PlayerFailureComponent`에서 `On Player Defeated` 이벤트를 추가한다.

첫 테스트는 아래 정도면 충분하다.

```text
On Player Defeated
  -> Print String "Game Over"
```

이벤트가 뜨고 플레이어 입력이 멈추면 실패 이벤트 처리는 성공이다. 이후에 같은 이벤트에서 Game Over UI, 리스타트 버튼, 체크포인트 복귀를 연결한다.

## 리셋 연결

리스타트나 체크포인트 복귀를 만들 때는 기존 플레이어 BP가 HP/Aegis를 먼저 복구한 뒤 컴포넌트를 리셋한다.

```text
Restart / Respawn
  -> 기존 플레이어 BP에서 HP/Aegis 복구
  -> PlayerFailureComponent
  -> Reset Player Failure
```

`Reset Player Failure`는 사망 태그를 제거하고, 사망 때문에 멈췄던 입력/이동을 다시 열고, `On Player Failure Reset` 이벤트를 발생시킨다.

## 확인 로그

Output Log에서 `[PlayerFailure:`로 검색한다.

정상 흐름은 대략 아래처럼 보인다.

```text
[PlayerFailure:BP_ThirdPersonCharacter_C_0] Player failure handler ready.
[PlayerFailure:BP_ThirdPersonCharacter_C_0] Player defeated.
```

피해를 받을 때마다 이 컴포넌트 로그가 뜨면 안 된다. 피해 계산은 플레이어 BP의 기존 체력 시스템 안에서만 일어나야 한다.
