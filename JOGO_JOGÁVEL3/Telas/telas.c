// telas.c
// Implementação do HUD, minimapa, efeitos visuais e menus interativos.
#include "telas.h"
#include "../Gameplay/gameplay.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

extern Vector2 g_virtualMouse;

static void DrawSciFiBox(Rectangle r, Color col);

// ============================================================================
// DEFINIÇÃO DOS BOTÕES DAS TELAS (GLOBAIS DA UI)
// ============================================================================
static UIButton menuButtons[] = {
    { { 490, 275, 300, 48 }, "NOVO JOGO", false, false },
    { { 490, 338, 300, 48 }, "CARREGAR JOGO", false, false },
    { { 490, 401, 300, 48 }, "CONTROLES", false, false },
    { { 490, 464, 300, 48 }, "SAIR", false, false }
};

static UIButton pauseButtons[] = {
    { { 490, 220, 300, 50 }, "VOLTAR AO JOGO", false, false },
    { { 490, 290, 300, 50 }, "SALVAR PROGRESSO", false, false },
    { { 490, 360, 300, 50 }, "CARREGAR ANTERIOR", false, false },
    { { 490, 430, 300, 50 }, "MENU PRINCIPAL", false, false }
};

static UIButton controlsButton = { { 490, 580, 300, 50 }, "VOLTAR", false, false };

static UIButton gameOverButtons[] = {
    { { 490, 390, 300, 50 }, "TENTAR NOVAMENTE", false, false },
    { { 490, 460, 300, 50 }, "MENU PRINCIPAL", false, false }
};

static UIButton victoryButtons[] = {
    { { 490, 390, 300, 50 }, "NOVA JORNADA", false, false },
    { { 490, 460, 300, 50 }, "MENU PRINCIPAL", false, false }
};

// ============================================================================
// AUXILIAR: DESENHA BOTÃO ESTILIZADO (GLASSMORPHISM + GRADIENTE DE HOVER)
// ============================================================================
void DrawButton(UIButton botao, Font font, bool enabled)
{
    // Cores de base
    Color corFundo = Fade((Color){ 26, 21, 44, 255 }, 0.75f);
    Color corBorda = (Color){ 104, 76, 172, 255 };
    Color corTexto = WHITE;

    if (!enabled)
    {
        corFundo = Fade((Color){ 15, 12, 24, 255 }, 0.5f);
        corBorda = Fade(GRAY, 0.25f);
        corTexto = Fade(GRAY, 0.4f);
    }
    else
    {
        if (botao.hover)
        {
            corFundo = Fade((Color){ 84, 52, 148, 255 }, 0.85f);
            corBorda = (Color){ 0, 229, 255, 255 }; // Cyan brilhante no hover
            corTexto = (Color){ 0, 229, 255, 255 };
        }

        if (botao.clicked)
        {
            corFundo = (Color){ 120, 80, 220, 255 };
            corTexto = WHITE;
        }
    }

    // Desenha fundo e borda arredondados (Premium feel)
    DrawRectangleRounded(botao.bounds, 0.25f, 8, corFundo);
    DrawRectangleRoundedLines(botao.bounds, 0.25f, 8, corBorda);

    // Centralização do texto com fonte personalizada
    int fontSize = 24;
    Vector2 textSize = MeasureTextEx(font, botao.text, (float)fontSize, 1.0f);
    Vector2 textPos = {
        botao.bounds.x + (botao.bounds.width / 2.0f) - (textSize.x / 2.0f),
        botao.bounds.y + (botao.bounds.height / 2.0f) - (textSize.y / 2.0f)
    };

    DrawTextEx(font, botao.text, textPos, (float)fontSize, 1.0f, corTexto);
}

// ============================================================================
// AUXILIAR: HOVER/CLICK GENÉRICO
// ============================================================================
static void UpdateBtnState(UIButton *btn, Vector2 mouse)
{
    btn->hover = CheckCollisionPointRec(mouse, btn->bounds);
    btn->clicked = false;
    if (btn->hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        btn->clicked = true;
    }
}

// ============================================================================
// 1. TELA: MENU PRINCIPAL
// ============================================================================
void DrawTelaMenu(GameState *game, Font font, float time)
{
    // Fundo escuro místico com degradê do void
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                           (Color){ 10, 8, 20, 255 }, (Color){ 20, 12, 36, 255 });

    // Desenha as partículas de fundo (flutuando de baixo para cima)
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (game->particles[i].active)
        {
            DrawCircleV(game->particles[i].position, game->particles[i].size, 
                        Fade(game->particles[i].color, game->particles[i].lifeTime / game->particles[i].maxLifeTime));
        }
    }

    // Painel Central Sci-Fi (Dashboard)
    DrawSciFiBox((Rectangle){ 430, 45, 420, 560 }, (Color){ 0, 229, 255, 255 });

    // Título flutuante dinâmico (com cálculo de seno baseado no tempo)
    float titleOffsetY = sinf(time * 2.0f) * 4.0f;
    const char *titulo = "Void Venture";
    int titleFontSize = 52;
    Vector2 titleSize = MeasureTextEx(font, titulo, (float)titleFontSize, 2.0f);
    
    // Sombra do título
    Vector2 shadowPos = {
        (SCREEN_WIDTH / 2.0f) - (titleSize.x / 2.0f) + 3.0f,
        75.0f + titleOffsetY + 3.0f
    };
    DrawTextEx(font, titulo, shadowPos, (float)titleFontSize, 2.0f, Fade(BLACK, 0.45f));

    // Texto principal
    Vector2 titlePos = {
        (SCREEN_WIDTH / 2.0f) - (titleSize.x / 2.0f),
        75.0f + titleOffsetY
    };
    DrawTextEx(font, titulo, titlePos, (float)titleFontSize, 2.0f, (Color){ 0, 229, 255, 255 });

    // Subtítulo
    const char *sub = "Bem vindo ao vazio!";
    int subSizeVal = 18;
    Vector2 subSize = MeasureTextEx(font, sub, (float)subSizeVal, 1.0f);
    DrawTextEx(font, sub, (Vector2){ (SCREEN_WIDTH / 2.0f) - (subSize.x / 2.0f), 135.0f + titleOffsetY }, 
               (float)subSizeVal, 1.0f, GRAY);

    // Desenha Campo de Texto para o Nome do Jogador (Y = 208, Altura 40)
    Rectangle nameBounds = { 490, 208, 300, 40 };
    bool nameHover = CheckCollisionPointRec(g_virtualMouse, nameBounds);
    
    Color boxBg = Fade((Color){ 14, 10, 26, 255 }, 0.85f);
    Color boxBorder = (Color){ 84, 52, 148, 255 };
    
    if (game->nameInputActive)
    {
        boxBorder = (Color){ 0, 229, 255, 255 }; // Cyan se ativo
    }
    else if (nameHover)
    {
        boxBorder = YELLOW; // Amarelo no hover
    }
    
    DrawRectangleRounded(nameBounds, 0.2f, 6, boxBg);
    DrawRectangleRoundedLines(nameBounds, 0.2f, 6, boxBorder);
    
    // Rótulo acima do campo
    DrawTextEx(font, "Seu Personagem:", (Vector2){ nameBounds.x + 5, nameBounds.y - 18 }, 14.0f, 1.0f, GRAY);
    
    // Desenha o texto do nome ou placeholder
    int fontSize = 20;
    if (game->player.name[0] == '\0')
    {
        if (game->nameInputActive)
        {
            const char *cursorStr = ((int)(time * 2.0f) % 2 == 0) ? "|" : "";
            Vector2 textSz = MeasureTextEx(font, cursorStr, (float)fontSize, 1.0f);
            Vector2 textPos = {
                nameBounds.x + (nameBounds.width / 2.0f) - (textSz.x / 2.0f),
                nameBounds.y + (nameBounds.height / 2.0f) - (textSz.y / 2.0f)
            };
            DrawTextEx(font, cursorStr, textPos, (float)fontSize, 1.0f, WHITE);
        }
        else
        {
            Vector2 textSz = MeasureTextEx(font, "DIGITE SEU NOME...", (float)fontSize, 1.0f);
            Vector2 textPos = {
                nameBounds.x + (nameBounds.width / 2.0f) - (textSz.x / 2.0f),
                nameBounds.y + (nameBounds.height / 2.0f) - (textSz.y / 2.0f)
            };
            DrawTextEx(font, "DIGITE SEU NOME...", textPos, (float)fontSize, 1.0f, Fade(GRAY, 0.5f));
        }
    }
    else
    {
        char nameWithCursor[32];
        if (game->nameInputActive && ((int)(time * 2.0f) % 2 == 0))
        {
            sprintf(nameWithCursor, "%s|", game->player.name);
        }
        else
        {
            sprintf(nameWithCursor, "%s", game->player.name);
        }
        
        Vector2 textSz = MeasureTextEx(font, nameWithCursor, (float)fontSize, 1.0f);
        Vector2 textPos = {
            nameBounds.x + (nameBounds.width / 2.0f) - (textSz.x / 2.0f),
            nameBounds.y + (nameBounds.height / 2.0f) - (textSz.y / 2.0f)
        };
        DrawTextEx(font, nameWithCursor, textPos, (float)fontSize, 1.0f, WHITE);
    }

    // Se existe arquivo de save em qualquer um dos 3 slots, indica que pode carregar
    bool anySaveExists = false;
    for (int i = 1; i <= 3; i++)
    {
        char path[64];
        sprintf(path, "Saves/save_slot_%d.txt", i);
        FILE *fTest = fopen(path, "r");
        if (fTest != NULL)
        {
            fclose(fTest);
            anySaveExists = true;
            break;
        }
    }

    // Desenha Botões do Menu
    for (int i = 0; i < 4; i++)
    {
        DrawButton(menuButtons[i], font, (i == 1) ? anySaveExists : true);
    }

    // Rodapé decorativo no painel
    DrawLineEx((Vector2){ 460, 532 }, (Vector2){ 820, 532 }, 1.0f, Fade((Color){ 0, 229, 255, 255 }, 0.25f));
    const char *verText = "SYSTEM STATUS: ACTIVE v1.0.4";
    Vector2 verSize = MeasureTextEx(font, verText, 12.0f, 1.0f);
    DrawTextEx(font, verText, (Vector2){ 640.0f - verSize.x / 2.0f, 545.0f }, 12.0f, 1.0f, Fade((Color){ 0, 229, 255, 255 }, 0.5f));

    // Rodapé
    DrawTextEx(font, "Pressione ESC para pausar/sair | Desenvolvido com Raylib", (Vector2){ 20, SCREEN_HEIGHT - 35 }, 16.0f, 1.0f, DARKGRAY);
}

