@echo off
cls

echo ===================================================================================================
echo Code Cleaning started
echo.
REM Cleaning code
for %%f in (bin\*.obj) do del "%%f"
for %%f in (bin\*.res) do del "%%f"
echo.
echo Code Cleaning Finished
echo ===================================================================================================
echo.
echo.


echo ===================================================================================================
echo Code Compilation started
echo.
REM Compile the source file
cl.exe /c /EHsc /I "C:\\glew-2.1.0\\include" /I "C:\\MyProjects\\PersonalProjects\\OpenGL_PP_Engine\\OpenGL_PP_Engine\\include" src/EngineMain.cpp /Fo"bin\\"
if errorlevel 1 goto :SourceCompilationError
echo.
echo Code Compilation Finished
echo ===================================================================================================
echo.
echo.

echo ===================================================================================================
echo Resource Compilation started
echo.
REM Compile the resource file
rc.exe /I "C:\\MyProjects\\PersonalProjects\\OpenGL_PP_Engine\\OpenGL_PP_Engine\\include" /fo "bin\\EngineMain.res" src/EngineMain.rc
if errorlevel 1 goto :resourceCompilationError
echo.
echo Resource Compilation Finished
echo ===================================================================================================
echo.
echo.


echo ===================================================================================================
echo Linking started
echo.
REM Link the object and resource files
link.exe bin/EngineMain.obj bin/EngineMain.res User32.lib GDI32.lib /LIBPATH:"C:\\glew-2.1.0\\lib\\Release\\x64" /SUBSYSTEM:WINDOWS
if errorlevel 1 goto :linkingError
echo.
echo Linking Finished
echo ===================================================================================================
echo.
echo.



echo Build Sucessfull
goto :eof

:SourceCompilationError
echo Source files compilation failed !!!

:resourceCompilationError
echo Resources compilation failed !!!

:linkingError
echo Linking failed !!!

exit /b 1
