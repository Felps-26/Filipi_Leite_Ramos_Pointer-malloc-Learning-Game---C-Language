// telas.c
// Implementacao da logica de input e do desenho de cada tela do jogo

#include "telas.h"
#include <math.h>

void DrawFire(int baseX, int baseY) {
    float t = (float)GetTime() * 10.0f;
    // Fogo principal (vermelho)
    float h1 = 30.0f + sinf(t + baseX) * 10.0f;
    DrawTriangle((Vector2){baseX - 15, baseY}, (Vector2){baseX + 15, baseY}, (Vector2){baseX, baseY - h1}, RED);
    
    // Fogo medio (laranja)
    float h2 = 20.0f + sinf(t * 1.5f + baseX) * 8.0f;
    DrawTriangle((Vector2){baseX - 10, baseY}, (Vector2){baseX + 10, baseY}, (Vector2){baseX, baseY - h2}, ORANGE);
    
    // Fogo pequeno (amarelo)
    float h3 = 10.0f + sinf(t * 2.0f + baseX) * 5.0f;
    DrawTriangle((Vector2){baseX - 5, baseY}, (Vector2){baseX + 5, baseY}, (Vector2){baseX, baseY - h3}, YELLOW);
}

// =============================================================
// LOGICA DE INPUT (Update)
// =============================================================

void UpdateTelaMenu(GameState *game) {
    Vector2   mousePoint = GetMousePosition();
    Rectangle rectJogar  = { SCREEN_WIDTH / 2.0f - MeasureText("JOGAR", 30) / 2.0f,  SCREEN_HEIGHT / 2.0f,        (float)MeasureText("JOGAR", 30),  30.0f };
    Rectangle rectSkins  = { SCREEN_WIDTH / 2.0f - MeasureText("SKINS", 30) / 2.0f,  SCREEN_HEIGHT / 2.0f + 50.f, (float)MeasureText("SKINS", 30),  30.0f };
    Rectangle rectSair   = { SCREEN_WIDTH / 2.0f - MeasureText("SAIR",  30) / 2.0f,  SCREEN_HEIGHT / 2.0f + 100.f,(float)MeasureText("SAIR",  30),  30.0f };

    // Hover com o mouse
    if      (CheckCollisionPointRec(mousePoint, rectJogar)) game->menuSelection = 0;
    else if (CheckCollisionPointRec(mousePoint, rectSkins)) game->menuSelection = 1;
    else if (CheckCollisionPointRec(mousePoint, rectSair))  game->menuSelection = 2;

    // Navegacao com teclado
    if (IsKeyPressed(KEY_UP))   { game->menuSelection--; if (game->menuSelection < 0) game->menuSelection = 2; }
    if (IsKeyPressed(KEY_DOWN)) { game->menuSelection++; if (game->menuSelection > 2) game->menuSelection = 0; }

    // Confirmar selecao (ENTER ou clique)
    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                   (CheckCollisionPointRec(mousePoint, rectJogar) ||
                    CheckCollisionPointRec(mousePoint, rectSkins) ||
                    CheckCollisionPointRec(mousePoint, rectSair));

    if (IsKeyPressed(KEY_ENTER) || clicked) {
        if      (game->menuSelection == 0) { game->currentScreen = SCREEN_GAMEPLAY; }
        else if (game->menuSelection == 1) { game->currentScreen = SCREEN_SKINS; game->menuSelection = (int)game->skinAtual; }
        else if (game->menuSelection == 2) { CloseWindow(); }
    }
}

void UpdateTelasSkins(GameState *game) {
    Vector2 mousePoint = GetMousePosition();

    for (int i = 0; i < SKIN_COUNT; i++) {
        Rectangle rectSkin = { SCREEN_WIDTH / 2.0f - 100.f, SCREEN_HEIGHT / 2.0f - 20.f + i * 40.f, 200.f, 30.f };
        if (CheckCollisionPointRec(mousePoint, rectSkin)) game->menuSelection = i;
    }

    if (IsKeyPressed(KEY_UP))   { game->menuSelection--; if (game->menuSelection < 0)          game->menuSelection = SKIN_COUNT - 1; }
    if (IsKeyPressed(KEY_DOWN)) { game->menuSelection++; if (game->menuSelection >= SKIN_COUNT) game->menuSelection = 0; }

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->skinAtual     = (PaddleSkin)game->menuSelection;
        game->currentScreen = SCREEN_MENU;
        game->menuSelection = 1;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        game->currentScreen = SCREEN_MENU;
        game->menuSelection = 1;
    }
}

