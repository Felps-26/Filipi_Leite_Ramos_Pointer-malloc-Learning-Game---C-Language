// TennisF1.c
// Ponto de entrada do Block Breaker
// Logica separada em:
//   Gameplay/gameplay.h+c  -> fisica, blocos, power-ups, bolas
//   Telas/telas.h+c        -> update e draw de cada tela
//   skins/skins.h+c        -> visuais da raquete

#include "Gameplay/gameplay.h"
#include "Telas/telas.h"

// Globais para escalonamento
float    g_scale             = 1.0f;
Vector2  g_mouseOffset       = { 0.0f, 0.0f };
Vector2  g_virtualMouse      = { 0.0f, 0.0f };
Vector2  g_virtualMouseDelta = { 0.0f, 0.0f };
Font     g_mainFont;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "BlockStrike - Bem vindo!");
    SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Adiciona suporte a tela cheia adaptada ao monitor do usuario
    int monitor = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
    ToggleFullscreen();
    
    SetTargetFPS(60);
    SetExitKey(0); // Desabilita ESC para fechar a janela (usamos no menu)

    // Carrega fonte bold de qualidade para renderizar em todos os tamanhos
    g_mainFont = LoadFontEx("Assets/font.ttf", 128, 0, 0);
    SetTextureFilter(g_mainFont.texture, TEXTURE_FILTER_BILINEAR);

    InitAudioDevice();
    Sound sndPong       = LoadSound("Assets/pong.mp3");
    Sound sndGameOver   = LoadSound("Assets/gameover.mp3");
    Music musCrazyTrain = LoadMusicStream("Assets/Crazy_Train.mp3");

    // Estado do jogo alocado inteiramente na heap
    GameState *game = InitGame();

    // Render target para o escalonamento em letterbox
    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    while (!WindowShouldClose()) {
        // Atualiza logica de escalonamento para se adaptar à tela e ao resize
        float scaleX = (float)GetScreenWidth() / SCREEN_WIDTH;
        float scaleY = (float)GetScreenHeight() / SCREEN_HEIGHT;
        g_scale = (scaleX < scaleY) ? scaleX : scaleY;
        g_mouseOffset.x = (GetScreenWidth() - (SCREEN_WIDTH * g_scale)) * 0.5f;
        g_mouseOffset.y = (GetScreenHeight() - (SCREEN_HEIGHT * g_scale)) * 0.5f;

        // Calcula posicao e delta do mouse em coordenadas virtuais (uma vez por frame)
        Vector2 rawMouse = GetMousePosition();
        g_virtualMouse.x = (rawMouse.x - g_mouseOffset.x) / g_scale;
        g_virtualMouse.y = (rawMouse.y - g_mouseOffset.y) / g_scale;
        Vector2 rawDelta = GetMouseDelta();
        g_virtualMouseDelta.x = rawDelta.x / g_scale;
        g_virtualMouseDelta.y = rawDelta.y / g_scale;

        // Atualiza stream da musica do Crazy Mode
        if (game->isCrazyMode) UpdateMusicStream(musCrazyTrain);

        // ── UPDATE ─────────────────────────────────────────────────
        switch (game->currentScreen) {
            case SCREEN_MENU:
                UpdateTelaMenu(game);
                break;

            case SCREEN_SKINS:
                UpdateTelasSkins(game);
                break;

            case SCREEN_BACKGROUNDS:
                UpdateTelaBackgrounds(game);
                break;

            case SCREEN_GAMEPLAY:
                if (IsKeyPressed(KEY_ESCAPE) || 
                   (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(g_virtualMouse, GetPauseButtonRect()))) {
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

            case SCREEN_WIN:
                // Banner de onda controlado internamente pelo waveTimer
                break;
        }

        // ── DRAW ───────────────────────────────────────────────────
        BeginTextureMode(target);
        ClearBackground(BLACK);

        // Desenha o background universal baseado no bgAtual
        if (game->currentScreen != SCREEN_GAMEOVER) {
            DrawBackground(game->bgAtual, (float)GetTime(), (game->isCrazyMode && game->currentScreen == SCREEN_GAMEPLAY));
        }

        switch (game->currentScreen) {
            case SCREEN_MENU:         DrawTelaMenu(game);         break;
            case SCREEN_SKINS:        DrawTelasSkins(game);       break;
            case SCREEN_BACKGROUNDS:  DrawTelaBackgrounds(game);  break;
            case SCREEN_GAMEPLAY:     DrawTelaGameplay(game);     break;
            case SCREEN_PAUSE:        DrawTelaPausa(game);        break;
            case SCREEN_GAMEOVER:     DrawTelaGameOver(game);     break;
            case SCREEN_WIN:          DrawTelaGameplay(game);     break; // gameplay congelado com banner
        }

        EndTextureMode();

        // Joga a textura renderizada para a tela física
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture, 
                       (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                       (Rectangle){ g_mouseOffset.x, g_mouseOffset.y, SCREEN_WIDTH * g_scale, SCREEN_HEIGHT * g_scale },
                       (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    // Limpeza
    UnloadRenderTexture(target);
    FreeGame(game);
    UnloadSound(sndPong);
    UnloadSound(sndGameOver);
    UnloadMusicStream(musCrazyTrain);
    UnloadFont(g_mainFont);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
