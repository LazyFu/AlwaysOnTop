# Always On Top

Always On Top is a lightweight application that allows pinning any window on top. It is written in C++, so it does not require the .NET runtime.

Always On Top是一款轻量化软件用于顶置任意Windows窗口。本软件使用c++编写，所以不需要.NET运行时。

I wrote this because [PinWin](https://github.com/Bluegrams/PinWin) does not have a hotkey, and [PowerToys](https://github.com/microsoft/PowerToys) is too heavy.

我编写这个程序因为PinWin没有快捷键，PowerToys过于臃肿。

## Setup 安装

### Compile Options 编译选项

`g++ AlwaysOnTop.cpp -o AlwaysOnTop.exe -lgdi32 -luser32 -lshell32 -mwindows`

> Note: C++ standard >= C++11
>
> 注意：C++标准 >= C++11

Alternatively, you can directly download executable file from the release page.

或者你可以直接在release页面下载可执行文件。

## Usage 使用

Use the hotkey `Ctrl+Shift+T` to pin or unpin window, you can see the list of windows you pinned in the system tray, which are marked as "(Pinned)".

使用快捷键`Ctrl+Shift+T`来顶置或取消顶置窗口，你可以在系统托盘查看你顶置的窗口列表，他们被标记为“(Pinned)”。
