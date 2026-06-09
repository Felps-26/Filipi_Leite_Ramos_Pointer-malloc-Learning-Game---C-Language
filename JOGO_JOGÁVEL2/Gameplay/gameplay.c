// gameplay.c
// Inicializacao, fisica e logica do Block Breaker
// Usa malloc/realloc para gerenciar bolas, blocos e power-ups na heap

#include "gameplay.h"
#include <stdio.h>

// ══════════════════════════════════════════════════════════════════
// HELPERS INTERNOS
// ══════════════════════════════════════════════════════════════════


// Adiciona uma bola com velocidade e angulo aleatorio
static void SpawnBall(GameState *game, float x, float y, float speedX, float speedY) {
    if (game->ballCount >= MAX_BALLS) return;
    game->balls[game->ballCount] = (Ball){
        .x      = x,
        .y      = y,
        .speedX = speedX,
        .speedY = speedY,
        .radius = 10.0f,
        .color  = game->isCrazyMode ? ORANGE : RED,
        .active = true
    };
    game->ballCount++;
}

// Tenta spawnar um power-up na posicao dada (30% de chance)
static void TrySpawnPowerUp(GameState *game, float x, float y) {
    if (GetRandomValue(0, 99) >= 30) return; // 30% chance
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!game->powerUps[i].active) {
            game->powerUps[i] = (PowerUp){
                .x      = x,
                .y      = y,
                .speedY = 3.0f,
                .type   = (PowerUpType)GetRandomValue(0, PU_COUNT - 1),
                .active = true
            };
            return;
        }
    }
}

// ══════════════════════════════════════════════════════════════════
// INIT / RESET / FREE
// ══════════════════════════════════════════════════════════════════

GameState *InitGame(void) {
    GameState *game = (GameState *)malloc(sizeof(GameState));

    // Bolas — array pre-alocado com tamanho maximo
    game->balls     = (Ball *)malloc(sizeof(Ball) * MAX_BALLS);
    game->ballCount = 0;

    // Raquete — y posicionado com BOTTOM_PADDING para nao cortar skins
    game->paddle = (Paddle *)malloc(sizeof(Paddle));
    game->paddle->width      = 160.0f;
    game->paddle->height     = 15.0f;
    game->paddle->x          = SCREEN_WIDTH / 2.0f - game->paddle->width / 2.0f;
    game->paddle->y          = SCREEN_HEIGHT - BOTTOM_PADDING;
    game->paddle->speed      = 9.0f;
    game->paddle->velocityX  = 0.0f;
    game->paddle->legSwing   = 0.0f;
    game->paddle->legVelocity= 0.0f;
    game->paddle->color      = DARKBLUE;

    // Blocos
    game->blocks = (Block *)malloc(sizeof(Block) * MAX_BLOCKS);
    for (int i = 0; i < MAX_BLOCKS; i++) game->blocks[i].active = false;

    // Power-ups
    game->powerUps = (PowerUp *)malloc(sizeof(PowerUp) * MAX_POWERUPS);
    for (int i = 0; i < MAX_POWERUPS; i++) game->powerUps[i].active = false;

    game->widePaddleTimer = 0.0f;
    game->paddleOriginalW = game->paddle->width;
    game->pierceTimer     = 0.0f;

    game->score         = 0;
    game->wave          = 1;
    game->waveTimer     = 0;
    game->newRowTimer   = 0.0f;
    game->playTime      = 0.0f;
    game->isCrazyMode   = false;
    game->crazyModeTimer= 0.0f;
    game->currentScreen = SCREEN_MENU;
    game->menuSelection = 0;
    game->skinAtual     = SKIN_PADRAO;
    game->bgAtual       = BG_PADRAO;

    InitWave(game, 1);
    return game;
}

