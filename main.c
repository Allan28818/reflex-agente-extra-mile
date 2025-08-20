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
#define REALLOC(ptr, type, size) ((type *)std::realloc(ptr, sizeof(type) * (size)))
#define FREE(pointer) std::free(pointer)
#else
#include <stdlib.h>
#define MALLOC(type, size) ((type *)malloc(sizeof(type) * (size)))
#define REALLOC(ptr, type, size) ((type *)realloc(ptr, sizeof(type) * (size)))
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
  int obstaclesAmount;
  int dirtAmount;
  char **grid;
  int columns;
  int rows;
  Point robotBase;
} Map;

typedef struct
{
  double battery;
  int cleanedCells;
  int blockedAttempts;
  int visitedPointsAmount;
  Point *visitedPoints;
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

Map *allocateMap(Point initialPosition, Point maxPoint);

Robot *instantiateRobot(Point robotBase);
void freeRobot(Robot *robot);

void fillMap(Map *grid);
void showMap(Map *grid, Robot *robot);

Point allocateRobot(const Point maxPoint, int num_args, ...);
void writeRobotBase(Map *map);

bool isInside(Map *map, Point point);
bool isRobotBase(Point robotBase, Point point);
bool hasDirt(Map *map, Point point);
bool hasObstacle(Map *map, Point point);
bool hasDifficult(Map *map, Point point);

MappedPoint **getMappedPoints(Map *map, Point robotPosition, Point robotBase);
void freeMappedPoints(MappedPoint **points);

void cleanCell(Map *map, MappedPoint *newPosition, Robot *robot);
void updateLastPoint(Map *map, Robot *robot);

bool hasAlreadyCleaned(Robot *robot, Point nextPosition);
void leftToRightClean(Map *map, Robot *robot);
void spiralClean(Map *map, Robot *robot);
void zigzagClean(Map *map, Robot *robot);

void goBackRobot(Map *map, Robot *robot);

void leftToRightAnimation();
void spiralAnimation();
void zigzagAnimation();

void showSummary(Map *map, Robot *robot, double cpu);

int main()
{
  setlocale(LC_ALL, "");
  int columns = 20;
  int rows = 10;
  Point maxPoint = {columns, rows};

  char allocateRobotRes[10];
  int robotColumn, robotRow;

  Point robotInitialPosition;

  char cleaningModeRes[10];
  CleaningMode cleaningMode;

  consoleLog("Deseja digitar as coordenadas do robo? (s) ", LOG_INFO);
  scanf("%1s", allocateRobotRes);

  if (strcmp(allocateRobotRes, "s") == 0)
  {
    system(CLEAR_COMMAND);

    while (true)
    {
      printf("Digite uma coluna entre 0 - %d...\n", columns - 1);
      scanf("%d", &robotColumn);

      if (robotColumn >= columns || robotColumn < 0)
      {
        printf("O numero da coluna deve estar entre 0 e %d\n", columns - 1);
        continue;
      }

      system(CLEAR_COMMAND);
      break;
    }

    while (true)
    {
      printf("Digite a linha entre 0 - %d...\n", rows - 1);
      scanf("%d", &robotRow);

      if (robotRow >= rows || robotRow < 0)
      {
        printf("O numero da linha deve estar entre 0 e %d\n", rows - 1);
        continue;
      }

      system(CLEAR_COMMAND);
      break;
    }

    robotInitialPosition = allocateRobot(maxPoint, 2, robotRow, robotColumn);
  }
  else
  {
    system(CLEAR_COMMAND);
    consoleLog("O sistema ira randomizar automaticamente a posicao do robo!", LOG_INFO);

    robotInitialPosition = allocateRobot(maxPoint, 0);
  }

  while (true)
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

  Map *map = allocateMap(robotInitialPosition, maxPoint);
  fillMap(map);
  writeRobotBase(map);

  Robot *robot = instantiateRobot(robotInitialPosition);

  clock_t t0 = clock();
  if (strcmp(cleaningModeRes, "1") == 0)
  {
    leftToRightClean(map, robot);
    goBackRobot(map, robot);
    showMap(map, robot);
    consoleLog("Carregando...", LOG_INFO);
  }

  clock_t t1 = clock();
  double cpu = (double)(t1 - t0) / CLOCKS_PER_SEC;
  showSummary(map, robot, cpu);
  for (int i = 0; i < map->rows; i++)
  {
    FREE(map->grid[i]);
  }

  FREE(map->grid);
  FREE(map);
  freeRobot(robot);

  return 0;
}

void consoleLog(char *content, LogOption option)
{
  char *presentationContent = MALLOC(char, strlen(content) + 12);

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

Map *allocateMap(Point initialPosition, Point maxPoint)
{
  Map *map = MALLOC(Map, 1);

  if (!map)
  {
    consoleLog("[ERROR][map] Nao foi possivel alocar memoria", LOG_ERROR);
  }

  map->columns = maxPoint.column;
  map->rows = maxPoint.row;
  map->robotBase = initialPosition;
  map->grid = MALLOC(char *, maxPoint.row);

  if (!map->grid)
  {
    consoleLog("[ERROR][grid:1] Nao foi possivel alocar memoria", LOG_ERROR);
  }

  for (int i = 0; i < maxPoint.row; i++)
  {
    map->grid[i] = MALLOC(char, maxPoint.column);

    if (!map->grid[i])
    {
      consoleLog("[ERROR][grid:2] Nao foi possivel alocar memoria", LOG_ERROR);
    }
  }

  return map;
}

Robot *instantiateRobot(Point robotBase)
{
  Robot *robot = MALLOC(Robot, 1);

  if (!robot)
  {
    consoleLog("[ERROR][robot] Nao foi possivel alocar memoria", LOG_ERROR);
    FREE(robot);
    return NULL;
  }

  robot->battery = 100.0f;
  robot->blockedAttempts = 0;
  robot->cleanedCells = 0;

  robot->visitedPoints = MALLOC(Point, 1);

  if (!robot->visitedPoints)
  {
    consoleLog("[ERROR][robot:2] Nao foi possivel alocar memoria", LOG_ERROR);
    FREE(robot->visitedPoints);
    FREE(robot);
    return NULL;
  }

  robot->visitedPoints[robot->visitedPointsAmount++] = robotBase;

  return robot;
}

void freeRobot(Robot *robot)
{
  int visitedPointsAmount = robot->visitedPointsAmount;

  FREE(robot->visitedPoints);
  FREE(robot);
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
      int isDifficult = (random % 10) == 0;

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
      else if (isDifficult)
      {
        map->grid[i][j] = '!';
        map->dirtAmount++;
      }
      else
      {
        map->grid[i][j] = '.';
      }
    }
  }
}

