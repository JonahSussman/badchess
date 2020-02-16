/* 
TODO: Pawn promotion
TODO: Cannot put self in check
TODO: Check if castle is legal
TODO: Move number
*/

#include <cstring>   // memcopy
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <utility>   // pair
#include <algorithm> // find

typedef std::pair<int, int> IntPair;

char swap_case(char ch) {
    if (isupper(ch)) {
        ch = tolower(ch);
    } else if (islower(ch)) {
        ch = toupper(ch);
    }

    return ch;
}

// Could use lookup table to make faster
std::string ind_alg(const int ind) {
    return std::string({(char)((ind % 12) + 'a' - 2), (char)(10 - (ind / 12) + '0')});
}

int alg_ind(const std::string alg) {
    if (!isalpha(alg[0]) || !isdigit(alg[1])) return -1;
    return alg[0] - 'a' + 2 + (10 - (alg[1] - '0')) * 12;
}

int alg_ind(const char* alg) {
    if (!isalpha(alg[0]) || !isdigit(alg[1])) return -1;
    return alg[0] - 'a' + 2 + (10 - (alg[1] - '0')) * 12;
}

char OPENING[144] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', 'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r', ' ', ' ', 
    ' ', ' ', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p', ' ', ' ', 
    ' ', ' ', '.', '.', '.', '.', '.', '.', '.', '.', ' ', ' ', 
    ' ', ' ', '.', '.', '.', '.', '.', '.', '.', '.', ' ', ' ', 
    ' ', ' ', '.', '.', '.', '.', '.', '.', '.', '.', ' ', ' ', 
    ' ', ' ', '.', '.', '.', '.', '.', '.', '.', '.', ' ', ' ', 
    ' ', ' ', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', ' ', ' ', 
    ' ', ' ', 'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
};

const int N = -12;
const int E =  1;
const int S =  12;
const int W = -1;

const int A1 = alg_ind("a1");
const int H1 = alg_ind("h1");
const int A8 = alg_ind("a8");
const int H8 = alg_ind("h8");

const std::map<char, std::vector<int>> DIRECTIONS = {
    {'P', {N+W, N, N+E, N+N}},
    {'B', {N+E, N+W, S+E, S+W}},
    {'N', {N+N+E, N+E+E, S+E+E, S+S+E, S+S+W, S+W+W, N+W+W, N+N+W}},
    {'R', {N, E, S, W}},
    {'Q', {N, N+E, E, S+E, S, S+W, W, N+W}},
    {'K', {N, N+E, E, S+E, S, S+W, W, N+W}}
};

class Position {
public:
    char* board_array;
    // Castling rights, interpreting data differently:
    
    // Left side, right side. However, when swapping data, king becomes queen
    // {queenside, kingside}
    std::pair<bool, bool> w_castle;
    // {kingside, queenside}
    std::pair<bool, bool> b_castle;

    // En passant square
    int  en_passant;
    bool white_to_move;

    int move_number;

    Position() {
        board_array = new char[144];
        for (int i = 0; i < 144; i++) {
            board_array[i] = ' ';
        }

        w_castle = {false, false};
        b_castle = {false, false};

        en_passant = -1;
        white_to_move = true;
    }

    Position(char b[144], std::pair<bool, bool> wc, std::pair<bool, bool> bc, int ep, bool wtm) {
        board_array = new char[144];
        std::memcpy(board_array, b, 144);

        w_castle = wc;
        b_castle = bc;

        en_passant = ep;
        white_to_move = wtm;
    }

    void assign_from(const Position& rhs) {
        board_array = new char[144];
        std::memcpy(board_array, rhs.board_array, 144);

        w_castle = rhs.w_castle;
        b_castle = rhs.b_castle;

        en_passant = rhs.en_passant;
        white_to_move = rhs.white_to_move;
    }

    // Copy constructor: Previously unititialized object w/ other's data
    Position(const Position& rhs) {
        assign_from(rhs);
    }
    
    // Assignment operator: Previously initialized object
    Position& operator=(const Position& rhs) {
        delete[] board_array;
        assign_from(rhs);

        return *this;
    }

    ~Position() {
        delete[] board_array;
    }

