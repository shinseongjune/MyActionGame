# 첫 스테이지 블록아웃 설계안

목표: 에셋이 부족한 상태에서도 `입장 -> 전투 -> 클리어 결과 -> 상호작용 장애물 -> 두 번째 전투 -> 보상/종료`까지 한 판이 제대로 돌아가는지 검증한다.

이 문서는 예쁜 맵 설계가 아니라, 실제 게임 루프와 레벨 흐름을 확인하기 위한 블록아웃 기준이다.

## 핵심 방향

- 방은 2개만 만든다.
- 길은 선형으로 만들되, 전투방 안에서는 좌우 선택지가 보이게 한다.
- 각 구역의 목적을 분명히 한다.
- 모든 구조물은 나중에 에셋 교체가 가능하도록 의미 있는 프록시 액터로 배치한다.
- Space Marine 2식 상호작용 오브젝트는 애니메이션 없이 `파괴 가능한 장애물`로 먼저 구현한다.

## 전체 흐름

```text
Start Safe Area
  -> Encounter Room 01
  -> Clear opens Gate 01
  -> Short Corridor
  -> Breakable Obstruction
  -> Encounter Room 02
  -> Reward / Exit
```

## 탑다운 배치도

```text
Legend
P  = Player Start
E  = Enemy
Z  = Encounter Zone
G  = Gate / Door
X  = Breakable obstruction
C  = Cover / pillar
R  = Reward
F  = Finish / exit

        [ Room 01: 기본 전투 ]
    +--------------------------------+
    |                                |
    |       C            C           |
    |                                |
    |            Z                   |
    |        E       E               |
    |                                |
P ->|                                |-> G
    +--------------------------------+
                                     
              [ Corridor ]
               +--------+
               |        |
               |   X    |  <- siphon/공격으로 파괴
               |        |
               +--------+

        [ Room 02: 약간 넓은 전투 ]
    +----------------------------------------+
    |                                        |
    |      C                         C       |
    |                                        |
G ->|              Z                         |-> F
    |          E       E                     |
    |                  E                     |
    |                                        |
    |                         R              |
    +----------------------------------------+
```

## 권장 크기

언리얼 기준 1칸을 대략 `400cm`로 생각한다.

| 구역 | 권장 크기 | 목적 |
| --- | --- | --- |
| Start Safe Area | 1200 x 800 | 전투 시작 전 조작 확인 |
| Room 01 | 2400 x 1800 | 기본 적 2마리 전투 |
| Corridor | 800 x 1600 | 전투 사이 호흡 |
| Breakable Obstruction | 600 x 300 x 300 | 상호작용/스킬 사용 체크 |
| Room 02 | 3200 x 2200 | 적 3마리와 공간 활용 테스트 |
| Exit Area | 800 x 800 | 한 판 종료 확인 |

## Room 01 설계

목적: 첫 EncounterZone과 `On Encounter Cleared`가 자연스럽게 이해되는지 확인한다.

배치:

- 플레이어는 방 입구에서 적을 볼 수 있어야 한다.
- 적 2마리는 방 중앙보다 약간 뒤쪽에 둔다.
- 기둥/엄폐물 2개를 좌우에 둔다.
- 클리어 시 `Gate 01`이 열린다.

검증할 것:

- 플레이어가 들어가면 전투가 시작되는가
- 적 처치 후 `On Encounter Cleared`가 발생하는가
- 문이 열려서 다음 공간으로 진행되는가

## Corridor 설계

목적: 전투 사이에 짧은 리듬을 만든다.

배치:

- 길은 짧고 좁게 만든다.
- 중간에 `BP_BreakableObstruction`을 둔다.
- 장애물은 처음에는 길을 막고 있다가, 파괴되면 사라진다.

이 구간은 퍼즐이 아니라 `상호작용 오브젝트가 게임 루프에 들어갈 수 있는지` 확인하는 자리다.

## Breakable Obstruction 아이디어

지금은 쓰러진 나무 모델이나 전용 애니메이션이 없으니, 아래처럼 가짜로 시작한다.