void UpdateTelaPausa(GameState *game, Music musCrazyTrain) {
    Vector2   mousePoint   = GetMousePosition();
    Rectangle rectContinuar = { SCREEN_WIDTH / 2.0f - MeasureText("CONTINUAR",     30) / 2.0f, SCREEN_HEIGHT / 2.0f,       (float)MeasureText("CONTINUAR",     30), 30.0f };
    Rectangle rectMenu      = { SCREEN_WIDTH / 2.0f - MeasureText("MENU PRINCIPAL", 30) / 2.0f, SCREEN_HEIGHT / 2.0f + 50.f, (float)MeasureText("MENU PRINCIPAL", 30), 30.0f };

    if      (CheckCollisionPointRec(mousePoint, rectContinuar)) game->menuSelection = 0;
    else if (CheckCollisionPointRec(mousePoint, rectMenu))      game->menuSelection = 1;

    if (IsKeyPressed(KEY_UP))   { game->menuSelection--; if (game->menuSelection < 0) game->menuSelection = 1; }
    if (IsKeyPressed(KEY_DOWN)) { game->menuSelection++; if (game->menuSelection > 1) game->menuSelection = 0; }

    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                   (CheckCollisionPointRec(mousePoint, rectContinuar) ||
                    CheckCollisionPointRec(mousePoint, rectMenu));

    if (IsKeyPressed(KEY_ENTER) || clicked) {
        if (game->menuSelection == 0) {
            game->currentScreen = SCREEN_GAMEPLAY;
        } else if (game->menuSelection == 1) {
            ResetGame(game);
            StopMusicStream(musCrazyTrain);
        }
    }

    // ESC volta direto ao jogo
    if (IsKeyPressed(KEY_ESCAPE)) game->currentScreen = SCREEN_GAMEPLAY;
}

void UpdateTelaGameOver(GameState *game, Music musCrazyTrain) {
    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ResetGame(game);
        game->currentScreen = SCREEN_GAMEPLAY;
        StopMusicStream(musCrazyTrain);
    }
}

// =============================================================
// DESENHO (Draw)
// =============================================================

void DrawTelaMenu(GameState *game) {
    DrawText("TENNIS FOR ONE",
             SCREEN_WIDTH / 2 - MeasureText("TENNIS FOR ONE", 50) / 2,
             SCREEN_HEIGHT / 4, 50, DARKBLUE);

    Color colorJogar = (game->menuSelection == 0) ? RED : DARKGRAY;
    DrawText("JOGAR", SCREEN_WIDTH / 2 - MeasureText("JOGAR", 30) / 2, SCREEN_HEIGHT / 2,       30, colorJogar);

    Color colorSkins = (game->menuSelection == 1) ? RED : DARKGRAY;
    DrawText("SKINS", SCREEN_WIDTH / 2 - MeasureText("SKINS", 30) / 2, SCREEN_HEIGHT / 2 + 50,  30, colorSkins);

    Color colorSair  = (game->menuSelection == 2) ? RED : DARKGRAY;
    DrawText("SAIR",  SCREEN_WIDTH / 2 - MeasureText("SAIR",  30) / 2, SCREEN_HEIGHT / 2 + 100, 30, colorSair);

    int cursorY = (game->menuSelection == 0) ? SCREEN_HEIGHT / 2
                : (game->menuSelection == 1) ? SCREEN_HEIGHT / 2 + 50
                :                              SCREEN_HEIGHT / 2 + 100;
    DrawText(">", SCREEN_WIDTH / 2 - 80, cursorY, 30, RED);

    DrawText("Use o Mouse ou UP/DOWN para escolher, CLICK ou ENTER para confirmar",
             SCREEN_WIDTH / 2 - MeasureText("Use o Mouse ou UP/DOWN para escolher, CLICK ou ENTER para confirmar", 20) / 2,
             SCREEN_HEIGHT - 50, 20, GRAY);
}

void DrawTelasSkins(GameState *game) {
    DrawText("ESCOLHA SUA SKIN",
             SCREEN_WIDTH / 2 - MeasureText("ESCOLHA SUA SKIN", 50) / 2,
             SCREEN_HEIGHT / 4 - 30, 50, DARKBLUE);

    const char *nomesSkins[SKIN_COUNT] = { "PADRAO", "PATO", "DIABO", "PINTINHO", "ALIEN" };

    for (int i = 0; i < SKIN_COUNT; i++) {
        Color cor = (game->menuSelection == i) ? RED : DARKGRAY;
        DrawText(nomesSkins[i],
                 SCREEN_WIDTH / 2 - MeasureText(nomesSkins[i], 30) / 2,
                 SCREEN_HEIGHT / 2 - 20 + i * 40, 30, cor);

        if (game->menuSelection == i) {
            DrawText(">", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20 + i * 40, 30, RED);

            // Previa da skin
            Paddle previewPaddle = { SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT - 140, 120, 15, 0, 0, 0, 0, DARKBLUE };
            DrawPaddleSkin(&previewPaddle, (PaddleSkin)i);
            DrawText("PREVIA",
                     SCREEN_WIDTH / 2 - MeasureText("PREVIA", 20) / 2,
                     SCREEN_HEIGHT - 170, 20, GRAY);
        }
    }

    DrawText("Use o Mouse ou UP/DOWN para escolher, CLICK ou ENTER para confirmar",
             SCREEN_WIDTH / 2 - MeasureText("Use o Mouse ou UP/DOWN para escolher, CLICK ou ENTER para confirmar", 20) / 2,
             SCREEN_HEIGHT - 30, 20, GRAY);
}

