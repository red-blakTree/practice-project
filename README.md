# practice-project

## 项目概述

本项目包含多个子项目，涵盖C++、C、Python等编程语言。主要功能包括音乐播放、任务管理系统、鼠标模拟器以及网络ARP欺骗工具。

## 子项目介绍

### 1. 音乐播放器 (`music.cpp`)

- **功能**: 播放预设的音乐旋律。
- **实现**: 使用MIDI接口播放音乐。
- **依赖**: `winmm.lib` 库。
- **编译**: 使用C++编译器，确保链接 `winmm.lib`。

### 2. 任务管理系统 (`app.py`)

- **功能**: 任务的创建、查询、更新和删除。
- **实现**: 使用Flask框架。
- **依赖**: Flask, json, os, datetime。
- **运行**: 安装Flask库后，运行 `app.py`。

### 3. 鼠标模拟器 (`MouseConnector.c`)

- **功能**: 模拟鼠标点击，支持Windows和Linux平台。
- **实现**: 使用Windows API和X11库。
- **依赖**: Windows API, X11库。
- **编译**: 使用C编译器，确保包含相应的库。

### 4. ARP欺骗工具 (`server.cpp`)

- **功能**: 执行ARP欺骗攻击，用于网络监控和安全测试。
- **实现**: 使用httplib, nlohmann/json, pcap库。
- **依赖**: httplib, nlohmann/json, pcap。
- **运行**: 安装依赖库后，编译并运行 `server.cpp`。

## 使用方法

### 音乐播放器

1. 编译 `music.cpp`。
2. 运行生成的可执行文件。

### 任务管理系统

1. 安装Flask: `pip install flask`
2. 运行 `app.py`: `python app.py`
3. 访问 `http://127.0.0.1:5000/tasks` 进行任务管理。

### 鼠标模拟器

1. 编译 `MouseConnector.c`。
2. 运行生成的可执行文件。
3. 按F2键开始/停止鼠标点击。

### ARP欺骗工具

1. 安装依赖库: `httplib`, `nlohmann/json`, `pcap`。
2. 编译 `server.cpp`。
3. 运行生成的可执行文件。
4. 访问 `http://127.0.0.1:8080` 进行控制。

## 注意事项

- ARP欺骗工具涉及网络安全，仅限合法用途。
- 使用鼠标模拟器时，请确保不会影响正常工作。
- 任务管理系统存储数据在本地文件，注意数据安全。