```text
BP_BreakableObstruction
  Static Mesh: SM_ChamferCube 또는 SM_Cube
  모양: 길을 막는 긴 박스
  상태:
    Intact
    Destroyed
  동작:
    플레이어가 Siphon/공격 사용
    -> Print String "Obstruction destroyed"
    -> Collision 끄기
    -> Mesh 숨기기 또는 Destroy Actor
```

처음부터 `쓰러진 나무를 치우는 애니메이션`을 만들 필요는 없다. Space Marine 2 같은 느낌은 나중에 붙이고, 지금은 `특정 오브젝트가 플레이 진행을 막고, 스킬/액션으로 제거된다`는 구조만 검증한다.

## Siphon 스킬과 연결하는 방식

가장 단순한 1차 구현:

```text
플레이어가 Siphon 사용
  -> 앞쪽 Line Trace 또는 Sphere Overlap
  -> BP_BreakableObstruction 감지
  -> Destroy / Hide / Disable Collision
```

애니메이션이 없어도 되는 이유:

- Siphon 모션이 이미 있다면 그것을 사용한다.
- 오브젝트는 피격 반응 없이 사라져도 된다.
- 나중에 에셋이 생기면 파편, 흔들림, 사운드, VFX를 추가한다.

처음에는 `키 입력 -> 장애물 제거`만 되어도 충분하다.

## Room 02 설계

목적: 첫 방보다 조금 복잡한 전투를 확인한다.

배치:

- 적 3마리 배치
- 한 마리는 입구에서 바로 보이게 둔다.
- 나머지 둘은 좌우로 벌려서 플레이어가 시야와 위치를 조정하게 만든다.
- 방 안에 기둥/엄폐물 2개를 둔다.
- 클리어 시 보상 또는 종료 지점을 활성화한다.

검증할 것:

- 좁은 복도 뒤 넓은 전투방으로 들어갈 때 카메라가 크게 망가지지 않는가
- 적 3마리가 동시에 와도 전투가 읽히는가
- 클리어 후 보상/종료가 명확한가

## Encounter 결과 연결

각 EncounterZone은 `ClearReceivers` 배열로 서로 다른 결과를 실행한다.

| 구역 | ClearReceivers | 결과 |
| --- | --- | --- |
| Room 01 EncounterZone | Gate 01 | 문 열기 |
| Breakable Obstruction | 없음 또는 직접 상호작용 | 장애물 제거 |
| Room 02 EncounterZone | Reward, Finish | 보상 표시 및 종료 활성화 |

## 필요한 프록시 액터 목록

| 액터 | 역할 | 임시 메쉬 |
| --- | --- | --- |
| `BP_BlockoutWall` | 벽 | `SM_Cube` |
| `BP_BlockoutFloor` | 바닥 | `SM_Plane` 또는 `SM_Cube` |
| `BP_BlockoutPillar` | 기둥/엄폐물 | `SM_ChamferCube` |
| `BP_BlockoutGate` | 열리는 문 | `SM_Door` 또는 `SM_Cube` |
| `BP_BreakableObstruction` | 파괴 가능한 장애물 | 길게 늘린 `SM_ChamferCube` |
| `BP_BlockoutReward` | 보상 표시 | `SM_Cylinder` 또는 `SM_Cube` |
| `BP_BlockoutExit` | 종료 지점 | `SM_Plane` 또는 Trigger |

## 첫 구현 순서

1. `BP_BlockoutGate`를 만든다.
2. `BPI_EncounterClearReceiver`를 구현해서 `OnEncounterCleared` 때 문이 열리게 한다.
3. `BP_BreakableObstruction`을 만든다.
4. Siphon 또는 임시 키 입력으로 장애물을 제거한다.
5. `Lvl_CombatLoop_Blockout` 맵을 만든다.
6. 위 탑다운 배치대로 방 2개를 만든다.
7. `EncounterZone` 2개를 배치한다.
8. 첫 구역 클리어는 문 열기, 두 번째 구역 클리어는 보상/종료로 연결한다.

## 이 스테이지에서 아직 하지 않을 것

- 환경 아트 완성
- 복잡한 퍼즐
- 많은 적 종류
- 긴 던전
- 전용 상호작용 애니메이션
- 세밀한 카메라 연출

지금 목표는 `게임이 한 판으로 성립하는가`를 보는 것이다.
