#include <stdio.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define SLEEP(time) Sleep(time * 1000)
#define CLEAR_COMMAND "cls"
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#define SLEEP(time) sleep(time)
#define CLEAR_COMMAND "clear"
#endif

#ifdef __cplusplus
#include <cstdlib>
#define MALLOC(type, size) ((type *)std::malloc(sizeof(type) * (size)))
#define FREE(pointer) std::free(pointer)
#else
#include <stdlib.h>
#define MALLOC(type, size) ((type *)malloc(sizeof(type) * (size)))
#define FREE(pointer) free(pointer)
#endif

#define MAX_BATTERY 100.00

typedef struct
{
  int column;
  int row;
} Point;

typedef struct
{
  int column;
  int row;
  int isBlocked;
} MappedPoint;

typedef struct
{
  Point **visitedPoints;
  int obstaclesAmount;
  int dirtAmount;
  char **grid;
  int columns;
  int rows;
} Map;

typedef struct
{
  float battery;
  int cleanedCells;
  int blockedAttempts;
} Robot;

typedef enum
{
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO
} LogOption;

typedef enum
{
  LEFT_TO_RIGHT,
  SPIRAL,
  ZIGZAG
} CleaningMode;

/**
 *       N
 *       ^
 *       |
 * W <---0---> E
 *       |
 *       v
 *       S
 */

typedef enum
{
  NORTH,
  SOUTH,
  EAST,
  WEST
} Locations;

void consoleLog(char *content, LogOption option);

Map *allocateMap(int columns, int rows);

Robot *instantiateRobot();

void fillMap(Map *grid);
void showMap(Map *grid);

Point allocateRobot(const Map *map, int num_args, ...);
void writeRobotBase(Map *map, Point point);

bool isInside(Map *map, Point point);
bool isRobotBase(Point robotBase, Point point);
bool hasDirt(Map *map, Point point);
bool hasObstacle(Map *map, Point point);
bool hasDifficult(Map *map, Point point);

MappedPoint **getMappedPoints(Map *map, Point robotPosition);
void freeMappedPoints(MappedPoint **points);

void cleanCell(Map *map, MappedPoint *newPosition, Robot *robot);

void leftToRightClean(Map *map, Robot *robot, Point);

void leftToRightAnimation();

int main()
{
  setlocale(LC_ALL, "");
  int columns = 20;
  int rows = 10;

  Map *map = allocateMap(columns, rows);
  Robot *robot = instantiateRobot();

  char allocateRobotRes[10];
  int robotColumn, robotRow;

  Point robotInitialPosition;

  char cleaningModeRes[10];
  CleaningMode cleaningMode;

  fillMap(map);

  consoleLog("Deseja digitar as coordenadas do robo? (s) ", LOG_INFO);
  scanf("%1s", allocateRobotRes);

  if (strcmp(allocateRobotRes, "s") == 0)
  {
    system(CLEAR_COMMAND);

    while (1)
    {
      printf("Digite uma coluna entre 0 - %d...\n", columns - 1);
      scanf("%d", &robotColumn);

      if (robotColumn >= map->columns || robotColumn < 0)
      {
        printf("O numero da coluna deve estar entre 0 e %d\n", map->columns - 1);
        continue;
      }

      system(CLEAR_COMMAND);
      break;
    }

    while (1)
    {
      printf("Digite a linha entre 0 - %d...\n", rows - 1);
      scanf("%d", &robotRow);

      if (robotRow >= map->rows || robotRow < 0)
      {
        printf("O numero da linha deve estar entre 0 e %d\n", map->rows - 1);
        continue;
      }

      system(CLEAR_COMMAND);
      break;
    }

    robotInitialPosition = allocateRobot(map, 2, robotRow, robotColumn);
    Point currentRobotPosition = {robotInitialPosition.column, robotInitialPosition.row};

    writeRobotBase(map, currentRobotPosition);
  }
  else
  {
    system(CLEAR_COMMAND);
    consoleLog("O sistema ira randomizar automaticamente a posicao do robo!", LOG_INFO);

    robotInitialPosition = allocateRobot(map, 0);
    Point currentRobotPosition = {robotInitialPosition.column, robotInitialPosition.row};

    writeRobotBase(map, currentRobotPosition);
  }

  while (1)
  {
    printf(
        "Selecione um modo de limpeza:\n"
        "(1) ESQUERDA PARA DIREITA: o robo ira limpar da esquerda para direita (de cima para baixo)\n"
        "(2) ESPIRAl: o robo ira limpar em circulos espirais (de cima para baixo)\n"
        "(3) ZIG-ZAG: o robo fara zigue-zague para limpar (de cima para baixo)\n");

    scanf("%1s", cleaningModeRes);

    if (strcmp(cleaningModeRes, "1") == 0)
    {
      cleaningMode = LEFT_TO_RIGHT;
      system(CLEAR_COMMAND);
      leftToRightAnimation();
      break;
    }
    system(CLEAR_COMMAND);
    consoleLog("Digite um opcao valida (1, 2 ou 3)", ERROR);
    continue;
  }

  showMap(map);

  for (int i = 0; i < map->rows; i++)
  {
    FREE(map->grid[i]);
  }

  FREE(map->grid);
  FREE(map->visitedPoints);
  FREE(map);

  return 0;
}

