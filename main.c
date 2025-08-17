#include <stdio.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
#define CLEAR_COMMAND "cls"
#elif defined(__linux__) || defined(__unix__)
#define CLEAR_COMMAND "clear"
#elif defined(__APPLE__)
#define CLEAR_COMMAND "clear"
#else
#define CLEAR_COMMAND "cls"
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

typedef struct
{
  int column;
  int row;
} Point;

typedef struct
{
  Point **visitedPoints;
  int obstaclesAmount;
  int dirtAmount;
  char **grid;
  int columns;
  int rows;
} Map;

typedef enum
{
  ERROR,
  WARN,
  INFO
} LogOption;

typedef struct
{
  int column;
  int row;
} AllocateRobotRes;

void consoleLog(char *content, LogOption option);

Map *allocateMap(int columns, int rows);

void fillMap(Map *grid);
void showMap(Map *grid);

AllocateRobotRes allocateRobot(const Map *map, int num_args, ...);
void writeRobotBase(Map *map, Point point);

int main()
{
  setlocale(LC_ALL, "");
  int columns = 20;
  int rows = 10;

  Map *map = allocateMap(columns, rows);

  char allocateRobotRes[10];
  int robotColumn, robotRow;
  AllocateRobotRes robotInitialPosition;

  fillMap(map);

  consoleLog("Do you want to type your robot coordinates? (y) ", INFO);
  scanf("%1s", allocateRobotRes);

  if (strcmp(allocateRobotRes, "y") == 0)
  {
    system(CLEAR_COMMAND);

    while (1)
    {
      printf("Type the column...\n");
      scanf("%d", &robotColumn);

      if (robotColumn >= map->columns || robotColumn < 0)
      {
        printf("The column number must be between 0 and %d\n", map->columns - 1);
        continue;
      }

      system(CLEAR_COMMAND);
      break;
    }

    while (1)
    {
      printf("Type the row...\n");
      scanf("%d", &robotRow);

      if (robotRow >= map->rows || robotRow < 0)
      {
        printf("The column number must be between 0 and %d\n", map->rows - 1);
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
    consoleLog("The system will randomly select the position of your robot!", INFO);

    robotInitialPosition = allocateRobot(map, 0);
    Point currentRobotPosition = {robotInitialPosition.column, robotInitialPosition.row};

    writeRobotBase(map, currentRobotPosition);
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
  case ERROR:
    strcpy(presentationContent, "\x1b[31m");
    strcat(presentationContent, content);
    strcat(presentationContent, "\x1b[0m\n");
    printf(presentationContent);
    break;
  case WARN:
    strcpy(presentationContent, "\x1b[33m");
    strcat(presentationContent, content);
    strcat(presentationContent, "\x1b[0m\n");
    printf(presentationContent);
    break;
  case INFO:
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
    consoleLog("[ERROR][map] It wasn't possible to allocate memory!", ERROR);
  }

  map->columns = columns;
  map->rows = rows;

  map->grid = MALLOC(char *, rows);

  if (!map->grid)
  {
    consoleLog("[ERROR][grid:1] It wasn't possible to allocate memory!", ERROR);
  }

  for (int i = 0; i < rows; i++)
  {
    map->grid[i] = MALLOC(char, columns);

    if (!map->grid[i])
    {
      consoleLog("[ERROR][grid:2] It wasn't possible to allocate memory!", ERROR);
    }
  }

  return map;
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

  printf("Obstaculos %d | Sujeira: %d", map->obstaclesAmount, map->dirtAmount);
}

AllocateRobotRes allocateRobot(const Map *map, int num_args, ...)
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

  AllocateRobotRes allocationRes = {column, row};

  return allocationRes;
}

void writeRobotBase(Map *map, Point point)
{
  map->grid[point.row][point.column] = 'B';
}