   @echo off
   REM Compileaza si porneste doua procese separate
   gcc -Wall -o numarare.exe numarare.c || exit /b 1
   start "Proces 1" cmd /k numarare.exe
   start "Proces 2" cmd /k numarare.exe