void consoleLog(char *content, LogOption option)
{
  char *presentationContent = MALLOC(char, sizeof(content) + 12);

  switch (option)
  {
  case LOG_ERROR:
    strcpy(presentationContent, "\x1b[31m");
    strcat(presentationContent, content);
    strcat(presentationContent, "\x1b[0m\n");
    printf(presentationContent);
    break;
  case LOG_WARN:
    strcpy(presentationContent, "\x1b[33m");
    strcat(presentationContent, content);
    strcat(presentationContent, "\x1b[0m\n");
    printf(presentationContent);
    break;
  case LOG_INFO:
    strcpy(presentationContent, "\x1b[34m");
    strcat(presentationContent, content);
    strcat(presentationContent, "\x1b[0m\n");
    printf(presentationContent);
    break;
  default:
    printf(content);
  }

  FREE(presentationContent);
}

Map *allocateMap(int columns, int rows)
{
  Map *map = MALLOC(Map, 1);

  if (!map)
  {
    consoleLog("[ERROR][map] Nao foi possivel alocar memoria", LOG_ERROR);
  }

  map->columns = columns;
  map->rows = rows;

  map->grid = MALLOC(char *, rows);

  if (!map->grid)
  {
    consoleLog("[ERROR][grid:1] Nao foi possivel alocar memoria", LOG_ERROR);
  }

  for (int i = 0; i < rows; i++)
  {
    map->grid[i] = MALLOC(char, columns);

    if (!map->grid[i])
    {
      consoleLog("[ERROR][grid:2] Nao foi possivel alocar memoria", LOG_ERROR);
    }
  }

  return map;
}

Robot *instantiateRobot()
{
  Robot *robot = MALLOC(Robot, 1);

  robot->battery = 100;
  robot->blockedAttempts = 0;
  robot->cleanedCells = 0;

  return robot;
}

void fillMap(Map *map)
{
  int columns = map->columns;
  int rows = map->rows;

  map->obstaclesAmount = 0;
  map->dirtAmount = 0;

  srand(time(NULL));
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < columns; j++)
    {
      int random = rand() % 100 + 1;

      int isObstacle = (random % 8) == 0;
      int isDirty = (random % 3) == 0;

      if (isObstacle)
      {
        map->grid[i][j] = '#';
        map->obstaclesAmount++;
      }
      else if (isDirty)
      {
        map->grid[i][j] = '*';
        map->dirtAmount++;
      }
      else
      {
        map->grid[i][j] = '.';
      }
    }
  }
}

void showMap(Map *map)
{
  int rows = map->rows;
  int columns = map->columns;

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < columns; j++)
    {
      printf("%c", map->grid[i][j]);

      if (j != (columns - 1))
      {
        printf("|");
      }
    }
    printf("\n");
  }

  printf("Obstaculos %d | Sujeira: %d\n", map->obstaclesAmount, map->dirtAmount);
}

Point allocateRobot(const Map *map, int num_args, ...)
{
  va_list args;
  va_start(args, num_args);
  int maxRowSize = map->rows - 1;
  int maxColumnSize = map->columns - 1;

  int row = rand() % maxRowSize;
  int column = rand() % maxColumnSize;

  srand(time(NULL));

  if (num_args >= 1)
  {
    row = va_arg(args, int);
  }
  if (num_args >= 2)
  {
    column = va_arg(args, int);
  }

  Point allocationRes = {column, row};

  return allocationRes;
}

void writeRobotBase(Map *map, Point point)
{
  map->grid[point.row][point.column] = 'B';
}

