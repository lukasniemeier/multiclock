# MultiClock

This Windows 8.1 shell extension displays an additional clock on each secondary taskbar. 

## Install

1. Copy MultiClock.exe and MultiClock.dll in some folder you remember.
2. Execute MultiClock.exe.
3. A clock icon should appear in your _systray_.

## Uninstall

1. Stop MultiClock by clicking on the clock icon in your _systray_.
2. Remove MultiClock.exe and MultiClock.dll from your system.

## Troubleshooting

* There is no clock icon in my _systray_!
  1. Is MultiClock.exe running? Check the Task Manager.
  2. Is there some error popup when starting MultiClock.exe?
* I'm seeing the clock icon but nothing happens...
  1. Do you have at least one secondary taskbar? Check your _Taskbar and Navigation properties_.
  2. Right-Click on the clock icon and check how many clocks MultiClock is displaying.
* My explorer.exe keeps restarting when executing MultiClock!
  1. Should not happen, you've found a bug!

## Building

Build this shell extension with Visual Studio 2013, there are no thrid-party libraries used.
