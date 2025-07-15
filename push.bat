@ECHO OFF
CD /D "%~DP0"
GOTO ADD

:ADD
git add .
IF %ERRORLEVEL%==0 (GOTO COMMIT) ELSE (GOTO FAILURE)

:COMMIT
git commit -m Update
IF %ERRORLEVEL%==0 (GOTO PUSH) ELSE (GOTO FAILURE)

:PUSH
git push
IF %ERRORLEVEL%==0 (GOTO SUCCESS) ELSE (GOTO FAILURE)

:SUCCESS
ECHO Please press any key to exit (0). 
PAUSE>NUL
EXIT 0

:FAILURE
ECHO Please press any key to exit (1). 
PAUSE>NUL
EXIT 1
