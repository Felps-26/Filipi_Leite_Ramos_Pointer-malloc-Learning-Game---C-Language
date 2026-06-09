// gameplay.c
// Implementação da física, movimentação, IA, colisão e persistência.
#include "gameplay.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

// ============================================================================
// AUXILIAR: EMISSOR DE PARTÍCULAS
// ============================================================================
void SpawnParticle(GameState *game, Vector2 position, Vector2 velocity, Color color, float size, float lifeTime)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!game->particles[i].active)
        {
            game->particles[i].position = position;
            game->particles[i].velocity = velocity;
            game->particles[i].color = color;
            game->particles[i].size = size;
            game->particles[i].lifeTime = lifeTime;
            game->particles[i].maxLifeTime = lifeTime;
            game->particles[i].active = true;
            break;
        }
    }
}

// Emite uma explosão radial de partículas
static void SpawnParticleExplosion(GameState *game, Vector2 pos, Color col, int count, float minSpeed, float maxSpeed, float size, float life)
{
    for (int i = 0; i < count; i++)
    {
        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
        float speed = (float)GetRandomValue((int)(minSpeed * 10), (int)(maxSpeed * 10)) / 10.0f;
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        SpawnParticle(game, pos, vel, col, size, life + (float)GetRandomValue(-2, 2)/10.0f);
    }
}

// ============================================================================
// AUXILIAR: SPAWN DE POWER-UPS
// ============================================================================
void SpawnPowerUpAt(GameState *game, Vector2 position, int forcedType)
{
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        if (!game->powerUps[i].active)
        {
            game->powerUps[i].position = position;
            game->powerUps[i].type = (forcedType >= 0) ? (PowerUpType)forcedType : (PowerUpType)GetRandomValue(0, 3);
            game->powerUps[i].active = true;
            game->powerUps[i].pulseTimer = 0.0f;
            break;
        }
    }
}

// ============================================================================
// AUXILIAR: SPAWN DE PROJÉTEIS
// ============================================================================
void SpawnProjectile(GameState *game, Vector2 pos, Vector2 target, ProjectileType type, int dmg)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!game->projectiles[i].active)
        {
            game->projectiles[i].position = pos;
            Vector2 dir = Vector2Normalize(Vector2Subtract(target, pos));
            float speed = 300.0f;
            if (type == PROJ_ACID_ARC) speed = 250.0f;
            if (type == PROJ_VOID_BOLT) speed = 400.0f;
            
            game->projectiles[i].velocity = Vector2Scale(dir, speed);
            game->projectiles[i].active = true;
            game->projectiles[i].type = type;
            game->projectiles[i].damage = dmg;
            game->projectiles[i].hitbox = (Rectangle){ pos.x - 10, pos.y - 10, 20, 20 };
            game->projectiles[i].currentFrame = 0;
            game->projectiles[i].frameTimer = 0.0f;
            break;
        }
    }
}

// ============================================================================
// INICIALIZAÇÃO DO JOGO
// ============================================================================
void InitGame(GameState *game)
{
    // Preserva o nome do jogador e skin se ja estiver definido
    char tempName[16] = "";
    int tempSkin = 0;
    if (game != NULL)
    {
        snprintf(tempName, sizeof(tempName), "%s", game->player.name);
        tempSkin = game->player.activeSkin;
    }

    memset(game, 0, sizeof(GameState));

    if (tempName[0] != '\0')
    {
        snprintf(game->player.name, sizeof(game->player.name), "%s", tempName);
    }
    else
    {
        strcpy(game->player.name, "HERO");
    }
    game->player.activeSkin = tempSkin;

    // Jogador inicial
    game->player.position = (Vector2){ MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f };
    game->player.speed = 280.0f;
    game->player.hp = 100;
    game->player.maxHp = 100;
    game->player.score = 0;
    game->player.level = 1;
    game->player.xp = 0;
    game->player.xpNeeded = 100;
    game->player.attackPower = 25;
    game->player.attackCooldown = 0.0f;

    // Sistema
    game->wave = 1;
    game->totalEnemiesKilled = 0;
    game->timeElapsed = 0.0f;
    game->screenShake = 0.0f;
    game->slashAnimTimer = 0.0f;

    // Câmera
    game->camera.target = game->player.position;
    game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    game->camera.rotation = 0.0f;
    game->camera.zoom = 1.0f;

    // Limpa vetores
    for (int i = 0; i < MAX_ENEMIES; i++) game->enemies[i].active = false;
    for (int i = 0; i < MAX_POWERUPS; i++) game->powerUps[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) game->particles[i].active = false;

    // Spawna os inimigos da primeira onda
    StartNextWave(game);
}

