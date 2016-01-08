#include <iostream>

#include <cstdlib>
#include <ctime>

#include <SDL2/SDL.h>

const int GRID_WIDTH = 160;
const int GRID_HEIGHT = 120;
const int CELL_SIZE = 4;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

void createWindow()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Game of Life",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              CELL_SIZE * GRID_WIDTH,
                              CELL_SIZE * GRID_HEIGHT,
                              SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    screenSurface = SDL_GetWindowSurface(window);
}

void initGrid(bool** grid, bool** oldGrid)
{
    for (int i = 0; i < GRID_WIDTH; i++)
    {
        for (int j = 0; j < GRID_HEIGHT; j++)
        {
            grid[i][j] = rand() % 2;
            oldGrid[i][j] = false;
        }
    }
}

void init(bool** grid, bool** oldGrid)
{
    srand(time(NULL));

    createWindow();
    initGrid(grid, oldGrid);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
}

bool isAlive(int x, int y, bool** grid)
{
    if (x >= GRID_WIDTH)
        return isAlive(x - GRID_WIDTH, y, grid);
    if (y >= GRID_HEIGHT)
        return isAlive(x, y - GRID_HEIGHT, grid);
    if (x < 0)
        return isAlive(x + GRID_WIDTH, y, grid);
    if (y < 0)
        return isAlive(x, y + GRID_HEIGHT, grid);

    return grid[x][y];
}

int calcNeighbours(int x, int y, bool** grid)
{
    int neighbours = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i == 1 && j == 1) // Do not count self
                continue;
            
            if (isAlive(x - 1 + i, y - 1 + j, grid))
                neighbours++;
        }
    }

    return neighbours;
}

void stepSimulation(bool** grid, bool** oldGrid)
{
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            grid[x][y] = false;
            int neighbours = calcNeighbours(x, y, oldGrid);

            if (neighbours == 3)
                grid[x][y] = true;
            if (neighbours == 2 && isAlive(x, y, oldGrid))
                grid[x][y] = true;

            //if (rand() % 100000 == 0) // Create new cells
            //    grid[x][y] = true;
        }
    }
}

void drawCell(int x, int y, char r, char g, char b)
{
    SDL_Rect rect;
    rect.x = x * CELL_SIZE;
    rect.y = y * CELL_SIZE;
    rect.w = rect.h = CELL_SIZE;
    SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, r, g, b));
}

void drawNewCell(int x, int y, bool** grid)
{
    drawCell(x, y, rand() % 0xFF, rand() % 0xFF, rand() % 0xFF);
}

void clearCell(int x, int y, bool** grid)
{
    drawCell(x, y, 0xFF, 0xFF, 0xFF);
}

void draw(bool** grid, bool** oldGrid)
{
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            if (grid[x][y] && !oldGrid[x][y])
                drawNewCell(x, y, grid);

            if (!grid[x][y] && oldGrid[x][y])
                clearCell(x, y, grid);
        }
    }
}

void swapGrids(bool** &grid, bool** &oldGrid)
{
    bool** temp = oldGrid;
    oldGrid = grid;
    grid = temp;
}

void clearGrid(bool** grid)
{
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            grid[x][y] = false;
        }
    }
}

bool quit = false;
bool paused = false;

void handleKeyDownEvent(const SDL_Event& e, bool** grid)
{
    switch(e.key.keysym.sym)
    {
    case SDLK_SPACE:
        paused = !paused;
        break;
    case SDLK_ESCAPE:
        quit = true;
        break;
    case SDLK_c:
        clearGrid(grid);
        SDL_UpdateWindowSurface(window);
        SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
        break;
    }
}

enum MouseState {
    None,
    Add,
    Remove
};
MouseState currentState = None;

void handleMouseUpEvent(const SDL_Event& event)
{
    currentState = None;
}

void handleMouseMoveEvent(const SDL_Event& event, bool** grid)
{
    int x, y;
    SDL_GetMouseState(&x, &y);

    x /= CELL_SIZE;
    y /= CELL_SIZE;

    if (currentState == Add)
    {
        grid[x][y] = true;
        drawNewCell(x, y, grid);
    }
    else if (currentState == Remove)
    {
        grid[x][y] = false;
        clearCell(x, y, grid);
    }

    SDL_UpdateWindowSurface(window);
}

void handleMouseDownEvent(const SDL_Event& event, bool** grid)
{
    if (event.button.button == SDL_BUTTON_LEFT)
        currentState = Add;
    else if (event.button.button == SDL_BUTTON_RIGHT)
        currentState = Remove;

    handleMouseMoveEvent(event, grid);
}

void loop(bool** grid, bool** oldGrid)
{
    SDL_Event e;
    float elapsedTime = 0;
    int previousTicks = SDL_GetTicks();
    float updateInterval = 0.1f;

    while (!quit)
    {
        while(SDL_PollEvent(&e) != 0)
        {
            switch(e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                handleKeyDownEvent(e, grid);
                break;
            case SDL_MOUSEBUTTONDOWN:
                handleMouseDownEvent(e, grid);
                break;
            case SDL_MOUSEBUTTONUP:
                handleMouseUpEvent(e);
                break;
            case SDL_MOUSEMOTION:
                handleMouseMoveEvent(e, grid);
                break;
            default:
                break;
            }
        }

        int currentTicks = SDL_GetTicks();
        float deltaTime = float(currentTicks - previousTicks) / 1000.0f;
        previousTicks = currentTicks;

        if (!paused)
            elapsedTime += deltaTime;

        if (elapsedTime > updateInterval)
        {
            draw(grid, oldGrid);
            SDL_UpdateWindowSurface(window);

            elapsedTime -= updateInterval;
            swapGrids(grid, oldGrid);
            stepSimulation(grid, oldGrid);
        }
        //SDL_Delay(50);
    }
}

int main()
{
    bool **grid = new bool*[GRID_WIDTH];
    bool **oldGrid = new bool*[GRID_WIDTH];
    for (int i = 0; i < GRID_WIDTH; i++) {
        grid[i] = new bool[GRID_HEIGHT];
        oldGrid[i] = new bool[GRID_HEIGHT];
    }

    init(grid, oldGrid);
    loop(grid, oldGrid);
}
