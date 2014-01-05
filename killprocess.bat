echo Killing %1
tasklist /FI "IMAGENAME eq %1" 2>NUL | find /I /N "%1">NUL
if "%ERRORLEVEL%"=="0" (
	Taskkill /IM %1
	echo Wait for the process to terminate...
	powershell -command "Start-Sleep -s 3"
) else (
	echo %1 is not running
)