// ============================================================================
// PRÓXIMA ONDA DE INIMIGOS
// ============================================================================
void StartNextWave(GameState *game)
{
    // Aumenta quantidade a cada onda
    int numEnemies = 8 + game->wave * 4;
    if (numEnemies > MAX_ENEMIES) numEnemies = MAX_ENEMIES;

    game->enemiesRemaining = numEnemies;

    // Spawna inimigos longe do jogador (distância mínima de 450 px)
    for (int i = 0; i < numEnemies; i++)
    {
        Vector2 spawnPos;
        float distance = 0.0f;
        
        do
        {
            spawnPos.x = (float)GetRandomValue(100, MAP_WIDTH - 100);
            spawnPos.y = (float)GetRandomValue(100, MAP_HEIGHT - 100);
            distance = Vector2Distance(game->player.position, spawnPos);
        } while (distance < 450.0f);

        game->enemies[i].position = spawnPos;
        game->enemies[i].active = true;

        // Determina tipo e dificuldade do inimigo
        int randVal = GetRandomValue(0, 100);
        if (randVal < 70 || game->wave == 1)
        {
            // Inimigo Comum (Patrulha / Persegue)
            game->enemies[i].type = 0;
            game->enemies[i].tier = TIER_1;
            game->enemies[i].maxHp = 30 + game->wave * 10;
            game->enemies[i].hp = game->enemies[i].maxHp;
            game->enemies[i].speed = 140.0f + GetRandomValue(-20, 20);
        }
        else if (randVal < 90)
        {
            // Inimigo Rápido
            game->enemies[i].type = 1;
            game->enemies[i].tier = TIER_2;
            game->enemies[i].isRanged = true;
            game->enemies[i].maxHp = 20 + game->wave * 5;
            game->enemies[i].hp = game->enemies[i].maxHp;
            game->enemies[i].speed = 210.0f + GetRandomValue(-15, 15);
        }
        else
        {
            // Inimigo Elite (Tamanho maior, muita vida)
            game->enemies[i].type = 2;
            if (game->wave >= 5 && i == 0) {
                game->enemies[i].tier = TIER_3_BOSS;
                game->enemies[i].maxHp = 1000;
            } else {
                game->enemies[i].tier = TIER_3;
                game->enemies[i].maxHp = 80 + game->wave * 25;
            }
            game->enemies[i].hp = game->enemies[i].maxHp;
            game->enemies[i].speed = 90.0f + GetRandomValue(-10, 10);
            game->enemies[i].isRanged = true;
        }

        game->enemies[i].state = IDLE;
        game->enemies[i].patrolTarget = spawnPos;
        game->enemies[i].patrolTimer = (float)GetRandomValue(2, 5);
    }

    // Garante que existam alguns power-ups espalhados no mapa no início da onda
    int powerUpsCount = 4 + game->wave;
    if (powerUpsCount > 10) powerUpsCount = 10;
    for (int i = 0; i < powerUpsCount; i++)
    {
        Vector2 itemPos = {
            (float)GetRandomValue(200, MAP_WIDTH - 200),
            (float)GetRandomValue(200, MAP_HEIGHT - 200)
        };
        SpawnPowerUpAt(game, itemPos, -1); // Tipo aleatório
    }

    // Partículas azuis de invocação de nova onda
    for (int p = 0; p < 30; p++)
    {
        Vector2 vel = { (float)GetRandomValue(-150, 150), (float)GetRandomValue(-150, 150) };
        SpawnParticle(game, game->player.position, vel, SKYBLUE, 6.0f, 1.2f);
    }
}

