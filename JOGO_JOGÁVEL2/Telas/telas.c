// telas.c
// Implementacao da logica de input e do desenho de cada tela do jogo
// Block Breaker — HUD header dedicado, janela 960x720, layout com BOTTOM_PADDING

#include "telas.h"
#include <math.h>
#include <stdio.h>

extern Font g_mainFont;

static inline void DrawTextC(const char *text, int x, int y, int fontSize, Color color) {
    DrawTextEx(g_mainFont, text, (Vector2){(float)x, (float)y}, (float)fontSize, 0.5f, color);
}
static inline int MeasureTextC(const char *text, int fontSize) {
    return (int)MeasureTextEx(g_mainFont, text, (float)fontSize, 0.5f).x;
}

#define DrawText DrawTextC
#define MeasureText MeasureTextC

// ══════════════════════════════════════════════════════════════════
// CONSTANTE: posicao do botao de pause (usada em telas.c e TennisF1.c)
// ══════════════════════════════════════════════════════════════════
// Retorna o Rectangle do botao de pause no HUD
Rectangle GetPauseButtonRect(void) {
    return (Rectangle){ SCREEN_WIDTH - 95.0f, 10.0f, 85.0f, HUD_HEIGHT - 20.0f };
}

// ══════════════════════════════════════════════════════════════════
// HELPERS VISUAIS
// ══════════════════════════════════════════════════════════════════

static void DrawFire(int baseX, int baseY) {
    float t = (float)GetTime() * 10.0f;
    float h1 = 30.0f + sinf(t + baseX) * 10.0f;
    DrawTriangle((Vector2){baseX-15, baseY}, (Vector2){baseX+15, baseY}, (Vector2){baseX, baseY - h1}, RED);
    float h2 = 20.0f + sinf(t * 1.5f + baseX) * 8.0f;
    DrawTriangle((Vector2){baseX-10, baseY}, (Vector2){baseX+10, baseY}, (Vector2){baseX, baseY - h2}, ORANGE);
    float h3 = 10.0f + sinf(t * 2.0f + baseX) * 5.0f;
    DrawTriangle((Vector2){baseX-5, baseY}, (Vector2){baseX+5, baseY}, (Vector2){baseX, baseY - h3}, YELLOW);
}

// Icone de power-up: diamante animado com letra central
static void DrawPowerUpIcon(float x, float y, PowerUpType type, float t) {
    float pulse = 1.0f + sinf(t * 6.0f) * 0.12f;
    float r = 13.0f * pulse;

    Color fillColor, borderColor;
    const char *label;

    switch (type) {
        case PU_MULTI_BALL:  fillColor = (Color){ 30, 80,220,255}; borderColor = SKYBLUE;  label = "M"; break;
        case PU_WIDE_PADDLE: fillColor = (Color){ 30,180, 60,255}; borderColor = GREEN;    label = "W"; break;
        case PU_FAST_BALL:   fillColor = (Color){220, 40, 40,255}; borderColor = ORANGE;   label = "F"; break;
        case PU_SLOW_BALL:   fillColor = (Color){200,180, 20,255}; borderColor = YELLOW;   label = "S"; break;
        case PU_PIERCE:      fillColor = (Color){150, 40,200,255}; borderColor = PURPLE;   label = "P"; break;
        default:             fillColor = PURPLE;                    borderColor = PINK;     label = "?"; break;
    }

    Vector2 pts[4] = {
        {x,     y - r}, {x + r, y},
        {x,     y + r}, {x - r, y}
    };
    DrawTriangle(pts[0], pts[1], pts[2], fillColor);
    DrawTriangle(pts[0], pts[2], pts[3], fillColor);
    DrawLineEx(pts[0], pts[1], 2, borderColor);
    DrawLineEx(pts[1], pts[2], 2, borderColor);
    DrawLineEx(pts[2], pts[3], 2, borderColor);
    DrawLineEx(pts[3], pts[0], 2, borderColor);

    int tw = MeasureText(label, 14);
    DrawText(label, (int)(x - tw/2.0f), (int)(y - 7), 14, WHITE);
}

// Cor do bloco baseada na vida relativa
static Color BlockHudColor(int hp, int maxHp) {
    float ratio = (float)hp / (float)maxHp;
    if      (ratio > 0.75f) return (Color){ 50,200, 80,255};
    else if (ratio > 0.50f) return (Color){230,210, 30,255};
    else if (ratio > 0.25f) return (Color){240,130, 20,255};
    else                    return (Color){210, 30, 30,255};
}

