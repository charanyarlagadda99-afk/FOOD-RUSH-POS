@echo off
echo ===================================================
echo               FOODRUSH LAUNCHER
echo ===================================================
echo.

IF NOT EXIST foodrush_engine.exe (
    echo Compiling FoodRush C++ Engine...
    g++ -std=c++17 -O2 -static src/main.cpp -o foodrush_engine.exe
    IF ERRORLEVEL 1 (
        echo Compilation failed! Please check your C++ compiler.
        pause
        exit /b 1
    )
    echo C++ Engine compiled successfully!
)

echo Starting FoodRush Server on http://localhost:3000 ...
echo Open your browser at http://localhost:3000
echo.
node server/server.js
pause
