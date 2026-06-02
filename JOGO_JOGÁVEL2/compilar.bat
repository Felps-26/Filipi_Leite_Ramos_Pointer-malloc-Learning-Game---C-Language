@echo off
:: compilar.bat — Compila o TennisF1 com todos os modulos separados por pasta

:: Configuracoes do Raylib e GCC
set "GCC=C:\raylib\w64devkit\bin\gcc.exe"
set "INCLUDE=C:\raylib\raylib\src"
set "LIB=C:\raylib\raylib\src"
set "FLAGS=-O2 -Wall -Wno-missing-braces"
set "LIBS=-lraylib -lopengl32 -lgdi32 -lwinmm -lm"

:: Adiciona a pasta bin ao PATH para o assembler funcionar
set "PATH=C:\raylib\w64devkit\bin;%PATH%"

:: Lista de todos os arquivos .c do projeto
set "SOURCES=TennisF1.c Gameplay\gameplay.c Telas\telas.c skins\skins.c"

echo [INFO] Compilando TennisF1 (modular)...
echo [INFO] Arquivos: %SOURCES%

"%GCC%" %SOURCES% -o TennisF1.exe %FLAGS% -I "%INCLUDE%" -L "%LIB%" %LIBS%

if %errorlevel% equ 0 (
    echo [OK] Compilacao concluida com sucesso!
    echo [INFO] Para rodar: .\TennisF1.exe
) else (
    echo [ERRO] Falha na compilacao.
)