void DrawTelaGameplay(GameState *game) {
    DrawPaddleSkin(game->paddle, game->skinAtual);

    if (game->isCrazyMode) {
        // Aura brilhante na bola (fogo)
        DrawCircleV((Vector2){ game->ball->x, game->ball->y }, game->ball->radius * 1.8f, Fade(ORANGE, 0.4f));
        DrawCircleV((Vector2){ game->ball->x, game->ball->y }, game->ball->radius * 2.5f, Fade(RED, 0.2f));
    }

    DrawCircleV((Vector2){ game->ball->x, game->ball->y }, game->ball->radius, game->ball->color);

    DrawText(TextFormat("Score: %d", game->score), 20, 20, 30,
             game->isCrazyMode ? RAYWHITE : DARKGRAY);

    if (game->isCrazyMode) {
        Color titleColor = (sinf((float)GetTime() * 8.0f) > 0) ? RED : ORANGE;
        DrawText("CRAZY TRAIN MODE!",
                 SCREEN_WIDTH / 2 - MeasureText("CRAZY TRAIN MODE!", 30) / 2,
                 20, 30, titleColor);

        // Foguinhos no topo perto do texto
        int textX = SCREEN_WIDTH / 2 - MeasureText("CRAZY TRAIN MODE!", 30) / 2;
        DrawFire(textX - 30, 45); 
        DrawFire(textX + MeasureText("CRAZY TRAIN MODE!", 30) + 30, 45); 

        // Foguinhos em toda a base da tela (chao em chamas)
        for (int i = 0; i <= SCREEN_WIDTH; i += 35) {
            DrawFire(i, SCREEN_HEIGHT);
        }
    }
}

void DrawTelaPausa(GameState *game) {
    // Desenha o jogo congelado ao fundo
    DrawTelaGameplay(game);

    // Overlay escuro semitransparente
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));

    DrawText("JOGO PAUSADO",
             SCREEN_WIDTH / 2 - MeasureText("JOGO PAUSADO", 50) / 2,
             SCREEN_HEIGHT / 4, 50, RAYWHITE);

    Color colorContinuar = (game->menuSelection == 0) ? RED : LIGHTGRAY;
    DrawText("CONTINUAR", SCREEN_WIDTH / 2 - MeasureText("CONTINUAR", 30) / 2,
             SCREEN_HEIGHT / 2, 30, colorContinuar);

    Color colorMenu = (game->menuSelection == 1) ? RED : LIGHTGRAY;
    DrawText("MENU PRINCIPAL", SCREEN_WIDTH / 2 - MeasureText("MENU PRINCIPAL", 30) / 2,
             SCREEN_HEIGHT / 2 + 50, 30, colorMenu);

    int cursorY = (game->menuSelection == 0) ? SCREEN_HEIGHT / 2 : SCREEN_HEIGHT / 2 + 50;
    DrawText(">", SCREEN_WIDTH / 2 - MeasureText("MENU PRINCIPAL", 30) / 2 - 40, cursorY, 30, RED);
}

void DrawTelaGameOver(GameState *game) {
    DrawText("GAME OVER!",
             SCREEN_WIDTH / 2 - MeasureText("GAME OVER!", 40) / 2,
             SCREEN_HEIGHT / 2 - 40, 40, RED);
    DrawText(TextFormat("Pontuacao Final: %d", game->score),
             SCREEN_WIDTH / 2 - MeasureText(TextFormat("Pontuacao Final: %d", game->score), 20) / 2,
             SCREEN_HEIGHT / 2 + 10, 20, DARKGRAY);
    DrawText("Pressione ENTER para Reiniciar",
             SCREEN_WIDTH / 2 - MeasureText("Pressione ENTER para Reiniciar", 20) / 2,
             SCREEN_HEIGHT / 2 + 40, 20, GRAY);
}
