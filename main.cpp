#include <SDL3/SDL.h>
#include <bits/stdc++.h>
#include <ncurses.h>
#include "vec2.h"



class Piece {
  public:
    std::vector<point2<int>> points;
    point2<double> rotation_point;

    Piece() {}

    Piece(std::vector<point2<int>> points, point2<double> rotation_point) {
        this->points = points;
        this->rotation_point = rotation_point;
    }

    Piece* rotateClockwise() {  // rotates the points vector around the rotation_point; returns pointer to itself
        std::vector<point2<int>> rotated_points;
        for(auto point : points) {
            auto new_point = static_cast<point2<double>>(point) - rotation_point;
            new_point = point2<double>{new_point.y(), -new_point.x()};
            new_point += rotation_point;
            
            rotated_points.push_back(static_cast<vec2<int>>(new_point));
        }

        this->points = rotated_points;

        return this;
    }


};

struct Block {
    uint8_t is_occupied = 0; // 0 - not occupied
                          // 1 - occupied by a normal block
                          // 2 - occupied by a currently active piece

};

class Grid {
  private:
    Piece current_piece;
    Piece next_piece;
    point2<int> piece_position;
    point2<int> piece_position_last_frame;
    int piece_fall_timer = 10; // time it takes for a piece to descend 1 block in ms
    int input_cooldown_time = 100; // time it takes to move a piece again after holding a key in ms
    uint64_t timer = 0;

  public:
    std::vector<std::vector<Block>> grid{20, std::vector<Block>(10, {0})};

    Grid() {}


    Piece getRandomPiece() {
        int piece = rand() % 7;
        switch(piece) {
            case 0:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 1}, {2, 1}, {3, 1}},
                    point2<double>{1.5, 1.5}
                );
            case 1:
                return Piece(
                    std::vector<point2<int>>{{0, 0}, {0, 1}, {1, 1}, {2, 1}},
                    point2<double>{1, 1}
                );
            case 2:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 1}, {2, 1}, {2, 0}},
                    point2<double>{1, 1}
                );
            case 3:
                return Piece(
                    std::vector<point2<int>>{{0, 0}, {0, 1}, {1, 1}, {1, 0}},
                    point2<double>{0.5, 0.5}
                );
            case 4:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 0}, {1, 1}, {2, 0}},
                    point2<double>{1, 1}
                );
            case 5:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 0}, {1, 1}, {2, 1}},
                    point2<double>{1, 1}
                );
            case 6:
                return Piece(
                    std::vector<point2<int>>{{0, 0}, {1, 0}, {1, 1}, {2, 1}},
                    point2<double>{1, 1}
                );
        }
        exit(-1);
    }

    void gameInit() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        curs_set(0);
        next_piece = getRandomPiece();
        current_piece = getRandomPiece();
    }


    void render() {
        clear();

        for(auto row : grid) {
            for(auto block : row) {
                switch(block.is_occupied) {
                    case 0:
                        printw("--");
                        break;
                    case 1:
                        printw("++");
                        break;
                    case 2:
                        printw("##");
                        break;
                }
            }
            printw("\n");
        }

        refresh();
    }




    void update(int pressed_key) {
        // current_piece = next_piece;
        // next_piece = getRandomPiece();
        timer++;
        piece_position_last_frame = piece_position;

        for(auto& row : grid) {
            for(auto& block : row) {
                if(block.is_occupied == 2) block.is_occupied = 0;
            }
        }


        for(auto block : current_piece.points) {
            grid[block.x() + piece_position.x()][block.y() + piece_position.y()].is_occupied = 2;
        }



        if(pressed_key == 'x') {
            current_piece.rotateClockwise();
        }
        
        if(pressed_key == 'c') {
            current_piece = getRandomPiece();
        }


        if(pressed_key == KEY_RIGHT) {
            piece_position.y()++;
        }
        if(pressed_key == KEY_LEFT) {
            piece_position.y()--;
        }

        if(timer % 30 == 0)
            piece_position.x()++;
        // for(auto block : current_piece.points) {

            
        // }

        // if(timer % 180 == 0) {
        //     current_piece = getRandomPiece();
        // }

    }

};


