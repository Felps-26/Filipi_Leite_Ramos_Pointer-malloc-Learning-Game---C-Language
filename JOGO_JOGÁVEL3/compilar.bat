@echo off
:: compilar.bat — Compila o RPG do JOGO_JOGÁVEL3 com os módulos do projeto

:: Configurações do Raylib e GCC
set "GCC=C:\raylib\w64devkit\bin\gcc.exe"
set "INCLUDE=C:\raylib\raylib\src"
set "LIB=C:\raylib\raylib\src"
set "FLAGS=-O2 -Wall -Wno-missing-braces"
set "LIBS=-lraylib -lopengl32 -lgdi32 -lwinmm -lm"

:: Adiciona a pasta bin ao PATH para o assembler funcionar
set "PATH=C:\raylib\w64devkit\bin;%PATH%"

:: Lista de todos os arquivos .c do projeto
set "SOURCES=rpg.c Gameplay\gameplay.c Telas\telas.c"

echo [INFO] Compilando Legend of the Square RPG (modular)...
echo [INFO] Arquivos: %SOURCES%

"%GCC%" %SOURCES% -o rpg_game.exe %FLAGS% -I "%INCLUDE%" -L "%LIB%" %LIBS%

if %errorlevel% equ 0 (
    echo [OK] Compilacao concluida com sucesso!
    echo [INFO] Para rodar: .\rpg_game.exe
) else (
    echo [ERRO] Falha na compilacao.
)
