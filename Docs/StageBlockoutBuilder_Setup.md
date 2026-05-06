# StageBlockoutBuilder 사용법

`StageBlockoutBuilder`는 Unity의 Inspector 버튼 생성기처럼 쓰는 언리얼 액터다. 레벨에 배치하고 Details 패널에서 설계도를 넣은 뒤 버튼을 누르면 큐브/문/기둥/장애물 같은 블록아웃 부품이 자동으로 생성된다.

## 기본 사용 순서

1. 에디터를 닫고 프로젝트를 한 번 빌드하거나, 열 때 `rebuild` 창이 뜨면 `Yes`를 누른다.
2. 레벨에 `StageBlockoutBuilder` 액터를 배치한다.
3. Details 패널에서 `Stage Blockout > Layout`을 펼친다.
4. `Inline Layout Rows`를 직접 수정하거나 `Csv Layout File`에 CSV 경로를 넣는다.
5. `Build From Inline Layout` 또는 `Build From Csv File` 버튼을 누른다.
6. 다시 만들고 싶으면 `Clear Generated Blockout` 또는 `Build` 버튼을 다시 누른다.

생성된 부품은 Builder 액터의 Instanced Static Mesh 컴포넌트로 붙는다. Builder를 움직이면 생성된 맵 전체가 같이 움직인다.

## 설계도 기호

| 기호 | 의미 | 기본 생성물 |
| --- | --- | --- |
| `.` | 비어 있음 | 아무것도 생성하지 않음 |
| `_` | 바닥만 | 낮은 큐브 바닥 |
| `W` | 벽 | 큰 큐브 |
| `C` | 기둥/엄폐물 | 챔퍼 큐브 |
| `G` | 문/게이트 위치 | 바닥 위 주황색 마커 |
| `X` | 파괴 가능한 장애물 | 통로를 가로막는 빨간색 낮은 블록 |
| `E` | 적 배치 마커 | 분홍색 세로 실린더 |
| `Z` | EncounterZone 마커 | 파란색 낮은 실린더 |
| `R` | 보상 마커 | 노란색 낮은 실린더 |
| `S` | 시작 위치 마커 | 초록색 낮은 실린더 |
| `F` | 종료 위치 마커 | 보라색 낮은 실린더 |

각 기호는 기본 색이 다르다.

| 기호 | 기본 색 |
| --- | --- |
| `_` | 녹색 바닥 |
| `W` | 밝은 회색 벽 |
| `C` | 청록색 기둥/엄폐물 |
| `G` | 주황색 문/게이트 |
| `X` | 빨간색 파괴 장애물 |
| `E` | 분홍색 적 마커 |
| `Z` | 파란색 EncounterZone 마커 |
| `R` | 노란색 보상 마커 |
| `S` | 초록색 시작 마커 |
| `F` | 보라색 종료 마커 |

`Tile Definitions`에서 기호, 메쉬, 머티리얼, 색, 스케일, 충돌 여부를 바꿀 수 있다. 나중에 에셋 팩을 가져오면 여기서 메쉬만 교체하면 된다.

색은 `Material Override`의 `Base Color` 파라미터를 바꾸는 방식으로 적용된다. 기본값은 `MI_DefaultColorway`를 사용한다.

`Snap Bottom To Cell Floor`가 켜진 부품은 메쉬 피벗이 중앙이든 바닥이든 자동으로 셀 바닥에 붙는다. 벽이나 기둥이 공중에 떠 보이면 예전 Builder 인스턴스가 오래된 Z 오프셋을 들고 있을 가능성이 있으니, `Clear Generated Blockout`을 누른 뒤 다시 Build하거나 Builder를 새로 배치한다.

Builder는 메쉬 피벗이 아니라 메쉬 bounds의 중심을 셀 중앙에 맞춘다. 시작 마커나 적 마커가 벽에 반쯤 박혀 보이면 예전 생성물이 남아 있는 것이므로 `Clear Generated Blockout` 후 다시 Build한다. 그래도 이상하면 Builder 액터를 삭제하고 새로 배치한다.

## CSV / Inline Rows 형식

쉼표로 칸을 나눈다.

```csv
W,W,W,W,W
W,S,_,E,W
W,_,Z,_,G
W,W,W,W,W
```

쉼표 없이 한 글자씩 써도 된다.

```text
WWWWW
WS_EW
W_Z_G
WWWWW
```

층을 나누고 싶으면 빈 줄이나 `Layer 1` 같은 줄을 넣는다. 새 Layer는 `Layer Height`만큼 위에 생성된다.

```csv
Layer 0
W,W,W
W,S,W
W,W,W

Layer 1
.,C,.
.,_,.
.,C,.
```

## 첫 스테이지 예시

샘플 CSV는 여기에 있다.

`Docs/SampleStageBlockout.csv`

이 파일을 `Csv Layout File`에 넣고 `Build From Csv File`을 누르면 첫 전투방, 복도, 장애물, 두 번째 전투방이 한 번에 생성된다.

## 블록아웃 이후 연결

이 Builder는 구조와 위치를 빠르게 뽑는 용도다. 실제 게임 플레이 액터는 생성된 마커를 보고 따로 배치한다.

- `E` 위치 근처에 실제 적 배치
- `Z` 위치에 `BP_EncounterZone` 배치
- `G` 위치에 `BP_BlockoutGate` 또는 실제 문을 별도 액터로 배치
- `X` 위치에 `BP_BreakableObstruction` 배치
- `R` 위치에 보상 액터 배치
- `F` 위치에 종료 트리거 배치

처음부터 모든 것을 자동 생성하려고 하지 말고, Builder는 `설계도 시각화 + 블록아웃 배치`로 쓰는 것이 안전하다.
