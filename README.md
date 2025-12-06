# BitBrowser



A simple tabbed browser built with **C++** and **Qt 6** using **Qt WebEngine**.



## Features

- Tab support

- Address bar

- Back / Forward / Reload

- Home button

- Shortcuts:

&nbsp; - **Ctrl+T**: New tab

&nbsp; - **Ctrl+W**: Close tab

&nbsp; - **Ctrl+L**: Focus address bar



## Tech

- C++

- Qt 6 Widgets

- Qt WebEngine (Chromium)



## Requirements (Windows)

- **Qt 6.10.1**

- Modules:

&nbsp; - `Widgets`

&nbsp; - `WebEngineWidgets`

&nbsp; - `WebChannel`

&nbsp; - `Positioning`

- **MSVC 2022 x64**

- CMake + Ninja (via Qt Tools)



## Build (Windows)



Open **x64 Developer Command Prompt** (Visual Studio):



```bat

cd C:\Users\REPLACE\Documents\BitBrowser



rmdir /s /q build

mkdir build



"C:\QtToolsCMake_64bincmake.exe" -S . -B build -G Ninja ^

-DCMAKE_PREFIX_PATH=C:Qt6.10.1msvc2022_64



"C:\QtToolsCMake_64bincmake.exe" --build build