bool UpdateButtonsMenu(GameState *game, Vector2 mouse)
{
    // Emite partículas decorativas subindo
    if (GetRandomValue(0, 8) == 0)
    {
        Vector2 pPos = { (float)GetRandomValue(0, SCREEN_WIDTH), SCREEN_HEIGHT + 10.0f };
        Vector2 pVel = { (float)GetRandomValue(-20, 20), (float)GetRandomValue(-60, -25) };
        Color pCol = (GetRandomValue(0, 1) == 0) ? SKYBLUE : VIOLET;
        SpawnParticle(game, pPos, pVel, pCol, (float)GetRandomValue(2, 5), (float)GetRandomValue(3, 6));
    }

    // Atualiza lógica das partículas decorativas
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (game->particles[i].active)
        {
            game->particles[i].position.y += game->particles[i].velocity.y * GetFrameTime();
            game->particles[i].position.x += game->particles[i].velocity.x * GetFrameTime();
            game->particles[i].lifeTime -= GetFrameTime();
            if (game->particles[i].lifeTime <= 0.0f) game->particles[i].active = false;
        }
    }

    // Lógica do Input Box do Nome do Jogador (Y = 208, Altura 40)
    Rectangle nameBounds = { 490, 208, 300, 40 };
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, nameBounds))
        {
            game->nameInputActive = true;
        }
        else
        {
            game->nameInputActive = false;
        }
    }

    // Digitação se ativo
    if (game->nameInputActive)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125) && (strlen(game->player.name) < 15))
            {
                int len = strlen(game->player.name);
                game->player.name[len] = (char)key;
                game->player.name[len + 1] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            int len = strlen(game->player.name);
            if (len > 0)
            {
                game->player.name[len - 1] = '\0';
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        {
            game->nameInputActive = false;
        }
    }

    // Determina se existem arquivos de save
    bool anySaveExists = false;
    for (int i = 1; i <= 3; i++)
    {
        char path[64];
        sprintf(path, "Saves/save_slot_%d.txt", i);
        FILE *fTest = fopen(path, "r");
        if (fTest != NULL)
        {
            fclose(fTest);
            anySaveExists = true;
            break;
        }
    }

    // Evita hover e clique acidentais nos botões do menu durante a digitação
    if (!game->nameInputActive)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == 1 && !anySaveExists)
            {
                menuButtons[i].hover = false;
                menuButtons[i].clicked = false;
            }
            else
            {
                UpdateBtnState(&menuButtons[i], mouse);
            }
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            menuButtons[i].hover = false;
            menuButtons[i].clicked = false;
        }
    }

    // Ações
    if (menuButtons[0].clicked) // NOVO JOGO
    {
        InitGame(game);
        game->currentScreen = SCREEN_GAMEPLAY;
    }
    else if (menuButtons[1].clicked) // CARREGAR JOGO
    {
        // Verifica se algum save existe antes de abrir a tela de slots
        bool anySaveExists = false;
        for (int i = 1; i <= 3; i++)
        {
            char path[64];
            sprintf(path, "Saves/save_slot_%d.txt", i);
            FILE *fTest = fopen(path, "r");
            if (fTest != NULL)
            {
                fclose(fTest);
                anySaveExists = true;
                break;
            }
        }
        
        if (anySaveExists)
        {
            // Carrega os metadados dos slots para a tela de seleção
            for (int i = 0; i < 3; i++)
            {
                game->slotsMeta[i] = CarregarMetadadosSlot(i + 1);
            }
            game->currentScreen = SCREEN_LOAD_SELECT;
        }
    }
    else if (menuButtons[2].clicked) // CONTROLES
    {
        game->currentScreen = SCREEN_CONTROLS;
    }

    return menuButtons[3].clicked; // Retorna true se clicou em SAIR
}

// ============================================================================
// 2. TELA: CONTROLES / CRÉDITOS
// ============================================================================
void DrawTelaControles(GameState *game, Font font)
{
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                           (Color){ 14, 10, 26, 255 }, (Color){ 20, 15, 35, 255 });

    DrawTextEx(font, "CONTROLES & REGRAS", (Vector2){ 420, 60 }, 42.0f, 1.5f, SKYBLUE);

    // Painel de Instruções centralizado
    DrawRectangleRounded((Rectangle){ 240, 140, 800, 400 }, 0.05f, 6, Fade(BLACK, 0.45f));
    DrawRectangleRoundedLines((Rectangle){ 240, 140, 800, 400 }, 0.05f, 6, (Color){ 84, 52, 148, 255 });

    int startY = 170;
    int spacing = 38;
    
    // Atalhos
    DrawTextEx(font, "W, A, S, D ou SETAS", (Vector2){ 280, (float)startY }, 22.0f, 1.0f, YELLOW);
    DrawTextEx(font, "- Mover o Herói pelo campo", (Vector2){ 540, (float)startY }, 22.0f, 1.0f, WHITE);
    
    DrawTextEx(font, "ESPAÇO ou CLIQUE ESQUERDO", (Vector2){ 280, (float)(startY + spacing) }, 22.0f, 1.0f, YELLOW);
    DrawTextEx(font, "- Realizar Ataque Circular (Slash)", (Vector2){ 540, (float)(startY + spacing) }, 22.0f, 1.0f, WHITE);

    DrawTextEx(font, "TECLA ESC", (Vector2){ 280, (float)(startY + spacing * 2) }, 22.0f, 1.0f, YELLOW);
    DrawTextEx(font, "- Pausar partida / Acessar Menu", (Vector2){ 540, (float)(startY + spacing * 2) }, 22.0f, 1.0f, WHITE);

    DrawTextEx(font, "SALVAR & CARREGAR", (Vector2){ 280, (float)(startY + spacing * 3.5f) }, 22.0f, 1.0f, YELLOW);
    DrawTextEx(font, "- Acesse as opções de slots pausando a partida (ESC)", (Vector2){ 540, (float)(startY + spacing * 3.5f) }, 22.0f, 1.0f, WHITE);

    // Legenda de Itens
    DrawTextEx(font, "LEGENDA DE POWER-UPS (AMARELOS NO MAPA):", (Vector2){ 280, (float)(startY + spacing * 5.5) }, 20.0f, 1.0f, GOLD);
    DrawTextEx(font, "Cura (+35 HP) | Velocidade (+60%) | Escudo (Invencivel) | Ataque Dobrado (x2 Dano)", (Vector2){ 280, (float)(startY + spacing * 6.5) }, 18.0f, 1.0f, LIGHTGRAY);

    // Botão voltar
    DrawButton(controlsButton, font, true);
}

