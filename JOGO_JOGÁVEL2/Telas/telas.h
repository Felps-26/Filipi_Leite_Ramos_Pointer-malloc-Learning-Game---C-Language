// telas.h
// Declaracoes das funcoes de logica de input e desenho de cada tela
// Block Breaker edition

#ifndef TELAS_H
#define TELAS_H

#include "../Gameplay/gameplay.h"

// --- LOGICA DE INPUT (Update) ---

Rectangle GetPauseButtonRect(void);

void UpdateTelaMenu(GameState *game);
void UpdateTelasSkins(GameState *game);
void UpdateTelaBackgrounds(GameState *game);
void UpdateTelaPausa(GameState *game, Music musCrazyTrain);
void UpdateTelaGameOver(GameState *game, Music musCrazyTrain);
void UpdateTelaWin(GameState *game);   // banner de nova onda

// --- DESENHO (Draw) ---

void DrawBackground(BackgroundType bg, float time, bool isCrazyMode);

void DrawTelaMenu(GameState *game);
void DrawTelasSkins(GameState *game);
void DrawTelaBackgrounds(GameState *game);
void DrawTelaGameplay(GameState *game);
void DrawTelaPausa(GameState *game);
void DrawTelaGameOver(GameState *game);

#endif // TELAS_H