// ============================================================================
// LOGICA DE COMBATE: ATAQUE DO JOGADOR
// ============================================================================
void PlayerAttack(GameState *game)
{
    if (game->player.attackCooldown > 0.0f) return;

    // Define cooldown base
    game->player.attackCooldown = 0.35f;

    // Configura animação de ataque (Slash)
    game->slashAnimTimer = 0.22f;
    game->slashAnimPos = game->player.position;
    game->slashAnimRadius = 140.0f;

    // Determina força do ataque
    int dano = game->player.attackPower;
    if (game->player.attackBoostTimer > 0.0f) dano *= 2; // Buff de ataque dobra dano

    // Efeitos visuais: explosão de partículas de slash (brancas e azul claro)
    SpawnParticleExplosion(game, game->player.position, LIGHTGRAY, 12, 100.0f, 250.0f, 4.0f, 0.4f);
    SpawnParticleExplosion(game, game->player.position, SKYBLUE, 8, 150.0f, 300.0f, 3.5f, 0.35f);

    // Câmera dá uma leve chacoalhada no ataque
    game->screenShake = 0.25f;

    // Verifica colisão com inimigos na área do ataque
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!game->enemies[i].active) continue;

        float dist = Vector2Distance(game->player.position, game->enemies[i].position);
        if (dist <= game->slashAnimRadius)
        {
            // Acertou! Causa dano
            game->enemies[i].hp -= dano;

            // Empurrão (Knockback) na direção oposta ao jogador
            Vector2 knockbackDir = Vector2Subtract(game->enemies[i].position, game->player.position);
            if (knockbackDir.x == 0.0f && knockbackDir.y == 0.0f) knockbackDir = (Vector2){ 1.0f, 0.0f };
            knockbackDir = Vector2Normalize(knockbackDir);
            
            // Empurra o inimigo a uma distância segura
            game->enemies[i].position = Vector2Add(game->enemies[i].position, Vector2Scale(knockbackDir, 55.0f));

            // Partículas de sangue/dano no local do inimigo
            Color hitColor = (game->enemies[i].type == 2) ? MAROON : RED;
            SpawnParticleExplosion(game, game->enemies[i].position, hitColor, 15, 80.0f, 180.0f, 3.0f, 0.5f);

            // Se o inimigo morreu
            if (game->enemies[i].hp <= 0)
            {
                game->enemies[i].state = DEATH;
                game->enemies[i].spriteRow = 4;
                game->enemies[i].currentFrame = 0;
                game->enemies[i].active = false; // Em um sistema ideal, a animação terminaria antes de desativar
                game->enemiesRemaining--;
                game->totalEnemiesKilled++;

                // Aumenta score
                int xpGanho = 20 * (game->enemies[i].type + 1);
                int scoreGanho = 100 * (game->enemies[i].type + 1);
                game->player.score += scoreGanho;
                game->player.xp += xpGanho;

                // Partículas de morte e ganho de xp (amarelo/gold flutuante)
                SpawnParticleExplosion(game, game->enemies[i].position, GOLD, 20, 50.0f, 150.0f, 4.0f, 0.7f);

                // Chance de drop de PowerUp (25%)
                if (GetRandomValue(0, 100) < 25)
                {
                    SpawnPowerUpAt(game, game->enemies[i].position, -1);
                }

                // Verifica se eliminou todos
                if (game->enemiesRemaining <= 0)
                {
                    game->wave++;
                    if (game->wave > 5)
                    {
                        game->currentScreen = SCREEN_VICTORY;
                    }
                    else
                    {
                        StartNextWave(game);
                    }
                    return;
                }
            }
        }
    }
}