void UpdateButtonsControles(GameState *game, Vector2 mouse)
{
    UpdateBtnState(&controlsButton, mouse);
    if (controlsButton.clicked)
    {
        game->currentScreen = SCREEN_MENU;
    }
}

// ============================================================================
// 3. TELA: PAUSA (OVERLAY)
// ============================================================================
void DrawTelaPausa(GameState *game, Font font)
{
    // Escurece a tela de fundo da gameplay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.65f));

    // Caixa de diálogo central (Glassmorphism místico do void)
    DrawRectangleRounded((Rectangle){ 420, 130, 440, 400 }, 0.05f, 6, Fade((Color){ 12, 8, 22, 255 }, 0.92f));
    DrawRectangleRoundedLines((Rectangle){ 420, 130, 440, 400 }, 0.05f, 6, (Color){ 84, 52, 148, 255 });

    DrawTextEx(font, "PARTIDA PAUSADA", (Vector2){ 505, 165 }, 28.0f, 1.0f, (Color){ 0, 229, 255, 255 });

    for (int i = 0; i < 4; i++)
    {
        DrawButton(pauseButtons[i], font, true);
    }
}

void UpdateButtonsPause(GameState *game, Vector2 mouse)
{
    for (int i = 0; i < 4; i++)
    {
        UpdateBtnState(&pauseButtons[i], mouse);
    }

    if (pauseButtons[0].clicked) // VOLTAR
    {
        game->currentScreen = SCREEN_GAMEPLAY;
    }
    else if (pauseButtons[1].clicked) // SALVAR
    {
        // Carrega metadados dos slots para a tela de salvamento
        for (int j = 0; j < 3; j++)
        {
            game->slotsMeta[j] = CarregarMetadadosSlot(j + 1);
        }
        game->currentScreen = SCREEN_SAVE_SELECT;
    }
    else if (pauseButtons[2].clicked) // CARREGAR
    {
        // Carrega metadados dos slots para a tela de carregamento
        for (int j = 0; j < 3; j++)
        {
            game->slotsMeta[j] = CarregarMetadadosSlot(j + 1);
        }
        game->currentScreen = SCREEN_LOAD_SELECT;
    }
    else if (pauseButtons[3].clicked) // MENU
    {
        game->currentScreen = SCREEN_MENU;
    }
}

// ============================================================================
// 4. TELA: GAME OVER
// ============================================================================
void DrawTelaGameOver(GameState *game, Font font)
{
    // Fundo vermelho escuro/preto degradê do void
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                           (Color){ 20, 6, 12, 255 }, (Color){ 10, 4, 8, 255 });

    // Título pulsante
    float pulse = 1.0f + sinf((float)GetTime() * 4.0f) * 0.05f;
    const char *txt = "FIM DE JOGO";
    int fSize = 65;
    Vector2 txtSize = MeasureTextEx(font, txt, (float)fSize, 2.0f);
    Vector2 txtPos = {
        (SCREEN_WIDTH / 2.0f) - (txtSize.x * pulse / 2.0f),
        120.0f - (txtSize.y * pulse / 2.0f)
    };
    DrawTextEx(font, txt, txtPos, (float)fSize * pulse, 2.0f, RED);

    // Painel de estatísticas (Estilo SciFi)
    DrawSciFiBox((Rectangle){ 400, 200, 480, 160 }, MAROON);

    DrawTextEx(font, TextFormat("Pontuacao Final: %d", game->player.score), (Vector2){ 440, 225 }, 22.0f, 1.0f, WHITE);
    DrawTextEx(font, TextFormat("Nivel Alcancado: Lvl %d", game->player.level), (Vector2){ 440, 260 }, 22.0f, 1.0f, WHITE);
    DrawTextEx(font, TextFormat("Inimigos Derrotados: %d", game->totalEnemiesKilled), (Vector2){ 440, 295 }, 22.0f, 1.0f, WHITE);

    // Botões
    for (int i = 0; i < 2; i++)
    {
        DrawButton(gameOverButtons[i], font, true);
    }
}

void UpdateButtonsGameOver(GameState *game, Vector2 mouse)
{
    for (int i = 0; i < 2; i++)
    {
        UpdateBtnState(&gameOverButtons[i], mouse);
    }

    if (gameOverButtons[0].clicked) // TENTAR NOVAMENTE
    {
        InitGame(game);
        game->currentScreen = SCREEN_GAMEPLAY;
    }
    else if (gameOverButtons[1].clicked) // MENU
    {
        game->currentScreen = SCREEN_MENU;
    }
}

// ============================================================================
// 5. TELA: VITÓRIA
// ============================================================================
void DrawTelaVitoria(GameState *game, Font font)
{
    // Fundo azul escuro do espaço sideral degradê
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                           (Color){ 10, 16, 36, 255 }, (Color){ 6, 8, 20, 255 });

    // Efeito de estrelas/partículas subindo
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (game->particles[i].active)
        {
            DrawCircleV(game->particles[i].position, game->particles[i].size, 
                        Fade(game->particles[i].color, game->particles[i].lifeTime / game->particles[i].maxLifeTime));
        }
    }

    // Título
    float pulse = 1.0f + sinf((float)GetTime() * 3.5f) * 0.04f;
    const char *txt = "VITÓRIA SUPREMA!";
    int fSize = 65;
    Vector2 txtSize = MeasureTextEx(font, txt, (float)fSize, 2.0f);
    Vector2 txtPos = {
        (SCREEN_WIDTH / 2.0f) - (txtSize.x * pulse / 2.0f),
        120.0f - (txtSize.y * pulse / 2.0f)
    };
    DrawTextEx(font, txt, txtPos, (float)fSize * pulse, 2.0f, GOLD);

    // Parabéns
    const char *congrats = "Voce defendeu o reino dos blocos contra todas as hordas!";
    Vector2 congratsSize = MeasureTextEx(font, congrats, 20.0f, 1.0f);
    DrawTextEx(font, congrats, (Vector2){ (SCREEN_WIDTH / 2.0f) - (congratsSize.x / 2.0f), 160.0f }, 20.0f, 1.0f, SKYBLUE);

    // Painel de estatísticas (Estilo SciFi)
    DrawSciFiBox((Rectangle){ 400, 200, 480, 160 }, GOLD);

    DrawTextEx(font, TextFormat("Pontuacao de Ouro: %d", game->player.score), (Vector2){ 440, 225 }, 22.0f, 1.0f, GOLD);
    DrawTextEx(font, TextFormat("Nivel Final: Lvl %d", game->player.level), (Vector2){ 440, 260 }, 22.0f, 1.0f, WHITE);
    DrawTextEx(font, TextFormat("Total de Inimigos Mortos: %d", game->totalEnemiesKilled), (Vector2){ 440, 295 }, 22.0f, 1.0f, WHITE);

    // Botões
    for (int i = 0; i < 2; i++)
    {
        DrawButton(victoryButtons[i], font, true);
    }
}

