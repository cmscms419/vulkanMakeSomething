@echo off
REM Compile every shader in this directory for release.
REM Use compile_debug.bat instead when you need debug info
REM (glslangValidator -g -Od).
REM
REM Output names follow the convention the engine requires:
REM   <stage prefix> + <source base name> + .spv
REM     cloth.vert       -> vertcloth.spv
REM     cloth.frag       -> fragcloth.spv
REM     clothSolve.comp  -> compclothSolve.spv
REM ShaderManager looks up shaders by these names only. A wrong name still
REM compiles fine but the shader is not found at run time.
REM
REM *.glsl files are include fragments, so they are never compiled directly.
REM
REM Do NOT add -O here. The engine reflects on the SPIR-V to collect
REM descriptor bindings and push constants by NAME (see
REM ShaderManager::collectPerPipelineBindings). -O strips those names, and
REM the engine then logs "Binding name is empty in shader" and crashes while
REM building descriptor layouts. Compiling without -O keeps the names.
REM
REM Comments here are ASCII on purpose: cmd reads a batch file in the OEM
REM code page, so non-ASCII comment text can break line parsing.

setlocal enabledelayedexpansion

set GLSLC=%VULKAN_SDK%\Bin\glslc.exe

if not exist "%GLSLC%" (
    echo [error] glslc.exe not found: %GLSLC%
    echo         Check the VULKAN_SDK environment variable.
    pause
    exit /b 1
)

set FAILED=0

echo === vert ===
for %%f in (*.vert) do (
    "%GLSLC%" "%%f" -o "vert%%~nf.spv"
    if errorlevel 1 (
        echo   [FAIL] %%f
        set /a FAILED+=1
    ) else (
        echo   vert%%~nf.spv
    )
)

echo.
echo === frag ===
for %%f in (*.frag) do (
    "%GLSLC%" "%%f" -o "frag%%~nf.spv"
    if errorlevel 1 (
        echo   [FAIL] %%f
        set /a FAILED+=1
    ) else (
        echo   frag%%~nf.spv
    )
)

echo.
echo === comp ===
for %%f in (*.comp) do (
    "%GLSLC%" "%%f" -o "comp%%~nf.spv"
    if errorlevel 1 (
        echo   [FAIL] %%f
        set /a FAILED+=1
    ) else (
        echo   comp%%~nf.spv
    )
)

echo.
if %FAILED%==0 (
    echo All shaders compiled.
) else (
    echo %FAILED% shader^(s^) failed. See the [FAIL] lines above.
)

pause
exit /b %FAILED%