// ── HUD HEADER ─────────────────────────────────────────────────────
// Faixa superior fixa com score, onda, bolas e botao de pause
static void DrawHUD(GameState *game) {
    float t = (float)GetTime();

    // Fundo do header: gradiente escuro
    Color hudBg    = game->isCrazyMode
        ? (Color){60, 5, 5, 240}
        : (Color){22, 26, 42, 245};


    // Fundo solido
    DrawRectangle(0, 0, SCREEN_WIDTH, HUD_HEIGHT, hudBg);
    // Linha degradê inferior do header (borda brilhante)
    Color lineColor = game->isCrazyMode
        ? (Color){200, 40, 0, 255}
        : (Color){60, 100, 220, 200};
    DrawRectangle(0, HUD_HEIGHT - 2, SCREEN_WIDTH, 2, lineColor);

    // ── SCORE ──────────────────────────────────────────────────────
    Color scoreColor = game->isCrazyMode
        ? ((sinf(t*8)>0) ? (Color){255,80,0,255} : ORANGE)
        : (Color){255,220,50,255};

    DrawText("SCORE", 20, 8, 14, (Color){140,140,180,255});
    DrawText(TextFormat("%d", game->score), 20, 24, 24, scoreColor);

    // ── ONDA ───────────────────────────────────────────────────────
    DrawText("ONDA",  140, 8, 14, (Color){140,140,180,255});
    DrawText(TextFormat("%d", game->wave), 140, 24, 24, (Color){100,200,255,255});

    // ── BOLAS ──────────────────────────────────────────────────────
    int aliveBalls = 0;
    for (int b = 0; b < game->ballCount; b++)
        if (game->balls[b].active) aliveBalls++;

    DrawText("BOLAS", 260, 8, 14, (Color){140,140,180,255});
    // Bolinha(s) como ícones
    for (int i = 0; i < aliveBalls && i < 8; i++) {
        Color bc = (i == 0) ? (Color){255,80,80,255} : (Color){80,160,255,255};
        DrawCircle(268 + i * 22, 33, 8, bc);
        DrawCircle(265 + i * 22, 30, 3, Fade(WHITE, 0.5f));
    }
    if (aliveBalls == 0) {
        DrawText("—", 268, 24, 22, (Color){180,60,60,255});
    }

    // ── POWER-UP ATIVO ─────────────────────────────────────────────
    int active_pus = 0;
    if (game->widePaddleTimer > 0.0f) active_pus++;
    if (game->pierceTimer > 0.0f) active_pus++;

    int px_base = SCREEN_WIDTH / 2 - 90;

    if (game->widePaddleTimer > 0.0f) {
        int px = (active_pus == 2) ? (SCREEN_WIDTH / 2 - 190) : px_base;
        DrawText("RAQUETE LARGA", px, 8, 13, GREEN);
        float frac = game->widePaddleTimer / 10.0f;
        DrawRectangle(px, 26, 180, 10, (Color){20,60,20,200});
        DrawRectangle(px, 26, (int)(180 * frac), 10, (Color){60,220,80,255});
        DrawRectangleLines(px, 26, 180, 10, (Color){80,255,100,180});
        DrawText(TextFormat("%.1fs", game->widePaddleTimer), px + 185, 24, 14, (Color){100,255,120,255});
    }

    if (game->pierceTimer > 0.0f) {
        int px = (active_pus == 2) ? (SCREEN_WIDTH / 2 + 10) : px_base;
        DrawText("PERFURANTE", px, 8, 13, PURPLE);
        float frac = game->pierceTimer / 10.0f;
        DrawRectangle(px, 26, 180, 10, (Color){60,20,60,200});
        DrawRectangle(px, 26, (int)(180 * frac), 10, (Color){220,60,220,255});
        DrawRectangleLines(px, 26, 180, 10, (Color){255,100,255,180});
        DrawText(TextFormat("%.1fs", game->pierceTimer), px + 185, 24, 14, (Color){255,120,255,255});
    }

    // ── TITULO (Crazy Mode) ────────────────────────────────────────
    if (game->isCrazyMode) {
        Color ct = (sinf(t*8)>0) ? (Color){255,60,0,255} : ORANGE;
        const char *ctText = "* CRAZY TRAIN *";
        DrawText(ctText,
                 SCREEN_WIDTH/2 - MeasureText(ctText,18)/2,
                 (HUD_HEIGHT - 18) / 2,
                 18, ct);
    }

    // ── BOTAO PAUSE ────────────────────────────────────────────────
    Rectangle btn = GetPauseButtonRect();
    Vector2   mouse = g_virtualMouse;
    bool      hover = CheckCollisionPointRec(mouse, btn);

    Color btnBg     = hover ? (Color){70, 80,130,255} : (Color){38,45, 80,255};
    Color btnBorder = hover ? (Color){120,160,255,255}: (Color){ 70, 90,180,255};
    Color btnText   = hover ? WHITE : (Color){180,200,255,255};

    DrawRectangleRec(btn, btnBg);
    DrawRectangleLinesEx(btn, 1.5f, btnBorder);

    // Ícone de pause: dois retângulos verticais + texto
    float bx = btn.x + 12;
    float by = btn.y + (btn.height - 14) / 2.0f;
    DrawRectangle((int)bx,      (int)by, 5, 14, btnText);
    DrawRectangle((int)bx + 10, (int)by, 5, 14, btnText);
    DrawText("PAUSE", (int)(btn.x + 30), (int)(btn.y + (btn.height - 16)/2.0f), 16, btnText);
}