void UpdateButtonsVitoria(GameState *game, Vector2 mouse)
{
    // Partículas douradas festivas
    if (GetRandomValue(0, 5) == 0)
    {
        Vector2 pPos = { (float)GetRandomValue(0, SCREEN_WIDTH), SCREEN_HEIGHT + 10.0f };
        Vector2 pVel = { (float)GetRandomValue(-20, 20), (float)GetRandomValue(-80, -35) };
        SpawnParticle(game, pPos, pVel, GOLD, (float)GetRandomValue(2, 5), (float)GetRandomValue(2.5f, 5));
    }

    // Atualiza lógica das partículas festivas
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (game->particles[i].active)
        {
            game->particles[i].position.y += game->particles[i].velocity.y * GetFrameTime();
            game->particles[i].position.x += game->particles[i].velocity.x * GetFrameTime();
            game->particles[i].lifeTime -= GetFrameTime();
            if (game->particles[i].lifeTime <= 0.0f) game->particles[i].active = false;
        }
    }

    for (int i = 0; i < 2; i++)
    {
        UpdateBtnState(&victoryButtons[i], mouse);
    }

    if (victoryButtons[0].clicked) // JOGAR DE NOVO
    {
        InitGame(game);
        game->currentScreen = SCREEN_GAMEPLAY;
    }
    else if (victoryButtons[1].clicked) // MENU
    {
        game->currentScreen = SCREEN_MENU;
    }
}

// ============================================================================
// 6. MINI-MAPA & HUD DA GAMEPLAY
// ============================================================================
// Auxiliary Sci-fi corner brackets container helper
static void DrawSciFiBox(Rectangle r, Color col)
{
    // Deep dark translucent panel background (cockpit glass style)
    DrawRectangleRec(r, Fade((Color){ 8, 6, 16, 255 }, 0.65f));
    
    // Sleek thin border
    DrawRectangleLinesEx(r, 1.0f, Fade(col, 0.25f));
    
    // High-tech corner bracket markings
    float len = 10.0f;
    float thickness = 2.0f;
    // Top-left corner
    DrawLineEx((Vector2){ r.x, r.y }, (Vector2){ r.x + len, r.y }, thickness, col);
    DrawLineEx((Vector2){ r.x, r.y }, (Vector2){ r.x, r.y + len }, thickness, col);
    // Top-right corner
    DrawLineEx((Vector2){ r.x + r.width, r.y }, (Vector2){ r.x + r.width - len, r.y }, thickness, col);
    DrawLineEx((Vector2){ r.x + r.width, r.y }, (Vector2){ r.x + r.width, r.y + len }, thickness, col);
    // Bottom-left corner
    DrawLineEx((Vector2){ r.x, r.y + r.height }, (Vector2){ r.x + len, r.y + r.height }, thickness, col);
    DrawLineEx((Vector2){ r.x, r.y + r.height }, (Vector2){ r.x, r.y + r.height - len }, thickness, col);
    // Bottom-right corner
    DrawLineEx((Vector2){ r.x + r.width, r.y + r.height }, (Vector2){ r.x + r.width - len, r.y + r.height }, thickness, col);
    DrawLineEx((Vector2){ r.x + r.width, r.y + r.height }, (Vector2){ r.x + r.width, r.y + r.height - len }, thickness, col);
}

