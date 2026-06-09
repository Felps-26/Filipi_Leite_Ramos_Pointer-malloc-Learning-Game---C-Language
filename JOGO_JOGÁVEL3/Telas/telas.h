// telas.h
// Declarações das funções de desenho de menus, HUD, minimapa e botões interativos.
#ifndef TELAS_H
#define TELAS_H

#include "../game.h"

// Desenha um botão UIButton com o estilo Premium da interface
void DrawButton(UIButton botao, Font font, bool enabled);

// Funções de desenho das respectivas telas
void DrawHUD(GameState *game, Font font);
void DrawTelaMenu(GameState *game, Font font, float time);
void DrawTelaControles(GameState *game, Font font);
void DrawTelaGameplay(GameState *game, Font font, bool drawHUD);
void DrawTelaPausa(GameState *game, Font font);
void DrawTelaGameOver(GameState *game, Font font);
void DrawTelaVitoria(GameState *game, Font font);
void DrawTelaSkins(GameState *game, Font font);

// Funções de atualização dos estados de hover e clique dos botões em cada tela
bool UpdateButtonsMenu(GameState *game, Vector2 mouse);
void UpdateButtonsControles(GameState *game, Vector2 mouse);
void UpdateButtonsPause(GameState *game, Vector2 mouse);
void UpdateButtonsGameOver(GameState *game, Vector2 mouse);
void UpdateButtonsVitoria(GameState *game, Vector2 mouse);
void UpdateButtonsSkins(GameState *game, Vector2 mouse);
// Funções de desenho e atualização das telas de seleção de save/load
void DrawTelaSaveSelect(GameState *game, Font font, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3]);
void DrawTelaLoadSelect(GameState *game, Font font, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3]);
int UpdateButtonsSaveSelect(GameState *game, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3]);
int UpdateButtonsLoadSelect(GameState *game, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3]);

#endif // TELAS_H