// ══════════════════════════════════════════════════════════════════
// UPDATE
// ══════════════════════════════════════════════════════════════════

void UpdateTelaMenu(GameState *game) {
    Vector2 mp = g_virtualMouse;

    float optY = SCREEN_HEIGHT * 0.45f;
    Rectangle rJogar = { SCREEN_WIDTH/2.0f - MeasureText("JOGAR",32)/2.0f, optY,       (float)MeasureText("JOGAR",32), 32.0f };
    Rectangle rSkins = { SCREEN_WIDTH/2.0f - MeasureText("SKINS",32)/2.0f, optY + 55,  (float)MeasureText("SKINS",32), 32.0f };
    Rectangle rCen   = { SCREEN_WIDTH/2.0f - MeasureText("CENARIOS",32)/2.0f, optY + 110, (float)MeasureText("CENARIOS",32), 32.0f };
    Rectangle rSair  = { SCREEN_WIDTH/2.0f - MeasureText("SAIR", 32)/2.0f, optY + 165, (float)MeasureText("SAIR", 32), 32.0f };

    if      (CheckCollisionPointRec(mp, rJogar)) game->menuSelection = 0;
    else if (CheckCollisionPointRec(mp, rSkins)) game->menuSelection = 1;
    else if (CheckCollisionPointRec(mp, rCen))   game->menuSelection = 2;
    else if (CheckCollisionPointRec(mp, rSair))  game->menuSelection = 3;

    if (IsKeyPressed(KEY_UP))   { game->menuSelection--; if (game->menuSelection < 0) game->menuSelection = 3; }
    if (IsKeyPressed(KEY_DOWN)) { game->menuSelection++; if (game->menuSelection > 3) game->menuSelection = 0; }

    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                   (CheckCollisionPointRec(mp, rJogar) || CheckCollisionPointRec(mp, rSkins) || 
                    CheckCollisionPointRec(mp, rCen) || CheckCollisionPointRec(mp, rSair));

    if (IsKeyPressed(KEY_ENTER) || clicked) {
        if      (game->menuSelection == 0) game->currentScreen = SCREEN_GAMEPLAY;
        else if (game->menuSelection == 1) { game->currentScreen = SCREEN_SKINS; game->menuSelection = (int)game->skinAtual; }
        else if (game->menuSelection == 2) { game->currentScreen = SCREEN_BACKGROUNDS; game->menuSelection = (int)game->bgAtual; }
        else if (game->menuSelection == 3) CloseWindow();
    }
}

void UpdateTelasSkins(GameState *game) {
    Vector2 mp = g_virtualMouse;

    for (int i = 0; i < SKIN_COUNT; i++) {
        Rectangle r = { 50.0f, SCREEN_HEIGHT/2.0f - 80.f + i*46.f, 260.f, 38.f };
        if (CheckCollisionPointRec(mp, r)) game->menuSelection = i;
    }

    if (IsKeyPressed(KEY_UP))   { game->menuSelection--; if (game->menuSelection < 0)          game->menuSelection = SKIN_COUNT-1; }
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

void UpdateTelaBackgrounds(GameState *game) {
    Vector2 mp = g_virtualMouse;

    for (int i = 0; i < BG_COUNT; i++) {
        Rectangle r = { 50.0f, SCREEN_HEIGHT/2.0f - 60.f + i*46.f, 260.f, 38.f };
        if (CheckCollisionPointRec(mp, r)) game->menuSelection = i;
    }

    if (IsKeyPressed(KEY_UP))   { game->menuSelection--; if (game->menuSelection < 0)        game->menuSelection = BG_COUNT-1; }
    if (IsKeyPressed(KEY_DOWN)) { game->menuSelection++; if (game->menuSelection >= BG_COUNT) game->menuSelection = 0; }

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->bgAtual       = (BackgroundType)game->menuSelection;
        game->currentScreen = SCREEN_MENU;
        game->menuSelection = 2;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->currentScreen = SCREEN_MENU;
        game->menuSelection = 2;
    }
}