static void DrawHUD(GameState *game, Font font)
{
    // ------------------------------------------------------------------------
    // A. BARRA DE STATUS DO JOGADOR (HP & XP)
    // ------------------------------------------------------------------------
    // Painel superior esquerdo minimalista estilo cockpit espacial (360x95)
    DrawSciFiBox((Rectangle){ 20, 20, 360, 95 }, (Color){ 0, 229, 255, 255 });

    // Informações básicas (Nome à esquerda, Nível à direita)
    DrawTextEx(font, game->player.name, (Vector2){ 35, 28 }, 18.0f, 1.0f, (Color){ 0, 229, 255, 255 });
    
    char lvlStr[16];
    sprintf(lvlStr, "LV. %d", game->player.level);
    Vector2 lvlSize = MeasureTextEx(font, lvlStr, 14.0f, 1.0f);
    DrawTextEx(font, lvlStr, (Vector2){ 365.0f - lvlSize.x, 30.0f }, 14.0f, 1.0f, GOLD);

    // Barra de HP (Fundo escuro, preenchimento neon laser cyan/red)
    DrawRectangleRounded((Rectangle){ 35, 54, 220, 10 }, 0.5f, 4, (Color){ 45, 10, 15, 255 });
    float hpPercent = (float)game->player.hp / game->player.maxHp;
    if (hpPercent > 0.0f)
    {
        Color hpColor = (hpPercent > 0.45f) ? (Color){ 0, 229, 255, 255 } : (hpPercent > 0.2f) ? ORANGE : RED;
        DrawRectangleRounded((Rectangle){ 35, 54, 220.0f * hpPercent, 10 }, 0.5f, 4, hpColor);
    }
    
    char hpStr[32];
    sprintf(hpStr, "%d/%d HP", game->player.hp, game->player.maxHp);
    Vector2 hpSize = MeasureTextEx(font, hpStr, 13.0f, 1.0f);
    DrawTextEx(font, hpStr, (Vector2){ 365.0f - hpSize.x, 52.0f }, 13.0f, 1.0f, WHITE);

    // Barra de XP (Laser violeta bem fino)
    DrawTextEx(font, "XP", (Vector2){ 35, 73 }, 11.0f, 1.0f, (Color){ 224, 64, 251, 255 });
    DrawRectangleRounded((Rectangle){ 60, 75, 305, 6 }, 0.5f, 4, BLACK);
    float xpPercent = (float)game->player.xp / game->player.xpNeeded;
    if (xpPercent > 0.0f)
    {
        DrawRectangleRounded((Rectangle){ 60, 75, 305.0f * xpPercent, 6 }, 0.5f, 4, (Color){ 224, 64, 251, 255 });
    }

    // ------------------------------------------------------------------------
    // B. PAINEL DE ONDA / HORDA (SUPERIOR CENTRAL - BANNER HOLO EM CAIXA SCI-FI)
    // ------------------------------------------------------------------------
    Rectangle waveBox = { 490, 20, 300, 60 };
    DrawSciFiBox(waveBox, (Color){ 0, 229, 255, 255 });

    const char *waveTxt = TextFormat("ONDA DE ATAQUE: %d / 5", game->wave);
    Vector2 waveTxtSize = MeasureTextEx(font, waveTxt, 16.0f, 1.0f);
    DrawTextEx(font, waveTxt, (Vector2){ 640.0f - waveTxtSize.x / 2.0f, 28.0f }, 16.0f, 1.0f, GOLD);

    const char *remTxt = TextFormat("Inimigos Restantes: %d", game->enemiesRemaining);
    Vector2 remTxtSize = MeasureTextEx(font, remTxt, 13.0f, 1.0f);
    DrawTextEx(font, remTxt, (Vector2){ 640.0f - remTxtSize.x / 2.0f, 48.0f }, 13.0f, 1.0f, WHITE);

    // ------------------------------------------------------------------------
    // C. PONTUAÇÃO (SUPERIOR DIREITO)
    // ------------------------------------------------------------------------
    DrawSciFiBox((Rectangle){ 900, 20, 150, 55 }, (Color){ 0, 229, 255, 255 });
    DrawTextEx(font, "PONTOS", (Vector2){ 915, 27 }, 12.0f, 1.0f, GRAY);
    DrawTextEx(font, TextFormat("%06d", game->player.score), (Vector2){ 915, 42 }, 20.0f, 1.0f, YELLOW);

    // ------------------------------------------------------------------------
    // D. INDICADORES VISUAIS DE BUFFS ATIVOS (ABAIXO DO PAINEL DE STATUS)
    // ------------------------------------------------------------------------
    int buffCount = 0;
    
    // Buff Velocidade
    if (game->player.speedTimer > 0.0f)
    {
        Rectangle rBuff = { 20, 130.0f + (float)buffCount * 32.0f, 190, 26 };
        DrawRectangleRec(rBuff, Fade((Color){ 10, 8, 22, 255 }, 0.75f));
        DrawRectangle((int)rBuff.x, (int)rBuff.y, 4, (int)rBuff.height, YELLOW);
        DrawRectangleLinesEx(rBuff, 1.0f, Fade(YELLOW, 0.25f));
        
        DrawTextEx(font, TextFormat("VELOCIDADE: %.1fs", game->player.speedTimer), 
                   (Vector2){ rBuff.x + 12, rBuff.y + 7 }, 12.0f, 1.0f, YELLOW);
        buffCount++;
    }

    // Buff Escudo
    if (game->player.shieldTimer > 0.0f)
    {
        Rectangle rBuff = { 20, 130.0f + (float)buffCount * 32.0f, 190, 26 };
        DrawRectangleRec(rBuff, Fade((Color){ 10, 8, 22, 255 }, 0.75f));
        DrawRectangle((int)rBuff.x, (int)rBuff.y, 4, (int)rBuff.height, SKYBLUE);
        DrawRectangleLinesEx(rBuff, 1.0f, Fade(SKYBLUE, 0.25f));
        
        DrawTextEx(font, TextFormat("ESCUDO: %.1fs", game->player.shieldTimer), 
                   (Vector2){ rBuff.x + 12, rBuff.y + 7 }, 12.0f, 1.0f, SKYBLUE);
        buffCount++;
    }

    // Buff Dano
    if (game->player.attackBoostTimer > 0.0f)
    {
        Rectangle rBuff = { 20, 130.0f + (float)buffCount * 32.0f, 190, 26 };
        DrawRectangleRec(rBuff, Fade((Color){ 10, 8, 22, 255 }, 0.75f));
        DrawRectangle((int)rBuff.x, (int)rBuff.y, 4, (int)rBuff.height, ORANGE);
        DrawRectangleLinesEx(rBuff, 1.0f, Fade(ORANGE, 0.25f));
        
        DrawTextEx(font, TextFormat("DANO x2: %.1fs", game->player.attackBoostTimer), 
                   (Vector2){ rBuff.x + 12, rBuff.y + 7 }, 12.0f, 1.0f, ORANGE);
        buffCount++;
    }

    // ------------------------------------------------------------------------
    // E. RADAR ESPACIAL DENTRO DO HUD (CIRCULAR, CENTRADO NO PLAYER)
    // ------------------------------------------------------------------------
    Vector2 radarCenter = { 1195.0f, 85.0f };
    float radarRadius = 65.0f;
    float radarRange = 1200.0f; // Alcance do radar local
    
    // Fundo do Radar
    DrawCircleV(radarCenter, radarRadius, Fade((Color){ 10, 8, 22, 255 }, 0.65f));
    
    // Moldura decorativa sci-fi ao redor do radar
    Rectangle radarFrame = { radarCenter.x - radarRadius - 5, radarCenter.y - radarRadius - 5, radarRadius * 2 + 10, radarRadius * 2 + 10 };
    DrawRectangleLinesEx(radarFrame, 1.0f, Fade((Color){ 0, 229, 255, 255 }, 0.3f));
    // Moldura decorativa nos cantos
    float len = 6.0f;
    DrawLineEx((Vector2){ radarFrame.x, radarFrame.y }, (Vector2){ radarFrame.x + len, radarFrame.y }, 1.5f, (Color){ 0, 229, 255, 255 });
    DrawLineEx((Vector2){ radarFrame.x, radarFrame.y }, (Vector2){ radarFrame.x, radarFrame.y + len }, 1.5f, (Color){ 0, 229, 255, 255 });
    DrawLineEx((Vector2){ radarFrame.x + radarFrame.width, radarFrame.y }, (Vector2){ radarFrame.x + radarFrame.width - len, radarFrame.y }, 1.5f, (Color){ 0, 229, 255, 255 });
    DrawLineEx((Vector2){ radarFrame.x + radarFrame.width, radarFrame.y }, (Vector2){ radarFrame.x + radarFrame.width, radarFrame.y + len }, 1.5f, (Color){ 0, 229, 255, 255 });
    DrawLineEx((Vector2){ radarFrame.x, radarFrame.y + radarFrame.height }, (Vector2){ radarFrame.x + len, radarFrame.y + radarFrame.height }, 1.5f, (Color){ 0, 229, 255, 255 });
    DrawLineEx((Vector2){ radarFrame.x, radarFrame.y + radarFrame.height }, (Vector2){ radarFrame.x, radarFrame.y + radarFrame.height - len }, 1.5f, (Color){ 0, 229, 255, 255 });
    DrawLineEx((Vector2){ radarFrame.x + radarFrame.width, radarFrame.y + radarFrame.height }, (Vector2){ radarFrame.x + radarFrame.width - len, radarFrame.y + radarFrame.height }, 1.5f, (Color){ 0, 229, 255, 255 });
    DrawLineEx((Vector2){ radarFrame.x + radarFrame.width, radarFrame.y + radarFrame.height }, (Vector2){ radarFrame.x + radarFrame.width, radarFrame.y + radarFrame.height - len }, 1.5f, (Color){ 0, 229, 255, 255 });

    // Anéis Concêntricos Neon
    DrawCircleLines(radarCenter.x, radarCenter.y, radarRadius, (Color){ 0, 229, 255, 255 });
    DrawCircleLines(radarCenter.x, radarCenter.y, radarRadius * 0.66f, Fade((Color){ 0, 229, 255, 255 }, 0.25f));
    DrawCircleLines(radarCenter.x, radarCenter.y, radarRadius * 0.33f, Fade((Color){ 0, 229, 255, 255 }, 0.15f));
    
    // Retículos do Radar
    DrawLineV((Vector2){ radarCenter.x - radarRadius, radarCenter.y }, (Vector2){ radarCenter.x + radarRadius, radarCenter.y }, Fade((Color){ 0, 229, 255, 255 }, 0.2f));
    DrawLineV((Vector2){ radarCenter.x, radarCenter.y - radarRadius }, (Vector2){ radarCenter.x, radarCenter.y + radarRadius }, Fade((Color){ 0, 229, 255, 255 }, 0.2f));
    
    // Efeito sweep (varredura laser giratória)
    float sweepTime = (float)GetTime() * 3.0f;
    Vector2 sweepEnd = {
        radarCenter.x + cosf(sweepTime) * radarRadius,
        radarCenter.y + sinf(sweepTime) * radarRadius
    };
    DrawLineEx(radarCenter, sweepEnd, 1.5f, Fade((Color){ 0, 229, 255, 255 }, 0.5f));

    // A. Desenha os Power-Ups no radar (Pontos amarelos)
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        if (game->powerUps[i].active)
        {
            Vector2 diff = Vector2Subtract(game->powerUps[i].position, game->player.position);
            float dist = Vector2Length(diff);
            if (dist <= radarRange)
            {
                float scale = radarRadius / radarRange;
                Vector2 dotPos = Vector2Add(radarCenter, Vector2Scale(diff, scale));
                DrawCircleV(dotPos, 2.5f, YELLOW);
            }
        }
    }

    // B. Desenha os Inimigos no radar (Pontos vermelhos/laranjas/boss maior)
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (game->enemies[i].active)
        {
            Vector2 diff = Vector2Subtract(game->enemies[i].position, game->player.position);
            float dist = Vector2Length(diff);
            if (dist <= radarRange)
            {
                float scale = radarRadius / radarRange;
                Vector2 dotPos = Vector2Add(radarCenter, Vector2Scale(diff, scale));
                Color dotCol = (game->enemies[i].type == 2) ? MAROON : (game->enemies[i].state == CHASE) ? RED : ORANGE;
                float dotSize = (game->enemies[i].type == 2) ? 3.5f : 2.0f;
                DrawCircleV(dotPos, dotSize, dotCol);
            }
        }
    }

    // C. Desenha o Jogador no centro do radar (Ponto azul ciano pulsando)
    float pPulse = 3.0f + sinf((float)GetTime() * 6.0f) * 0.8f;
    DrawCircleV(radarCenter, pPulse, SKYBLUE);
    DrawCircleLines(radarCenter.x, radarCenter.y, pPulse + 2.0f, Fade(SKYBLUE, 0.5f));

    // Rótulo do Radar
    DrawTextEx(font, "RADAR LOCAL", (Vector2){ radarCenter.x - 38.0f, radarCenter.y + radarRadius + 8.0f }, 11.0f, 1.0f, GRAY);
    
    // Notificação de salvamento no rodapé
    if (game->saveLoaded)
    {
        DrawRectangleRounded((Rectangle){ 490, 670, 300, 30 }, 0.4f, 4, Fade(GREEN, 0.2f));
        DrawRectangleRoundedLines((Rectangle){ 490, 670, 300, 30 }, 0.4f, 4, GREEN);
        DrawTextEx(font, "JOGO CARREGADO COM SUCESSO!", (Vector2){ 515, 678 }, 14.0f, 1.0f, GREEN);
        
        // Some após 3 segundos
        if (game->timeElapsed > 3.0f) game->saveLoaded = false;
    }
}

