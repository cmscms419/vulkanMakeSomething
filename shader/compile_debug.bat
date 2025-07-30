@echo off
REM Vulkan SDK 경로를 환경변수에서 가져옵니다.
set GLSLANG_VALIDATOR=%VULKAN_SDK%/Bin/glslangValidator.exe

echo vert compile start

for %%f in (*.vert) do (
    echo compiling: %%f
    %GLSLANG_VALIDATOR% -e main -gVS -V -o vert%%~nf.spv %%f
)

echo.
echo vert compile complete

echo.
echo frag compile start

for %%f in (*.frag) do (
    echo compiling: %%f
    %GLSLANG_VALIDATOR% -e main -gFS -V -o frag%%~nf.spv %%f
)

echo.
echo frag compile complete

echo.
echo comp compile start

for %%f in (*.comp) do (
    echo compiling: %%f
    %GLSLANG_VALIDATOR% -V -g -Od -gCS %%f -o comp%%~nf.spv
)

pause