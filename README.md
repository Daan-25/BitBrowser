# BitBrowser

A lightweight tabbed web browser built with **C++** and **Qt 6 WebEngine (Chromium)**.

---

## Features

- Multi-tab browsing
- Address bar
- Back / Forward / Reload
- Home button
- Keyboard shortcuts:
  - **Ctrl + T** → New tab
  - **Ctrl + W** → Close tab
  - **Ctrl + L** → Focus address bar

---

## Tech Stack

- **C++**
- **Qt 6 Widgets**
- **Qt WebEngine (Chromium)**
- **CMake**
- **Ninja**

---

## Requirements (Windows)

Install:

- **Qt 6.10.1** (or newer)
- Qt modules:
  - **Widgets**
  - **WebEngineWidgets**
  - **WebChannel**
  - **Positioning**
- **MSVC 2022 x64**
- **CMake**
- **Ninja** (via Qt Tools)

---

## Build (Windows)

Open **x64 Developer Command Prompt** (Visual Studio):

```bash
cd C:\Users\REPLACE\Documents\BitBrowser

rmdir /s /q build
mkdir build
cd build

cmake -G "Ninja" .. ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.10.1\msvc2022_64"

ninja
.\BitBrowser.exe
