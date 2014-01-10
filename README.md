# MultiClock

This Windows 8/8.1 (x64 only) shell extension displays an additional clock on each secondary taskbar.

## Install

0. Install the [Visual C++ Redistributable Packages for Visual Studio 2013](http://www.microsoft.com/en-us/download/details.aspx?id=40784)
1. Copy MultiClock.exe and MultiClock.dll in some folder you remember.
2. Execute MultiClock.exe.
3. A clock icon should appear in your _notification area_.

## Uninstall

1. Stop MultiClock by clicking on the clock icon in your _notification area_.
2. Remove MultiClock.exe and MultiClock.dll from your system.

## Troubleshooting

* I get a _System Error_ stating that I'm missing _mfc120u.dll_!
  1. I told you to install the [Visual C++ Redistributable Packages for Visual Studio 2013](http://www.microsoft.com/en-us/download/details.aspx?id=40784)!
* There is no clock icon in my _notification area_!
  1. Is MultiClock.exe running? Check the Task Manager.
  2. Is there some error popup when starting MultiClock.exe?
* I'm seeing the clock icon but nothing happens...
  1. Do you have at least one secondary taskbar? Check your _Taskbar and Navigation properties_.
  2. Hover the clock icon and check how many clocks MultiClock is displaying.
* My explorer.exe keeps restarting when executing MultiClock!
  1. Should not happen, you've found a bug!

## Building

Build this shell extension with Visual Studio 2013, there are no third-party libraries used.
