// rpg.c
// Ponto de entrada principal do RPG modular.
#include "game.h"
#include "Gameplay/gameplay.h"
#include "Telas/telas.h"
#include <stdlib.h>
#include <stdio.h>

// Globais para escalonamento da janela física e virtual (Letterbox)
float g_scale = 1.0f;
Vector2 g_mouseOffset = { 0.0f, 0.0f };
Vector2 g_virtualMouse = { 0.0f, 0.0f };
Font g_gameFont;

int main(void)
{
    // Habilita janela redimensionável e sincronização vertical (VSync)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Void Venture - RPG");
    SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTargetFPS(60);
    SetExitKey(0); // Desativa ESC fechar direto para podermos usar como pause/voltar

    // Carrega fonte bold de qualidade ou usa fonte padrão da Raylib
    g_gameFont = LoadFontEx("Assets/font.ttf", 96, 0, 0);
    if (g_gameFont.texture.id == 0)
    {
        // Se a pasta Assets ou a fonte não existirem localmente, cai no fallback seguro
        g_gameFont = GetFontDefault();
    }
    else
    {
        SetTextureFilter(g_gameFont.texture, TEXTURE_FILTER_BILINEAR);
    }

    // Inicializa o estado global do jogo
    GameState game;
    game.currentScreen = SCREEN_MENU;

    // Variaveis de controle dos slots de save, texturas e screenshots
    Texture2D slotTextures[3] = { 0 };
    bool slotTexturesLoaded[3] = { false };
    GameScreen previousScreen = SCREEN_MENU;
    GameScreen loadSelectBackScreen = SCREEN_MENU;
    Image screenshotTemp = { 0 };
    bool hasScreenshotTemp = false;

    // Inicialização do Áudio e Carregamento do Tema (DeepVoid.mp3)
    InitAudioDevice();
    Music musicA = LoadMusicStream("Assets/Musica/DeepVoid.mp3");
    Music musicB = LoadMusicStream("Assets/Musica/DeepVoid.mp3");
    bool streamAPlaying = true;
    bool crossfadeActive = false;
    bool musicLoaded = (musicA.frameCount > 0) && (musicB.frameCount > 0);

    if (musicLoaded)
    {
        musicA.looping = false;
        musicB.looping = false;
        PlayMusicStream(musicA);
        SetMusicVolume(musicA, 1.0f);
        SetMusicVolume(musicB, 0.0f);
    }

    // Render target para o escalonamento adaptativo (Letterbox virtual)
    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    // Loop de jogo principal
    while (!WindowShouldClose())
    {
        // Atualiza a música de fundo com Crossfade de 12 segundos
        if (musicLoaded)
        {
            if (streamAPlaying)
            {
                UpdateMusicStream(musicA);
                float time = GetMusicTimePlayed(musicA);
                float length = GetMusicTimeLength(musicA);
                
                if (length > 12.0f && time >= (length - 12.0f))
                {
                    if (!crossfadeActive)
                    {
                        PlayMusicStream(musicB);
                        crossfadeActive = true;
                    }
                    UpdateMusicStream(musicB);
                    float fadeProgress = (time - (length - 12.0f)) / 12.0f;
                    if (fadeProgress > 1.0f) fadeProgress = 1.0f;
                    if (fadeProgress < 0.0f) fadeProgress = 0.0f;
                    
                    SetMusicVolume(musicA, 1.0f - fadeProgress);
                    SetMusicVolume(musicB, fadeProgress);
                }
                else
                {
                    SetMusicVolume(musicA, 1.0f);
                    SetMusicVolume(musicB, 0.0f);
                    crossfadeActive = false;
                }
                
                if (time >= length - 0.1f || !IsMusicStreamPlaying(musicA))
                {
                    StopMusicStream(musicA);
                    streamAPlaying = false;
                    crossfadeActive = false;
                    SetMusicVolume(musicB, 1.0f);
                }
            }
            else
            {
                UpdateMusicStream(musicB);
                float time = GetMusicTimePlayed(musicB);
                float length = GetMusicTimeLength(musicB);
                
                if (length > 12.0f && time >= (length - 12.0f))
                {
                    if (!crossfadeActive)
                    {
                        PlayMusicStream(musicA);
                        crossfadeActive = true;
                    }
                    UpdateMusicStream(musicA);
                    float fadeProgress = (time - (length - 12.0f)) / 12.0f;
                    if (fadeProgress > 1.0f) fadeProgress = 1.0f;
                    if (fadeProgress < 0.0f) fadeProgress = 0.0f;
                    
                    SetMusicVolume(musicB, 1.0f - fadeProgress);
                    SetMusicVolume(musicA, fadeProgress);
                }
                else
                {
                    SetMusicVolume(musicB, 1.0f);
                    SetMusicVolume(musicA, 0.0f);
                    crossfadeActive = false;
                }
                
                if (time >= length - 0.1f || !IsMusicStreamPlaying(musicB))
                {
                    StopMusicStream(musicB);
                    streamAPlaying = true;
                    crossfadeActive = false;
                    SetMusicVolume(musicA, 1.0f);
                }
            }
        }

        // --------------------------------------------------------------------
        // A. CÁLCULO DE ESCALONAMENTO E COORDENADAS VIRTUAIS
        // --------------------------------------------------------------------
        float scaleX = (float)GetScreenWidth() / SCREEN_WIDTH;
        float scaleY = (float)GetScreenHeight() / SCREEN_HEIGHT;
        g_scale = (scaleX < scaleY) ? scaleX : scaleY;
        
        g_mouseOffset.x = (GetScreenWidth() - (SCREEN_WIDTH * g_scale)) * 0.5f;
        g_mouseOffset.y = (GetScreenHeight() - (SCREEN_HEIGHT * g_scale)) * 0.5f;

        Vector2 rawMouse = GetMousePosition();
        g_virtualMouse.x = (rawMouse.x - g_mouseOffset.x) / g_scale;
        g_virtualMouse.y = (rawMouse.y - g_mouseOffset.y) / g_scale;

        // --------------------------------------------------------------------
        // B. ATUALIZAÇÃO DA LÓGICA CONFORME A TELA ATUAL
        // --------------------------------------------------------------------
        switch (game.currentScreen)
        {
            case SCREEN_MENU:
                if (UpdateButtonsMenu(&game, g_virtualMouse))
                {
                    // Se clicou no botão de SAIR, encerra o loop principal
                    goto finalizacao;
                }
                break;

            case SCREEN_CONTROLS:
                UpdateButtonsControles(&game, g_virtualMouse);
                break;

            case SCREEN_GAMEPLAY:
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    game.currentScreen = SCREEN_PAUSE;
                }
                else if (IsKeyPressed(KEY_F5))
                {
                    // Garante renderização da gameplay limpa antes de capturar
                    BeginTextureMode(target);
                    ClearBackground(BLACK);
                    DrawTelaGameplay(&game, g_gameFont, false);
                    EndTextureMode();

                    // Quicksave para o Slot 1 (Padrão)
                    SalvarJogoSlot(&game, 1);
                    
                    // Captura e exporta screenshot limpa
                    Image quicksaveImg = LoadImageFromTexture(target.texture);
                    ImageFlipVertical(&quicksaveImg);
                    ImageResize(&quicksaveImg, 280, 158);
                    ExportImage(quicksaveImg, "Saves/screenshot_slot_1.png");
                    UnloadImage(quicksaveImg);
                    
                    game.saveLoaded = true;
                    game.timeElapsed = 0.0f;
                }
                else if (IsKeyPressed(KEY_F9))
                {
                    // Quickload do Slot 1 se existir
                    if (FileExists("Saves/save_slot_1.txt"))
                    {
                        CarregarJogoSlot(&game, 1);
                        game.saveLoaded = true;
                        game.timeElapsed = 0.0f;
                    }
                }
                else
                {
                    UpdateGameplay(&game, GetFrameTime());
                }
                break;

            case SCREEN_PAUSE:
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    game.currentScreen = SCREEN_GAMEPLAY;
                }
                else
                {
                    UpdateButtonsPause(&game, g_virtualMouse);
                }
                break;

            case SCREEN_SAVE_SELECT:
                {
                    int slotSelected = UpdateButtonsSaveSelect(&game, g_virtualMouse, slotTextures, slotTexturesLoaded);
                    if (slotSelected > 0)
                    {
                        SalvarJogoSlot(&game, slotSelected);
                        if (hasScreenshotTemp)
                        {
                            char path[64];
                            sprintf(path, "Saves/screenshot_slot_%d.png", slotSelected);
                            ExportImage(screenshotTemp, path);
                        }
                        game.currentScreen = SCREEN_GAMEPLAY;
                        game.saveLoaded = true;
                        game.timeElapsed = 0.0f;
                    }
                    else if (slotSelected == -1)
                    {
                        game.currentScreen = SCREEN_PAUSE;
                    }
                }
                break;

            case SCREEN_LOAD_SELECT:
                {
                    int slotSelected = UpdateButtonsLoadSelect(&game, g_virtualMouse, slotTextures, slotTexturesLoaded);
                    if (slotSelected > 0)
                    {
                        CarregarJogoSlot(&game, slotSelected);
                        game.currentScreen = SCREEN_GAMEPLAY;
                        game.saveLoaded = true;
                        game.timeElapsed = 0.0f;
                    }
                    else if (slotSelected == -1)
                    {
                        game.currentScreen = loadSelectBackScreen;
                    }
                }
                break;

            case SCREEN_GAMEOVER:
                UpdateButtonsGameOver(&game, g_virtualMouse);
                break;

            case SCREEN_VICTORY:
                UpdateButtonsVitoria(&game, g_virtualMouse);
                break;
        }

        // --------------------------------------------------------------------
        // GESTÃO DE TRANSIÇÕES DE TELA (CAPTURA DE SCREENSHOTS E LOAD DE TEXTURAS)
        // Executado após a atualização para que transições no mesmo frame sejam detectadas
        // --------------------------------------------------------------------
        if (game.currentScreen != previousScreen)
        {
            // Se saiu de uma tela de slots ou de pausa, limpa a screenshot/texturas
            if (previousScreen == SCREEN_SAVE_SELECT || previousScreen == SCREEN_LOAD_SELECT || previousScreen == SCREEN_PAUSE)
            {
                // Limpa texturas dos slots se saiu deles
                if (previousScreen == SCREEN_SAVE_SELECT || previousScreen == SCREEN_LOAD_SELECT)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        if (slotTexturesLoaded[i])
                        {
                            UnloadTexture(slotTextures[i]);
                            slotTextures[i] = (Texture2D){ 0 };
                            slotTexturesLoaded[i] = false;
                        }
                    }
                }
                
                // Limpa o screenshot temporário se não estamos salvando e não estamos na pausa
                if (game.currentScreen != SCREEN_SAVE_SELECT && game.currentScreen != SCREEN_PAUSE)
                {
                    if (hasScreenshotTemp)
                    {
                        UnloadImage(screenshotTemp);
                        hasScreenshotTemp = false;
                    }
                }
            }

            // Entrando na tela de salvamento: garante screenshot e carrega metadados/texturas
            if (game.currentScreen == SCREEN_SAVE_SELECT)
            {
                if (hasScreenshotTemp)
                {
                    UnloadImage(screenshotTemp);
                    hasScreenshotTemp = false;
                }

                // Renderiza a jogabilidade limpa (sem overlays) para a render target
                BeginTextureMode(target);
                ClearBackground(BLACK);
                DrawTelaGameplay(&game, g_gameFont, false);
                EndTextureMode();

                screenshotTemp = LoadImageFromTexture(target.texture);
                ImageFlipVertical(&screenshotTemp);
                ImageResize(&screenshotTemp, 280, 158);
                hasScreenshotTemp = true;

                for (int i = 0; i < 3; i++)
                {
                    char path[64];
                    sprintf(path, "Saves/screenshot_slot_%d.png", i + 1);
                    if (FileExists(path))
                    {
                        slotTextures[i] = LoadTexture(path);
                        slotTexturesLoaded[i] = true;
                    }
                    else
                    {
                        slotTexturesLoaded[i] = false;
                    }
                }

                // Carrega os metadados mais recentes dos slots
                for (int i = 0; i < 3; i++)
                {
                    game.slotsMeta[i] = CarregarMetadadosSlot(i + 1);
                }
            }

            // Entrando na tela de carregamento: carrega metadados/texturas
            if (game.currentScreen == SCREEN_LOAD_SELECT)
            {
                loadSelectBackScreen = previousScreen;

                for (int i = 0; i < 3; i++)
                {
                    char path[64];
                    sprintf(path, "Saves/screenshot_slot_%d.png", i + 1);
                    if (FileExists(path))
                    {
                        slotTextures[i] = LoadTexture(path);
                        slotTexturesLoaded[i] = true;
                    }
                    else
                    {
                        slotTexturesLoaded[i] = false;
                    }
                }

                // Carrega os metadados mais recentes dos slots
                for (int i = 0; i < 3; i++)
                {
                    game.slotsMeta[i] = CarregarMetadadosSlot(i + 1);
                }
            }
        }

        previousScreen = game.currentScreen;

        // --------------------------------------------------------------------
        // C. RENDERIZAÇÃO NA TEXTURA VIRTUAL (1280x720)
        // --------------------------------------------------------------------
        BeginTextureMode(target);
        ClearBackground(BLACK);

        switch (game.currentScreen)
        {
            case SCREEN_MENU:
                DrawTelaMenu(&game, g_gameFont, (float)GetTime());
                break;

            case SCREEN_CONTROLS:
                DrawTelaControles(&game, g_gameFont);
                break;

            case SCREEN_GAMEPLAY:
                DrawTelaGameplay(&game, g_gameFont, true);
                break;

            case SCREEN_PAUSE:
                // Desenha a jogabilidade pausada no fundo
                DrawTelaGameplay(&game, g_gameFont, false);
                DrawTelaPausa(&game, g_gameFont);
                break;

            case SCREEN_SAVE_SELECT:
                DrawTelaGameplay(&game, g_gameFont, false);
                DrawTelaSaveSelect(&game, g_gameFont, g_virtualMouse, slotTextures, slotTexturesLoaded);
                break;

            case SCREEN_LOAD_SELECT:
                if (loadSelectBackScreen == SCREEN_PAUSE)
                {
                    DrawTelaGameplay(&game, g_gameFont, false);
                }
                DrawTelaLoadSelect(&game, g_gameFont, g_virtualMouse, slotTextures, slotTexturesLoaded);
                break;

            case SCREEN_GAMEOVER:
                DrawTelaGameOver(&game, g_gameFont);
                break;

            case SCREEN_VICTORY:
                DrawTelaVitoria(&game, g_gameFont);
                break;
        }

        EndTextureMode();

        // --------------------------------------------------------------------
        // D. DESENHA A TEXTURA REDIMENSIONADA NA TELA FÍSICA
        // --------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        // Renderiza a textura virtual centralizada na tela com letterbox
        DrawTexturePro(
            target.texture,
            (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
            (Rectangle){ g_mouseOffset.x, g_mouseOffset.y, SCREEN_WIDTH * g_scale, SCREEN_HEIGHT * g_scale },
            (Vector2){ 0, 0 }, 
            0.0f, 
            WHITE
        );

        EndDrawing();
    }

finalizacao:
    // Limpeza e encerramento de áudio e texturas
    if (musicLoaded)
    {
        UnloadMusicStream(musicA);
        UnloadMusicStream(musicB);
    }
    CloseAudioDevice();

    UnloadRenderTexture(target);
    if (g_gameFont.texture.id != GetFontDefault().texture.id)
    {
        UnloadFont(g_gameFont);
    }
    CloseWindow();

    return 0;
}
