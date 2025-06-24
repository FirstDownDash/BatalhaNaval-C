#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 10
#define MAX_SHIPS 5
#define SHIP_TYPES 4
#define MAX_SPECIAL_ABILITIES 3

typedef struct {
    int size;
    int count;
    char symbol;
    char name[20];
} ShipType;

typedef struct {
    char symbol;
    char name[20];
    int radius;
    int uses;
} SpecialAbility;

typedef struct {
    char board[SIZE][SIZE];
    int ships_remaining;
} Player;

ShipType ship_types[SHIP_TYPES] = {
    {5, 1, 'P', "Porta-Aviões"},
    {4, 1, 'C', "Cruzador"},
    {3, 2, 'D', "Destroyer"},
    {2, 1, 'S', "Submarino"}
};

SpecialAbility abilities[MAX_SPECIAL_ABILITIES] = {
    {'B', "Bomba", 1, 2},
    {'R', "Radar", 2, 1},
    {'M', "Míssil", 3, 1}
};

Player player, computer;
int turn = 0;

void initializeGame();
void printBoard(char board[SIZE][SIZE], bool show_ships);
void placeShips(Player *p, bool manual);
bool placeShip(Player *p, ShipType ship, int x, int y, char direction);
bool isValidPlacement(Player *p, int size, int x, int y, char direction);
int makeMove(Player *defender, int x, int y);
void computerMove();
void useSpecialAbility(Player *defender, SpecialAbility ability, int x, int y);
void printMenu();
void gameLoop();
void clearScreen();
void printShipPlacementMenu();
void printGameStatus();

int main() {
    srand(time(NULL));
    initializeGame();
    
    int choice;
    do {
        clearScreen();
        printMenu();
        printf("Escolha: ");
        scanf("%d", &choice);
        
        switch(choice) {
            case 1:
                gameLoop();
                break;
            case 2:
                printf("\n=== INSTRUÇÕES ===\n");
                printf("1. Posicione seus navios no tabuleiro\n");
                printf("2. Alternadamente, atire no tabuleiro inimigo\n");
                printf("3. Use habilidades especiais estrategicamente\n");
                printf("4. O primeiro a afundar todos os navios inimigos vence\n");
                printf("\nSímbolos:\n");
                printf("~ = Água\nP/C/D/S = Navios\nX = Acerto\nO = Erro\n");
                printf("B/R/M = Áreas de habilidade\n");
                printf("\nPressione Enter para continuar...");
                getchar(); getchar();
                break;
            case 3:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while(choice != 3);

    return 0;
}

void initializeGame() {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            player.board[i][j] = '~';
            computer.board[i][j] = '~';
        }
    }
    player.ships_remaining = 0;
    computer.ships_remaining = 0;
    turn = 0;
}

void printBoard(char board[SIZE][SIZE], bool show_ships) {
    printf("   ");
    for(int i = 0; i < SIZE; i++) printf("%2d", i);
    printf("\n");
    
    for(int i = 0; i < SIZE; i++) {
        printf("%2d ", i);
        for(int j = 0; j < SIZE; j++) {
            char c = board[i][j];
            if(!show_ships && c != 'X' && c != 'O' && c != 'B' && c != 'R' && c != 'M') {
                c = '~';
            }
            printf("%2c", c);
        }
        printf("\n");
    }
}

void placeShips(Player *p, bool manual) {
    clearScreen();
    printf("=== POSICIONAMENTO DE NAVIOS ===\n\n");
    
    for(int i = 0; i < SHIP_TYPES; i++) {
        for(int j = 0; j < ship_types[i].count; j++) {
            bool placed = false;
            
            if(manual) {
                int x, y;
                char dir;
                
                do {
                    printBoard(p->board, true);
                    printf("\nPosicionando %s (tamanho %d) - %d de %d\n", 
                           ship_types[i].name, ship_types[i].size, j+1, ship_types[i].count);
                    printf("Digite linha, coluna e direção (H/V): ");
                    scanf("%d %d %c", &x, &y, &dir);
                    dir = toupper(dir);
                    
                    if(isValidPlacement(p, ship_types[i].size, x, y, dir)) {
                        placeShip(p, ship_types[i], x, y, dir);
                        placed = true;
                    } else {
                        printf("Posição inválida! Tente novamente.\n");
                    }
                } while(!placed);
            } else {
                do {
                    int x = rand() % SIZE;
                    int y = rand() % SIZE;
                    char dir = (rand() % 2) ? 'H' : 'V';
                    
                    if(isValidPlacement(p, ship_types[i].size, x, y, dir)) {
                        placeShip(p, ship_types[i], x, y, dir);
                        placed = true;
                    }
                } while(!placed);
            }
        }
    }
    p->ships_remaining = MAX_SHIPS;
}

bool placeShip(Player *p, ShipType ship, int x, int y, char direction) {
    if(!isValidPlacement(p, ship.size, x, y, direction)) {
        return false;
    }
    
    if(direction == 'H') {
        for(int i = 0; i < ship.size; i++) {
            p->board[x][y+i] = ship.symbol;
        }
    } else {
        for(int i = 0; i < ship.size; i++) {
            p->board[x+i][y] = ship.symbol;
        }
    }
    
    return true;
}

bool isValidPlacement(Player *p, int size, int x, int y, char direction) {
    if(direction == 'H') {
        if(y + size > SIZE) return false;
        for(int i = 0; i < size; i++) {
            if(p->board[x][y+i] != '~') return false;
        }
    } else if(direction == 'V') {
        if(x + size > SIZE) return false;
        for(int i = 0; i < size; i++) {
            if(p->board[x+i][y] != '~') return false;
        }
    } else {
        return false;
    }
    return true;
}