void UpdateTelaPausa(GameState *game, Music musCrazyTrain) {
    Vector2   mp  = g_virtualMouse;
    float     optY = SCREEN_HEIGHT / 2.0f;
    Rectangle rCont = { SCREEN_WIDTH/2.0f - MeasureText("CONTINUAR",32)/2.0f,     optY,       (float)MeasureText("CONTINUAR",32),     32.0f };
    Rectangle rMenu = { SCREEN_WIDTH/2.0f - MeasureText("MENU PRINCIPAL",32)/2.0f, optY + 55, (float)MeasureText("MENU PRINCIPAL",32), 32.0f };

    if      (CheckCollisionPointRec(mp, rCont)) game->menuSelection = 0;
    else if (CheckCollisionPointRec(mp, rMenu)) game->menuSelection = 1;

    if (IsKeyPressed(KEY_UP))   { game->menuSelection--; if (game->menuSelection < 0) game->menuSelection = 1; }
    if (IsKeyPressed(KEY_DOWN)) { game->menuSelection++; if (game->menuSelection > 1) game->menuSelection = 0; }

    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                   (CheckCollisionPointRec(mp, rCont) || CheckCollisionPointRec(mp, rMenu));

    if (IsKeyPressed(KEY_ENTER) || clicked) {
        if (game->menuSelection == 0) game->currentScreen = SCREEN_GAMEPLAY;
        else { ResetGame(game); StopMusicStream(musCrazyTrain); }
    }
    if (IsKeyPressed(KEY_ESCAPE)) game->currentScreen = SCREEN_GAMEPLAY;
}

void UpdateTelaGameOver(GameState *game, Music musCrazyTrain) {
    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ResetGame(game);
        game->currentScreen = SCREEN_GAMEPLAY;
        StopMusicStream(musCrazyTrain);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        ResetGame(game);
        StopMusicStream(musCrazyTrain);
    }
}

void UpdateTelaWin(GameState *game) { (void)game; }

// ══════════════════════════════════════════════════════════════════
// DRAW
// ══════════════════════════════════════════════════════════════════

void DrawTelaMenu(GameState *game) {
    float t = (float)GetTime();

    // Titulo com efeito basico de "sombra"
    const char *title = "BLOCKSTRIKE";
    int titleFs = 70;
    Color titleColor = (Color){
        (unsigned char)(100 + sinf(t*1.2f)*50),
        (unsigned char)(120 + sinf(t*0.9f)*30),
        255, 255
    };
    DrawText(title, SCREEN_WIDTH/2 - MeasureText(title,titleFs)/2 + 4, SCREEN_HEIGHT/3 - 40 + 4, titleFs, (Color){0,0,0,120});
    DrawText(title, SCREEN_WIDTH/2 - MeasureText(title,titleFs)/2,     SCREEN_HEIGHT/3 - 40,     titleFs, titleColor);

    // Subtitulo
    const char *sub = "BEM VINDO AO MEU PRIMEIRO JOGO!";
    DrawText(sub, SCREEN_WIDTH/2 - MeasureText(sub,22)/2, SCREEN_HEIGHT/3 + 38, 22, (Color){120,140,200,255});

    // Decoracao: estrelinhas girando
    for (int i = 0; i < 6; i++) {
        float sx = 80.0f + i * 165.0f;
        float sy = SCREEN_HEIGHT/3.0f - 10.0f + sinf(t*2.0f + i)*16.0f;
        DrawPoly((Vector2){sx, sy}, 4, 8, t*50.0f + i*60.0f, (Color){255,210,50,160});
    }

    // Opcoes de menu
    float optY = SCREEN_HEIGHT * 0.45f;
    const char *opts[4] = {"JOGAR", "SKINS", "CENARIOS", "SAIR"};
    for (int i = 0; i < 4; i++) {
        bool sel = (game->menuSelection == i);
        Color bg  = sel ? (Color){50,70,160,220} : (Color){25,30,60,180};
        Color border = sel ? (Color){100,150,255,255} : (Color){50,60,110,200};
        Color txt = sel ? WHITE : (Color){160,170,210,255};

        float rw = 240.0f, rh = 42.0f;
        float rx = SCREEN_WIDTH/2.0f - rw/2.0f;
        float ry = optY + i * 55.0f;

        DrawRectangleRounded((Rectangle){rx, ry, rw, rh}, 0.3f, 6, bg);
        DrawRectangleRoundedLines((Rectangle){rx, ry, rw, rh}, 0.3f, 6, border);
        DrawText(opts[i], (int)(SCREEN_WIDTH/2.0f - MeasureText(opts[i],26)/2.0f), (int)(ry + 8), 26, txt);

        // Seta animada
        if (sel) {
            float ap = sinf(t*5.0f) * 5.0f;
            DrawText(">", (int)(rx - 30 + ap), (int)(ry + 8), 26, (Color){100,160,255,255});
        }
    }

    // Legenda de controles
    const char *hint = "Mouse / A-D / Setas para mover  |  ENTER ou CLICK para confirmar";
    DrawText(hint, SCREEN_WIDTH/2 - MeasureText(hint,15)/2, SCREEN_HEIGHT - 60, 15, (Color){90,100,140,255});

    // Legenda power-ups
    const char *pu = "Power-Ups:  [M] Multi-Bola   [W] Larga   [F] Rapida   [S] Lenta   [P] Perfurante";
    DrawText(pu, SCREEN_WIDTH/2 - MeasureText(pu,15)/2, SCREEN_HEIGHT - 35, 15, (Color){70,90,130,255});
}