void showMap(Map *map, Robot *robot)
{
  int rows = map->rows;
  int columns = map->columns;

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < columns; j++)
    {
      if (map->grid[i][j] == 'O')
      {
        printf("\x1b[34mO\x1b[0m");
      }
      else
      {
        printf("%c", map->grid[i][j]);
      }

      if (j != (columns - 1))
      {
        printf("|");
      }
    }
    printf("\n");
  }

  printf("Obstaculos %d | Sujeira: %d | Bateria: %.2f%%\n", map->obstaclesAmount, map->dirtAmount, robot->battery);
}

Point allocateRobot(const Point maxPoint, int num_args, ...)
{
  va_list args;
  va_start(args, num_args);
  int maxRowSize = maxPoint.row - 1;
  int maxColumnSize = maxPoint.column - 1;

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

void writeRobotBase(Map *map)
{
  Point robotBase = map->robotBase;
  map->grid[robotBase.row][robotBase.column] = 'O';
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

MappedPoint **getMappedPoints(Map *map, Point robotPosition, Point robotBase)
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
  points[NORTH]->isBlocked = !isInside(map, northPoint) || hasObstacle(map, northPoint) || isRobotBase(robotBase, northPoint);

  Point southPoint = {robotPosition.column, robotPosition.row + 1};
  points[SOUTH]->column = southPoint.column;
  points[SOUTH]->row = southPoint.row;
  points[SOUTH]->isBlocked = !isInside(map, southPoint) || hasObstacle(map, southPoint) || isRobotBase(robotBase, southPoint);

  Point eastPoint = {robotPosition.column + 1, robotPosition.row};
  points[EAST]->column = eastPoint.column;
  points[EAST]->row = eastPoint.row;
  points[EAST]->isBlocked = !isInside(map, eastPoint) || hasObstacle(map, eastPoint) || isRobotBase(robotBase, eastPoint);

  Point westPoint = {robotPosition.column - 1, robotPosition.row};
  points[WEST]->column = westPoint.column;
  points[WEST]->row = westPoint.row;
  points[WEST]->isBlocked = !isInside(map, westPoint) || hasObstacle(map, westPoint) || isRobotBase(robotBase, westPoint);

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

    return;
  }

  if (!newPosition->isBlocked)
  {

    updateLastPoint(map, robot);
  }

  int isDirt = hasDirt(map, point);
  int isDifficult = hasDifficult(map, point);

  if (newPosition->isBlocked)
  {
    return;
  }
  else if (isDirt || isDifficult)
  {
    map->grid[newPosition->row][newPosition->column] = 'O';
    robot->cleanedCells++;
  }
  else
  {
    map->grid[newPosition->row][newPosition->column] = 'O';
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

  Point *temporary = REALLOC(robot->visitedPoints, Point, robot->visitedPointsAmount + 1);

  if (!temporary)
  {
    consoleLog("[ERROR][temporary] Nao foi possivel alocar memoria", LOG_ERROR);
    return;
  }

  robot->visitedPoints = temporary;

  robot->visitedPoints[robot->visitedPointsAmount] = point;
  robot->visitedPointsAmount++;
};

void updateLastPoint(Map *map, Robot *robot)
{

  Point lastPoint = robot->visitedPoints[robot->visitedPointsAmount - 1];

  if (lastPoint.row == map->robotBase.row && lastPoint.column == map->robotBase.column)
  {
    map->grid[lastPoint.row][lastPoint.column] = 'B';
    return;
  }

  map->grid[lastPoint.row][lastPoint.column] = '.';
}

bool hasAlreadyCleaned(Robot *robot, Point nextPosition)
{
  for (int i = 0; i < robot->visitedPointsAmount; i++)
  {
    Point currentRobotPoint = robot->visitedPoints[i];
    if (currentRobotPoint.column == nextPosition.column && currentRobotPoint.row == nextPosition.row)
    {
      return true;
    }
  }

  return false;
}

void leftToRightClean(Map *map, Robot *robot)
{
  int nextMove = EAST;

  while (robot->battery > 51)
  {
    Point robotPosition = robot->visitedPoints[robot->visitedPointsAmount - 1];
    MappedPoint **mappedPoints = getMappedPoints(map, robotPosition, map->robotBase);

    int isNorthBlocked = mappedPoints[NORTH]->isBlocked;
    int isSouthBlocked = mappedPoints[SOUTH]->isBlocked;
    int isEastBlocked = mappedPoints[EAST]->isBlocked;
    int isWestBlocked = mappedPoints[WEST]->isBlocked;

    int areAllBlocked = isNorthBlocked && isSouthBlocked && isEastBlocked && isWestBlocked;

    if (areAllBlocked)
    {
      consoleLog("O robo esta preso!", LOG_ERROR);
      return;
    }

    if (!isEastBlocked && nextMove == EAST)
    {
      cleanCell(map, mappedPoints[EAST], robot);
      Point robotPosition = {mappedPoints[EAST]->column, mappedPoints[EAST]->row};
      MappedPoint **nextMappedPoints = getMappedPoints(map, robotPosition, map->robotBase);

      Point westPoint = {nextMappedPoints[WEST]->column, nextMappedPoints[WEST]->row};
      bool alreadyCleanedWest = hasAlreadyCleaned(robot, westPoint);
      bool isNorthBlocked = (bool)nextMappedPoints[NORTH]->isBlocked;

      if (nextMappedPoints[EAST]->isBlocked && !nextMappedPoints[SOUTH]->isBlocked)
      {
        nextMove = SOUTH;
        robot->blockedAttempts++;
      }
      else if (!alreadyCleanedWest && !nextMappedPoints[WEST]->isBlocked && nextMappedPoints[EAST]->isBlocked)
      {
        nextMove = WEST;
      }
      else if (alreadyCleanedWest && !isNorthBlocked)
      {
        nextMove = NORTH;
      }
      else if (!nextMappedPoints[WEST]->isBlocked && nextMappedPoints[EAST]->isBlocked)
      {
        nextMove = WEST;
        robot->blockedAttempts++;
      }
      freeMappedPoints(nextMappedPoints);
    }
    else if (!isSouthBlocked && nextMove == SOUTH)
    {
      Point southPoint = {mappedPoints[SOUTH]->column, mappedPoints[SOUTH]->row};

      cleanCell(map, mappedPoints[SOUTH], robot);
      Point robotPosition = {mappedPoints[SOUTH]->column, mappedPoints[SOUTH]->row};
      MappedPoint **nextMappedPoints = getMappedPoints(map, robotPosition, map->robotBase);

      if (!nextMappedPoints[EAST]->isBlocked)
      {
        nextMove = EAST;
      }
      else if (!nextMappedPoints[WEST]->isBlocked)
      {
        nextMove = WEST;
      }
      freeMappedPoints(nextMappedPoints);
    }
    else if (!isWestBlocked && nextMove == WEST)
    {
      cleanCell(map, mappedPoints[WEST], robot);
      Point robotPosition = {mappedPoints[WEST]->column, mappedPoints[WEST]->row};
      MappedPoint **nextMappedPoints = getMappedPoints(map, robotPosition, map->robotBase);

      if (nextMappedPoints[WEST]->isBlocked && !nextMappedPoints[SOUTH]->isBlocked)
      {
        nextMove = SOUTH;
        robot->blockedAttempts++;
      }

      else if (!nextMappedPoints[EAST]->isBlocked && nextMappedPoints[WEST]->isBlocked)
      {
        nextMove = EAST;
        robot->blockedAttempts++;
      }

      freeMappedPoints(nextMappedPoints);
    }
    else if (!isNorthBlocked && nextMove == NORTH)
    {
      cleanCell(map, mappedPoints[NORTH], robot);
      Point robotPosition = {mappedPoints[NORTH]->column, mappedPoints[NORTH]->row};
      MappedPoint **nextMappedPoints = getMappedPoints(map, robotPosition, map->robotBase);

      if (!nextMappedPoints[EAST]->isBlocked)
      {
        nextMove = EAST;
      }
      else if (!nextMappedPoints[WEST]->isBlocked)
      {
        nextMove = WEST;
      }
      freeMappedPoints(nextMappedPoints);
    }

    showMap(map, robot);
    SLEEP(.25);
    system(CLEAR_COMMAND);

    freeMappedPoints(mappedPoints);
  }
}

void goBackRobot(Map *map, Robot *robot)
{
  for (int i = robot->visitedPointsAmount - 1; i >= 0; i--)
  {
    Point currentPoint = robot->visitedPoints[i];
    MappedPoint *previousPoint = MALLOC(MappedPoint, 1);
    previousPoint->column = currentPoint.column;
    previousPoint->row = currentPoint.row;
    previousPoint->isBlocked = 0;

    cleanCell(map, previousPoint, robot);
    showMap(map, robot);
    consoleLog("Voltando para a estacao de carregamento!", LOG_ERROR);
    SLEEP(0.25);
    system(CLEAR_COMMAND);
  }
}

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

void showSummary(Map *map, Robot *robot, double cpu)
{
  int cleanedCells = map->dirtAmount - robot->cleanedCells;
  double cleanedCellsPercentage = 100 - (robot->cleanedCells * 100) / map->dirtAmount;

  int blockedAttempts = robot->blockedAttempts;
  double blockedAttempsPercentage = (blockedAttempts * 100) / (robot->visitedPointsAmount / 2);
  printf(
      "______________________________________\n"
      "|     Celulas limpas: %d (%.2f%%)    |\n"
      "|            CPU: %.6fs         |\n"
      "| Tentativas bloqueadas: %d (%.2f%%)  |\n"
      "______________________________________\n",
      cleanedCells, cleanedCellsPercentage, cpu, blockedAttempts, blockedAttempsPercentage);
}