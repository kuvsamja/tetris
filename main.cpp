#include <bits/stdc++.h>
#include <curses.h>
#include <chrono>
#include <thread>
#include "vec2.h"



class Piece {
  public:
    std::vector<point2<int>> blocks;
    point2<double> rotation_point;
    point2<int> position;
    int color;

    Piece() {}

    Piece(std::vector<point2<int>> blocks, point2<double> rotation_point, point2<int> position, int color) {
        this->blocks = blocks;
        this->rotation_point = rotation_point;
        this->position = position;
        this->color = color;
    }

    Piece* rotateClockwise() {  // rotates the blocks vector around the rotation_point; returns pointer to itself
        std::vector<point2<int>> rotated_points;
        for(auto point : blocks) {
            auto new_point = static_cast<point2<double>>(point) - rotation_point;
            new_point = point2<double>{-new_point.y(), new_point.x()};
            new_point += rotation_point;

            rotated_points.push_back(static_cast<vec2<int>>(new_point));
        }

        this->blocks = rotated_points;

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
    std::vector<std::vector<Block>> grid{20, std::vector<Block>(10, {0})};
    Piece current_piece;
    Piece next_piece;
    int piece_fall_timer = 10; // time it takes for a piece to descend 1 block in ms
    uint64_t global_timer = 0;
    uint64_t score = 0;
    uint64_t lines_cleared = 0;
    uint64_t level = 1;

  public:
    Grid() {}


    Piece getRandomPiece() {
        int piece = rand() % 7;
        switch(piece) {
            case 0:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 1}, {2, 1}, {3, 1}},
                    point2<double>{1.5, 1.5},
                    point2<int>{0, 4},
                    1
                );
            case 1:
                return Piece(
                    std::vector<point2<int>>{{0, 0}, {0, 1}, {1, 1}, {2, 1}},
                    point2<double>{1, 1},
                    point2<int>{0, 4},
                    1
                );
            case 2:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 1}, {2, 1}, {2, 0}},
                    point2<double>{1, 1},
                    point2<int> {0, 4},
                    1
                );
            case 3:
                return Piece(
                    std::vector<point2<int>>{{0, 0}, {0, 1}, {1, 1}, {1, 0}},
                    point2<double>{0.5, 0.5},
                    point2<int> {0, 4},
                    1
                );
            case 4:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 0}, {1, 1}, {2, 0}},
                    point2<double>{1, 1},
                    point2<int> {0, 4},
                    1
                );
            case 5:
                return Piece(
                    std::vector<point2<int>>{{0, 1}, {1, 0}, {1, 1}, {2, 1}},
                    point2<double>{1, 1},
                    point2<int> {0, 4},
                    1
                );
            case 6:
                return Piece(
                    std::vector<point2<int>>{{0, 0}, {1, 0}, {1, 1}, {2, 1}},
                    point2<double>{1, 1},
                    point2<int> {0, 4},
                    1
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
        current_piece = next_piece;
        next_piece = getRandomPiece();
    }


    void render() {
        clear();

        for(auto row : grid) {
            for(auto block : row) {
                switch(block.is_occupied) {
                    case 0:
                        printw(" - ");
                        break;
                    case 1:
                        printw("[ ]");
                        break;
                    case 2:
                        printw("[-]");
                        break;
                }
            }
            printw("\n");
        }

        printw("%d", score);
        refresh();
    }


    bool pieceOverlapping(Piece piece) {
        for(auto block : piece.blocks) {
            point2<int> block_position = {
                block.x() + piece.position.x(),
                block.y() + piece.position.y(),
            };
            if(
                block_position.x() >= 20
                || block_position.x() < 0
                || block_position.y() < 0
                || block_position.y() >= 10
            ) return 1;

            if(grid[block_position.x()][block_position.y()].is_occupied == 1) {
                return 1;
            }
        }
        return 0;
    }

    void freezePiece() {
        for(auto block : current_piece.blocks) {
            grid[block.x() + current_piece.position.x()][block.y() + current_piece.position.y()].is_occupied = 1;
        }
    }

    /* freezes the current piece and makes a new one */
    void setUpNewPiece() {
        freezePiece();
        current_piece = getRandomPiece();
        current_piece.position = point2<int>(0, 4);

    }

    void clearLine(int x) {
        for(auto& block : grid[x]) if(block.is_occupied == 1) block.is_occupied = 0;
    }

    void pushDown(int x) {
        for(int i = x-1; i >= 0; i--) {
            clearLine(i+1);
            grid[i+1] = grid[i]; // TODO: check if blocks are active or not 
        }


    }

    /* clears the board up and return the number of rows cleared */
    int clearUp() {
        int row_num = 0;
        for(int i = 0; i < grid.size(); i++) {
            bool should_clear = 1;
            for(auto& block : grid[i]) {
                if(block.is_occupied != 1) {should_clear = 0; break;}
            }

            if(should_clear) {
                pushDown(i);
                row_num++;
            }


        }


        return row_num;
    }

    /* moves the active piece, returns 1 if it should freeze */
    bool move(int pressed_key) {
        bool falls_down = 0;
        double G = pow((0.8 - (level-1) * 0.007), level-1);
        if(global_timer % (int)(60 * G) == 0) falls_down = 1;

        Piece test_piece = current_piece;
        bool stop = 0;
        if(falls_down) {
            test_piece.position.x()++;
            if(pieceOverlapping(test_piece)) return 1;
            current_piece = test_piece;

        }



        switch(pressed_key) {
          case 'x':
            test_piece.rotateClockwise(); break;
          case KEY_LEFT:
            test_piece.position.y()--; break;
          case KEY_RIGHT:
            test_piece.position.y()++; break;
          case KEY_DOWN:
            test_piece.position.x()++; break;
        }

        if(pieceOverlapping(test_piece)) return 0;
        current_piece = test_piece;

        return 0;

    }


    void update(int pressed_key) {
        global_timer++;

        for(auto& row : grid) {
            for(auto& block : row) {
                if(block.is_occupied == 2) block.is_occupied = 0;
            }
        }


        for(auto block : current_piece.blocks) {
            grid[block.x() + current_piece.position.x()][block.y() + current_piece.position.y()].is_occupied = 2;
        }


        int freeze = move(pressed_key);
        if(freeze) setUpNewPiece();

        int lines_cleared_curr = clearUp();
        lines_cleared += lines_cleared_curr;

        level = lines_cleared / 10 + 1;

        switch(lines_cleared_curr) {
            case 0: break;
            case 1: score += 40 * (level+1); break;
            case 2: score += 100 * (level+1); break;
            case 3: score += 300 * (level+1); break;
            case 4: score += 1200 * (level+1); break;
        }
        
        
    }
};


int main() {
    srand(time(NULL));

    Grid grid;
    grid.gameInit();

    bool running = true;
    while(running) {
        int pressed_key = getch();
        grid.update(pressed_key);
        grid.render();

        std::this_thread::sleep_for(std::chrono::microseconds(16000));
    }


    endwin();
    return 0;
}
