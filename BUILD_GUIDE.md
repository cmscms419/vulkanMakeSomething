# VS Code에서 VS2022 프로젝트 빌드 가이드

## 개요

`.vscode/tasks.json`에 MSBuild 태스크를 하나씩 추가해서 각 프로젝트를 독립적으로 빌드한다.
솔루션(`.sln`) 없이 `.vcxproj`를 직접 지정하는 방식을 사용한다.

---

## 프로젝트 의존성 구조

```
[구 엔진 체인]
common → basicEngine → component → (00.basic, 01.Square, ...)

[신 엔진 체인]
common → basicEngine2 → component2 → (05.postprocessing, 06.GLTF, 14.gltfLoader, ...)
```

출력 경로: `external/<프로젝트명>/x64/Debug/<프로젝트명>.lib`

---

## tasks.json 기본 구조

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build: <이름> (Debug)",
      "type": "shell",
      "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
      "args": [
        "${workspaceFolder}/Project/<폴더>/<파일>.vcxproj",
        "/p:Configuration=Debug",
        "/p:Platform=x64",
        "/p:SolutionDir=${workspaceFolder}/Project/",
        "/v:minimal"
      ],
      "group": "build",
      "problemMatcher": "$msCompile",
      "presentation": {
        "reveal": "always",
        "panel": "shared"
      }
    }
  ]
}
```

### 핵심 인자 설명

| 인자 | 설명 |
|---|---|
| `<파일>.vcxproj` | 솔루션 대신 프로젝트 파일 직접 지정 |
| `/p:Configuration=Debug` | Debug / Release 선택 |
| `/p:Platform=x64` | 플랫폼 지정 |
| `/p:SolutionDir=...` | vcxproj 내부의 `$(SolutionDir)` 매크로 수동 제공 (출력 경로 계산에 필요) |
| `/v:minimal` | 출력 최소화 (normal, detailed, diagnostic 으로 변경 가능) |

> `.vcxproj.filters` 파일은 IDE에서 파일 트리 표시용이며 빌드에는 관여하지 않는다.

---

## 빌드 태스크 추가 순서

### STEP 1 — common (정적 라이브러리)

출력: `external/common/x64/Debug/common.lib`

```json
{
  "label": "Build: common (Debug)",
  "type": "shell",
  "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
  "args": [
    "${workspaceFolder}/Project/common/common.vcxproj",
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/p:SolutionDir=${workspaceFolder}/Project/",
    "/v:minimal"
  ],
  "group": {
    "kind": "build",
    "isDefault": true
  },
  "problemMatcher": "$msCompile",
  "presentation": { "reveal": "always", "panel": "shared" }
}
```

---

### STEP 2 — basicEngine (구 엔진, 정적 라이브러리)

의존: `common`
출력: `external/basicEngine/x64/Debug/basicEngine.lib`

```json
{
  "label": "Build: basicEngine (Debug)",
  "type": "shell",
  "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
  "args": [
    "${workspaceFolder}/Project/basicEngine/basicEngine.vcxproj",
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/p:SolutionDir=${workspaceFolder}/Project/",
    "/v:minimal"
  ],
  "dependsOn": ["Build: common (Debug)"],
  "dependsOrder": "sequence",
  "group": "build",
  "problemMatcher": "$msCompile",
  "presentation": { "reveal": "always", "panel": "shared" }
}
```

---

### STEP 3 — basicEngine2 (신 엔진, 정적 라이브러리)

의존: `common`
출력: `external/basicEngine2/x64/Debug/basicEngine2.lib`

```json
{
  "label": "Build: basicEngine2 (Debug)",
  "type": "shell",
  "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
  "args": [
    "${workspaceFolder}/Project/basicEngine2/basicEngine2.vcxproj",
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/p:SolutionDir=${workspaceFolder}/Project/",
    "/v:minimal"
  ],
  "dependsOn": ["Build: common (Debug)"],
  "dependsOrder": "sequence",
  "group": "build",
  "problemMatcher": "$msCompile",
  "presentation": { "reveal": "always", "panel": "shared" }
}
```

---

### STEP 4 — component (구 엔진, 정적 라이브러리)

의존: `common`, `basicEngine`
출력: `external/component/x64/Debug/component.lib`

```json
{
  "label": "Build: component (Debug)",
  "type": "shell",
  "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
  "args": [
    "${workspaceFolder}/Project/component/component.vcxproj",
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/p:SolutionDir=${workspaceFolder}/Project/",
    "/v:minimal"
  ],
  "dependsOn": ["Build: basicEngine (Debug)"],
  "dependsOrder": "sequence",
  "group": "build",
  "problemMatcher": "$msCompile",
  "presentation": { "reveal": "always", "panel": "shared" }
}
```

---

### STEP 5 — component2 (신 엔진, 정적 라이브러리)

의존: `common`, `basicEngine2`
출력: `external/component2/x64/Debug/component2.lib`

```json
{
  "label": "Build: component2 (Debug)",
  "type": "shell",
  "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
  "args": [
    "${workspaceFolder}/Project/component2/component2.vcxproj",
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/p:SolutionDir=${workspaceFolder}/Project/",
    "/v:minimal"
  ],
  "dependsOn": ["Build: basicEngine2 (Debug)"],
  "dependsOrder": "sequence",
  "group": "build",
  "problemMatcher": "$msCompile",
  "presentation": { "reveal": "always", "panel": "shared" }
}
```

---

### STEP 6 — 실행 파일 프로젝트 (Application)

의존: 해당 엔진 체인의 마지막 라이브러리
출력: `exe/<프로젝트명>/x64/Debug/<프로젝트명>.exe`

**구 엔진 사용 프로젝트 (00.basic 예시)**
```json
{
  "label": "Build: 00.basic (Debug)",
  "type": "shell",
  "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
  "args": [
    "${workspaceFolder}/Project/00.basic/00.basic.vcxproj",
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/p:SolutionDir=${workspaceFolder}/Project/",
    "/v:minimal"
  ],
  "dependsOn": ["Build: component (Debug)"],
  "dependsOrder": "sequence",
  "group": "build",
  "problemMatcher": "$msCompile",
  "presentation": { "reveal": "always", "panel": "shared" }
}
```

**신 엔진 사용 프로젝트 (05.postprocessing 예시)**

> 주의: 프로젝트 이름의 `.`은 MSBuild 타겟명에서 `_2E`로 인코딩되지만,
> `.vcxproj` 직접 지정 방식에서는 파일 경로를 그대로 쓰면 된다.

```json
{
  "label": "Build: 05.postprocessing (Debug)",
  "type": "shell",
  "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe",
  "args": [
    "${workspaceFolder}/Project/05.postprocessing/05.postprocessing.vcxproj",
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/p:SolutionDir=${workspaceFolder}/Project/",
    "/v:minimal"
  ],
  "dependsOn": ["Build: component2 (Debug)"],
  "dependsOrder": "sequence",
  "group": "build",
  "problemMatcher": "$msCompile",
  "presentation": { "reveal": "always", "panel": "shared" }
}
```

---

## dependsOn 작동 방식

```
"Build: 05.postprocessing (Debug)"
    └─ dependsOn: "Build: component2 (Debug)"
            └─ dependsOn: "Build: basicEngine2 (Debug)"
                    └─ dependsOn: "Build: common (Debug)"