bool isInside(Map *map, Point point)
{
  int columns = map->columns;
  int rows = map->rows;

  if (point.column < 0 || point.column >= columns)
  {
    return false;
  }
  else if (point.row < 0 || point.row >= rows)
  {
    return false;
  }

  return true;
}

bool isRobotBase(Point robotBase, Point point)
{
  if (robotBase.column == point.column && robotBase.row == point.row)
  {
    return true;
  }

  return false;
}

bool hasDirt(Map *map, Point point)
{
  int column = point.column;
  int row = point.row;

  char cell = map->grid[row][column];

  if (cell == '*')
  {
    return true;
  }

  return false;
}

bool hasObstacle(Map *map, Point point)
{
  int column = point.column;
  int row = point.row;

  char cell = map->grid[row][column];

  if (cell == '#')
  {
    return true;
  }

  return false;
}

bool hasDifficult(Map *map, Point point)
{
  int column = point.column;
  int row = point.row;

  char cell = map->grid[row][column];

  if (cell == '!')
  {
    return true;
  }

  return false;
}

MappedPoint **getMappedPoints(Map *map, Point robotPosition)
{
  MappedPoint **points = MALLOC(MappedPoint *, 4);

  if (!points)
    return NULL;

  for (int i = 0; i < 4; i++)
  {
    points[i] = MALLOC(MappedPoint, 1);
    if (!points[i])
    {
      for (int j = 0; j < i; j++)
        FREE(points[j]);
      FREE(points);
      return NULL;
    }
  }

  Point northPoint = {robotPosition.column, robotPosition.row - 1};
  points[NORTH]->column = northPoint.column;
  points[NORTH]->row = northPoint.row;
  points[NORTH]->isBlocked = !isInside(map, northPoint) || hasObstacle(map, northPoint);

  Point southPoint = {robotPosition.column, robotPosition.row + 1};
  points[SOUTH]->column = southPoint.column;
  points[SOUTH]->row = southPoint.row;
  points[SOUTH]->isBlocked = !isInside(map, southPoint) || hasObstacle(map, southPoint);

  Point eastPoint = {robotPosition.column + 1, robotPosition.row};
  points[EAST]->column = eastPoint.column;
  points[EAST]->row = eastPoint.row;
  points[EAST]->isBlocked = !isInside(map, eastPoint) || hasObstacle(map, eastPoint);

  Point westPoint = {robotPosition.column - 1, robotPosition.row};
  points[WEST]->column = westPoint.column;
  points[WEST]->row = westPoint.row;
  points[WEST]->isBlocked = !isInside(map, westPoint) || hasObstacle(map, westPoint);

  return points;
}

void freeMappedPoints(MappedPoint **points)
{
  for (int i = 0; i < 4; i++)
  {
    FREE(points[i]);
  }

  FREE(points);
}

void cleanCell(Map *map, MappedPoint *newPosition, Robot *robot)
{
  Point point = {newPosition->column, newPosition->row};
  int inside = isInside(map, point);

  if (!inside)
  {
    robot->blockedAttempts++;
    return;
  }

  int isDirt = hasDirt(map, point);
  int isDifficult = hasDifficult(map, point);

  if (newPosition->isBlocked)
  {
    robot->blockedAttempts++;
    return;
  }
  else if (isDirt || isDifficult)
  {
    map->grid[newPosition->row][newPosition->column] = '.';
    robot->cleanedCells++;
  }

  if (isDirt)
  {
    robot->battery -= 1;
  }
  else if (isDifficult)
  {
    robot->battery -= 2;
  }
  else
  {
    robot->battery -= 1;
  }
};

void leftToRightAnimation()
{
  printf("Esquerda para direita selecionado /\n");
  printf("->\n");
  SLEEP(0.25);
  system(CLEAR_COMMAND);
  printf("Esquerda para direita selecionado -\n");
  printf("-->\n");
  SLEEP(0.25);
  system(CLEAR_COMMAND);
  printf("Esquerda para direita selecionado |\n");
  printf("--->\n");
  SLEEP(0.25);
  system(CLEAR_COMMAND);
  printf("Esquerda para direita selecionado \\\n");
  printf("---->\n");
  SLEEP(0.25);
  system(CLEAR_COMMAND);
  printf("Esquerda para direita selecionado |\n");
  printf("----->\n");
  SLEEP(0.25);
  system(CLEAR_COMMAND);
  printf("Esquerda para direita selecionado /\n");
  printf("------>\n");
  SLEEP(0.25);
  system(CLEAR_COMMAND);
}