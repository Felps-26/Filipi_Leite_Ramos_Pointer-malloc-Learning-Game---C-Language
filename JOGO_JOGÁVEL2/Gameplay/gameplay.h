// gameplay.h
// Definicoes dos tipos do jogo e declaracoes de funcoes de gameplay

#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../skins/skins.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

// Enum que controla qual tela está ativa
typedef enum {
    SCREEN_MENU,
    SCREEN_SKINS,
    SCREEN_GAMEPLAY,
    SCREEN_PAUSE,
    SCREEN_GAMEOVER
} GameScreen;

// Estrutura da bola
typedef struct {
    float x, y;
    float speedX, speedY;
    float radius;
    Color color;
} Ball;

// Estado completo do jogo
typedef struct {
    Ball      *ball;
    Paddle    *paddle;
    int        score;
    bool       isCrazyMode;
    GameScreen currentScreen;
    int        menuSelection;
    PaddleSkin skinAtual;
} GameState;

// Aloca e inicializa um novo estado de jogo
GameState *InitGame();

// Reseta os valores do jogo para o estado inicial sem realocar memoria
void ResetGame(GameState *game);

// Libera toda a memória alocada
void FreeGame(GameState *game);

// Executa a lógica de um frame de gameplay (física e colisões)
// Retorna true se soar o pong, false caso contrário
void UpdateGameplay(GameState *game, Sound sndPong, Sound sndGameOver, Music musCrazyTrain);

#endif // GAMEPLAY_H