void DrawTelasSkins(GameState *game) {
    const char *title = "ESCOLHA SUA SKIN";
    DrawText(title, SCREEN_WIDTH/2 - MeasureText(title,50)/2, 60, 50, (Color){80,120,220,255});

    const char *nomes[SKIN_COUNT] = {"PADRAO","PATO","DIABO","PINTINHO","ALIEN"};
    float optY = SCREEN_HEIGHT/2.0f - 80.0f;

    // Lista na esquerda
    for (int i = 0; i < SKIN_COUNT; i++) {
        bool sel = (game->menuSelection == i);
        Color bg     = sel ? (Color){50,70,160,220} : (Color){25,30,60,180};
        Color border = sel ? (Color){100,150,255,255} : (Color){50,60,110,200};
        Color txt    = sel ? WHITE : (Color){160,170,210,255};

        float rw = 260.0f, rh = 38.0f;
        float rx = 50.0f;
        float ry = optY + i * 46.0f;

        DrawRectangleRounded((Rectangle){rx, ry, rw, rh}, 0.3f, 6, bg);
        DrawRectangleRoundedLines((Rectangle){rx, ry, rw, rh}, 0.3f, 6, border);
        DrawText(nomes[i], (int)(rx + rw/2.0f - MeasureText(nomes[i],24)/2.0f), (int)(ry + 7), 24, txt);

        if (sel) {
            DrawText(">", (int)(rx - 28), (int)(ry + 7), 24, (Color){100,160,255,255});
        }
    }

    // Preview na direita (maior, centralizado na parte direita da tela)
    float previewX = SCREEN_WIDTH * 0.70f;
    float previewY = SCREEN_HEIGHT/2.0f + 50.0f;
    
    // Fundo do preview
    DrawRectangleRounded((Rectangle){previewX - 160, previewY - 180, 320, 240}, 0.1f, 10, (Color){15,20,35,200});
    DrawRectangleRoundedLines((Rectangle){previewX - 160, previewY - 180, 320, 240}, 0.1f, 10, (Color){50,80,140,150});
    DrawText("PREVIA DA SKIN", (int)(previewX - MeasureText("PREVIA DA SKIN",20)/2), (int)(previewY - 165), 20, (Color){140,160,220,255});

    Paddle previewPaddle = {
        previewX - 60.0f,
        previewY - 20.0f,
        120.0f, 15.0f, 0, 0, 0, 0, DARKBLUE
    };
    
    // Passar posicao do mouse como focusPoint para que os olhos sigam o mouse no menu!
    DrawPaddleSkin(&previewPaddle, (PaddleSkin)game->menuSelection, g_virtualMouse);

    const char *hint = "UP/DOWN para escolher  |  ENTER ou CLICK para confirmar  |  ESC para voltar";
    DrawText(hint, SCREEN_WIDTH/2 - MeasureText(hint,16)/2, SCREEN_HEIGHT - 25, 16, (Color){80,100,140,255});
}

