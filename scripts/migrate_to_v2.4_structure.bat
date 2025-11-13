@echo off
echo ========================================
echo Migrating to v2.4 structure
echo ========================================
echo.

cd ..

echo [1/5] Creating new directory structure...
if not exist include\gui mkdir include\gui
if not exist include\models mkdir include\models
if not exist src\gui mkdir src\gui
if not exist src\models mkdir src\models
if not exist resources mkdir resources
if not exist resources\icons mkdir resources\icons
if not exist resources\styles mkdir resources\styles

echo [2/5] Directory structure created!
echo.
echo [3/5] Current files location:
echo   Headers: include\*.h
echo   Sources: src\*.cpp
echo.
echo [4/5] New GUI files should go to:
echo   Headers: include\gui\*.h and include\models\*.h
echo   Sources: src\gui\*.cpp and src\models\*.cpp
echo   Resources: resources\
echo.
echo [5/5] Migration complete!
echo.
echo NEXT STEPS:
echo 1. Copy GUI header files to include\gui\
echo 2. Copy GUI source files to src\gui\
echo 3. Copy models files to include\models\ and src\models\
echo 4. Add resources to resources\
echo 5. Run build_gui.bat
echo.

pause