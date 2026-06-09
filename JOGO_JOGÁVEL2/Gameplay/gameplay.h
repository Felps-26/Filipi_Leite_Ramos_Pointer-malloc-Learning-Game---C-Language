// gameplay.h
// Definicoes dos tipos do jogo e declaracoes de funcoes de gameplay
// BLOCK BREAKER — refatorado para blocos, multiplas bolas e power-ups

#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "../skins/skins.h"

// ─── Variaveis Globais para Escalonamento de Janela ────────────────
extern float    g_scale;
extern Vector2  g_mouseOffset;

// Posicao e delta do mouse ja convertidos para coordenadas virtuais
// (atualizados uma vez por frame em TennisF1.c antes do update)
extern Vector2  g_virtualMouse;
extern Vector2  g_virtualMouseDelta;

// ─── Dimensoes lógicas da janela (resolução base) ──────────────────
#define SCREEN_WIDTH   1280
#define SCREEN_HEIGHT  720

// ─── Zonas de layout ──────────────────────────────────────────────
// HUD_HEIGHT: altura do header superior (score, onda, botao pause)
// BOTTOM_PADDING: espaco livre abaixo da raquete para as skins
#define HUD_HEIGHT     55
#define BOTTOM_PADDING 95

// ─── Grade de blocos ───────────────────────────────────────────────
// Grade 11x6 centralizada: 11*100 + 10*5 = 1150; offset = (1280-1150)/2 = 65
#define BLOCK_ROWS      6
#define BLOCK_COLS      11
#define MAX_BLOCKS      300
#define BLOCK_WIDTH     100
#define BLOCK_HEIGHT    28
#define BLOCK_PADDING   5
#define BLOCK_OFFSET_X  65
#define BLOCK_OFFSET_Y  HUD_HEIGHT  // encostar no header

// ─── Limites dinamicos ─────────────────────────────────────────────
#define MAX_BALLS      10
#define MAX_POWERUPS   20

// ─── Enum de telas ─────────────────────────────────────────────────
typedef enum {
    SCREEN_MENU,
    SCREEN_SKINS,
    SCREEN_BACKGROUNDS, // nova: tela de selecao de cenarios
    SCREEN_GAMEPLAY,
    SCREEN_PAUSE,
    SCREEN_GAMEOVER,
    SCREEN_WIN          // nova: tela de vitoria de onda
} GameScreen;

// ─── Estrutura da bola ─────────────────────────────────────────────
typedef struct {
    float x, y;
    float speedX, speedY;
    float radius;
    Color color;
    bool  active;
} Ball;

// ─── Bloco ─────────────────────────────────────────────────────────
typedef struct {
    float x, y;
    float width, height;
    int   hp;
    int   maxHp;
    bool  active;
    int   flashFrames;   // frames de flash branco ao ser atingido
} Block;

// ─── Power-Up ──────────────────────────────────────────────────────
typedef enum {
    PU_MULTI_BALL,    // +2 bolas extras
    PU_WIDE_PADDLE,   // raquete 50% mais larga por 10s
    PU_FAST_BALL,     // velocidade +30%
    PU_SLOW_BALL,     // velocidade -30%
    PU_PIERCE,        // bolas perfuram blocos
    PU_COUNT
} PowerUpType;

typedef struct {
    float       x, y;
    float       speedY;
    PowerUpType type;
    bool        active;
} PowerUp;

// ─── Cenários ──────────────────────────────────────────────────────
typedef enum {
    BG_PADRAO,
    BG_FARM,
    BG_HELL,
    BG_SPACE,
    BG_COUNT
} BackgroundType;

// ─── Estado completo do jogo ───────────────────────────────────────
typedef struct {
    // Bolas (array dinamico no heap)
    Ball      *balls;
    int        ballCount;   // bolas ativas no momento

    // Raquete
    Paddle    *paddle;

    // Blocos (array estatico, alocado com malloc)
    Block     *blocks;      // MAX_BLOCKS

    // Power-ups em queda
    PowerUp   *powerUps;    // MAX_POWERUPS slots

    // Estado de power-up ativo
    float      widePaddleTimer;  // segundos restantes do efeito
    float      paddleOriginalW;  // largura original antes do efeito
    float      pierceTimer;      // segundos restantes do efeito perfurante

    // Pontuacao e progressao
    int        score;
    int        wave;        // numero da onda atual (comeca em 1)
    int        waveTimer;   // frames para mostrar banner "WAVE X!"
    float      newRowTimer; // timer para adicionar nova fileira
    float      playTime;    // tempo de jogo decorrido

    // Crazy Mode
    bool       isCrazyMode;
    float      crazyModeTimer; // timer para a duracao do "boss"

    // Controle de telas e menus
    GameScreen currentScreen;
    int        menuSelection;
    PaddleSkin skinAtual;
    BackgroundType bgAtual;
} GameState;

// ─── Prototipos ────────────────────────────────────────────────────

// Aloca e inicializa um novo estado de jogo
GameState *InitGame(void);

// Reinicia blocos e bolas para uma nova onda
void InitWave(GameState *game, int wave);

// Reseta o jogo completamente (menu)
void ResetGame(GameState *game);

// Libera toda a memoria alocada
void FreeGame(GameState *game);

// Logica de um frame de gameplay
void UpdateGameplay(GameState *game, Sound sndPong, Sound sndGameOver, Music musCrazyTrain);

#endif // GAMEPLAY_H
