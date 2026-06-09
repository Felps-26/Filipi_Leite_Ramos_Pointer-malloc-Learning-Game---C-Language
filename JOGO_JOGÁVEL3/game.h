// game.h
// Definições de constantes, enums e estruturas do RPG.
#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

// ============================================================================
// CONSTANTES DO JOGO
// ============================================================================
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAP_WIDTH 4000
#define MAP_HEIGHT 4000

#define MAX_ENEMIES 60
#define MAX_POWERUPS 20
#define MAX_PARTICLES 250

// ============================================================================
// ENUMS
// ============================================================================
typedef enum GameScreen
{
    SCREEN_MENU,
    SCREEN_CONTROLS,
    SCREEN_GAMEPLAY,
    SCREEN_PAUSE,
    SCREEN_GAMEOVER,
    SCREEN_VICTORY,
    SCREEN_SAVE_SELECT,
    SCREEN_LOAD_SELECT
} GameScreen;

typedef enum EnemyState
{
    PATROL,
    CHASE
} EnemyState;

typedef enum PowerUpType
{
    HP_RECOVERY,
    SPEED_BOOST,
    SHIELD,
    ATTACK_BOOST
} PowerUpType;

// ============================================================================
// ESTRUTURAS DE ENTIDADES
// ============================================================================
typedef struct Player
{
    char name[16];        // Nome do jogador
    Vector2 position;
    float speed;
    int hp;
    int maxHp;
    int score;
    int level;
    int xp;
    int xpNeeded;
    int attackPower;
    
    // Timers de Buffs (ativos se > 0)
    float speedTimer;
    float shieldTimer;
    float attackBoostTimer;
    
    // Combate
    float attackCooldown; // Tempo até o próximo ataque
} Player;

typedef struct Enemy
{
    Vector2 position;
    float speed;
    int hp;
    int maxHp;
    EnemyState state;
    Vector2 patrolTarget;
    float patrolTimer;
    int type;        // 0 = Comum (Laranja/Vermelho), 1 = Rápido (Rosa/Roxo), 2 = Elite/Boss (Carmesim)
    bool active;     // Ativo/vivo no jogo
} Enemy;

typedef struct PowerUp
{
    Vector2 position;
    PowerUpType type;
    bool active;
    float pulseTimer; // Para efeito visual pulsante no HUD e mapa
} PowerUp;

typedef struct Particle
{
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float lifeTime;
    float maxLifeTime;
    bool active;
} Particle;

// ============================================================================
// ESTRUTURAS DE UI
// ============================================================================
typedef struct UIButton
{
    Rectangle bounds;
    const char *text;
    bool hover;
    bool clicked;
} UIButton;

// ============================================================================
// METADADOS DO SLOT DE SAVE
// ============================================================================
typedef struct SaveSlotMeta
{
    bool exists;
    char name[16];
    int level;
    int score;
    int wave;
    char date[32];
} SaveSlotMeta;

// ============================================================================
// ESTADO GLOBAL DO JOGO
// ============================================================================
typedef struct GameState
{
    GameScreen currentScreen;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    PowerUp powerUps[MAX_POWERUPS];
    Particle particles[MAX_PARTICLES];
    
    int totalEnemiesKilled;
    int enemiesRemaining;
    int wave;
    Camera2D camera;
    
    // Sistema
    bool saveLoaded;
    float timeElapsed;
    float screenShake;
    
    // Controle de Input do Nome
    bool nameInputActive;
    
    // Animação de ataque (Slash)
    float slashAnimTimer;
    Vector2 slashAnimPos;
    float slashAnimRadius;

    // Metadados dos slots carregados na tela de seleção
    SaveSlotMeta slotsMeta[3];
} GameState;

#endif // GAME_H
