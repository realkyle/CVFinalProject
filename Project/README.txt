Parking Lot Occupancy Detector
================================
Kyle Hale & Jingyi Zhong — UW Computer Vision, Spring 2026

REQUIREMENTS
------------
  - Visual Studio 2022
  - OpenCV 4.11.0 installed at C:\opencv\
  - OpenCV DLL directory: C:\opencv\build\x64\vc16\bin\ (must be on PATH)

BUILD
-----
Option A — Visual Studio:
  1. Open Project\Project.sln
  2. Set platform to x64 (dropdown at top)
  3. Build > Build Solution (Ctrl+Shift+B)

Option B — PowerShell:
  & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
    "Project.sln" /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal

RUN
---
Add OpenCV to PATH first (required once per terminal session):
  $env:Path += ";C:\opencv\build\x64\vc16\bin"

Single image:
  cd Project\x64\Debug
  .\Project.exe "..\..\images\PUCPR1.jpg"

All images (batch script):
  cd Project
  scripts\run.bat

OUTPUT
------
Annotated images are saved to Project\output\result_<filename>.jpg
The terminal prints: Occupied: N | Empty: N | Total: N