struct Mesh {
    float* vertices;
    uint16_t* triangles;
    uint64_t block_count;
};

Mesh generateVerticies(std::vector<std::vector<Block>>& grid) {
    float*    vertices  = (float*)   malloc(sizeof(float)    * 200 * 24);
    uint16_t* triangles = (uint16_t*)malloc(sizeof(uint16_t) * 200 * 36);
    uint64_t  block_count = 0;

    for(int i = 0; i < grid.size(); i++) {
        for(int j = 0; j < grid[0].size(); j++) {
            if(!grid[i][j].is_occupied) continue;

            vertices[block_count*24     ] = 0+j;
            vertices[block_count*24 + 1 ] = 0+i;
            vertices[block_count*24 + 2 ] = 0;

            vertices[block_count*24 + 3 ] = 1+j;
            vertices[block_count*24 + 4 ] = 0+i;
            vertices[block_count*24 + 5 ] = 0;

            vertices[block_count*24 + 6 ] = 1+j;
            vertices[block_count*24 + 7 ] = 0+i;
            vertices[block_count*24 + 8 ] = 1;

            vertices[block_count*24 + 9] =  0+j;
            vertices[block_count*24 + 10] = 0+i;
            vertices[block_count*24 + 11] = 1;

            vertices[block_count*24 + 12] = 0+j;
            vertices[block_count*24 + 13] = 1+i;
            vertices[block_count*24 + 14] = 0;

            vertices[block_count*24 + 15] = 1+j;
            vertices[block_count*24 + 16] = 1+i;
            vertices[block_count*24 + 17] = 0;

            vertices[block_count*24 + 18] = 1+j;
            vertices[block_count*24 + 19] = 1+i;
            vertices[block_count*24 + 20] = 1;

            vertices[block_count*24 + 21] = 0+j;
            vertices[block_count*24 + 22] = 1+i;
            vertices[block_count*24 + 23] = 1;



            triangles[block_count*36 + 0]  = 0 + block_count*8;
            triangles[block_count*36 + 1]  = 1 + block_count*8;
            triangles[block_count*36 + 2]  = 2 + block_count*8;

            triangles[block_count*36 + 3]  = 2 + block_count*8;
            triangles[block_count*36 + 4]  = 3 + block_count*8;
            triangles[block_count*36 + 5]  = 0 + block_count*8;


            triangles[block_count*36 + 6 ] = 4 + block_count*8;
            triangles[block_count*36 + 7 ] = 5 + block_count*8;
            triangles[block_count*36 + 8 ] = 6 + block_count*8;

            triangles[block_count*36 + 9 ] = 6 + block_count*8;
            triangles[block_count*36 + 10] = 7 + block_count*8;
            triangles[block_count*36 + 11] = 4 + block_count*8;


            triangles[block_count*36 + 12] = 0 + block_count*8;
            triangles[block_count*36 + 13] = 1 + block_count*8;
            triangles[block_count*36 + 14] = 5 + block_count*8;

            triangles[block_count*36 + 15] = 5 + block_count*8;
            triangles[block_count*36 + 16] = 4 + block_count*8;
            triangles[block_count*36 + 17] = 0 + block_count*8;


            triangles[block_count*36 + 18] = 1 + block_count*8;
            triangles[block_count*36 + 19] = 2 + block_count*8;
            triangles[block_count*36 + 20] = 6 + block_count*8;

            triangles[block_count*36 + 21] = 6 + block_count*8;
            triangles[block_count*36 + 22] = 5 + block_count*8;
            triangles[block_count*36 + 23] = 1 + block_count*8;


            triangles[block_count*36 + 24] = 2 + block_count*8;
            triangles[block_count*36 + 25] = 6 + block_count*8;
            triangles[block_count*36 + 26] = 7 + block_count*8;

            triangles[block_count*36 + 27] = 7 + block_count*8;
            triangles[block_count*36 + 28] = 3 + block_count*8;
            triangles[block_count*36 + 29] = 2 + block_count*8;


            triangles[block_count*36 + 30] = 3 + block_count*8;
            triangles[block_count*36 + 31] = 7 + block_count*8;
            triangles[block_count*36 + 32] = 4 + block_count*8;

            triangles[block_count*36 + 33] = 3 + block_count*8;
            triangles[block_count*36 + 34] = 4 + block_count*8;
            triangles[block_count*36 + 35] = 0 + block_count*8;

            block_count++;
        }
    }

    
    return Mesh{vertices, triangles, block_count};
}