// ============================================================================
// CICLO PRINCIPAL DE ATUALIZAÇÃO DA LOGICA
// ============================================================================
void UpdateGameplay(GameState *game, float delta)
{
    game->timeElapsed += delta;

    // Decaimento do screen shake
    if (game->screenShake > 0.0f)
    {
        game->screenShake -= delta * 1.5f;
        if (game->screenShake < 0.0f) game->screenShake = 0.0f;
    }

    // Decaimento da animação de slash
    if (game->slashAnimTimer > 0.0f)
    {
        game->slashAnimTimer -= delta;
    }

    // ------------------------------------------------------------------------
    // 1. ATUALIZA TIMERS E STATS DO JOGADOR
    // ------------------------------------------------------------------------
    if (game->player.speedTimer > 0.0f) game->player.speedTimer -= delta;
    if (game->player.shieldTimer > 0.0f) game->player.shieldTimer -= delta;
    if (game->player.attackBoostTimer > 0.0f) game->player.attackBoostTimer -= delta;
    if (game->player.attackCooldown > 0.0f) game->player.attackCooldown -= delta;

    // ------------------------------------------------------------------------
    // 2. MOVIMENTAÇÃO DO JOGADOR
    // ------------------------------------------------------------------------
    float currentSpeed = game->player.speed;
    if (game->player.speedTimer > 0.0f) currentSpeed *= 1.6f; // Buff de velocidade

    Vector2 moveDir = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveDir.x += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  moveDir.x -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  moveDir.y += 1.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    moveDir.y -= 1.0f;

    if (moveDir.x != 0.0f || moveDir.y != 0.0f)
    {
        moveDir = Vector2Normalize(moveDir);
        game->player.position = Vector2Add(
            game->player.position,
            Vector2Scale(moveDir, currentSpeed * delta)
        );

        // Emite fumaça suave de movimento
        if (GetRandomValue(0, 10) == 0)
        {
            Vector2 smokeVel = { -moveDir.x * 30.0f, -moveDir.y * 30.0f };
            Color pCol = (game->player.speedTimer > 0.0f) ? YELLOW : LIGHTGRAY;
            SpawnParticle(game, game->player.position, smokeVel, Fade(pCol, 0.4f), 5.0f, 0.5f);
        }
    }

    // Player Animations
    game->player.frameTimer += delta;
    if (game->player.frameTimer > 0.15f) {
        game->player.currentFrame = (game->player.currentFrame + 1) % 4;
        game->player.frameTimer = 0.0f;
    }

    if (game->player.attackCooldown > 0.0f) {
        game->player.spriteRow = 2; // Attack/Jump
    } else if (moveDir.x != 0.0f || moveDir.y != 0.0f) {
        game->player.spriteRow = 1; // Move
    } else {
        game->player.spriteRow = 0; // Idle
    }

    // Limites do mapa para o jogador
    float playerRadius = 20.0f;
    if (game->player.position.x < playerRadius) game->player.position.x = playerRadius;
    if (game->player.position.x > MAP_WIDTH - playerRadius) game->player.position.x = MAP_WIDTH - playerRadius;
    if (game->player.position.y < playerRadius) game->player.position.y = playerRadius;
    if (game->player.position.y > MAP_HEIGHT - playerRadius) game->player.position.y = MAP_HEIGHT - playerRadius;

    // ------------------------------------------------------------------------
    // 3. ENTRADA DE COMBATE
    // ------------------------------------------------------------------------
    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        PlayerAttack(game);
    }

    // Entrada de combate e fluxo de save serao tratados no rpg.c para capturar a screenshot

    // ------------------------------------------------------------------------
    // 4. ATUALIZA PARTÍCULAS
    // ------------------------------------------------------------------------
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (game->particles[i].active)
        {
            game->particles[i].position = Vector2Add(
                game->particles[i].position,
                Vector2Scale(game->particles[i].velocity, delta)
            );
            game->particles[i].lifeTime -= delta;
            if (game->particles[i].lifeTime <= 0.0f)
            {
                game->particles[i].active = false;
            }
        }
    }

    // ------------------------------------------------------------------------
    // 5. ATUALIZA POWER-UPS (COLETA)
    // ------------------------------------------------------------------------
    for (int i = 0; i < MAX_POWERUPS; i++)
    {
        if (game->powerUps[i].active)
        {
            game->powerUps[i].pulseTimer += delta;

            // Distância jogador -> item
            float dist = Vector2Distance(game->player.position, game->powerUps[i].position);
            if (dist < 35.0f) // Colidiu/Coletou
            {
                game->powerUps[i].active = false;
                game->player.score += 50;

                // Efeito radial de coleta
                SpawnParticleExplosion(game, game->powerUps[i].position, YELLOW, 15, 60.0f, 160.0f, 4.5f, 0.6f);

                // Aplica efeitos baseados no tipo do Power-up
                switch (game->powerUps[i].type)
                {
                    case HP_RECOVERY:
                        game->player.hp += 35;
                        if (game->player.hp > game->player.maxHp) game->player.hp = game->player.maxHp;
                        // Partículas verdes para cura
                        SpawnParticleExplosion(game, game->player.position, GREEN, 15, 40.0f, 100.0f, 4.0f, 0.8f);
                        break;
                    case SPEED_BOOST:
                        game->player.speedTimer = 8.0f; // 8 segundos
                        break;
                    case SHIELD:
                        game->player.shieldTimer = 7.0f; // 7 segundos
                        break;
                    case ATTACK_BOOST:
                        game->player.attackBoostTimer = 8.0f; // 8 segundos
                        break;
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    // 6. ATUALIZA INIMIGOS E INTELIGÊNCIA ARTIFICIAL (IA)
    // ------------------------------------------------------------------------
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!game->enemies[i].active) continue;

        Enemy *enemy = &game->enemies[i];
        
        // Atualiza animação
        enemy->frameTimer += delta;
        if (enemy->frameTimer > 0.15f) {
            enemy->currentFrame = (enemy->currentFrame + 1) % 4;
            enemy->frameTimer = 0.0f;
        }

        float distToPlayer = Vector2Distance(game->player.position, enemy->position);

        // State Machine Update
        if (enemy->state == HURT) {
            enemy->cooldownTimer -= delta;
            if (enemy->cooldownTimer <= 0.0f) enemy->state = IDLE;
        }
        else if (enemy->state == ATTACK) {
            enemy->chargeTimer -= delta;
            if (enemy->chargeTimer <= 0.0f) {
                // Atira
                ProjectileType ptype = PROJ_ACID_ARC;
                if (enemy->tier == TIER_2) ptype = PROJ_BULLET_SPREAD;
                if (enemy->tier >= TIER_3) ptype = PROJ_VOID_BOLT;
                
                int dmg = 10 + enemy->tier * 5;
                SpawnProjectile(game, enemy->position, game->player.position, ptype, dmg);
                if (enemy->tier == TIER_3_BOSS) {
                    Vector2 off1 = { game->player.position.x + 100, game->player.position.y };
                    Vector2 off2 = { game->player.position.x - 100, game->player.position.y };
                    SpawnProjectile(game, enemy->position, off1, ptype, dmg);
                    SpawnProjectile(game, enemy->position, off2, ptype, dmg);
                }
                
                enemy->state = IDLE;
                enemy->cooldownTimer = 1.2f; // cooldown
            }
        }
        else if (enemy->isRanged && distToPlayer < 400.0f && enemy->cooldownTimer <= 0.0f) {
            enemy->state = ATTACK;
            enemy->chargeTimer = 0.6f;
        }
        else if (distToPlayer < 450.0f) {
            enemy->state = AGGRO;
        }
        else {
            enemy->state = IDLE;
            enemy->patrolTimer -= delta;
        }
        
        if (enemy->cooldownTimer > 0.0f && enemy->state != HURT && enemy->state != ATTACK) {
            enemy->cooldownTimer -= delta;
        }

        // Definindo linha de sprite
        if (enemy->state == IDLE) enemy->spriteRow = 0;
        else if (enemy->state == AGGRO) enemy->spriteRow = 1;
        else if (enemy->state == ATTACK) enemy->spriteRow = 2;
        else if (enemy->state == HURT) enemy->spriteRow = 3;
        else if (enemy->state == DEATH) enemy->spriteRow = 4;

        // Ações de Movimentação
        if (enemy->state == AGGRO && !enemy->isRanged)
        {
            Vector2 chaseDir = Vector2Subtract(game->player.position, enemy->position);
            chaseDir = Vector2Normalize(chaseDir);
            float chaseMult = (enemy->tier == TIER_2) ? 1.25f : 1.05f;
            enemy->position = Vector2Add(enemy->position, Vector2Scale(chaseDir, enemy->speed * chaseMult * delta));
        }
        else if (enemy->state == IDLE)
        {
            float distToTarget = Vector2Distance(enemy->position, enemy->patrolTarget);
            if (distToTarget < 15.0f || enemy->patrolTimer <= 0.0f)
            {
                float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                float radius = (float)GetRandomValue(100, 300);
                enemy->patrolTarget.x = enemy->position.x + cosf(angle) * radius;
                enemy->patrolTarget.y = enemy->position.y + sinf(angle) * radius;
                enemy->patrolTimer = (float)GetRandomValue(3, 7);
            }
            Vector2 patrolDir = Vector2Normalize(Vector2Subtract(enemy->patrolTarget, enemy->position));
            enemy->position = Vector2Add(enemy->position, Vector2Scale(patrolDir, enemy->speed * delta));
        }

        // Mantém inimigos nos limites do mapa
        float enemyRadius = (enemy->type == 2) ? 35.0f : 20.0f;
        if (enemy->position.x < enemyRadius) enemy->position.x = enemyRadius;
        if (enemy->position.x > MAP_WIDTH - enemyRadius) enemy->position.x = MAP_WIDTH - enemyRadius;
        if (enemy->position.y < enemyRadius) enemy->position.y = enemyRadius;
        if (enemy->position.y > MAP_HEIGHT - enemyRadius) enemy->position.y = MAP_HEIGHT - enemyRadius;

        // --------------------------------------------------------------------
        // COLISÃO: DANO NO JOGADOR
        // --------------------------------------------------------------------
        float colRange = (enemy->type == 2) ? 45.0f : 30.0f;
        if (distToPlayer < colRange)
        {
            // Se o escudo estiver ativo, absorve o dano e cria faíscas azuis
            if (game->player.shieldTimer > 0.0f)
            {
                SpawnParticleExplosion(game, game->player.position, SKYBLUE, 10, 80.0f, 150.0f, 3.0f, 0.4f);
            }
            else
            {
                // Causa dano
                int dmgBase = 8;
                if (enemy->type == 1) dmgBase = 12; // Rápido dói mais
                if (enemy->type == 2) dmgBase = 22; // Elite causa dano forte
                
                game->player.hp -= dmgBase;
                game->screenShake = 0.4f;

                // Partículas vermelhas de ferimento
                SpawnParticleExplosion(game, game->player.position, RED, 12, 50.0f, 130.0f, 3.5f, 0.5f);

                // Checa Game Over
                if (game->player.hp <= 0)
                {
                    game->player.hp = 0;
                    game->currentScreen = SCREEN_GAMEOVER;
                    return;
                }
            }

            // Repele o inimigo ligeiramente para trás após causar o dano
            Vector2 pushDir = Vector2Subtract(enemy->position, game->player.position);
            if (pushDir.x == 0.0f && pushDir.y == 0.0f) pushDir = (Vector2){ 0.0f, 1.0f };
            pushDir = Vector2Normalize(pushDir);
            enemy->position = Vector2Add(enemy->position, Vector2Scale(pushDir, 50.0f));
            enemy->state = HURT;
            enemy->cooldownTimer = 0.5f; // stun duration
        }
    }

    // ------------------------------------------------------------------------
    // 6.5 ATUALIZA PROJÉTEIS
    // ------------------------------------------------------------------------
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (game->projectiles[i].active)
        {
            game->projectiles[i].position = Vector2Add(game->projectiles[i].position, Vector2Scale(game->projectiles[i].velocity, delta));
            game->projectiles[i].hitbox.x = game->projectiles[i].position.x - 10;
            game->projectiles[i].hitbox.y = game->projectiles[i].position.y - 10;
            
            game->projectiles[i].frameTimer += delta;
            if (game->projectiles[i].frameTimer > 0.1f) {
                game->projectiles[i].currentFrame = (game->projectiles[i].currentFrame + 1) % 4;
                game->projectiles[i].frameTimer = 0.0f;
            }

            // Colisão com jogador
            Rectangle pRect = { game->player.position.x - 20, game->player.position.y - 20, 40, 40 };
            if (CheckCollisionRecs(game->projectiles[i].hitbox, pRect))
            {
                game->projectiles[i].active = false;
                if (game->player.shieldTimer > 0.0f) {
                    SpawnParticleExplosion(game, game->player.position, SKYBLUE, 10, 80.0f, 150.0f, 3.0f, 0.4f);
                } else {
                    game->player.hp -= game->projectiles[i].damage;
                    game->screenShake = 0.3f;
                    SpawnParticleExplosion(game, game->player.position, RED, 10, 50.0f, 100.0f, 3.0f, 0.5f);
                    if (game->player.hp <= 0) {
                        game->player.hp = 0;
                        game->currentScreen = SCREEN_GAMEOVER;
                        return;
                    }
                }
            }
            
            // Remove se sair do mapa longe
            if (game->projectiles[i].position.x < -100 || game->projectiles[i].position.x > MAP_WIDTH + 100 ||
                game->projectiles[i].position.y < -100 || game->projectiles[i].position.y > MAP_HEIGHT + 100) {
                game->projectiles[i].active = false;
            }
        }
    }

    // ------------------------------------------------------------------------
    // 7. SISTEMA DE LEVEL UP DO HERÓI
    // ------------------------------------------------------------------------
    if (game->player.xp >= game->player.xpNeeded)
    {
        // Consome XP e sobe de nível
        game->player.xp -= game->player.xpNeeded;
        game->player.level++;
        game->player.xpNeeded = (int)(game->player.xpNeeded * 1.5f);

        // Melhora atributos do herói
        game->player.maxHp += 15;
        game->player.hp = game->player.maxHp; // Cura total no level up
        game->player.attackPower += 6;
        game->player.speed += 10.0f;

        // Grande efeito visual festivo de Level Up (Verde e Gold)
        SpawnParticleExplosion(game, game->player.position, LIME, 30, 80.0f, 220.0f, 5.0f, 1.0f);
        SpawnParticleExplosion(game, game->player.position, GOLD, 20, 100.0f, 250.0f, 4.0f, 1.2f);
        
        game->screenShake = 0.5f;
    }

    // ------------------------------------------------------------------------
    // 8. ATUALIZAÇÃO DA CÂMERA (SUAVE COM LERP + SHAKE)
    // ------------------------------------------------------------------------
    float targetX = game->player.position.x;
    float targetY = game->player.position.y;
    
    // Lerp da câmera em direção ao jogador
    game->camera.target.x += (targetX - game->camera.target.x) * 0.085f;
    game->camera.target.y += (targetY - game->camera.target.y) * 0.085f;

    // Aplica chacoalhar de tela na câmera se ativo
    if (game->screenShake > 0.0f)
    {
        game->camera.offset.x = (SCREEN_WIDTH / 2.0f) + (float)GetRandomValue(-15, 15) * game->screenShake;
        game->camera.offset.y = (SCREEN_HEIGHT / 2.0f) + (float)GetRandomValue(-15, 15) * game->screenShake;
    }
    else
    {
        game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    }
}

