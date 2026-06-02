// TennisF1.c
// Ponto de entrada principal do jogo Tennis For One
// Toda a logica foi separada em:
//   Gameplay/gameplay.h+c  -> fisica, inicializacao e liberacao
//   Telas/telas.h+c        -> update e draw de cada tela
//   skins/skins.h+c        -> visuais da raquete

#include "Gameplay/gameplay.h"
#include "Telas/telas.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tennis For One - Aprendendo Malloc");
    SetTargetFPS(60);
    SetExitKey(0); // Desabilita o ESC para fechar a janela

    // Carrega recursos de audio
    InitAudioDevice();
    Sound sndPong      = LoadSound("Assets/pong.mp3");
    Sound sndGameOver  = LoadSound("Assets/gameover.mp3");
    Music musCrazyTrain = LoadMusicStream("Assets/Crazy_Train.mp3");

    // Inicializa o estado do jogo na heap
    GameState *game = InitGame();

    while (!WindowShouldClose()) {

        // Atualiza o stream de musica se o Crazy Mode estiver ativo
        if (game->isCrazyMode) UpdateMusicStream(musCrazyTrain);

        // ---- UPDATE (logica e input por tela) ----
        switch (game->currentScreen) {
            case SCREEN_MENU:
                UpdateTelaMenu(game);
                break;

            case SCREEN_SKINS:
                UpdateTelasSkins(game);
                break;

            case SCREEN_GAMEPLAY:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    game->currentScreen = SCREEN_PAUSE;
                    game->menuSelection = 0;
                } else {
                    UpdateGameplay(game, sndPong, sndGameOver, musCrazyTrain);
                }
                break;

            case SCREEN_PAUSE:
                UpdateTelaPausa(game, musCrazyTrain);
                break;

            case SCREEN_GAMEOVER:
                UpdateTelaGameOver(game, musCrazyTrain);
                break;
        }

        // ---- DRAW ----
        BeginDrawing();

        // Fundo: cor animada no Crazy Mode, branca nas demais telas
        if (game->isCrazyMode && game->currentScreen == SCREEN_GAMEPLAY) {
            float pulse = (sinf((float)GetTime() * 5.0f) + 1.0f) / 2.0f;
            Color bgColor = (Color){ (unsigned char)(20 + pulse * 40), 0, 0, 255 }; // Escuro, oscilando o vermelho
            ClearBackground(bgColor);
        } else {
            ClearBackground(RAYWHITE);
        }

        switch (game->currentScreen) {
            case SCREEN_MENU:     DrawTelaMenu(game);     break;
            case SCREEN_SKINS:    DrawTelasSkins(game);   break;
            case SCREEN_GAMEPLAY: DrawTelaGameplay(game); break;
            case SCREEN_PAUSE:    DrawTelaPausa(game);    break;
            case SCREEN_GAMEOVER: DrawTelaGameOver(game); break;
        }

        EndDrawing();
    }

    // Limpeza de memoria e recursos
    FreeGame(game);
    UnloadSound(sndPong);
    UnloadSound(sndGameOver);
    UnloadMusicStream(musCrazyTrain);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