void DrawTelaBackgrounds(GameState *game) {
    const char *title = "ESCOLHA O CENARIO";
    DrawText(title, SCREEN_WIDTH/2 - MeasureText(title,50)/2, 60, 50, (Color){80,120,220,255});

    const char *nomes[BG_COUNT] = {"MODERNO (PADRAO)", "FAZENDA ENSOLARADA", "CAVERNA INFERNAL", "ESPACO SIDERAL"};
    float optY = SCREEN_HEIGHT/2.0f - 60.0f;

    // Lista na esquerda
    for (int i = 0; i < BG_COUNT; i++) {
        bool sel = (game->menuSelection == i);
        Color bg     = sel ? (Color){50,70,160,220} : (Color){25,30,60,180};
        Color border = sel ? (Color){100,150,255,255} : (Color){50,60,110,200};
        Color txt    = sel ? WHITE : (Color){160,170,210,255};

        float rw = 260.0f, rh = 38.0f;
        float rx = 50.0f;
        float ry = optY + i * 46.0f;

        DrawRectangleRounded((Rectangle){rx, ry, rw, rh}, 0.3f, 6, bg);
        DrawRectangleRoundedLines((Rectangle){rx, ry, rw, rh}, 0.3f, 6, border);
        DrawText(nomes[i], (int)(rx + rw/2.0f - MeasureText(nomes[i],20)/2.0f), (int)(ry + 9), 20, txt);

        if (sel) {
            DrawText(">", (int)(rx - 28), (int)(ry + 7), 24, (Color){100,160,255,255});
        }
    }

    // Preview do background na direita
    float previewX = SCREEN_WIDTH * 0.65f;
    float previewY = SCREEN_HEIGHT/2.0f - 100.0f;
    float prevW = 320.0f;
    float prevH = 240.0f;
    
    // Moldura do preview
    DrawRectangleRounded((Rectangle){previewX, previewY, prevW, prevH}, 0.1f, 10, BLACK);
    
    // Mini-renderizacao adaptada do cenario no preview:
    BeginScissorMode((int)previewX, (int)previewY, (int)prevW, (int)prevH);
    // DrawBackground assumes full screen, we simulate it by translating temporarily if needed, 
    // but the simplest is just draw it and let scissor clip it.
    // However, DrawBackground draws at screen coordinates. We need to draw a scaled down version, or just a slice.
    // For simplicity, we just draw the full background and let ScissorMode clip a window of it.
    DrawBackground((BackgroundType)game->menuSelection, (float)GetTime(), false);
    EndScissorMode();

    DrawRectangleRoundedLines((Rectangle){previewX, previewY, prevW, prevH}, 0.1f, 10, (Color){100,150,255,255});
    DrawText("PREVIA", (int)(previewX + prevW/2 - MeasureText("PREVIA",20)/2), (int)(previewY - 25), 20, (Color){140,160,220,255});

    const char *hint = "UP/DOWN para escolher  |  ENTER ou CLICK para confirmar  |  ESC para voltar";
    DrawText(hint, SCREEN_WIDTH/2 - MeasureText(hint,16)/2, SCREEN_HEIGHT - 25, 16, (Color){80,100,140,255});
}

// ─── RENDERIZADOR UNIVERSAL DE BACKGROUNDS ────────────────────────
void DrawBackground(BackgroundType bg, float time, bool isCrazyMode) {
    if (isCrazyMode) {
        // Sobreposicao infernal pra qualquer cenario
        ClearBackground((Color){30, 0, 0, 255});
        for (int i=0; i<100; i++) {
            float x = fmodf((i * 47.0f), SCREEN_WIDTH);
            float y = fmodf((i * 83.0f + time*20.0f), SCREEN_HEIGHT);
            DrawPixelV((Vector2){x,y}, ORANGE);
        }
        return;
    }

    switch (bg) {
        case BG_PADRAO:
            ClearBackground((Color){14, 18, 32, 255});
            // Grade suave
            for(int x = 0; x < SCREEN_WIDTH; x += 40) DrawLine(x, 0, x, SCREEN_HEIGHT, (Color){20,30,50,100});
            for(int y = 0; y < SCREEN_HEIGHT; y += 40) DrawLine(0, y, SCREEN_WIDTH, y, (Color){20,30,50,100});
            break;

        case BG_FARM:
            // Ceu azul claro
            ClearBackground((Color){135, 206, 235, 255});
            // Sol
            DrawCircle(SCREEN_WIDTH - 100, 100, 40, YELLOW);
            // Nuvens simples
            DrawCircle(200 + sinf(time*0.2f)*20, 150, 30, RAYWHITE);
            DrawCircle(230 + sinf(time*0.2f)*20, 140, 40, RAYWHITE);
            DrawCircle(260 + sinf(time*0.2f)*20, 150, 30, RAYWHITE);
            // Grama ao fundo (abaixo dos blocos)
            DrawRectangle(0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200, (Color){34, 139, 34, 255});
            break;

        case BG_HELL:
            // Fundo escuro avermelhado
            ClearBackground((Color){20, 5, 5, 255});
            // Pedras de magma ao fundo
            for (int i=0; i<15; i++) {
                DrawCircle(100 + i*60, SCREEN_HEIGHT - 100 + sinf(i)*30, 80, (Color){40, 10, 10, 255});
            }
            break;

        case BG_SPACE:
            // Fundo preto puro com estrelas
            ClearBackground(BLACK);
            for (int i=0; i<150; i++) {
                float sx = fmodf((i * 123.4f), SCREEN_WIDTH);
                float sy = fmodf((i * 321.4f), SCREEN_HEIGHT);
                float radius = fmodf(i, 2.0f) + 1.0f;
                // Faz as estrelas piscarem suavemente
                unsigned char alpha = (unsigned char)(155 + 100 * sinf(time * (1.0f + fmodf(i, 3.0f))));
                DrawCircleV((Vector2){sx, sy}, radius, (Color){255, 255, 255, alpha});
            }
            // Planeta distante
            DrawCircle(150, 200, 60, (Color){40, 30, 80, 255});
            DrawCircleLines(150, 200, 80, (Color){60, 50, 100, 150});
            break;

        default:
            ClearBackground(BLACK);
            break;
    }
}

