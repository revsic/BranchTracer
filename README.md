# BranchTracer
C++ Implementation of Branch Tracer.

- Copyright (c) 2019 YoungJoong Kim. tf-branch-malware is licensed under the [MIT license](./LICENSE).
- This repository aims to write branch based windows debugger.

## Tested Environments
- Windows 10
- Visual Studio 2019

## Usage
Compile solution with visual studio or msbuild.
```
msbuild .\Brancher\Brancher.sln /p:configuration=Debug
```
Make log file on C:\dbg
```
mkdir C:\dbg
echo "" > C:\dbg\log.txt    # branch tracer open existing log file not create new
```
Default is debug mode and it will run internet explorer as sample.

CTRL + F5 to run Helper on visual studio, or run on cmd.
```
cd .\Brancher\Brancher
..\x64\Debug\Helper.exe     # Helper run branch tracer based on relative path
```
On release mode, place Brancher.dll on C:\dbg\Brancher64.dll and target program on C:\dbg\sample.exe.
```
msbuild .\Brancher\Brancher.sln /p:configuration=Release
mv .\Brancher\x64\Release\Brancher.dll C:\dbg\Brancher64.dll
cp C:\Windows\notepad.exe C:\dbg\sample.exe
echo "" > C:\dbg\log.txt                                        # reinitialize log
.\Brancher\x64\Release\Helper.exe
```
Sample log file.
```
+00007FF730C9B842,00007FFF4E004CC0,msvcrt.dll,memset
+00007FF730C84279,00007FFF4D83E420,KERNEL32.DLL,GetCommandLineW
+00007FF730C84289,00007FF730C81130,,
+00007FF730C8117C,00007FFF4F6CCDE0,ntdll.dll,EtwEventRegister
+00007FF730C811A9,00007FFF4F6F3720,ntdll.dll,EtwEventSetInformation
+00007FF730C811B9,00007FF730C9AEC0,,
+00007FF730C84298,00007FFF4DC0DE40,combase.dll,CoCreateGuid
+00007FF730C84331,00007FFF4D83EEB0,KERNEL32.DLL,HeapSetInformation
+00007FF730C8433E,00007FFF4DBBF1A0,combase.dll,CoInitializeEx
+00007FF730C84354,00007FF730C9A8B0,,
+00007FF730C9A8DE,00007FFF4DC5C850,combase.dll,RoInitialize
+00007FF730C9A903,00007FFF4DBC9260,combase.dll,WindowsCreateStringReference
+00007FF730C9A94E,00007FFF4DBC3FC0,combase.dll,RoGetActivationFactory
+00007FF730C9A971,00007FFF4F70FC10,ntdll.dll,RtlRetrieveNtUserPfn
+00007FF730C9AAD7,00007FFF4F70FC10,ntdll.dll,RtlRetrieveNtUserPfn
+00007FF730C9AAE6,00007FF730C9AEC0,,
```