    std::vector<IntPair> generate_moves() {
        std::vector<IntPair> move_list;
        std::vector<bool> attacked = squares_attacked();
        
        /*
        Go through the board array and continuously generate valid moves in the
        direction the current piece can move.

        p  = char of the piece we are currently looking at
        pi = index of p in board_array
        t  = char of the piece we are looking to move to
        ti = index of t in board_array
        */
        bool left_castle =  w_castle.first;
        bool right_castle = w_castle.second;

        for (int pi = 0; pi < 144; pi++) {
            char p = board_array[pi];
            //std::cout << "checking piece : " << board_array[0] << std::endl; 

            /* 
            Because we flip the board each time, the only pieces that we care
            about are the ones that are capital letters
            */

            if (!isupper(p)) continue;
            
            for (int dir : DIRECTIONS.at(p)) {
                for (int ti = pi + dir; ti < 144 && ti >= 0; ti += dir) {
                    char t = board_array[ti];

                    if (t == ' ' || isupper(t)) break;

                    if (p == 'P') {
                        if ((dir == N || dir == N+N) && t != '.') break;
                        if (dir == N+N && board_array[ti-N] != '.') break;
                        if (dir == N+N && !(alg_ind("a2") <= pi && pi <= alg_ind("h2"))) break;
                        if ((dir == N+W || dir == N+E) && t == '.' && ti != en_passant) break;
                    }

                    move_list.push_back({pi, ti});
                    // Remove attacked thing
                    /*if (p == 'K' && attacked[pi]) {
                        left_castle  = false;
                        right_castle = false;
                    }*/

                    if (p == 'P' || p == 'N' || p == 'K') break;
                    
                    if (p == 'R') {
                        // Left castle
                        if (left_castle && pi == A1 && dir == E) {
                            if (attacked[pi] || attacked[ti]) {
                                left_castle = false;
                            } else if (board_array[ti+E] == 'K' && !attacked[ti+E]) {
                                left_castle = false;
                                move_list.push_back({ti+E, ti+W});
                            }
                        }
                        // Right Castle
                        if (right_castle && pi == H1 && dir == W) {
                            if (attacked[pi] || attacked[ti]) {
                                right_castle = false;
                            } else if (board_array[ti+W] == 'K' && !attacked[ti+E]) {
                                right_castle = false;
                                move_list.push_back({ti+W, ti+E});
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i < move_list.size(); i++) {
            Position test_pos = *this;
            test_pos.move(move_list[i]);
            for (int j = 0; j < 144; j++) {
                if (test_pos.board_array[j] == 'K') {
                    if (test_pos.squares_attacked()[j]) {
                        move_list.erase(move_list.begin() + i);
                        i--;
                    }
                }
            }
        }

        return move_list;
    }

    void move(IntPair move) {
        int& pi = move.first;
        int& ti = move.second;
        char& p = board_array[pi];
        char& t = board_array[ti];

        t = p;
        p = '.';

        if (pi == A1) w_castle.first  = false;
        if (pi == H1) w_castle.second = false;
        if (pi == A8) b_castle.second = false;
        if (pi == H8) b_castle.first  = false;

        if (t == 'K') {
            w_castle = {false, false};
            if (abs(ti - pi) == 2) {
                int tri = (ti + pi) / 2;
                int ri  = (ti > pi) ? H1 : A1;
                board_array[ri]  = '.';
                board_array[tri] = 'R';
            }
        }

        if (t == 'P') {
            if (alg_ind("a8") <= ti && ti <= alg_ind("h8")) {
                t = 'Q';
            }
            if (ti - pi == N+N) {
                en_passant = pi + N;
            }
            if (ti == en_passant) {
                en_passant = -1;
                board_array[ti+S] = '.';
            }
        } else {
            en_passant = -1;
        }
    }

    void rotate() {
        char flipped[144];
        std::memcpy(flipped, board_array, 144);
        // TODO: Make this not fugly
        for (int i = 0, ii = 7; i < 8; i++, ii--) {
            for (int j = 0, jj = 7; j < 8; j++, jj--) {
                char &ch = flipped[(i + 2)*12 + j + 2];
                ch = board_array[(ii + 2)*12 + jj + 2];

                ch = swap_case(ch);
            }
        }

        std::memcpy(board_array, flipped, 144);

        std::swap(w_castle, b_castle);

        white_to_move = !white_to_move;
        en_passant = 143 - en_passant;
    }
    
    std::vector<bool> squares_attacked() {
        std::vector<bool> attacked_indicies(144, false);

        for (int pi = 0; pi < 144; pi++) {
            char p = board_array[pi];

            if (!islower(p)) continue;

            p = toupper(p);
            
            for (int dir : DIRECTIONS.at(p)) {
                dir = -dir;

                for (int ti = pi + dir; ti < 144 && ti >= 0; ti += dir) {
                    char t = board_array[ti];

                    if (t == ' ' || islower(t)) break;

                    if (p == 'P') {
                        if (dir == S || dir == S+S) break;
                    }

                    attacked_indicies[ti] = true;
                    
                    if (isupper(t)) break;

                    if (p == 'P' || p == 'N' || p == 'K') break;
                }
            }
        }
        /*
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (attacked_indicies[(i + 2)*12 + j + 2]) {
                    std::cout << "x";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        */
        return attacked_indicies;
    }
};

void print_position(Position& pos) {
    if (pos.white_to_move) {
        // std::cout << "White! En passant square: " << ind_alg(pos.en_passant) << std::endl;
        for (int i = 0; i < 8; i++) {
            std::cout << (char)('8' - i) << " ";
            for (int j = 0; j < 8; j++) {
                std::cout << pos.board_array[(i + 2)*12 + j + 2];
            }
            std::cout << std::endl;
        }
    } else {
        // std::cout << "Black! En passant square: " << ind_alg(143 - pos.en_passant) << std::endl;
        for (int i = 7; i >= 0; i--) {
            std::cout << (char)('8' - (7 - i)) << " ";
            for (int j = 7; j >= 0; j--) {
                char& ch = pos.board_array[(i + 2)*12 + j + 2];
                std::cout << swap_case(ch);
            }
            std::cout << std::endl;
        } 
    }
    std::cout << "  abcdefgh" << std::endl;
}

IntPair str_to_move(std::string& str, bool wtm) {
    if (!isalpha(str[0]) || !isalpha(str[3]) || !isdigit(str[1]) || !isdigit(str[4]) || str[2] != '-') {
        return {-1, -1};
    } else {
        if (wtm) {
            return {alg_ind(str.substr(0, 2)), alg_ind(str.substr(3, 2))};
        }
        else {
            char ip[2] = {(char)('h' - str[0] + 'a'), (char)('8' - str[1] + '1')};
            char tp[2] = {(char)('h' - str[3] + 'a'), (char)('8' - str[4] + '1')};
            return {alg_ind(ip), alg_ind(tp)};
        }
    }
}

namespace eng {
    class Engine {
    public:
        // Engine(bool w): is_white(w) {}
        virtual IntPair get_move(Position& pos) = 0;
    protected:
        bool is_white;
    };

    class Alphabetical: public Engine {
    public:
        Alphabetical(bool w) {}//: Engine(w) {}

        IntPair get_move(Position& pos) override {
            std::vector<IntPair> moves = pos.generate_moves();
            std::vector<std::pair<std::string, int>> alpha_moves;

            for (IntPair move : moves) {
                alpha_moves.push_back({is_white ? ind_alg(move.second) : ind_alg(143 - move.second), move.first});
                std::cout << ind_alg(143 - move.second);
            }

            std::sort(alpha_moves.begin(), alpha_moves.end());

            std::cout << alpha_moves[0].first << std::endl;
            return {alpha_moves[0].second, is_white ? alg_ind(alpha_moves[0].first) : 143 - alg_ind(alpha_moves[0].first)};
        }
    };
}

int main(int argc, char* argv[]) {
    std::vector<Position> move_history;
    bool game_running = true;

    std::map<std::string, bool> flags = {
        {"-import", false}, {"-help", false}
    };

    for (int i = 1; i < argc; i++) {
        if (flags.find(argv[i]) == flags.end()) {
            std::cout << "Invalid flag. Type \"./badchess -help\" for more information." << std::endl;
            return -1;
        }
        if (strcmp(argv[i], "-import") == 0) {

        // TODO: MAKE NOT FUGLY <------- VERY VERY VERY VERY VERY VERY IMPORTANT

            i++;
            if (i < argc && !flags["-import"] && flags.find(argv[i]) == flags.end()) {
                flags["-import"] = true;

                std::fstream in_file(argv[i], std::fstream::in);

                Position init;

                char ch;
                int phase = 0;
                int x = 0;
                int y = 0;
                bool must_rotate = false;

                while(in_file.get(ch)) {
                    if (ch == ' ') {
                        phase++;
                        continue;
                    }

                    if (phase == 0) {
                        if (isdigit(ch)) {
                            for (int j = 0; j < (ch - '0'); j++) {
                                init.board_array[(y + 2)*12 + x + 2] = '.';
                                x++;
                            }
                        } else if (isalpha(ch)) {
                            init.board_array[(y + 2)*12 + x + 2] = ch;
                            x++;
                        } else if (ch == '/') {
                            x = 0;
                            y++;
                        } 
                    }

                    if (phase == 1) {
                        if (ch == 'b' || ch == 'B') {
                            must_rotate = true;
                        }
                    }

                    if (phase == 2) {
                        if (ch == 'K') {
                            init.w_castle.second = true;
                        } else if (ch == 'Q') {
                            init.w_castle.first  = true;
                        } else if (ch == 'k') {
                            init.b_castle.first  = true;
                        } else if (ch == 'q') {
                            init.b_castle.second = true;
                        }
                    }

                    if (phase == 3) {
                        if (ch != '-') {
                            std::string ep_alg;
                            ep_alg += ch;
                            in_file.get(ch);
                            ep_alg += ch;
                            init.en_passant = alg_ind(ep_alg);
                        }
                    }

                    //TODO: implement move number
                }

                if (must_rotate) {
                    init.rotate();
                }
                
                in_file.close(); 

                move_history.emplace_back(init);
                
            } else {
                std::cout << "Incorrect usage. Type \"./badchess -help\" for more information." << std::endl;
                return -1;
            }
        }
    }

    if (move_history.size() == 0) {
        move_history.emplace_back(
            Position(
                OPENING, {true, true}, {true, true}, -1, true
            )
        );
    }
    
    eng::Engine* ai = new eng::Alphabetical(false);

    while(game_running) {
        move_history.push_back(move_history.back());
        Position& curr_pos = move_history.back();
        print_position(curr_pos);
        std::vector<IntPair> legal_moves = curr_pos.generate_moves();

        /*for (IntPair move : legal_moves) {
            std::cout << ind_alg(move.first) << "-" << ind_alg(move.second) << std::endl;
        }*/

        if (legal_moves.size() == 0) {
            for (int i = 0; i < 144; i++) {
                if (curr_pos.board_array[i] != 'K') continue;
                if (!curr_pos.squares_attacked()[i]) {
                    std::cout << "Stalemate!" << std::endl;
                } else {
                    std::cout << ((curr_pos.white_to_move) ? "Black " : "White ") << "wins!" << std::endl;
                }
            }
            game_running = false;
            break;
        }

        std::cout << "Your move: ";
        std::string move_str;
        std::cin >> move_str;
        IntPair attempted_move = str_to_move(move_str, curr_pos.white_to_move);
        while (std::find(legal_moves.begin(), legal_moves.end(), attempted_move) == legal_moves.end()) {
            std::cout << attempted_move.first << ", " << attempted_move.second << std::endl;
            std::cout << "Invalid move. Your move: ";
            std::cin >> move_str;
            attempted_move = str_to_move(move_str, curr_pos.white_to_move);
        }

        curr_pos.move(attempted_move);
        curr_pos.rotate();

        move_history.push_back(move_history.back());
        Position& curr_pos_1 = move_history.back();
        print_position(curr_pos_1);

        curr_pos_1.move(ai->get_move(curr_pos_1));
        curr_pos_1.rotate();
    }

    return 0;
}