void InitWave(GameState *game, int wave) {
    // HP escala de acordo com a pontuacao e tempo jogado
    int scaleByScore = game->score / 300;
    int scaleByTime  = (int)(game->playTime / 20.0f);
    int minHp = 1 + (scaleByScore + scaleByTime) / 3;
    int maxHp = 2 + (scaleByScore + scaleByTime);
    if (minHp < 1) minHp = 1;
    if (maxHp < minHp) maxHp = minHp;

    // Limpa todos os blocos
    for (int i = 0; i < MAX_BLOCKS; i++) game->blocks[i].active = false;

    for (int row = 0; row < BLOCK_ROWS; row++) {
        for (int col = 0; col < BLOCK_COLS; col++) {
            int hp = GetRandomValue(minHp, maxHp) + (BLOCK_ROWS - 1 - row) / 2;
            if (hp < 1) hp = 1;

            int idx = row * BLOCK_COLS + col;
            game->blocks[idx].x          = BLOCK_OFFSET_X + col * (BLOCK_WIDTH  + BLOCK_PADDING);
            game->blocks[idx].y          = BLOCK_OFFSET_Y + row * (BLOCK_HEIGHT + BLOCK_PADDING);
            game->blocks[idx].width      = BLOCK_WIDTH;
            game->blocks[idx].height     = BLOCK_HEIGHT;
            game->blocks[idx].hp         = hp;
            game->blocks[idx].maxHp      = hp;
            game->blocks[idx].active     = true;
            game->blocks[idx].flashFrames= 0;
        }
    }

    // Limpa power-ups da onda anterior
    for (int i = 0; i < MAX_POWERUPS; i++) game->powerUps[i].active = false;

    // Restaura raquete PRIMEIRO (para que a bola nasça na posicao correta)
    game->paddle->x       = SCREEN_WIDTH / 2.0f - game->paddle->width / 2.0f;
    game->paddle->y       = SCREEN_HEIGHT - BOTTOM_PADDING - game->paddle->height;
    game->paddle->velocityX   = 0.0f;
    game->paddle->legSwing    = 0.0f;
    game->paddle->legVelocity = 0.0f;
    game->widePaddleTimer     = 0.0f;
    game->pierceTimer         = 0.0f;
    game->paddle->width       = game->paddleOriginalW;
    game->paddle->x           = SCREEN_WIDTH / 2.0f - game->paddle->width / 2.0f;
    game->newRowTimer         = 0.0f;

    // Spawna a bola logo acima do paddle (posicao correta)
    game->ballCount = 0;
    SpawnBall(game,
              SCREEN_WIDTH / 2.0f,
              game->paddle->y - 20.0f,
              5.0f, -7.0f);
}

void ResetGame(GameState *game) {
    game->score         = 0;
    game->wave          = 1;
    game->isCrazyMode   = false;
    game->crazyModeTimer= 0.0f;
    game->waveTimer     = 0;
    game->playTime      = 0.0f;
    game->currentScreen = SCREEN_MENU;
    game->menuSelection = 0;
    game->paddle->width = 160.0f;
    game->paddleOriginalW = 160.0f;
    // Nao muda a skin selecionada pelo jogador
    InitWave(game, 1);
}

void FreeGame(GameState *game) {
    if (game == NULL) return;
    free(game->balls);
    free(game->paddle);
    free(game->blocks);
    free(game->powerUps);
    free(game);
}

// ══════════════════════════════════════════════════════════════════
// UPDATE PRINCIPAL
// ══════════════════════════════════════════════════════════════════

