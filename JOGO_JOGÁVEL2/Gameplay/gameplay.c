// gameplay.c
// Inicialização, liberação de memória e física do jogo

#include "gameplay.h"

GameState *InitGame() {
    GameState *game = (GameState *)malloc(sizeof(GameState));

    game->ball = (Ball *)malloc(sizeof(Ball));
    game->ball->radius = 10.0f;
    game->ball->x      = SCREEN_WIDTH  / 2.0f;
    game->ball->y      = SCREEN_HEIGHT / 2.0f;
    game->ball->speedX = 5.0f;
    game->ball->speedY = -5.0f;
    game->ball->color  = RED;

    game->paddle = (Paddle *)malloc(sizeof(Paddle));
    game->paddle->width  = 120.0f;
    game->paddle->height = 15.0f;
    game->paddle->x      = SCREEN_WIDTH / 2.0f - game->paddle->width / 2.0f;
    game->paddle->y      = SCREEN_HEIGHT - 110.0f;
    game->paddle->speed  = 8.0f;
    game->paddle->velocityX = 0.0f;
    game->paddle->legSwing  = 0.0f;
    game->paddle->legVelocity = 0.0f;
    game->paddle->color  = DARKBLUE;

    game->score         = 0;
    game->isCrazyMode   = false;
    game->currentScreen = SCREEN_MENU;
    game->menuSelection = 0;
    game->skinAtual     = SKIN_PADRAO;

    return game;
}

void ResetGame(GameState *game) {
    game->ball->x      = SCREEN_WIDTH  / 2.0f;
    game->ball->y      = SCREEN_HEIGHT / 2.0f;
    game->ball->speedX = 5.0f;
    game->ball->speedY = -5.0f;
    game->ball->color  = RED;

    game->paddle->x      = SCREEN_WIDTH / 2.0f - game->paddle->width / 2.0f;
    game->paddle->y      = SCREEN_HEIGHT - 110.0f;
    game->paddle->velocityX = 0.0f;
    game->paddle->legSwing  = 0.0f;
    game->paddle->legVelocity = 0.0f;
    game->paddle->color  = DARKBLUE;

    game->score         = 0;
    game->isCrazyMode   = false;
    game->currentScreen = SCREEN_MENU;
    game->menuSelection = 0;
    // Nao resetamos a skinAtual pra nao perder a selecao do usuario
}

void FreeGame(GameState *game) {
    if (game != NULL) {
        if (game->ball   != NULL) free(game->ball);
        if (game->paddle != NULL) free(game->paddle);
        free(game); // Libera o struct principal depois dos internos
    }
}

void UpdateGameplay(GameState *game, Sound sndPong, Sound sndGameOver, Music musCrazyTrain) {
    float oldX = game->paddle->x;

    // Movimento da raquete pelo mouse
    Vector2 mouseDelta = GetMouseDelta();
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
        game->paddle->x = GetMouseX() - game->paddle->width / 2.0f;
    } else {
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
            game->paddle->x -= game->paddle->speed;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
            game->paddle->x += game->paddle->speed;
    }

    // Limites da raquete
    if (game->paddle->x < 0)
        game->paddle->x = 0;
    if (game->paddle->x + game->paddle->width > SCREEN_WIDTH)
        game->paddle->x = SCREEN_WIDTH - game->paddle->width;

    // --- FISICA DAS PERNINHAS DO PINTINHO ---
    game->paddle->velocityX = game->paddle->x - oldX;
    
    // As perninhas arrastam na direcao oposta ao movimento
    float targetSwing = -game->paddle->velocityX * 0.8f;
    if (targetSwing > 25.0f) targetSwing = 25.0f;
    if (targetSwing < -25.0f) targetSwing = -25.0f;

    // Fisica de mola (Spring) para dar efeito de gelatina/inercia
    float springForce = (targetSwing - game->paddle->legSwing) * 0.3f;
    game->paddle->legVelocity += springForce;
    game->paddle->legVelocity *= 0.85f; // Amortecimento (Damping)
    game->paddle->legSwing += game->paddle->legVelocity;

    // --- FISICA COM SUB-STEPS (evita tunneling/ghost frames) ---
    // Divide o movimento em N passos menores por frame,
    // checando colisao em cada passo.
    int   steps             = 8;
    float stepX             = game->ball->speedX / (float)steps;
    float stepY             = game->ball->speedY / (float)steps;
    bool  hitPaddleThisFrame = false;

    for (int s = 0; s < steps; s++) {
        game->ball->x += stepX;
        game->ball->y += stepY;

        // Colisao com as paredes laterais
        if ((game->ball->x - game->ball->radius <= 0          && game->ball->speedX < 0) ||
            (game->ball->x + game->ball->radius >= SCREEN_WIDTH && game->ball->speedX > 0)) {
            game->ball->speedX *= -1.0f;
            stepX              *= -1.0f;
        }

        // Colisao com o teto
        if (game->ball->y - game->ball->radius <= 0 && game->ball->speedY < 0) {
            game->ball->speedY *= -1.0f;
            stepY              *= -1.0f;
        }

        // Colisao com a raquete (so processa uma vez por frame)
        if (!hitPaddleThisFrame) {
            Rectangle paddleRect = { game->paddle->x, game->paddle->y,
                                     game->paddle->width, game->paddle->height };
            Vector2   ballCenter = { game->ball->x, game->ball->y };

            if (CheckCollisionCircleRec(ballCenter, game->ball->radius, paddleRect)) {
                if (game->ball->speedY > 0) {
                    hitPaddleThisFrame = true;

                    game->ball->speedY *= -1.0f;

                    // Angulo baseado no ponto de impacto
                    float hitPoint = (game->ball->x - (game->paddle->x + game->paddle->width / 2))
                                   / (game->paddle->width / 2);
                    game->ball->speedX = hitPoint * 7.0f;

                    // Aumenta 1% da velocidade a cada rebatida (cap em 25)
                    game->ball->speedY *= 1.01f;
                    if (game->ball->speedY < -25.0f) game->ball->speedY = -25.0f;

                    // Recalcula o passo vertical apos mudanca de velocidade
                    stepY = game->ball->speedY / (float)steps;

                    game->score++;
                    PlaySound(sndPong);

                    // Ativa o Crazy Mode ao atingir 10 pontos
                    if (game->score >= 10 && !game->isCrazyMode) {
                        game->isCrazyMode    = true;
                        game->ball->color    = ORANGE;
                        PlayMusicStream(musCrazyTrain);
                    }
                }
            }
        }

        // Bola caiu (game over) — sai do loop imediatamente
        if (game->ball->y >= SCREEN_HEIGHT) {
            game->currentScreen = SCREEN_GAMEOVER;
            PlaySound(sndGameOver);
            StopMusicStream(musCrazyTrain);
            break;
        }
    }
}