// ============================================================================
// DIBUJA A TELA DE GAMEPLAY (MUNDO 2D + HUD + RADAR)
// ============================================================================
void DrawTelaGameplay(GameState *game, Font font, bool drawHUD)
{
    // Limpa tela com tom azul profundo do void
    ClearBackground((Color){ 8, 6, 16, 255 });

    // ------------------------------------------------------------------------
    // 1. DESENHA O MUNDO DO JOGO (AFETADO PELA CÂMERA 2D)
    // ------------------------------------------------------------------------
    BeginMode2D(game->camera);

    // Desenha grid do solo
    for (int x = 0; x <= MAP_WIDTH; x += 160)
    {
        DrawLine(x, 0, x, MAP_HEIGHT, Fade(PURPLE, 0.15f));
    }
    for (int y = 0; y <= MAP_HEIGHT; y += 160)
    {
        DrawLine(0, y, MAP_WIDTH, y, Fade(PURPLE, 0.15f));
    }

    // Desenha bordas de limite do campo de batalha (Thick frame)
    DrawRectangleLinesEx((Rectangle){ 0, 0, MAP_WIDTH, MAP_HEIGHT }, 15.0f, (Color){ 84, 52, 148, 255 });
    
    // Desenha marcadores de perigo fora da borda
    for (int x = 0; x < MAP_WIDTH; x += 300)
    {
        DrawTextEx(font, "WARNING - BATTLEFIELD LIMIT", (Vector2){ (float)(x + 20), 20.0f }, 20.0f, 1.0f, Fade(RED, 0.4f));
        DrawTextEx(font, "WARNING - BATTLEFIELD LIMIT", (Vector2){ (float)(x + 20), (float)(MAP_HEIGHT - 40) }, 20.0f, 1.0f, Fade(RED, 0.4f));
    }

    // A. Desenha os Power-Ups (Quadrados amarelos com efeito glow pulsante)
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        if (game->powerUps[i].active)
        {
            float pulse = 24.0f + sinf(game->powerUps[i].pulseTimer * 6.0f) * 4.0f;
            Vector2 pos = game->powerUps[i].position;

            // Efeito visual de luz ao redor
            DrawCircleV(pos, pulse + 6.0f, Fade(YELLOW, 0.18f));
            DrawRectangleRec((Rectangle){ pos.x - pulse/2.0f, pos.y - pulse/2.0f, pulse, pulse }, YELLOW);
            DrawRectangleLinesEx((Rectangle){ pos.x - pulse/2.0f, pos.y - pulse/2.0f, pulse, pulse }, 2.0f, WHITE);
            
            // Ícone interno descritivo desenhado com cor escura
            const char *itemChar = "P";
            if (game->powerUps[i].type == HP_RECOVERY) itemChar = "H";
            if (game->powerUps[i].type == SPEED_BOOST) itemChar = "S";
            if (game->powerUps[i].type == SHIELD)       itemChar = "D";
            if (game->powerUps[i].type == ATTACK_BOOST) itemChar = "A";
            DrawTextEx(font, itemChar, (Vector2){ pos.x - 4.0f, pos.y - 7.0f }, 14.0f, 1.0f, BLACK);
        }
    }

    // B. Desenha os Inimigos (Quadrados vermelhos/laranjas/roxos com barras de HP)
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (game->enemies[i].active)
        {
            Enemy *enemy = &game->enemies[i];
            
            // Determina tamanho e cor baseados no tipo do inimigo e estado
            float size = 42.0f;
            Color colBase = (enemy->state == CHASE) ? RED : ORANGE;

            if (enemy->type == 1) // Rápido
            {
                size = 32.0f;
                colBase = (enemy->state == CHASE) ? VIOLET : PINK;
            }
            else if (enemy->type == 2) // Elite/Boss
            {
                size = 68.0f;
                colBase = (enemy->state == CHASE) ? MAROON : PURPLE;
            }

            // Quadrado do inimigo
            Rectangle rEnemy = { enemy->position.x - size / 2.0f, enemy->position.y - size / 2.0f, size, size };
            DrawRectangleRec(rEnemy, colBase);
            DrawRectangleLinesEx(rEnemy, 2.5f, BLACK);

            // Olhos simples para caracterizar a entidade inimiga (quadrados pretos direcionados ao herói)
            Vector2 lookDir = Vector2Subtract(game->player.position, enemy->position);
            lookDir = Vector2Normalize(lookDir);
            float eyeOffset = size / 4.0f;
            Vector2 leftEye = { enemy->position.x - eyeOffset + lookDir.x * 4, enemy->position.y - eyeOffset + lookDir.y * 3 };
            Vector2 rightEye = { enemy->position.x + eyeOffset + lookDir.x * 4, enemy->position.y - eyeOffset + lookDir.y * 3 };
            DrawRectangle(leftEye.x - 3, leftEye.y - 3, 6, 6, BLACK);
            DrawRectangle(rightEye.x - 3, rightEye.y - 3, 6, 6, BLACK);

            // Barra de HP individual acima do inimigo
            float barW = size * 1.1f;
            float barH = 6.0f;
            Rectangle rHPBg = { enemy->position.x - barW / 2.0f, enemy->position.y - size/2.0f - 14.0f, barW, barH };
            DrawRectangleRec(rHPBg, Fade(RED, 0.4f));
            
            float enemyHpPercent = (float)enemy->hp / enemy->maxHp;
            if (enemyHpPercent > 0.0f)
            {
                Rectangle rHPFill = { rHPBg.x, rHPBg.y, barW * enemyHpPercent, barH };
                DrawRectangleRec(rHPFill, GREEN);
            }
            DrawRectangleLinesEx(rHPBg, 1.0f, BLACK);
        }
    }

    // C. Desenha o Herói (Quadrado azul estilizado)
    float playerSize = 45.0f;
    Rectangle rPlayer = { game->player.position.x - playerSize / 2.0f, game->player.position.y - playerSize / 2.0f, playerSize, playerSize };
    
    // Cor padrão ou dourada se ataque ativado
    Color pColor = SKYBLUE;
    if (game->player.attackBoostTimer > 0.0f) pColor = GOLD;

    DrawRectangleRec(rPlayer, pColor);
    DrawRectangleLinesEx(rPlayer, 3.0f, WHITE);

    // Olhinhos focados no herói
    Vector2 mouseRaw = GetMousePosition();
    Vector2 virtualMouse; // Converte tela física em virtual
    float scaleX = (float)GetScreenWidth() / SCREEN_WIDTH;
    float scaleY = (float)GetScreenHeight() / SCREEN_HEIGHT;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    float mOffsetX = (GetScreenWidth() - (SCREEN_WIDTH * scale)) * 0.5f;
    float mOffsetY = (GetScreenHeight() - (SCREEN_HEIGHT * scale)) * 0.5f;
    virtualMouse.x = (mouseRaw.x - mOffsetX) / scale;
    virtualMouse.y = (mouseRaw.y - mOffsetY) / scale;

    // Obtém coordenadas de mundo do mouse virtual para os olhos olharem pro mouse
    Vector2 worldMouse = GetScreenToWorld2D(virtualMouse, game->camera);
    Vector2 lookDirHero = Vector2Subtract(worldMouse, game->player.position);
    lookDirHero = Vector2Normalize(lookDirHero);
    
    Vector2 pLeftEye = { game->player.position.x - 10 + lookDirHero.x * 5, game->player.position.y - 10 + lookDirHero.y * 4 };
    Vector2 pRightEye = { game->player.position.x + 10 + lookDirHero.x * 5, game->player.position.y - 10 + lookDirHero.y * 4 };
    DrawRectangle(pLeftEye.x - 4, pLeftEye.y - 4, 8, 8, WHITE);
    DrawRectangle(pRightEye.x - 4, pRightEye.y - 4, 8, 8, WHITE);
    DrawRectangle(pLeftEye.x - 2 + lookDirHero.x * 2, pLeftEye.y - 2 + lookDirHero.y * 2, 4, 4, BLACK);
    DrawRectangle(pRightEye.x - 2 + lookDirHero.x * 2, pRightEye.y - 2 + lookDirHero.y * 2, 4, 4, BLACK);

    // D. Efeito do Escudo (Circulo ciano translúcido ao redor)
    if (game->player.shieldTimer > 0.0f)
    {
        float ringRad = playerSize * 0.95f + sinf((float)GetTime() * 12.0f) * 3.0f;
        DrawCircleLines(game->player.position.x, game->player.position.y, ringRad, Fade(SKYBLUE, 0.85f));
        DrawCircle(game->player.position.x, game->player.position.y, ringRad, Fade(SKYBLUE, 0.12f));
    }

    // E. Desenha a animação de ataque (Slash circular crescendo)
    if (game->slashAnimTimer > 0.0f)
    {
        // Progresso do slash: de 0.0 a 1.0
        float t = 1.0f - (game->slashAnimTimer / 0.22f);
        float currentRadius = t * game->slashAnimRadius;
        
        // Efeito de anel em expansão
        DrawCircleLines(game->slashAnimPos.x, game->slashAnimPos.y, currentRadius, Fade(WHITE, (1.0f - t) * 0.9f));
        DrawCircleLines(game->slashAnimPos.x, game->slashAnimPos.y, currentRadius - 8.0f, Fade(SKYBLUE, (1.0f - t) * 0.7f));
        DrawCircle(game->slashAnimPos.x, game->slashAnimPos.y, currentRadius, Fade(SKYBLUE, (1.0f - t) * 0.15f));
    }

    // F. Desenha as partículas ativas do mundo
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (game->particles[i].active)
        {
            float lifePercent = game->particles[i].lifeTime / game->particles[i].maxLifeTime;
            DrawCircleV(
                game->particles[i].position,
                game->particles[i].size * (0.3f + 0.7f * lifePercent),
                Fade(game->particles[i].color, lifePercent)
            );
        }
    }

    EndMode2D();

    // ------------------------------------------------------------------------
    // 2. RENDEREIZA A INTERFACE FIXA (HUD DO JOGADOR)
    // ------------------------------------------------------------------------
    if (drawHUD)
    {
        DrawHUD(game, font);
    }
}