// ============================================================================
// PERSISTÊNCIA: SALVAR JOGO POR SLOT
// ============================================================================
void SalvarJogoSlot(GameState *game, int slot)
{
    char path[64];
    sprintf(path, "Saves/save_slot_%d.txt", slot);

    FILE *arquivo = fopen(path, "w");
    if (arquivo != NULL)
    {
        // 0. Metadados do Slot para carregamento rápido
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char dateBuffer[32];
        strftime(dateBuffer, sizeof(dateBuffer), "%d/%m/%Y %H:%M", tm_info);

        fprintf(arquivo, "%s\n", (game->player.name[0] != '\0') ? game->player.name : "HERO");
        fprintf(arquivo, "%d\n", game->player.level);
        fprintf(arquivo, "%d\n", game->player.score);
        fprintf(arquivo, "%d\n", game->wave);
        fprintf(arquivo, "%s\n", dateBuffer);

        // 1. Dados do Jogador
        fprintf(arquivo, "%f\n", game->player.position.x);
        fprintf(arquivo, "%f\n", game->player.position.y);
        fprintf(arquivo, "%d\n", game->player.hp);
        fprintf(arquivo, "%d\n", game->player.maxHp);
        fprintf(arquivo, "%d\n", game->player.score);
        fprintf(arquivo, "%d\n", game->player.level);
        fprintf(arquivo, "%d\n", game->player.xp);
        fprintf(arquivo, "%d\n", game->player.xpNeeded);
        fprintf(arquivo, "%d\n", game->player.attackPower);
        fprintf(arquivo, "%f\n", game->player.speed);
        fprintf(arquivo, "%d\n", game->player.activeSkin);

        // 2. Estado do Mundo
        fprintf(arquivo, "%d\n", game->wave);
        fprintf(arquivo, "%d\n", game->totalEnemiesKilled);
        fprintf(arquivo, "%f\n", game->timeElapsed);

        // 3. Contagem e Estado dos Inimigos
        fprintf(arquivo, "%d\n", game->enemiesRemaining);

        // Escreve cada inimigo ativo
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (game->enemies[i].active)
            {
                fprintf(arquivo, "%f %f %d %d %d %f %d %d\n",
                        game->enemies[i].position.x,
                        game->enemies[i].position.y,
                        game->enemies[i].hp,
                        game->enemies[i].maxHp,
                        game->enemies[i].type,
                        game->enemies[i].speed,
                        game->enemies[i].tier,
                        game->enemies[i].isRanged);
            }
        }

        fclose(arquivo);

        // Efeito visual de salvamento concluído (Partículas verdes ao redor do jogador)
        SpawnParticleExplosion(game, game->player.position, GREEN, 15, 30.0f, 90.0f, 4.0f, 0.6f);
    }
}