void DrawTelaGameplay(GameState *game) {
    float t = (float)GetTime();

    // ── Linha separadora da area de jogo ───────────────────────────
    DrawRectangle(0, HUD_HEIGHT, SCREEN_WIDTH, 1, (Color){40,50,90,255});

    // ── Blocos ──────────────────────────────────────────────────────
    for (int i = 0; i < MAX_BLOCKS; i++) {
        Block *blk = &game->blocks[i];
        if (!blk->active) continue;

        Color fill = (blk->flashFrames > 0) ? WHITE : BlockHudColor(blk->hp, blk->maxHp);

        // Corpo
        DrawRectangle((int)blk->x, (int)blk->y, (int)blk->width, (int)blk->height, fill);

        // Reflexo/brilho no topo (vidro)
        DrawRectangle((int)blk->x + 2, (int)blk->y + 2, (int)blk->width - 4, 7,
                      (Color){255,255,255,55});

        // Sombra na base
        DrawRectangle((int)blk->x, (int)(blk->y + blk->height - 4), (int)blk->width, 4,
                      (Color){0,0,0,60});

        // Borda
        DrawRectangleLinesEx((Rectangle){blk->x, blk->y, blk->width, blk->height},
                             1.5f, (Color){0,0,0,100});

        // HP no centro — fonte adaptada ao tamanho do numero
        char hpTxt[4];
        snprintf(hpTxt, sizeof(hpTxt), "%d", blk->hp);
        int fs = (blk->hp >= 10) ? 14 : 17;
        int tw = MeasureText(hpTxt, fs);
        int tx = (int)(blk->x + blk->width/2.0f  - tw/2.0f);
        int ty = (int)(blk->y + blk->height/2.0f - fs/2.0f);
        DrawText(hpTxt, tx+1, ty+1, fs, (Color){0,0,0,130});  // sombra
        DrawText(hpTxt, tx,   ty,   fs, WHITE);
    }

    // ── Power-ups caindo ───────────────────────────────────────────
    for (int i = 0; i < MAX_POWERUPS; i++) {
        PowerUp *pu = &game->powerUps[i];
        if (!pu->active) continue;
        DrawPowerUpIcon(pu->x, pu->y, pu->type, t);
    }

    // ── Bolas ──────────────────────────────────────────────────────
    for (int b = 0; b < game->ballCount; b++) {
        Ball *ball = &game->balls[b];
        if (!ball->active) continue;

        if (game->isCrazyMode) {
            float aura = (sinf(t * 10.0f) + 1.5f) * ball->radius;
            DrawCircleV((Vector2){ball->x, ball->y}, aura*1.6f, Fade(RED,    0.18f));
            DrawCircleV((Vector2){ball->x, ball->y}, aura*1.2f, Fade(ORANGE, 0.28f));
        }

        Color renderColor = ball->color;
        if (game->pierceTimer > 0.0f) {
            renderColor = PURPLE;
            // Efeito perfurante (aura roxa)
            float aura = (sinf(t * 15.0f) + 1.2f) * ball->radius;
            DrawCircleV((Vector2){ball->x, ball->y}, aura*1.5f, Fade(PURPLE, 0.4f));
        }

        DrawCircleV((Vector2){ball->x, ball->y}, ball->radius, renderColor);
        // Reflexo interno
        DrawCircleV((Vector2){ball->x - ball->radius*0.3f, ball->y - ball->radius*0.3f},
                    ball->radius*0.3f, Fade(WHITE, 0.5f));
    }

    // ── Raquete ────────────────────────────────────────────────────
    Vector2 focus = { SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
    // Encontrar primeira bola ativa para a raquete olhar
    for (int b = 0; b < game->ballCount; b++) {
        if (game->balls[b].active) {
            focus.x = game->balls[b].x;
            focus.y = game->balls[b].y;
            break;
        }
    }
    DrawPaddleSkin(game->paddle, game->skinAtual, focus);

    // ── Zona inferior decorativa (asfalto / chao) ──────────────────

    DrawRectangle(0, SCREEN_HEIGHT - 8, SCREEN_WIDTH, 8, (Color){20,22,35,255});
    // Lininha decorativa acima do chao
    DrawRectangle(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 2, (Color){40,50,90,160});

    // ── Crazy Mode: foguinhos no chao ─────────────────────────────
    if (game->isCrazyMode) {
        for (int i = 0; i <= SCREEN_WIDTH; i += 35)
            DrawFire(i, SCREEN_HEIGHT - 8);
    }

    // ── HUD Header ─────────────────────────────────────────────────
    DrawHUD(game);

    // ── Banner de nova onda ─────────────────────────────────────────
    if (game->waveTimer > 0) {
        DrawRectangle(0, SCREEN_HEIGHT/2 - 70, SCREEN_WIDTH, 140, Fade(BLACK, 0.75f));
        char waveTxt[32];
        snprintf(waveTxt, sizeof(waveTxt), "WAVE %d!", game->wave);
        int wfs = 64;
        float pulse = 1.0f + sinf(t*6.0f)*0.06f;
        int wfsP = (int)(wfs * pulse);
        DrawText(waveTxt,
                 SCREEN_WIDTH/2 - MeasureText(waveTxt,wfsP)/2,
                 SCREEN_HEIGHT/2 - wfsP/2 - 10, wfsP, GOLD);
        DrawText("Nova onda — blocos mais resistentes!",
                 SCREEN_WIDTH/2 - MeasureText("Nova onda — blocos mais resistentes!",19)/2,
                 SCREEN_HEIGHT/2 + 42, 19, (Color){200,200,200,255});
    }
}

void DrawTelaPausa(GameState *game) {
    // Jogo congelado ao fundo
    DrawTelaGameplay(game);
    // Overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.58f));

    const char *title = "JOGO PAUSADO";
    DrawText(title, SCREEN_WIDTH/2 - MeasureText(title,54)/2, SCREEN_HEIGHT/4, 54, RAYWHITE);

    float optY = SCREEN_HEIGHT/2.0f;
    const char *opts[2] = {"CONTINUAR", "MENU PRINCIPAL"};
    for (int i = 0; i < 2; i++) {
        bool sel = (game->menuSelection == i);
        Color bg     = sel ? (Color){50,70,160,220} : (Color){25,30,60,160};
        Color border = sel ? (Color){100,150,255,255} : (Color){50,60,110,180};
        Color txt    = sel ? WHITE : (Color){160,170,210,200};

        float rw = 300.0f, rh = 44.0f;
        float rx = SCREEN_WIDTH/2.0f - rw/2.0f;
        float ry = optY + i * 58.0f;

        DrawRectangleRounded((Rectangle){rx, ry, rw, rh}, 0.3f, 6, bg);
        DrawRectangleRoundedLines((Rectangle){rx, ry, rw, rh}, 0.3f, 6, border);
        DrawText(opts[i], (int)(SCREEN_WIDTH/2.0f - MeasureText(opts[i],26)/2.0f),
                 (int)(ry + 9), 26, txt);
    }

    DrawText("ESC para voltar ao jogo",
             SCREEN_WIDTH/2 - MeasureText("ESC para voltar ao jogo",16)/2,
             SCREEN_HEIGHT - 50, 16, (Color){100,110,160,200});
}

void DrawTelaGameOver(GameState *game) {
    float t = (float)GetTime();

    // Fundo dramático vermelho pulsante
    float pulse = (sinf(t * 3.0f) + 1.0f) / 2.0f;
    ClearBackground((Color){(unsigned char)(20 + pulse*25), 0, 0, 255});

    // "GAME OVER" com sombra
    const char *go = "GAME OVER!";
    int goFs = 72;
    int goX  = SCREEN_WIDTH/2 - MeasureText(go, goFs)/2;
    DrawText(go, goX+4, SCREEN_HEIGHT/2 - 100 + 4, goFs, (Color){80,0,0,255});
    DrawText(go, goX,   SCREEN_HEIGHT/2 - 100,     goFs, (Color){255,50,50,255});

    // Score e onda
    DrawText(TextFormat("Pontuacao Final: %d", game->score),
             SCREEN_WIDTH/2 - MeasureText(TextFormat("Pontuacao Final: %d", game->score),28)/2,
             SCREEN_HEIGHT/2,  28, RAYWHITE);

    DrawText(TextFormat("Onda alcancada:  %d", game->wave),
             SCREEN_WIDTH/2 - MeasureText(TextFormat("Onda alcancada:  %d", game->wave),22)/2,
             SCREEN_HEIGHT/2 + 40, 22, (Color){200,200,200,255});

    // Instrucao piscante
    if ((int)(t * 2.0f) % 2 == 0) {
        const char *hint = "ENTER ou CLICK para Reiniciar  |  ESC para Menu";
        DrawText(hint, SCREEN_WIDTH/2 - MeasureText(hint,20)/2,
                 SCREEN_HEIGHT/2 + 95, 20, GOLD);
    }

    // Foguinhos
    for (int i = 0; i <= SCREEN_WIDTH; i += 35)
        DrawFire(i, SCREEN_HEIGHT);
}
