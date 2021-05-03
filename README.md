E2
==

1. Intallation on Windows 10 via the command prompt:

a) Install dependencies

Install CMake form https://cmake.org/
`c:\3rd\vcpkg>git clone `https://github.com/microsoft/vcpkg.git .
`c:\3rd\vcpkg>bootstrap-vcpkg.bat`
`c:\3rd\vcpkg>vcpkg.exe integrate install`
`c:\3rd\vcpkg>vcpkg.exe install glad glfw3 eigen3 lodepng boost --triplet=x64-windows-static-md`

b) Install E2

`c:\dev\E2>git clone `https://github.com/trtikm/E2.git .

2. Building on Windows 10 via the command prompt:

`c:\dev\E2\build\RelWithDebInfo>cmake ..\..\ -DCMAKE_TOOLCHAIN_FILE=c:\3rd\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -DCMAKE_BUILD_TYPE=RelWithDebInfo`
`c:\dev\E2\build\RelWithDebInfo>cmake --build . --target install --config RelWithDebInfo`

3. Building on Windows 10 via MS Visual Studio Code:

Create `c:\dev\E2\.vscode\settings.json` :
```
{
    "files.associations": {
        "dense": "cpp"
    },
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "c:/3rd/vcpkg/scripts/buildsystems/vcpkg.cmake",
        "VCPKG_TARGET_TRIPLET": "x64-windows-static-md"
    },
    "cmake.buildDirectory" : "${workspaceRoot}/build/${buildType}",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```
`c:\dev\E2>code .`
Choose "install" build target (Press button on the right of "Build" button).
Press "Build" button.
Optionally, you can create `c:\dev\E2\.vscode\launch.json` :
```
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "e2sim @ <scene-name>",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/dist/tools/e2sim_Windows_RelWithDebInfo.exe",
            "args": ["--scene", "<scene-name>"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/dist/tools/",
            "environment": [],
            "externalConsole": false
        }
    ]
}
```