// ============================================================================
// PERSISTÊNCIA: CARREGAR JOGO POR SLOT
// ============================================================================
void CarregarJogoSlot(GameState *game, int slot)
{
    char path[64];
    sprintf(path, "Saves/save_slot_%d.txt", slot);

    FILE *arquivo = fopen(path, "r");
    if (arquivo != NULL)
    {
        // Reseta primeiro para evitar lixo nas partículas e power-ups
        float shakeOld = game->screenShake;
        GameScreen oldScreen = game->currentScreen;

        // Limpa estados de buffs temporários
        memset(game, 0, sizeof(GameState));

        game->currentScreen = oldScreen;
        game->screenShake = shakeOld;

        // Pular/Ler metadados iniciais
        char nameLine[32];
        if (fgets(nameLine, sizeof(nameLine), arquivo) != NULL)
        {
            nameLine[strcspn(nameLine, "\r\n")] = '\0';
            strncpy(game->player.name, nameLine, 15);
            game->player.name[15] = '\0';
        }
        int dummyLevel, dummyScore, dummyWave;
        char dummyDate[32];
        fscanf(arquivo, "%d\n", &dummyLevel);
        fscanf(arquivo, "%d\n", &dummyScore);
        fscanf(arquivo, "%d\n", &dummyWave);
        if (fgets(dummyDate, sizeof(dummyDate), arquivo) != NULL)
        {
            // Apenas consome a linha da data
        }

        // 1. Dados do Jogador
        fscanf(arquivo, "%f\n", &game->player.position.x);
        fscanf(arquivo, "%f\n", &game->player.position.y);
        fscanf(arquivo, "%d\n", &game->player.hp);
        fscanf(arquivo, "%d\n", &game->player.maxHp);
        fscanf(arquivo, "%d\n", &game->player.score);
        fscanf(arquivo, "%d\n", &game->player.level);
        fscanf(arquivo, "%d\n", &game->player.xp);
        fscanf(arquivo, "%d\n", &game->player.xpNeeded);
        fscanf(arquivo, "%d\n", &game->player.attackPower);
        fscanf(arquivo, "%f\n", &game->player.speed);
        if (fscanf(arquivo, "%d\n", &game->player.activeSkin) <= 0) game->player.activeSkin = 0;

        // 2. Estado do Mundo
        fscanf(arquivo, "%d\n", &game->wave);
        fscanf(arquivo, "%d\n", &game->totalEnemiesKilled);
        fscanf(arquivo, "%f\n", &game->timeElapsed);

        // 3. Contagem e Inimigos
        fscanf(arquivo, "%d\n", &game->enemiesRemaining);

        for (int i = 0; i < game->enemiesRemaining; i++)
        {
            if (i < MAX_ENEMIES)
            {
                int t = 0, isR = 0;
                fscanf(arquivo, "%f %f %d %d %d %f %d %d\n",
                        &game->enemies[i].position.x,
                        &game->enemies[i].position.y,
                        &game->enemies[i].hp,
                        &game->enemies[i].maxHp,
                        &game->enemies[i].type,
                        &game->enemies[i].speed,
                        &t,
                        &isR);
                game->enemies[i].tier = (EnemyTier)t;
                game->enemies[i].isRanged = (bool)isR;
                
                game->enemies[i].active = true;
                game->enemies[i].state = IDLE;
                game->enemies[i].patrolTarget = game->enemies[i].position;
                game->enemies[i].patrolTimer = 3.0f;
            }
        }

        // Câmera re-alinhada instantaneamente
        game->camera.target = game->player.position;
        game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        game->camera.zoom = 1.0f;
        game->camera.rotation = 0.0f;

        game->saveLoaded = true;
        fclose(arquivo);

        // Efeito visual de carregamento completo (Partículas ciano)
        SpawnParticleExplosion(game, game->player.position, SKYBLUE, 20, 50.0f, 150.0f, 4.0f, 0.7f);
    }
}

// ============================================================================
// PERSISTÊNCIA: CARREGAR METADADOS DE UM SLOT
// ============================================================================
SaveSlotMeta CarregarMetadadosSlot(int slot)
{
    SaveSlotMeta meta = { 0 };
    char path[64];
    sprintf(path, "Saves/save_slot_%d.txt", slot);

    FILE *arquivo = fopen(path, "r");
    if (arquivo != NULL)
    {
        meta.exists = true;
        
        // Linha 1: Nome do Jogador
        if (fgets(meta.name, sizeof(meta.name), arquivo) != NULL)
        {
            meta.name[strcspn(meta.name, "\r\n")] = '\0';
        }
        
        // Linha 2: Nível
        fscanf(arquivo, "%d\n", &meta.level);
        
        // Linha 3: Score
        fscanf(arquivo, "%d\n", &meta.score);
        
        // Linha 4: Wave
        fscanf(arquivo, "%d\n", &meta.wave);
        
        // Linha 5: Data
        if (fgets(meta.date, sizeof(meta.date), arquivo) != NULL)
        {
            meta.date[strcspn(meta.date, "\r\n")] = '\0';
        }
        
        fclose(arquivo);
    }
    else
    {
        meta.exists = false;
    }
    return meta;
}
