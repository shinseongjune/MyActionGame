# WeaponTraceComponent 설정 가이드

목표: 기존 전방 `Sphere Trace` / `Line Trace` 임시 판정을 줄이고, 공격 모션의 특정 구간 동안 무기 소켓 궤적을 따라 판정이 나가게 만든다.

첫 목표는 1타 공격 하나만 제대로 맞추는 것이다.

```text
공격 입력
  -> 공격 모션 재생
  -> Anim Notify State 구간에서 Weapon Trace 시작
  -> 무기 소켓 궤적을 따라 Sweep
  -> 적 1회 피격
  -> 기존 적 피해/피격 반응 로직 호출
```

## 1단계: 플레이어 BP에 컴포넌트 추가

1. 에디터를 닫고 프로젝트를 빌드하거나, 에디터를 열 때 `rebuild` 창이 뜨면 `Yes`를 누른다.
2. 플레이어 캐릭터 블루프린트를 연다. 예: `BP_ThirdPersonCharacter`.
3. `Components` 패널에서 `Add`를 누르고 `WeaponTraceComponent`를 추가한다.
4. `WeaponTraceComponent`를 선택한다.
5. `Trace Start Socket Name`을 `WeaponTrace_Start`로 둔다.
6. `Trace End Socket Name`을 `WeaponTrace_End`로 둔다.
7. `Trace Radius`는 일단 `10`으로 시작한다.
8. `Trace Channel`은 일단 `Pawn`으로 둔다.
9. `Hit Actor Tags`에 `Enemy`가 들어 있는지 확인한다.
10. 처음 테스트할 때만 `Draw Debug Trace`를 켠다.

`Trace Source Component`는 비워 두면 플레이어 캐릭터의 기본 Skeletal Mesh를 자동으로 사용한다. 무기 소켓이 별도 무기 컴포넌트에 있으면, 나중에 블루프린트에서 `Set Trace Source Component`로 그 무기 컴포넌트를 지정한다.

## 2단계: 무기 소켓 만들기

무기나 손이 붙어 있는 Skeletal Mesh/Skeleton을 연다.

1. Skeleton Tree에서 오른손 무기 뼈, 손 뼈, 또는 무기 뼈를 찾는다.
2. 우클릭 후 `Add Socket`을 누른다.
3. 첫 소켓 이름을 `WeaponTrace_Start`로 바꾼다.
4. 무기의 손잡이 쪽 또는 날 시작점 근처에 둔다.
5. 같은 방식으로 `WeaponTrace_End` 소켓을 만든다.
6. 무기 끝 또는 공격 판정이 끝나는 지점에 둔다.

처음에는 아주 정확하지 않아도 된다. 중요한 것은 두 소켓이 무기를 따라 움직이고, 공격 중 디버그 라인이 무기 궤적을 대략 따라가는 것이다.

## 3단계: 공격 애니메이션에 Notify State 추가

공격 애니메이션 또는 공격 몽타주를 연다.

1. Notify 트랙에서 우클릭한다.
2. `Add Notify State`를 선택한다.
3. `Weapon Trace Window`를 고른다.
4. Notify State 길이를 실제 타격 구간에만 맞춘다.

예시:

```text
모션 시작: 0.00초
무기를 휘두르기 시작: 0.18초
실제 맞는 구간: 0.22초 ~ 0.38초
회수: 0.39초 이후
```

이 경우 `Weapon Trace Window`는 `0.22초 ~ 0.38초`에만 둔다.

## 4단계: 피격 이벤트 연결

플레이어 BP에서 `WeaponTraceComponent`의 `On Weapon Trace Hit` 이벤트를 추가한다.

처음 연결은 아래처럼 단순하게 한다.

```text
On Weapon Trace Hit
  -> Hit Actor를 BP_Enemy_Base로 Cast
  -> 기존 적 피해 함수 또는 Apply Damage 호출
  -> Print String "Weapon Hit"
```

이 컴포넌트는 피격 판정만 담당한다. 적 HP, 슈퍼아머, 사망, 피격 애니메이션은 기존 적 블루프린트가 처리한다.

한 번의 `Weapon Trace Window` 안에서는 같은 적을 한 번만 맞춘다. 1타가 적 하나를 여러 번 때리는 문제를 막기 위한 기본 규칙이다.

## 5단계: 테스트

1. 적 액터에 `Enemy` 태그가 있는지 확인한다.
2. 플레이어 공격 모션을 실행한다.
3. 디버그 라인이 무기 궤적을 따라가는지 본다.
4. 적을 맞추면 `Weapon Hit`가 한 번만 뜨는지 확인한다.
5. 빗나가면 아무 일도 없어야 한다.

잘 안 맞으면 아래 순서로 조정한다.

1. `WeaponTrace_Start` / `WeaponTrace_End` 소켓 위치
2. `Weapon Trace Window` 시작/끝 시간
3. `Trace Radius`
4. 적의 Collision 설정과 `Enemy` 태그

## 지금 하지 않을 것

아래는 무기 판정 1타가 확인된 뒤 붙인다.

- 2타/3타 콤보
- 패리 가능/불가 공격 데이터
- 패리 성공 동기화 애니메이션
- 처형 연출
- 사망 연출 분기
- 피격 방향별 리액션

순서는 이렇다.

```text
무기 판정 1타
  -> 적 피격 반응 개선
  -> 적 공격도 WeaponTrace로 변경
  -> 패리 윈도우
  -> 콤보
  -> 처형/사망 연출
```