void UpdateGameplay(GameState *game, Sound sndPong, Sound sndGameOver, Music musCrazyTrain) {

    float dt = GetFrameTime();

    // ── Temporizador do banner de onda ─────────────────────────────
    if (game->waveTimer > 0) {
        game->waveTimer--;
        return; // pausa o jogo enquanto mostra o banner
    }

    game->playTime += dt;

    // ══ RAQUETE ════════════════════════════════════════════════════
    float oldX = game->paddle->x;

    Vector2 mouseDelta = g_virtualMouseDelta;
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
        game->paddle->x = g_virtualMouse.x - game->paddle->width / 2.0f;
    } else {
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  game->paddle->x -= game->paddle->speed;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) game->paddle->x += game->paddle->speed;
    }
    if (game->paddle->x < 0)                                      game->paddle->x = 0;
    if (game->paddle->x + game->paddle->width > SCREEN_WIDTH)     game->paddle->x = SCREEN_WIDTH - game->paddle->width;

    // Fisica das perninhas
    game->paddle->velocityX = game->paddle->x - oldX;
    float targetSwing = -game->paddle->velocityX * 0.8f;
    if (targetSwing >  25.0f) targetSwing =  25.0f;
    if (targetSwing < -25.0f) targetSwing = -25.0f;
    float springForce = (targetSwing - game->paddle->legSwing) * 0.3f;
    game->paddle->legVelocity += springForce;
    game->paddle->legVelocity *= 0.85f;
    game->paddle->legSwing    += game->paddle->legVelocity;

    // ── Power-up de raquete larga ───────────────────────────────────
    if (game->widePaddleTimer > 0.0f) {
        game->widePaddleTimer -= dt;
        if (game->widePaddleTimer <= 0.0f) {
            game->paddle->width   = game->paddleOriginalW;
            game->widePaddleTimer = 0.0f;
        }
    }

    // ── Power-up perfurante ─────────────────────────────────────────
    if (game->pierceTimer > 0.0f) {
        game->pierceTimer -= dt;
        if (game->pierceTimer <= 0.0f) {
            game->pierceTimer = 0.0f;
        }
    }

    // ── Timer do Crazy Mode (Boss) ─────────────────────────────────
    if (game->isCrazyMode) {
        game->crazyModeTimer -= dt;
        if (game->crazyModeTimer <= 0.0f) {
            game->isCrazyMode = false;
            game->crazyModeTimer = 0.0f;
            StopMusicStream(musCrazyTrain);
            // Restaura cor das bolas
            for (int bb = 0; bb < game->ballCount; bb++) {
                if (game->balls[bb].active) {
                    game->balls[bb].color = RED;
                }
            }
        }
    }

    // ══ BOLAS ══════════════════════════════════════════════════════
    int aliveBalls = 0;
    bool hitSoundThisFrame = false;

    for (int b = 0; b < game->ballCount; b++) {
        Ball *ball = &game->balls[b];
        if (!ball->active) continue;
        aliveBalls++;

        // Sub-steps para evitar tunneling
        int   steps = 8;
        float stepX = ball->speedX / (float)steps;
        float stepY = ball->speedY / (float)steps;
        bool  hitPaddleThisFrame = false;

        for (int s = 0; s < steps; s++) {
            ball->x += stepX;
            ball->y += stepY;

            // Paredes laterais
            if ((ball->x - ball->radius <= 0          && ball->speedX < 0) ||
                (ball->x + ball->radius >= SCREEN_WIDTH && ball->speedX > 0)) {
                ball->speedX *= -1.0f;
                stepX        *= -1.0f;
            }
            // Teto (nao pode passar do HUD_HEIGHT)
            if (ball->y - ball->radius <= HUD_HEIGHT && ball->speedY < 0) {
                ball->speedY *= -1.0f;
                stepY        *= -1.0f;
            }

            // Colisao com a raquete
            if (!hitPaddleThisFrame) {
                Rectangle paddleRect = { game->paddle->x, game->paddle->y,
                                         game->paddle->width, game->paddle->height };
                Vector2   ballCenter = { ball->x, ball->y };

                if (CheckCollisionCircleRec(ballCenter, ball->radius, paddleRect) && ball->speedY > 0) {
                    hitPaddleThisFrame = true;
                    ball->speedY *= -1.0f;

                    float hitPoint = (ball->x - (game->paddle->x + game->paddle->width / 2.0f))
                                   / (game->paddle->width / 2.0f);
                    ball->speedX = hitPoint * 7.0f;

                    // Aumenta velocidade levemente a cada rebatida (cap)
                    float absY = ball->speedY < 0 ? -ball->speedY : ball->speedY;
                    absY *= 1.008f;
                    if (absY > 18.0f) absY = 18.0f;
                    ball->speedY = -absY;
                    stepY = ball->speedY / (float)steps;

                    if (!hitSoundThisFrame) {
                        PlaySound(sndPong);
                        hitSoundThisFrame = true;
                    }
                }
            }

            // ── Colisao com blocos ────────────────────────────────
            for (int i = 0; i < MAX_BLOCKS; i++) {
                Block *blk = &game->blocks[i];
                if (!blk->active) continue;

                Rectangle blkRect = { blk->x, blk->y, blk->width, blk->height };
                Vector2   bc      = { ball->x, ball->y };

                if (CheckCollisionCircleRec(bc, ball->radius, blkRect)) {
                    // Determina de qual lado a bola entrou para rebater corretamente
                    float overlapLeft   = (ball->x + ball->radius) - blk->x;
                    float overlapRight  = (blk->x + blk->width)  - (ball->x - ball->radius);
                    float overlapTop    = (ball->y + ball->radius) - blk->y;
                    float overlapBottom = (blk->y + blk->height)  - (ball->y - ball->radius);

                    float minOverlap = overlapLeft;
                    if (overlapRight  < minOverlap) minOverlap = overlapRight;
                    if (overlapTop    < minOverlap) minOverlap = overlapTop;
                    if (overlapBottom < minOverlap) minOverlap = overlapBottom;

                    if (game->pierceTimer <= 0.0f) {
                        if      (minOverlap == overlapLeft   || minOverlap == overlapRight)  { ball->speedX *= -1.0f; stepX *= -1.0f; }
                        else if (minOverlap == overlapTop    || minOverlap == overlapBottom) { ball->speedY *= -1.0f; stepY *= -1.0f; }
                    }

                    blk->hp--;
                    blk->flashFrames = 3;

                    if (blk->hp <= 0) {
                        blk->active = false;
                        // Pontos = vida maxima * 10
                        game->score += blk->maxHp * 10;

                        // Tenta spawnar power-up
                        TrySpawnPowerUp(game, blk->x + blk->width / 2.0f, blk->y + blk->height / 2.0f);

                        // Ativa Boss Mode (Crazy Train) aleatoriamente a partir de 1000 pontos
                        if (game->score >= 1000) {
                            int chance = 2 + game->wave; // 2% base + 1% por onda
                            if (GetRandomValue(0, 100) < chance) {
                                if (!game->isCrazyMode) {
                                    game->isCrazyMode = true;
                                    PlayMusicStream(musCrazyTrain);
                                    for (int bb = 0; bb < game->ballCount; bb++)
                                        if (game->balls[bb].active)
                                            game->balls[bb].color = ORANGE;
                                }
                                game->crazyModeTimer = 20.0f; // prolonga ou inicia o modo por 20 segundos
                            }
                        }
                    }

                    if (!hitSoundThisFrame) {
                        PlaySound(sndPong);
                        hitSoundThisFrame = true;
                    }

                    break; // uma bola nao pode atingir dois blocos no mesmo sub-step
                }
            }

            // Bola caiu (fundo)
            if (ball->y - ball->radius > SCREEN_HEIGHT) {
                ball->active = false;
                aliveBalls--;
                break;
            }
        }
    }

    // Decrementa flash dos blocos
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (game->blocks[i].flashFrames > 0) game->blocks[i].flashFrames--;
    }

    // ── Game Over se todas as bolas caíram ─────────────────────────
    // Recontagem real
    int realAlive = 0;
    for (int b = 0; b < game->ballCount; b++)
        if (game->balls[b].active) realAlive++;

    if (realAlive == 0) {
        game->currentScreen = SCREEN_GAMEOVER;
        PlaySound(sndGameOver);
        StopMusicStream(musCrazyTrain);
        return;
    }

    // ── Vitoria: todos os blocos destruidos ────────────────────────
    int activeBlocks = 0;
    for (int i = 0; i < MAX_BLOCKS; i++)
        if (game->blocks[i].active) activeBlocks++;

    if (activeBlocks == 0) {
        game->wave++;
        game->waveTimer = 120; // 2 segundos de banner (60fps)
        InitWave(game, game->wave);
        // Mantem isCrazyMode e score
    }

    // ── Descida em fileiras (Dificuldade Progressiva) ───────────────
    game->newRowTimer += dt;
    // O tempo para adicionar uma nova fileira diminui conforme a wave (dificuldade)
    float timeToDrop = 15.0f - (game->wave * 1.0f);
    if (timeToDrop < 5.0f) timeToDrop = 5.0f; // cap em 5 segundos no minimo

    bool blocksReachedBottom = false;

    if (game->newRowTimer >= timeToDrop) {
        game->newRowTimer = 0.0f;

        // Move todos os blocos ativos para baixo 1 fileira
        for (int i = 0; i < MAX_BLOCKS; i++) {
            if (game->blocks[i].active) {
                game->blocks[i].y += (BLOCK_HEIGHT + BLOCK_PADDING);
                // Verifica game over
                if (game->blocks[i].y + game->blocks[i].height >= game->paddle->y - 15.0f) {
                    blocksReachedBottom = true;
                }
            }
        }

        // Adiciona nova fileira de blocos na linha 1 (HUD_HEIGHT)
        // A vida (HP) agora eh aleatoria, escalando com o tempo e pontuacao
        int scaleByScore = game->score / 300;
        int scaleByTime  = (int)(game->playTime / 20.0f);
        
        int minHp = 1 + (scaleByScore + scaleByTime) / 3;
        int maxHp = 2 + (scaleByScore + scaleByTime);
        if (minHp < 1) minHp = 1;
        if (maxHp < minHp) maxHp = minHp;

        for (int col = 0; col < BLOCK_COLS; col++) {
            // Acha um slot vazio
            for (int i = 0; i < MAX_BLOCKS; i++) {
                if (!game->blocks[i].active) {
                    int randHp = GetRandomValue(minHp, maxHp);

                    game->blocks[i].x          = BLOCK_OFFSET_X + col * (BLOCK_WIDTH  + BLOCK_PADDING);
                    game->blocks[i].y          = BLOCK_OFFSET_Y;
                    game->blocks[i].width      = BLOCK_WIDTH;
                    game->blocks[i].height     = BLOCK_HEIGHT;
                    game->blocks[i].hp         = randHp;
                    game->blocks[i].maxHp      = randHp;
                    game->blocks[i].active     = true;
                    game->blocks[i].flashFrames= 0;
                    break;
                }
            }
        }
    }

    if (blocksReachedBottom) {
        game->currentScreen = SCREEN_GAMEOVER;
        PlaySound(sndGameOver);
        StopMusicStream(musCrazyTrain);
        return;
    }

    // ══ POWER-UPS (queda e coleta) ═════════════════════════════════
    for (int i = 0; i < MAX_POWERUPS; i++) {
        PowerUp *pu = &game->powerUps[i];
        if (!pu->active) continue;

        pu->y += pu->speedY;

        // Caiu fora
        if (pu->y > SCREEN_HEIGHT) { pu->active = false; continue; }

        // Colisao com raquete
        Rectangle paddleRect = { game->paddle->x, game->paddle->y,
                                  game->paddle->width, game->paddle->height };
        Rectangle puRect     = { pu->x - 12, pu->y - 12, 24, 24 };

        if (CheckCollisionRecs(puRect, paddleRect)) {
            pu->active = false;

            switch (pu->type) {
                case PU_MULTI_BALL: {
                    // Adiciona 2 bolas clonando a primeira ativa
                    Ball *src = NULL;
                    for (int b = 0; b < game->ballCount; b++)
                        if (game->balls[b].active) { src = &game->balls[b]; break; }
                    if (src) {
                        int toAdd = 2;
                        for (int b = 0; b < game->ballCount && toAdd > 0; b++) {}
                        // Calcula quantas bolas cabem
                        int canAdd = MAX_BALLS - game->ballCount;
                        if (canAdd > 2) canAdd = 2;
                        for (int k = 0; k < canAdd; k++) {
                            float angle = (k == 0) ? 0.4f : -0.4f;
                            float cs = cosf(angle), sn = sinf(angle);
                            float nx = src->speedX * cs - src->speedY * sn;
                            float ny = src->speedX * sn + src->speedY * cs;
                            SpawnBall(game, src->x, src->y, nx, ny);
                            if (game->isCrazyMode)
                                game->balls[game->ballCount - 1].color = ORANGE;
                        }
                    }
                    break;
                }
                case PU_WIDE_PADDLE:
                    if (game->widePaddleTimer <= 0.0f) {
                        game->paddleOriginalW = game->paddle->width;
                        game->paddle->width  *= 1.5f;
                        if (game->paddle->width > SCREEN_WIDTH * 0.5f)
                            game->paddle->width = SCREEN_WIDTH * 0.5f;
                    }
                    game->widePaddleTimer = 10.0f;
                    break;

                case PU_FAST_BALL:
                    for (int b = 0; b < game->ballCount; b++) {
                        if (!game->balls[b].active) continue;
                        game->balls[b].speedX *= 1.3f;
                        game->balls[b].speedY *= 1.3f;
                        // cap de velocidade
                        float spd = sqrtf(game->balls[b].speedX * game->balls[b].speedX +
                                          game->balls[b].speedY * game->balls[b].speedY);
                        if (spd > 20.0f) {
                            game->balls[b].speedX = game->balls[b].speedX / spd * 20.0f;
                            game->balls[b].speedY = game->balls[b].speedY / spd * 20.0f;
                        }
                    }
                    break;

                case PU_SLOW_BALL:
                    for (int b = 0; b < game->ballCount; b++) {
                        if (!game->balls[b].active) continue;
                        game->balls[b].speedX *= 0.7f;
                        game->balls[b].speedY *= 0.7f;
                        // velocidade minima para nao travar
                        float spd = sqrtf(game->balls[b].speedX * game->balls[b].speedX +
                                          game->balls[b].speedY * game->balls[b].speedY);
                        if (spd < 4.0f) {
                            game->balls[b].speedX = game->balls[b].speedX / spd * 4.0f;
                            game->balls[b].speedY = game->balls[b].speedY / spd * 4.0f;
                        }
                    }
                    break;

                case PU_PIERCE:
                    game->pierceTimer = 10.0f;
                    break;

                default: break;
            }
        }
    }
}
