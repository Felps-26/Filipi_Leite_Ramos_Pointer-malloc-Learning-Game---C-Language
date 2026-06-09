// skins.c
// Implementação do desenho de cada skin da raquete
// Agora com suporte a olhos que seguem um alvo (focusPoint)

#include "skins.h"

// Helper: Calcula o deslocamento da pupila baseado na posicao do alvo
static Vector2 CalcEyeOffset(Vector2 eyeCenter, Vector2 target, float maxOffset) {
    float dx = target.x - eyeCenter.x;
    float dy = target.y - eyeCenter.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist > 0.0f) {
        dx /= dist;
        dy /= dist;
    }
    // A pupila move ate o maxOffset na direcao do alvo
    float offsetMagnitude = fminf(dist * 0.02f, maxOffset);
    return (Vector2){ dx * offsetMagnitude, dy * offsetMagnitude };
}

void DrawPaddleSkin(Paddle *paddle, PaddleSkin skin, Vector2 focusPoint) {
    float midX = paddle->x + paddle->width / 2.0f;
    float topY = paddle->y;
    float botY = paddle->y + paddle->height; // Base da hitbox (15px)

    switch(skin) {

        case SKIN_PADRAO:
            DrawRectangleRec((Rectangle){paddle->x, paddle->y, paddle->width, paddle->height}, paddle->color);
            // Um detalhe de base (tipo um vidro/metal) para dar profundidade
            DrawRectangle((int)paddle->x, (int)botY, (int)paddle->width, 5, LIGHTGRAY);
            break;

        case SKIN_PATO: {
            // Topo (hitbox plana)
            DrawRectangle((int)paddle->x, (int)paddle->y, (int)paddle->width, (int)paddle->height, YELLOW);
            
            // Corpo de pato de borracha (redondo e largo)
            DrawEllipse((int)midX, (int)(botY + 15), paddle->width / 2.0f, 25.0f, YELLOW);
            
            // Asas nas laterais (mais suaves)
            DrawEllipse((int)paddle->x + 10, (int)botY + 10, 15.0f, 10.0f, ORANGE);
            DrawEllipse((int)(paddle->x + paddle->width - 10), (int)botY + 10, 15.0f, 10.0f, ORANGE);

            // Olhos
            Vector2 leftEye  = { midX - 25, topY + 12 };
            Vector2 rightEye = { midX + 25, topY + 12 };
            DrawCircleV(leftEye, 7, WHITE);
            DrawCircleV(rightEye, 7, WHITE);

            Vector2 offsetL = CalcEyeOffset(leftEye, focusPoint, 3.0f);
            Vector2 offsetR = CalcEyeOffset(rightEye, focusPoint, 3.0f);
            DrawCircleV((Vector2){leftEye.x + offsetL.x, leftEye.y + offsetL.y}, 3, BLACK);
            DrawCircleV((Vector2){rightEye.x + offsetR.x, rightEye.y + offsetR.y}, 3, BLACK);

            // Bico laranja de pato (gordinho)
            DrawEllipse((int)midX, (int)botY + 5, 20.0f, 12.0f, ORANGE);
            DrawLineEx((Vector2){midX - 15, botY + 5}, (Vector2){midX + 15, botY + 5}, 2, RED); // Sorriso do bico
        } break;

        case SKIN_DIABO: {
            // Topo plano vermelho
            DrawRectangle((int)paddle->x, (int)paddle->y, (int)paddle->width, (int)paddle->height, RED);
            // Corpo descendo num escudo/triangulo infernal
            DrawRectangle((int)paddle->x, (int)botY, (int)paddle->width, 30, RED);
            DrawTriangle(
                (Vector2){paddle->x, botY + 30},
                (Vector2){midX, botY + 65},
                (Vector2){paddle->x + paddle->width, botY + 30},
                RED);

            // Chifres saindo pros LADOS
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
            // Aqui nao e' circulo, mas podemos mover o triangulo central
            Vector2 leftEyeCenter  = { midX - 20, topY + 10 };
            Vector2 rightEyeCenter = { midX + 20, topY + 10 };
            Vector2 offsetL = CalcEyeOffset(leftEyeCenter, focusPoint, 4.0f);
            Vector2 offsetR = CalcEyeOffset(rightEyeCenter, focusPoint, 4.0f);

            // Fundo amarelo (fixo)
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
                
            // Pupila preta que se move
            DrawCircleV((Vector2){leftEyeCenter.x + offsetL.x, leftEyeCenter.y + offsetL.y}, 3, BLACK);
            DrawCircleV((Vector2){rightEyeCenter.x + offsetR.x, rightEyeCenter.y + offsetR.y}, 3, BLACK);

            // Boca de dentes de serra
            for(int i = -20; i < 20; i += 10) {
                DrawTriangle(
                    (Vector2){midX + i, botY + 20},
                    (Vector2){midX + i + 5, botY + 30},
                    (Vector2){midX + i + 10, botY + 20},
                    BLACK);
            }
        } break;

        case SKIN_PINTINHO: {
            Color amarelo = (Color){ 255, 240, 100, 255 };
            // Topo plano amarelo
            DrawRectangle((int)paddle->x, (int)paddle->y, (int)paddle->width, (int)paddle->height, amarelo);
            
            // Corpo bem redondo
            DrawEllipse((int)midX, (int)botY + 10, paddle->width / 2.0f, 30.0f, amarelo);
            
            float bodyBottom = botY + 35; // Fim da barriga
            float swing = paddle->legSwing;
            Color legColor = (Color){ 255, 160, 0, 255 };

            // Pernas
            float lX = midX - 25.0f;
            DrawLineEx((Vector2){ lX,              bodyBottom      },
                       (Vector2){ lX + swing,      bodyBottom + 15 }, 4, legColor);
            DrawLineEx((Vector2){ lX + swing,      bodyBottom + 15 },
                       (Vector2){ lX + swing + 8,  bodyBottom + 15 }, 4, legColor);

            float rX = midX + 25.0f;
            DrawLineEx((Vector2){ rX,              bodyBottom      },
                       (Vector2){ rX + swing,      bodyBottom + 15 }, 4, legColor);
            DrawLineEx((Vector2){ rX + swing,      bodyBottom + 15 },
                       (Vector2){ rX + swing + 8,  bodyBottom + 15 }, 4, legColor);

            // Olhos dinamicos
            Vector2 leftEye  = { midX - 15, topY + 12 };
            Vector2 rightEye = { midX + 15, topY + 12 };
            DrawCircleV(leftEye, 9, WHITE);
            DrawCircleV(rightEye, 9, WHITE);

            Vector2 offsetL = CalcEyeOffset(leftEye, focusPoint, 4.0f);
            Vector2 offsetR = CalcEyeOffset(rightEye, focusPoint, 4.0f);
            DrawCircleV((Vector2){leftEye.x + offsetL.x, leftEye.y + offsetL.y}, 4, BLACK);
            DrawCircleV((Vector2){rightEye.x + offsetR.x, rightEye.y + offsetR.y}, 4, BLACK);

            // Bico laranja
            DrawTriangle(
                (Vector2){midX - 10, botY},
                (Vector2){midX,      botY + 15},
                (Vector2){midX + 10, botY},
                ORANGE);

        } break;

        case SKIN_ALIEN: {
            // Topo plano verde (hitbox)
            DrawRectangle((int)paddle->x, (int)paddle->y, (int)paddle->width, (int)paddle->height, GREEN);
            
            // Corpo formato de disco voador
            DrawEllipse((int)midX, (int)botY + 5, paddle->width / 2.0f + 10.0f, 20.0f, GREEN);
            DrawEllipse((int)midX, (int)botY + 5, paddle->width / 2.0f - 10.0f, 10.0f, DARKGREEN); // Detalhe
            
            // Vidro do OVNI
            DrawEllipse((int)midX, (int)topY + 5, 30.0f, 20.0f, Fade(SKYBLUE, 0.8f));
            
            // Homenzinho verde lá dentro (mini olho seguindo a bola)
            Vector2 eyeCenter = { midX, topY + 5 };
            Vector2 offset = CalcEyeOffset(eyeCenter, focusPoint, 8.0f);
            DrawCircleV((Vector2){eyeCenter.x + offset.x, eyeCenter.y + offset.y}, 5, BLACK);
            
            // Tentaculos
            float swingA = paddle->legSwing * 0.7f;
            for(int i = -40; i <= 40; i += 20) {
                float startX = midX + i;
                float startY = botY + 20;
                
                Vector2 p1 = {startX, startY};
                Vector2 p2 = {startX + swingA * 0.5f, startY + 15};
                Vector2 p3 = {startX + swingA * 1.5f + sinf((float)GetTime()*5+i)*10, startY + 30};
                
                DrawLineEx(p1, p2, 4, PURPLE);
                DrawLineEx(p2, p3, 4, PURPLE);
                DrawCircleV(p3, 4, PURPLE);
            }
        } break;

        default: break;
    }
}
