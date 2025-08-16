#include <stdio.h>
#include <string.h>
#include <time.h>
#include <locale.h>

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

void consoleLog(char *content, LogOption option);

Map *allocateMap(int columns, int rows);

void fillMap(Map *grid);
void showMap(Map *grid);

int main()
{
  setlocale(LC_ALL, "");
  int columns = 20;
  int rows = 10;

  Map *map = allocateMap(columns, rows);

  fillMap(map);
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
    }
    printf("\n");
  }

  printf("Obstaculos %d | Sujeira: %d", map->obstaclesAmount, map->dirtAmount);
}
