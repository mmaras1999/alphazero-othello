#ifndef OTHELLO
#define OTHELLO

#include <vector>
#include <string>

struct GameState {
private:
    const std::vector <std::pair <char, char> > MOVE_DIRECTIONS = {
        {-1, -1},
        {-1,  0},
        {-1,  1},
        { 0, -1},
        { 0,  1},
        { 1, -1},
        { 1,  0},
        { 1,  1}
    };

    bool can_place(const int x, const int y, const int player) const {
        if (game_board[x][y]) {
            return false;
        }

        for (auto dir : MOVE_DIRECTIONS) {
            bool enemy_exists = false;
            bool ended_on_my = false;
            
            int next_x = x + dir.first;
            int next_y = y + dir.second;

            while (
                0 <= next_x and next_x < 8 and
                0 <= next_y and next_y < 8
            ) {
                if (game_board[next_x][next_y] == player) {
                    ended_on_my = true;
                    break;
                }
                else if (game_board[next_x][next_y] == (player ^ 3)) {
                    enemy_exists = true;
                }
                else {
                    break;
                }

                next_x += dir.first;
                next_y += dir.second;
            }
        
            if (enemy_exists and ended_on_my) {
                return true;
            }
        }

        return false;
    }

public:
    unsigned char game_board[8][8];
    int current_player = 2;  // black by default

    GameState() : game_board{} {
        // init game board
        game_board[3][3] = 1;  // white
        game_board[3][4] = 2;  // black
        game_board[4][3] = 2;  // black
        game_board[4][4] = 1;  // white

        current_player = 2;  // set the player to black
    }

    std::vector <std::pair <int, int> > get_valid_moves() const {
        std::vector <std::pair <int, int> > valid_moves;
        
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (can_place(i, j, current_player)) {
                    valid_moves.emplace_back(i, j);
                }
            }
        }

        if (valid_moves.size() == 0 and not is_terminal()) {
            valid_moves.emplace_back(-1, -1);
        }

        return valid_moves;
    }

    void make_move(const std::pair<int, int>& move) {
        // assumes that the move is valid!
        if (move.first == -1) {
            // skip move
            current_player ^= 3;
            return;
        }

        game_board[move.first][move.second] = current_player;

        for (auto dir : MOVE_DIRECTIONS) {
            bool ended_on_my = false;

            std::vector <std::pair <int, int> > enemies;
    
            int next_x = move.first + dir.first;
            int next_y = move.second + dir.second;

            while (
                0 <= next_x and next_x < 8 and
                0 <= next_y and next_y < 8
            ) {
                if (game_board[next_x][next_y] == current_player) {
                    ended_on_my = true;
                    break;
                }
                else if (game_board[next_x][next_y] == (current_player ^ 3)) {
                    enemies.emplace_back(next_x, next_y);
                }
                else {
                    break;
                }

                next_x += dir.first;
                next_y += dir.second;
            }
        
            if (ended_on_my) {
                for (auto& enemy : enemies) {
                    game_board[enemy.first][enemy.second] ^= 3;
                }
            }
        }

        for (auto& dir : MOVE_DIRECTIONS) {
            
        }

        current_player ^= 3;  // swap players
    }

    std::pair <int, int> get_scores() const {
        int scores[] = {0, 0};

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (game_board[i][j]) {
                    scores[game_board[i][j] - 1]++;
                }
            }
        }
        
        return {scores[0], scores[1]};
    }

    bool is_terminal() const {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (can_place(i, j, 1) or can_place(i, j, 2)) {
                    return false;
                }
            }
        }
    
        return true;
    }

    std::string draw() const {
        std::string board = "";

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (game_board[i][j] == 0) {
                    board += "_";
                }
                else if (game_board[i][j] == 1) {
                    board += "o";
                }
                else if (game_board[i][j] == 2) {
                    board += "@";
                }
            }
            board += "\n";
        }

        return board;
    }
};

#endif