int makeMove(Player *defender, int x, int y) {
    if(x < 0 || x >= SIZE || y < 0 || y >= SIZE) {
        return -1;
    }
    
    if(defender->board[x][y] == 'X' || defender->board[x][y] == 'O') {
        return -2;
    }
    
    if(defender->board[x][y] == '~') {
        defender->board[x][y] = 'O';
        return 0;
    }
    
    char ship_symbol = defender->board[x][y];
    defender->board[x][y] = 'X';
    
    bool ship_sunk = true;
    for(int i = 0; i < SIZE && ship_sunk; i++) {
        for(int j = 0; j < SIZE && ship_sunk; j++) {
            if(defender->board[i][j] == ship_symbol) {
                ship_sunk = false;
            }
        }
    }
    
    if(ship_sunk) {
        defender->ships_remaining--;
        return 2;
    }
    
    return 1;
}

void computerMove() {
    int x, y, result;
    
    do {
        x = rand() % SIZE;
        y = rand() % SIZE;
        result = makeMove(&player, x, y);
    } while(result < 0);
    
    printf("\nComputador atirou em (%d, %d): ", x, y);
    switch(result) {
        case 0:
            printf("Água!\n");
            break;
        case 1:
            printf("Acertou seu navio!\n");
            break;
        case 2:
            printf("Afundou seu %s!\n", 
                   ship_types[rand() % SHIP_TYPES].name);
            break;
    }
}

void useSpecialAbility(Player *defender, SpecialAbility ability, int x, int y) {
    if(ability.uses <= 0) {
        printf("Sem usos restantes para esta habilidade!\n");
        return;
    }
    
    printf("Usando %s em (%d, %d)...\n", ability.name, x, y);
    ability.uses--;
    
    for(int i = x - ability.radius; i <= x + ability.radius; i++) {
        for(int j = y - ability.radius; j <= y + ability.radius; j++) {
            if(i >= 0 && i < SIZE && j >= 0 && j < SIZE) {
                if(defender->board[i][j] == '~') {
                    defender->board[i][j] = ability.symbol;
                } else if(defender->board[i][j] != 'X' && defender->board[i][j] != 'O') {
                    defender->board[i][j] = ability.symbol;
                }
            }
        }
    }
}

void printMenu() {
    printf("\n=== BATALHA NAVAL ===\n");
    printf("1. Novo Jogo\n");
    printf("2. Instruções\n");
    printf("3. Sair\n");
}

void gameLoop() {
    initializeGame();
    
    int placement_choice;
    printf("\nEscolha o modo de posicionamento:\n");
    printf("1. Manual\n");
    printf("2. Automático\n");
    printf("Escolha: ");
    scanf("%d", &placement_choice);
    
    placeShips(&player, placement_choice == 1);
    placeShips(&computer, false);
    
    while(player.ships_remaining > 0 && computer.ships_remaining > 0) {
        turn++;
        clearScreen();
        printGameStatus();
        
        int choice;
        printf("\n=== SEU TURNO ===\n");
        printf("1. Atirar\n");
        printf("2. Usar habilidade especial\n");
        printf("Escolha: ");
        scanf("%d", &choice);
        
        if(choice == 1) {
            int x, y;
            printf("Digite as coordenadas (linha coluna): ");
            scanf("%d %d", &x, &y);
            
            int result = makeMove(&computer, x, y);
            switch(result) {
                case -1:
                    printf("Coordenadas inválidas!\n");
                    turn--;
                    break;
                case -2:
                    printf("Você já atirou nesta posição!\n");
                    turn--;
                    break;
                case 0:
                    printf("Água!\n");
                    break;
                case 1:
                    printf("Acertou um navio!\n");
                    break;
                case 2:
                    printf("Afundou um navio inimigo!\n");
                    break;
            }
        } else if(choice == 2) {
            printf("\nHabilidades disponíveis:\n");
            for(int i = 0; i < MAX_SPECIAL_ABILITIES; i++) {
                printf("%d. %s (%c) - Usos: %d\n", 
                      i+1, abilities[i].name, abilities[i].symbol, abilities[i].uses);
            }
            
            int ab, x, y;
            printf("Escolha a habilidade e coordenadas (ex: 1 5 5): ");
            scanf("%d %d %d", &ab, &x, &y);
            
            if(ab >= 1 && ab <= MAX_SPECIAL_ABILITIES) {
                useSpecialAbility(&computer, abilities[ab-1], x, y);
            } else {
                printf("Habilidade inválida!\n");
                turn--;
            }
        }
        
        if(computer.ships_remaining <= 0) {
            break;
        }
        
        printf("\n=== TURNO DO COMPUTADOR ===\n");
        computerMove();
        
        printf("\nPressione Enter para continuar...");
        getchar(); getchar();
    }
    
    clearScreen();
    printf("\n=== FIM DE JOGO ===\n");
    printf("Total de turnos: %d\n", turn);
    
    if(computer.ships_remaining <= 0) {
        printf("\nPARABÉNS! Você venceu!\n");
    } else {
        printf("\nVocê perdeu! O computador afundou todos os seus navios.\n");
    }
    
    printf("\nSeu tabuleiro:\n");
    printBoard(player.board, true);
    printf("\nTabuleiro do computador:\n");
    printBoard(computer.board, true);
    
    printf("\nPressione Enter para voltar ao menu...");
    getchar(); getchar();
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void printGameStatus() {
    printf("=== STATUS DO JOGO ===\n");
    printf("Turno: %d\n", turn);
    printf("Seus navios restantes: %d\n", player.ships_remaining);
    printf("Navios inimigos restantes: %d\n\n", computer.ships_remaining);
    
    printf("SEU TABULEIRO:\n");
    printBoard(player.board, true);
    printf("\nTABULEIRO INIMIGO:\n");
    printBoard(computer.board, false);
}