void transform_verticies(float* vertices, uint64_t block_count, float* position, float angle_x) {
    float cos_a = cosf(angle_x);
    float sin_a = sinf(angle_x);

    for(int i = 0; i < block_count * 24; i+=3) {
        vertices[i] -= position[0];
        vertices[i+1] -= position[1];
        vertices[i+2] -= position[2];


        float y = vertices[i+1] * cos_a - vertices[i+2] * sin_a;
        float z = vertices[i+1] * sin_a + vertices[i+2] * cos_a;
        vertices[i+1] = y;
        vertices[i+2] = z;
    }
}

void screenPositions(Mesh& mesh) {
    float fov = 300;
    for(int i = 0; i < mesh.block_count * 24; i+=3) {

        if(mesh.vertices[i + 2] < 0.00001) {
            mesh.vertices[i+2] = -1;
            continue;
        }

        mesh.vertices[i]   = (mesh.vertices[i]   / mesh.vertices[i+2]) * fov + 300;
        mesh.vertices[i+1] = (mesh.vertices[i+1] / mesh.vertices[i+2]) * fov + 400;
    }
}

void draw3d(SDL_Renderer* renderer, Mesh& mesh) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for(int i = 0; i < mesh.block_count * 36; i += 3) {
        
        int a = mesh.triangles[i + 0] * 3;
        int b = mesh.triangles[i + 1] * 3;
        int c = mesh.triangles[i + 2] * 3;

        if (mesh.vertices[a + 2] == -1 ||
            mesh.vertices[b + 2] == -1 ||
            mesh.vertices[c + 2] == -1) continue;

        SDL_RenderLine(renderer, mesh.vertices[a], mesh.vertices[a+1],
                                 mesh.vertices[b], mesh.vertices[b+1]);
        SDL_RenderLine(renderer, mesh.vertices[b], mesh.vertices[b+1],
                                 mesh.vertices[c], mesh.vertices[c+1]);
        SDL_RenderLine(renderer, mesh.vertices[c], mesh.vertices[c+1],
                                 mesh.vertices[a], mesh.vertices[a+1]);
    }

}


void render3d(SDL_Renderer* renderer, Grid grid) {
    float pos[3] = {6, 1, -9};
    Mesh mesh = generateVerticies(grid.grid);
    transform_verticies(mesh.vertices, mesh.block_count, pos, 0);
    screenPositions(mesh);
    draw3d(renderer, mesh);
    free(mesh.triangles);
    free(mesh.vertices);

}



int main() {
    srand(time(NULL));

    Grid grid;
    grid.gameInit();
    bool inputs[6] {};


    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "nm",
        600, 800,
        0 
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        NULL
    );

    uint64_t timer = 0;

    SDL_Event event;
    bool running = true;
    while(running) {
        timer++;
        int pressed_key = getch();
        grid.update(pressed_key);
        grid.render();


        if(timer % 20 == 0){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            render3d(renderer, grid);
            SDL_RenderPresent(renderer);
        }

        usleep(16000);
    }
    

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    endwin();
    return 0;
}