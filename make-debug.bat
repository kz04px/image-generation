@echo off
g++ src\main.cpp src\callbacks.cpp src\paintings.cpp src\io.cpp src\other.cpp -Wall -Wextra -lglfw3 -lglew32 -lgdi32 -lopengl32 -o bin\main.exe
PAUSE