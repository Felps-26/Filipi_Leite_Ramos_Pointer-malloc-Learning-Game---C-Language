// skins.h
// Definicoes de tipos e declaracoes das skins da raquete

#ifndef SKINS_H
#define SKINS_H

#include "raylib.h"
#include <math.h>

// Enum com todas as skins disponíveis
typedef enum {
    SKIN_PADRAO,
    SKIN_PATO,
    SKIN_DIABO,
    SKIN_PINTINHO,
    SKIN_ALIEN,
    SKIN_COUNT
} PaddleSkin;

// Estrutura da raquete (definida aqui pois as skins dependem dela)
typedef struct {
    float x, y;
    float width, height;
    float speed;
    float velocityX;
    float legSwing;
    float legVelocity;
    Color color;
} Paddle;

// Desenha a raquete com o visual da skin escolhida e olhos seguindo um ponto
void DrawPaddleSkin(Paddle *paddle, PaddleSkin skin, Vector2 focusPoint);

#endif // SKINS_H