// ============================================================================
// 7. TELAS DE SELEÇÃO DE SAVE E LOAD
// ============================================================================

void DrawTelaSaveSelect(GameState *game, Font font, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3])
{
    // Fundo escuro translúcido para revelar a run desfocada/limpa no fundo
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade((Color){ 10, 8, 22, 255 }, 0.75f));

    // Título
    const char *titulo = "SALVAR PROGRESSO";
    Vector2 titleSize = MeasureTextEx(font, titulo, 42.0f, 1.5f);
    DrawTextEx(font, titulo, (Vector2){ (SCREEN_WIDTH / 2.0f) - (titleSize.x / 2.0f), 40.0f }, 42.0f, 1.5f, SKYBLUE);
    
    const char *sub = "Escolha um slot para salvar o jogo atual";
    Vector2 subSize = MeasureTextEx(font, sub, 18.0f, 1.0f);
    DrawTextEx(font, sub, (Vector2){ (SCREEN_WIDTH / 2.0f) - (subSize.x / 2.0f), 90.0f }, 18.0f, 1.0f, GRAY);

    // 3 Slots Cards
    for (int i = 0; i < 3; i++)
    {
        float cardX = 80.0f + (float)i * 400.0f;
        Rectangle cardBounds = { cardX, 140, 320, 430 };

        bool hover = CheckCollisionPointRec(mouse, cardBounds);

        Color cardBg = Fade((Color){ 20, 18, 32, 255 }, 0.85f);
        Color borderCol = hover ? (Color){ 0, 229, 255, 255 } : (Color){ 84, 52, 148, 255 };

        DrawRectangleRounded(cardBounds, 0.05f, 6, cardBg);
        DrawRectangleRoundedLines(cardBounds, 0.05f, 6, borderCol);

        // Screenshot Box (16:9 ratio, e.g. 280 x 158)
        Rectangle imgBounds = { cardX + 20, 160, 280, 158 };
        DrawRectangleRec(imgBounds, BLACK);

        if (slotTexturesLoaded[i])
        {
            DrawTexturePro(slotTextures[i],
                           (Rectangle){ 0, 0, (float)slotTextures[i].width, (float)slotTextures[i].height },
                           imgBounds, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
        else
        {
            DrawRectangleLinesEx(imgBounds, 1.0f, GRAY);
            Vector2 textSz = MeasureTextEx(font, "SEM PREVIEW", 18.0f, 1.0f);
            DrawTextEx(font, "SEM PREVIEW",
                       (Vector2){ imgBounds.x + imgBounds.width / 2.0f - textSz.x / 2.0f, imgBounds.y + imgBounds.height / 2.0f - textSz.y / 2.0f },
                       18.0f, 1.0f, DARKGRAY);
        }

        // Metadados do Slot
        SaveSlotMeta meta = game->slotsMeta[i];

        // Rótulo do Slot
        if (i == 0)
        {
            DrawTextEx(font, "SLOT 1 (PADRÃO)", (Vector2){ cardX + 20, 335 }, 22.0f, 1.0f, GOLD);
        }
        else
        {
            DrawTextEx(font, TextFormat("SLOT %d", i + 1), (Vector2){ cardX + 20, 335 }, 22.0f, 1.0f, GOLD);
        }

        if (meta.exists)
        {
            DrawTextEx(font, TextFormat("Heroi: %s", meta.name), (Vector2){ cardX + 20, 365 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Nivel: Lvl %d", meta.level), (Vector2){ cardX + 20, 390 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Onda: %d / 5", meta.wave), (Vector2){ cardX + 20, 415 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Pontos: %d", meta.score), (Vector2){ cardX + 20, 440 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Data: %s", meta.date), (Vector2){ cardX + 20, 470 }, 14.0f, 1.0f, GRAY);

            // Botão Apagar Save
            Rectangle deleteBounds = { cardX + 20, 495, 280, 35 };
            bool deleteHover = CheckCollisionPointRec(mouse, deleteBounds);
            Color delBg = deleteHover ? RED : Fade(RED, 0.3f);
            DrawRectangleRounded(deleteBounds, 0.25f, 6, delBg);
            DrawRectangleRoundedLines(deleteBounds, 0.25f, 6, RED);
            
            int delFontSize = 16;
            Vector2 delTextSz = MeasureTextEx(font, "APAGAR SAVE", (float)delFontSize, 1.0f);
            DrawTextEx(font, "APAGAR SAVE", (Vector2){ deleteBounds.x + deleteBounds.width/2.0f - delTextSz.x/2.0f, deleteBounds.y + deleteBounds.height/2.0f - delTextSz.y/2.0f }, (float)delFontSize, 1.0f, WHITE);

            // Hover do card como um todo (evitando desenhar se mouse está sobre o botão apagar)
            if (hover && !deleteHover)
            {
                DrawTextEx(font, "CLIQUE PARA SOBRESCREVER", (Vector2){ cardX + 20, 545 }, 14.0f, 1.0f, RED);
            }
        }
        else
        {
            DrawTextEx(font, "SLOT VAZIO", (Vector2){ cardX + 20, 365 }, 20.0f, 1.0f, DARKGRAY);

            if (hover)
            {
                DrawTextEx(font, "CLIQUE PARA SALVAR JOGO", (Vector2){ cardX + 20, 545 }, 14.0f, 1.0f, GREEN);
            }
        }
    }

    // Botão Voltar
    UIButton btnVoltar = { { 490, 600, 300, 50 }, "VOLTAR", false, false };
    btnVoltar.hover = CheckCollisionPointRec(mouse, btnVoltar.bounds);
    DrawButton(btnVoltar, font, true);
}

void DrawTelaLoadSelect(GameState *game, Font font, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3])
{
    // Fundo escuro translúcido para revelar a run desfocada/limpa no fundo
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade((Color){ 10, 8, 22, 255 }, 0.75f));

    // Título
    const char *titulo = "CARREGAR JOGO";
    Vector2 titleSize = MeasureTextEx(font, titulo, 42.0f, 1.5f);
    DrawTextEx(font, titulo, (Vector2){ (SCREEN_WIDTH / 2.0f) - (titleSize.x / 2.0f), 40.0f }, 42.0f, 1.5f, SKYBLUE);
    
    const char *sub = "Escolha um save para continuar sua jornada";
    Vector2 subSize = MeasureTextEx(font, sub, 18.0f, 1.0f);
    DrawTextEx(font, sub, (Vector2){ (SCREEN_WIDTH / 2.0f) - (subSize.x / 2.0f), 90.0f }, 18.0f, 1.0f, GRAY);

    // 3 Slots Cards
    for (int i = 0; i < 3; i++)
    {
        float cardX = 80.0f + (float)i * 400.0f;
        Rectangle cardBounds = { cardX, 140, 320, 430 };

        bool hover = CheckCollisionPointRec(mouse, cardBounds);
        SaveSlotMeta meta = game->slotsMeta[i];

        Color cardBg = Fade((Color){ 20, 18, 32, 255 }, 0.85f);
        Color borderCol = (hover && meta.exists) ? (Color){ 0, 229, 255, 255 } : (Color){ 84, 52, 148, 255 };

        if (!meta.exists)
        {
            borderCol = Fade(borderCol, 0.4f);
            cardBg = Fade(cardBg, 0.5f);
        }

        DrawRectangleRounded(cardBounds, 0.05f, 6, cardBg);
        DrawRectangleRoundedLines(cardBounds, 0.05f, 6, borderCol);

        // Screenshot Box (16:9 ratio, e.g. 280 x 158)
        Rectangle imgBounds = { cardX + 20, 160, 280, 158 };
        DrawRectangleRec(imgBounds, BLACK);

        if (slotTexturesLoaded[i])
        {
            DrawTexturePro(slotTextures[i],
                           (Rectangle){ 0, 0, (float)slotTextures[i].width, (float)slotTextures[i].height },
                           imgBounds, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
        else
        {
            DrawRectangleLinesEx(imgBounds, 1.0f, DARKGRAY);
            Vector2 textSz = MeasureTextEx(font, "SEM PREVIEW", 18.0f, 1.0f);
            DrawTextEx(font, "SEM PREVIEW",
                       (Vector2){ imgBounds.x + imgBounds.width / 2.0f - textSz.x / 2.0f, imgBounds.y + imgBounds.height / 2.0f - textSz.y / 2.0f },
                       18.0f, 1.0f, DARKGRAY);
        }

        // Rótulo do Slot
        if (i == 0)
        {
            DrawTextEx(font, "SLOT 1 (PADRÃO)", (Vector2){ cardX + 20, 335 }, 22.0f, 1.0f, GOLD);
        }
        else
        {
            DrawTextEx(font, TextFormat("SLOT %d", i + 1), (Vector2){ cardX + 20, 335 }, 22.0f, 1.0f, GOLD);
        }

        if (meta.exists)
        {
            DrawTextEx(font, TextFormat("Heroi: %s", meta.name), (Vector2){ cardX + 20, 365 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Nivel: Lvl %d", meta.level), (Vector2){ cardX + 20, 390 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Onda: %d / 5", meta.wave), (Vector2){ cardX + 20, 415 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Pontos: %d", meta.score), (Vector2){ cardX + 20, 440 }, 18.0f, 1.0f, WHITE);
            DrawTextEx(font, TextFormat("Data: %s", meta.date), (Vector2){ cardX + 20, 470 }, 14.0f, 1.0f, GRAY);

            // Botão Apagar Save
            Rectangle deleteBounds = { cardX + 20, 495, 280, 35 };
            bool deleteHover = CheckCollisionPointRec(mouse, deleteBounds);
            Color delBg = deleteHover ? RED : Fade(RED, 0.3f);
            DrawRectangleRounded(deleteBounds, 0.25f, 6, delBg);
            DrawRectangleRoundedLines(deleteBounds, 0.25f, 6, RED);
            
            int delFontSize = 16;
            Vector2 delTextSz = MeasureTextEx(font, "APAGAR SAVE", (float)delFontSize, 1.0f);
            DrawTextEx(font, "APAGAR SAVE", (Vector2){ deleteBounds.x + deleteBounds.width/2.0f - delTextSz.x/2.0f, deleteBounds.y + deleteBounds.height/2.0f - delTextSz.y/2.0f }, (float)delFontSize, 1.0f, WHITE);

            // Hover do card como um todo
            if (hover && !deleteHover)
            {
                DrawTextEx(font, "CLIQUE PARA CARREGAR", (Vector2){ cardX + 20, 545 }, 14.0f, 1.0f, SKYBLUE);
            }
        }
        else
        {
            DrawTextEx(font, "SLOT VAZIO", (Vector2){ cardX + 20, 365 }, 20.0f, 1.0f, DARKGRAY);
        }
    }

    // Botão Voltar
    UIButton btnVoltar = { { 490, 600, 300, 50 }, "VOLTAR", false, false };
    btnVoltar.hover = CheckCollisionPointRec(mouse, btnVoltar.bounds);
    DrawButton(btnVoltar, font, true);
}

int UpdateButtonsSaveSelect(GameState *game, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3])
{
    // Verifica cliques nos Cards
    for (int i = 0; i < 3; i++)
    {
        float cardX = 80.0f + (float)i * 400.0f;
        Rectangle cardBounds = { cardX, 140, 320, 430 };

        // Verifica primeiro clique no botão de Apagar se o slot existe
        if (game->slotsMeta[i].exists)
        {
            Rectangle deleteBounds = { cardX + 20, 495, 280, 35 };
            if (CheckCollisionPointRec(mouse, deleteBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                char pathTxt[64];
                char pathPng[64];
                sprintf(pathTxt, "Saves/save_slot_%d.txt", i + 1);
                sprintf(pathPng, "Saves/screenshot_slot_%d.png", i + 1);
                remove(pathTxt);
                remove(pathPng);

                if (slotTexturesLoaded[i])
                {
                    UnloadTexture(slotTextures[i]);
                    slotTextures[i] = (Texture2D){ 0 };
                    slotTexturesLoaded[i] = false;
                }

                game->slotsMeta[i] = CarregarMetadadosSlot(i + 1);
                return 0; // Atualiza a tela sem sair dela
            }
        }

        if (CheckCollisionPointRec(mouse, cardBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return i + 1; // Retorna o slot selecionado (1, 2 ou 3)
        }
    }

    // Verifica clique no botão Voltar
    UIButton btnVoltar = { { 490, 600, 300, 50 }, "VOLTAR", false, false };
    if (CheckCollisionPointRec(mouse, btnVoltar.bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        return -1; // Sinaliza que clicou em voltar
    }

    return 0;
}

int UpdateButtonsLoadSelect(GameState *game, Vector2 mouse, Texture2D slotTextures[3], bool slotTexturesLoaded[3])
{
    // Verifica cliques nos Cards (apenas slots que existem)
    for (int i = 0; i < 3; i++)
    {
        float cardX = 80.0f + (float)i * 400.0f;
        Rectangle cardBounds = { cardX, 140, 320, 430 };

        if (game->slotsMeta[i].exists)
        {
            // Verifica primeiro clique no botão de Apagar
            Rectangle deleteBounds = { cardX + 20, 495, 280, 35 };
            if (CheckCollisionPointRec(mouse, deleteBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                char pathTxt[64];
                char pathPng[64];
                sprintf(pathTxt, "Saves/save_slot_%d.txt", i + 1);
                sprintf(pathPng, "Saves/screenshot_slot_%d.png", i + 1);
                remove(pathTxt);
                remove(pathPng);

                if (slotTexturesLoaded[i])
                {
                    UnloadTexture(slotTextures[i]);
                    slotTextures[i] = (Texture2D){ 0 };
                    slotTexturesLoaded[i] = false;
                }

                game->slotsMeta[i] = CarregarMetadadosSlot(i + 1);
                return 0; // Atualiza a tela sem sair dela
            }

            if (CheckCollisionPointRec(mouse, cardBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                return i + 1; // Retorna o slot selecionado (1, 2 ou 3)
            }
        }
    }

    // Verifica clique no botão Voltar
    UIButton btnVoltar = { { 490, 600, 300, 50 }, "VOLTAR", false, false };
    if (CheckCollisionPointRec(mouse, btnVoltar.bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        return -1; // Sinaliza que clicou em voltar
    }

    return 0;
}
