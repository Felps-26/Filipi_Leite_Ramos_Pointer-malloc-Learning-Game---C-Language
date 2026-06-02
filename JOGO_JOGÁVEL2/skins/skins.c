// skins.c
// Implementação do desenho de cada skin da raquete

#include "skins.h"
#include <math.h>

void DrawPaddleSkin(Paddle *paddle, PaddleSkin skin) {
    float midX = paddle->x + paddle->width / 2.0f;
    float topY = paddle->y;
    float botY = paddle->y + paddle->height; // Base da hitbox (15px)

    switch(skin) {

        case SKIN_PADRAO:
            DrawRectangleRec((Rectangle){paddle->x, paddle->y, paddle->width, paddle->height}, paddle->color);
            // Um detalhe de base (tipo um vidro/metal) para dar profundidade
            DrawRectangle(paddle->x, botY, paddle->width, 5, LIGHTGRAY);
            break;

        case SKIN_PATO:
            // Topo (hitbox plana)
            DrawRectangle(paddle->x, paddle->y, paddle->width, paddle->height, YELLOW);
            
            // Corpo de pato de borracha (redondo e largo)
            DrawEllipse(midX, botY + 15, paddle->width / 2.0f, 25, YELLOW);
            
            // Asas nas laterais (mais suaves)
            DrawEllipse(paddle->x + 10, botY + 10, 15, 10, ORANGE);
            DrawEllipse(paddle->x + paddle->width - 10, botY + 10, 15, 10, ORANGE);

            // Olhos (movidos mais para cima para nao ter testao)
            DrawCircle(midX - 25, topY + 12, 7, WHITE);
            DrawCircle(midX + 25, topY + 12, 7, WHITE);
            DrawCircle(midX - 25, topY + 12, 3, BLACK);
            DrawCircle(midX + 25, topY + 12, 3, BLACK);

            // Bico laranja de pato (gordinho)
            DrawEllipse(midX, botY + 5, 20, 12, ORANGE);
            DrawLineEx((Vector2){midX - 15, botY + 5}, (Vector2){midX + 15, botY + 5}, 2, RED); // Sorriso do bico
            break;

        case SKIN_DIABO:
            // Topo plano vermelho
            DrawRectangle(paddle->x, paddle->y, paddle->width, paddle->height, RED);
            // Corpo descendo num escudo/triangulo infernal
            DrawRectangle(paddle->x, botY, paddle->width, 30, RED);
            DrawTriangle(
                (Vector2){paddle->x, botY + 30},
                (Vector2){midX, botY + 65},
                (Vector2){paddle->x + paddle->width, botY + 30},
                RED);

            // Chifres saindo pros LADOS (para não atrapalhar a hitbox reta em cima)
            DrawTriangle(
                (Vector2){paddle->x, topY},
                (Vector2){paddle->x - 20, topY + 5},
                (Vector2){paddle->x, botY},
                BLACK);
            DrawTriangle(
                (Vector2){paddle->x + paddle->width, topY},
                (Vector2){paddle->x + paddle->width, botY},
                (Vector2){paddle->x + paddle->width + 20, topY + 5},
                BLACK);

            // Olhos furiosos (Triangulos brancos e amarelos)
            DrawTriangle(
                (Vector2){midX - 30, topY + 5},
                (Vector2){midX - 30, topY + 15},
                (Vector2){midX - 10, topY + 15},
                YELLOW);
            DrawTriangle(
                (Vector2){midX + 30, topY + 5},
                (Vector2){midX + 10, topY + 15},
                (Vector2){midX + 30, topY + 15},
                YELLOW);
                
            // Boca de dentes de serra
            for(int i = -20; i < 20; i += 10) {
                DrawTriangle(
                    (Vector2){midX + i, botY + 20},
                    (Vector2){midX + i + 5, botY + 30},
                    (Vector2){midX + i + 10, botY + 20},
                    BLACK);
            }
            break;

        case SKIN_PINTINHO: {
            Color amarelo = (Color){ 255, 240, 100, 255 };
            // Topo plano amarelo
            DrawRectangle(paddle->x, paddle->y, paddle->width, paddle->height, amarelo);
            
            // Corpo bem redondo
            DrawEllipse(midX, botY + 10, paddle->width / 2.0f, 30, amarelo);
            
            float bodyBottom = botY + 35; // Fim da barriga
            float swing = paddle->legSwing;
            Color legColor = (Color){ 255, 160, 0, 255 };

            // Perna esquerda com joelho elástico
            float lX = midX - 25.0f;
            DrawLineEx((Vector2){ lX,              bodyBottom      },
                       (Vector2){ lX + swing,      bodyBottom + 15 }, 4, legColor);
            DrawLineEx((Vector2){ lX + swing,      bodyBottom + 15 },
                       (Vector2){ lX + swing + 8,  bodyBottom + 15 }, 4, legColor);

            // Perna direita com joelho elástico
            float rX = midX + 25.0f;
            DrawLineEx((Vector2){ rX,              bodyBottom      },
                       (Vector2){ rX + swing,      bodyBottom + 15 }, 4, legColor);
            DrawLineEx((Vector2){ rX + swing,      bodyBottom + 15 },
                       (Vector2){ rX + swing + 8,  bodyBottom + 15 }, 4, legColor);

            // Olhos (movidos para o topo da cabeça para remover o testão)
            DrawCircle(midX - 15, topY + 12, 9, WHITE);
            DrawCircle(midX + 15, topY + 12, 9, WHITE);
            DrawCircle(midX - 15, topY + 12, 4, BLACK);
            DrawCircle(midX + 15, topY + 12, 4, BLACK);

            // Bico laranja (logo abaixo dos olhos, sentido anti-horário)
            DrawTriangle(
                (Vector2){midX - 10, botY},
                (Vector2){midX,      botY + 15},
                (Vector2){midX + 10, botY},
                ORANGE);

        } break;

        case SKIN_ALIEN:
            // Topo plano verde (hitbox)
            DrawRectangle(paddle->x, paddle->y, paddle->width, paddle->height, GREEN);
            
            // Corpo formato de disco voador (muito menos quadrado, bem oval)
            DrawEllipse(midX, botY + 5, paddle->width / 2.0f + 10.0f, 20, GREEN);
            DrawEllipse(midX, botY + 5, paddle->width / 2.0f - 10.0f, 10, DARKGREEN); // Detalhe da nave
            
            // Vidro do OVNI no topo (subindo um pouco na hitbox)
            DrawEllipse(midX, topY + 5, 30, 20, Fade(SKYBLUE, 0.8f));
            // Homenzinho verde lá dentro (mini olho)
            DrawCircle(midX, topY + 5, 5, BLACK);
            
            // Tentaculos pendurados com FÍSICA APLICADA aprimorada
            float swingA = paddle->legSwing * 0.7f; // Intensidade da inercia no tentaculo
            for(int i = -40; i <= 40; i += 20) {
                float startX = midX + i;
                float startY = botY + 20; // Sai de baixo da nave
                
                // Ponto 1 (raiz), Ponto 2 (cotovelo que arrasta um pouco), Ponto 3 (ponta que arrasta MUITO)
                Vector2 p1 = {startX, startY};
                Vector2 p2 = {startX + swingA * 0.5f, startY + 15};
                Vector2 p3 = {startX + swingA * 1.5f + sinf(GetTime()*5+i)*10, startY + 30};
                
                DrawLineEx(p1, p2, 4, PURPLE);
                DrawLineEx(p2, p3, 4, PURPLE);
                DrawCircleV(p3, 4, PURPLE); // Ponta arredondada do tentáculo
            }
            break;

        default: break;
    }
}
