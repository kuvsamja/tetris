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
            new_point = point2<double>{-new_point.y(), new_point.x()};
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
    std::vector<std::vector<Block>> grid{20, std::vector<Block>(10, {0})};
    Piece current_piece;
    Piece next_piece;
    point2<int> piece_position;
    int piece_fall_timer = 10; // time it takes for a piece to descend 1 block in ms
    uint64_t timer = 0;

  public:
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
        piece_position.x() = 0;
        piece_position.y() = 4;
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

        refresh();
    }




    void update(int pressed_key) {
        // current_piece = next_piece;
        // next_piece = getRandomPiece();
        timer++;


        switch(pressed_key) {
            case KEY_LEFT:
                piece_position.y()--;
                break;
            case KEY_RIGHT:
                piece_position.y()++;
                break;
            case KEY_DOWN:
                piece_position.x()++;
                break;
            case 'x':
                current_piece.rotateClockwise();
        }


        for(auto& row : grid) {
            for(auto& block : row) {
                if(block.is_occupied == 2) block.is_occupied = 0;
            }
        }


        for(auto block : current_piece.points) {
            grid[block.x() + piece_position.x()][block.y() + piece_position.y()].is_occupied = 2;
        }

        if(timer % 30 == 0) piece_position.x()++;

        for(auto block : current_piece.points) { // matija rewrite ovaj block i napravi collision check8ing za sve
            if(block.x() + piece_position.x() == 19
                || grid[(block.x()+1)+piece_position.x()][block.y()+piece_position.y()].is_occupied == 1) {

                for(auto block1 : current_piece.points)
                    grid[block1.x() + piece_position.x()][block1.y() + piece_position.y()].is_occupied = 1;
                piece_position = {0, 4};
                current_piece = getRandomPiece();
                break;
            }
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

        usleep(16000);
    }
    

    endwin();
    return 0;
}