```

`Ctrl+Shift+B` 또는 태스크 실행 시 체인 전체가 순서대로 자동 실행된다.

---

## 빌드 실행 방법

| 단축키 / 메뉴 | 동작 |
|---|---|
| `Ctrl+Shift+B` | 기본 빌드 태스크 (`isDefault: true`) 실행 |
| `Ctrl+Shift+P` → `Tasks: Run Task` | 태스크 목록에서 원하는 항목 선택 |
| `Ctrl+Shift+P` → `Tasks: Run Build Task` | 빌드 그룹 태스크만 선택 |

---

## launch.json 연동 (디버그 실행)

실행 파일 프로젝트 빌드 태스크 추가 후, `launch.json`에 디버그 설정을 추가한다.

```json
{
  "name": "Debug: 00.basic",
  "type": "cppvsdbg",
  "request": "launch",
  "program": "${workspaceFolder}/exe/00.basic/x64/Debug/00.basic.exe",
  "args": [],
  "stopAtEntry": false,
  "cwd": "${workspaceFolder}/exe/00.basic/x64/Debug",
  "environment": [],
  "console": "integratedTerminal",
  "preLaunchTask": "Build: 00.basic (Debug)"
}
```

`preLaunchTask`에 태스크 label을 지정하면 `F5` 실행 시 빌드 → 실행이 자동으로 이어진다.
