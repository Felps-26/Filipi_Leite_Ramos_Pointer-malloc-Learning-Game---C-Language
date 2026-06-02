// telas.h
// Declaracoes das funcoes de logica de input e desenho de cada tela

#ifndef TELAS_H
#define TELAS_H

#include "../Gameplay/gameplay.h"

// --- LOGICA DE INPUT (Update) ---

// Processa input e navegação do menu principal
void UpdateTelaMenu(GameState *game);

// Processa input e navegação da tela de skins
void UpdateTelasSkins(GameState *game);

// Processa input do menu de pausa
void UpdateTelaPausa(GameState *game, Music musCrazyTrain);

// Processa input da tela de Game Over
void UpdateTelaGameOver(GameState *game, Music musCrazyTrain);


// --- DESENHO (Draw) ---

// Desenha o menu principal
void DrawTelaMenu(GameState *game);

// Desenha a tela de seleção de skins
void DrawTelasSkins(GameState *game);

// Desenha o jogo em andamento
void DrawTelaGameplay(GameState *game);

// Desenha a sobreposição de pausa sobre o jogo
void DrawTelaPausa(GameState *game);

// Desenha a tela de Game Over
void DrawTelaGameOver(GameState *game);

#endif // TELAS_H
