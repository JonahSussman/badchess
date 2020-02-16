// Pawn promotion needs work

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

inline int alg_ind(const char* str) {
    int result = (8 + '0' - str[1]) * 8 + (str[0] - 'a');
    return (0 <= result && result <= 63) ? result : -1;
}
inline int alg_ind_flip(const char* str) {
    int result = (str[1] - '1') * 8 + (str[0] - 'a');
    return (0 <= result && result <= 63) ? result : -1;
}
inline int alg_ind(const std::string str) {
    return alg_ind(str.c_str());
}
inline int alg_ind_flip(const std::string str) {
    return alg_ind_flip(str.c_str());
}
inline std::string ind_alg(const int i) {
    return {(char)((i % 8) + 'a'), (char)(8 - (i / 8) + '0')};
}
inline std::string ind_alg_flip(const int i) {
    return {(char)((i % 8) + 'a'), (char)((i / 8) + '1')};
}

const int MAILBOX[120] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
    -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
    -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
    -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
    -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
    -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
    -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
    -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
const int MAIL64[64] = {
    21,  22,  23,  24,  25,  26,  27,  28,
    31,  32,  33,  34,  35,  36,  37,  38,
    41,  42,  43,  44,  45,  46,  47,  48,
    51,  52,  53,  54,  55,  56,  57,  58,
    61,  62,  63,  64,  65,  66,  67,  68,
    71,  72,  73,  74,  75,  76,  77,  78,
    81,  82,  83,  84,  85,  86,  87,  88,
    91,  92,  93,  94,  95,  96,  97,  98
};
const int CONVERT[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
    8,  9,  10, 11, 12, 13, 14, 15,
    0,  1,  2,  3,  4,  5,  6,  7
};

char OPENING[64] = {
    'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
    'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p', 
    '.', '.', '.', '.', '.', '.', '.', '.', 
    '.', '.', '.', '.', '.', '.', '.', '.', 
    '.', '.', '.', '.', '.', '.', '.', '.', 
    '.', '.', '.', '.', '.', '.', '.', '.', 
    'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P', 
    'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'
};

enum MOVE_TYPE {
    QUIET,      DOUBLE_PAWN,
    K_CASTLE,   Q_CASTLE,
    CAPTURE,    EP_CAPTURE,
    PROM_N,     PROM_B,
    PROM_R,     PROM_Q,
    PROM_CAP_N, PROM_CAP_B,
    PROM_CAP_R, PROM_CAP_Q
};
struct Move {
    unsigned int from : 6;
    unsigned int to   : 6;
    unsigned int type : 4;

    char piece;
    char target;
};

enum MOVE_CODE {
    SUCCESS,
    ALG_MALFORM,    ALG_INVALID_PROMO,
    ALG_NO_TARGET,  ALG_NO_MATCH
};

const int N = -10;
const int S =  10;
const int E =  1;
const int W = -1;

const int A1 = 56;
const int A2 = 48;
const int H1 = 63;
const int H2 = 55;
const int A8 = 0;
const int H8 = 7;

std::map<char, std::vector<int>> DIRECTIONS = {
    {'P', {N+W, N, N+E, N+N}},
    {'B', {N+E, N+W, S+E, S+W}},
    {'N', {N+N+E, N+E+E, S+E+E, S+S+E, S+S+W, S+W+W, N+W+W, N+N+W}},
    {'R', {N, E, S, W}},
    {'Q', {N, N+E, E, S+E, S, S+W, W, N+W}},
    {'K', {N, N+E, E, S+E, S, S+W, W, N+W}}
};

struct Castle {
    bool wq : 1;
    bool wk : 1;
    bool bq : 1;
    bool bk : 1;
};

class Position {
protected:
    enum TOKEN {
        PIECE,  TAKES,
        RANK,   FILE,
        PROMO,  CHECK,
        Q_C,    K_C,
        DONE
    };

    void assign_from(const Position& rhs) {
        std::memcpy(board, rhs.board, 64);

        rights = rhs.rights;
        ep = rhs.ep;
        whites_move = rhs.whites_move;
        ply = rhs.ply;

        move_list = rhs.move_list;
        std::memcpy(attacked, rhs.attacked, 64);
    }
    
    void squares_attacked() {
        for (int i = 0; i < 64; i++) {
            attacked[i] = false;
        }

        for (int i = 0; i < 64; i++) {
            char p = board[i];
            if (!islower(p)) continue;

            for (int dir : DIRECTIONS.at(toupper(p))) {
                dir = -dir;

                for (int j = MAIL64[i] + dir; 0 <= j && j <= 120; j += dir) {
                    int ti = MAILBOX[j];
                    if (ti == -1) break;
                    if (p == 'p' && (dir == S || dir == S+S)) break; 
                    
                    attacked[ti] = true;

                    if (p == 'k' || p == 'n' || p == 'p') break;
                    if (board[ti] != '.') break;
                }

            }
        }
        /*
        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 8 ; j++) {
                std::cout << (attacked[i*8 + j]) ? "x" : ".";
            } std::cout << std::endl;
        } std::cout << std::endl;*/
    }
public:
    std::vector<Move> move_list;
    Move performed_move;
    bool attacked[64] = {false};
    char board[64];
    
    Castle rights;

    int ep;
    bool whites_move;

    int ply;

    Position(
        char _board[64], Castle _rights, int _ep, int _whites_move, int _ply
    ) {
        std::memcpy(board, _board, 64);

        rights = _rights;
        ep = _ep;
        whites_move = _whites_move;
        ply = _ply;

        squares_attacked();
        generate_moves();
    }

    void generate_moves() {
        move_list.clear();
        squares_attacked();

        bool qc = rights.wq;
        bool kc = rights.wk;

        for (unsigned int pi = 0; pi < 64; pi++) {
            char p = board[pi];
            if (!isupper(p)) continue;

            for (int d : DIRECTIONS.at(p)) {
                for (int j = MAIL64[pi] + d; 0 <= j && j <= 120; j += d) {
                    if (MAILBOX[j] == -1) break;
                    unsigned int ti = MAILBOX[j];                    

                    char t = board[ti];
                    if (isupper(t)) break;

                    if (p == 'P') {
                        if (d == N && t != '.') break;
                        
                        if (d == N+N) {
                            if (t != '.') break;
                            if (!(A2 <= pi && pi <= H2)) break;
                            if (board[ti+8] != '.') break;
                            
                            move_list.push_back({pi, ti, DOUBLE_PAWN, p});
                            break;
                        } 
                        if (d == N+E || d == N+W) {
                            if (t == '.') {
                                if ((int)ti != ep) break;
                                move_list.push_back({pi, ti, EP_CAPTURE, p, 'p'});
                                break;
                            } else if (A8 <= ti && ti <= H8) {
                                move_list.push_back({pi, ti, PROM_CAP_N, p, t});
                                move_list.push_back({pi, ti, PROM_CAP_B, p, t});
                                move_list.push_back({pi, ti, PROM_CAP_R, p, t});
                                move_list.push_back({pi, ti, PROM_CAP_Q, p, t});
                                break;
                            }
                        } else {
                            if (0 <= ti && ti <= 7) {
                                move_list.push_back({pi, ti, PROM_N, p});
                                move_list.push_back({pi, ti, PROM_B, p});
                                move_list.push_back({pi, ti, PROM_R, p});
                                move_list.push_back({pi, ti, PROM_Q, p});
                                break;
                            }
                        }
                    }

                    if (t == '.') {
                        move_list.push_back({pi, ti, QUIET, p});
                    } else {
                        move_list.push_back({pi, ti, CAPTURE, p, t});
                        break;
                    }

                    if (p == 'K' || p == 'N' || p == 'P') break;

                    if (p == 'R') {
                        // Queen castle
                        if (qc && pi == A1 && d == E) {
                            if (attacked[pi] || attacked[ti]) {
                                qc = false;
                            } else if (board[ti+E] == 'K' && !attacked[ti+E]) {
                                qc = false;
                                move_list.push_back({ti+E, ti+W, Q_CASTLE, 'K'});
                            }
                        }
                        // King Castle
                        if (kc && pi == H1 && d == W) {
                            if (attacked[pi] || attacked[ti]) {
                                kc = false;
                            } else if (board[ti+W] == 'K' && !attacked[ti+E]) {
                                kc = false;
                                move_list.push_back({ti+W, ti+E, K_CASTLE, 'K'});
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i < move_list.size(); i++) {
            Position test_pos = *this;
            test_pos.move(move_list[i]);
            
            for (int j = 0; j < 64; j++) {
                if (test_pos.board[j] == 'K') {
                    if (test_pos.attacked[j]) {
                        move_list.erase(move_list.begin() + i);
                        i--;
                    }

                }
            }
        }
    }

    /*
    int count_rank(int index, char ch) {
        int count = 0;
        int rank  = index / 8;

        for (int i = 0; i < 8; i++) {
            if (board[rank + i] == ch) count++;
        }

        return count;
    }

    int count_file(int index, char ch) {
        int count = 0;
        int file  = index % 8;

        for (int i = 0; i < 8; i++) {
            if (board[i*8 + file] == ch) count++;
        }

        return count;
    }*/

    // BAD BAD BAD REDO REDO REDO
    /*
    std::vector<std::string> generate_alg(std::vector<Move> moves) {
        std::vector<std::string> output;

        for (int i = 0; i < moves.size(); i++) {
            Move& move = moves[i];

            output.push_back("");
            std::string& alg = output[i];

            alg += move.piece;
            alg += whites_move ? ind_alg(move.from) : ind_alg_flip(move.from);

            if (move.type == CAPTURE || move.type == EP_CAPTURE ||
                move.type == PROM_CAP_N || move.type == PROM_CAP_B ||
                move.type == PROM_CAP_R || move.type == PROM_CAP_Q) 
            {
                alg += 'x';
            }

            alg += whites_move ? ind_alg(move.to) : ind_alg_flip(move.to);

            if (move.type == PROM_N || move.type == PROM_CAP_N) {
                alg += "=N";
            }
            if (move.type == PROM_B || move.type == PROM_CAP_B) {
                alg += "=B";
            }
            if (move.type == PROM_R || move.type == PROM_CAP_R) {
                alg += "=R";
            }
            if (move.type == PROM_Q || move.type == PROM_CAP_Q) {
                alg += "=Q";
            }
        }

        return output;
    }*/

    MOVE_CODE move(Move move) {
        int pi = move.from;
        int ti = move.to;
        char& p = board[pi];
        char& t = board[ti];

        t = p;
        p = '.';

        if (pi == A1) rights.wq = false;
        if (pi == H1) rights.wk = false;
        if (pi == A8) rights.bq = false;
        if (pi == H8) rights.bk = false;

        if (t == 'K') {
            rights.wk = false;
            rights.wq = false;

            if (move.type == K_CASTLE || move.type == Q_CASTLE) {
                int tri = (ti + pi) / 2;
                int ri  = (ti > pi) ? H1 : A1;
                board[ri]  = '.';
                board[tri] = 'R';
            }
        }

        if (t == 'P') {
            switch(move.type) {
                case PROM_N:
                case PROM_CAP_N:
                    t = 'N';
                    break;
                case PROM_B:
                case PROM_CAP_B:
                    t = 'B';
                    break;
                case PROM_R:
                case PROM_CAP_R:
                    t = 'R';
                    break;
                case PROM_Q:
                case PROM_CAP_Q:
                    t = 'Q';
                    break;
                case DOUBLE_PAWN:
                    ep = pi + 8;
                    break;
                case EP_CAPTURE:
                    ep = -1;
                    board[ti-8] = '.';
            }
        } else {
            ep = -1;
        }

        performed_move = move;

        return SUCCESS;
    }

    MOVE_CODE move(std::string alg) { // Most likely can combine tokens with string itself
        if (alg.length() < 2) return ALG_MALFORM;

        Move guess_move = {0, 0, QUIET, 'P', -1};
        std::vector<Move> filtered;
        std::vector<TOKEN> move_tok;
        char from_rank = -1; 
        char from_file = -1;

        for (int i = 0; i < alg.length(); i++) {
            switch (alg[i]) {
                case 'P': case 'R': case 'N': 
                case 'B': case 'Q': case 'K': 
                    //std::cout << "PIECE ";
                    move_tok.push_back(PIECE);
                    break;
                case 'x':
                    //std::cout << "TAKES ";
                    move_tok.push_back(TAKES);
                    break;
                case '1': case '2': case '3': case '4': 
                case '5': case '6': case '7': case '8':
                    //std::cout << "RANK ";
                    move_tok.push_back(RANK);
                    break;
                case 'a': case 'b': case 'c': case 'd': 
                case 'e': case 'f': case 'g': case 'h':
                    //std::cout << "FILE "; 
                    move_tok.push_back(FILE);
                    break;
                case '=':
                    switch(alg[++i]) {
                        case 'N': case 'B':
                        case 'R': case 'Q': 
                            //std::cout << "PROMO ";
                            move_tok.push_back(PROMO);
                            break;
                        default:
                            return ALG_INVALID_PROMO;
                    }
                    break;
                case '+': case '#':
                    //std::cout << "CHECK ";
                    move_tok.push_back(CHECK);
                    break;
                case 'O':
                    if (alg == "O-O") {
                        move_tok.push_back(K_C);
                        i = alg.length();
                        break;
                    } else if (alg == "O-O-O") {
                        move_tok.push_back(Q_C);
                        i = alg.length();
                        break;
                    }
                    
                default:
                    return ALG_MALFORM;
            }
        }

        /*for (TOKEN t : move_tok) {
            std::cout << t << " ";
        }*/
        
        int ai    = alg.length() - 1;
        int ti    = move_tok.size() - 1;
        TOKEN tok = move_tok.back();

        if (tok == K_C) {
            std::cout << "king";

            guess_move = {alg_ind("e1"), alg_ind("g1"), K_CASTLE, 'K'};
            tok = DONE;
        }
        if (tok == Q_C) {
            std::cout << "queen";
            guess_move = {alg_ind("e1"), alg_ind("c1"), Q_CASTLE, 'K'};
            tok = DONE;
        }

        if (tok == CHECK) {
            tok = move_tok[--ti];
            ai--;
        }
        if (tok == PROMO) {
            tok = move_tok[--ti];

            if (alg[ai] == 'R') {
                guess_move.type = PROM_R;
            } else if (alg[ai] == 'B') {
                guess_move.type = PROM_B;
            } else if (alg[ai] == 'N') {
                guess_move.type = PROM_N;
            } else if (alg[ai] == 'Q') {
                guess_move.type = PROM_Q;
            }

            ai -= 2;
        }
        if (tok != RANK && tok != DONE) return ALG_NO_TARGET;
        
        if (whites_move && tok != DONE) {
            guess_move.to = alg_ind({alg[ai-1], alg[ai]});
        } else if (!whites_move && tok != DONE) {
            guess_move.to = alg_ind_flip({alg[ai-1], alg[ai]});
        }
        
        ti -= 2;
        tok = ti >= 0 ? move_tok[ti] : DONE;
        ai -= 2;

        if (tok == TAKES) {
            tok = ti > 0 ? move_tok[--ti] : DONE;
            ai--;

            if (guess_move.type != QUIET) {
                guess_move.type = CAPTURE;
            } else {
                guess_move.type += 4;
            }
        }
        if (tok == RANK) {
            tok = ti > 0 ? move_tok[--ti] : DONE;
            from_rank = alg[ai--];
        }
        if (tok == FILE) {
            tok = ti > 0 ? move_tok[--ti] : DONE;
            from_file = alg[ai--];
        }
        
        if (tok == PIECE) {
            tok = DONE;
            guess_move.piece = alg[ai--];
        }

        ///std::cout << "guess: {" << ind_alg(guess_move.from) << ", " << ind_alg(guess_move.to) << ", " << guess_move.type << ", " << guess_move.piece << ", " << guess_move.target << "}" << std::endl;
        /*
        for (Move m : move_list) {
            std::cout << "begin: {" << ind_alg(m.from) << ", " << ind_alg(m.to) << ", " << m.type << ", " << m.piece << ", " << m.target << "}" << std::endl;
        }*/

        for (Move m: move_list) {
            if (m.piece != guess_move.piece) continue;
            if (m.to != guess_move.to) continue;

            filtered.push_back(m);
        }

        if (from_file != -1 && from_rank != -1) {
            guess_move.from = whites_move ? alg_ind({from_file, from_rank}) : alg_ind_flip({from_file, from_rank});
            for (int i = 0; i < filtered.size(); i++) {
                if (filtered[i].from != guess_move.from) {
                    filtered.erase(filtered.begin() + i);
                    i--;
                }
            }

        } else if (from_file != -1) {
            for (int i = 0; i < filtered.size(); i++) {
                if ((filtered[i].from % 8) != (from_file - 'a')) {
                    filtered.erase(filtered.begin() + i);
                    i--;
                }
            }
        } else if (from_rank != -1) {
            for (int i = 0; i < filtered.size(); i++) {
                if ((filtered[i].from / 8) != (from_rank - '1')) {
                    filtered.erase(filtered.begin() + i);
                    i--;
                }
            }
        }
        
        /*for (Move m : filtered) {
            std::cout << "matches: {" << ind_alg(m.from) << ", " << ind_alg(m.to) << ", " << m.type << ", " << m.piece << ", " << m.target << "}" << std::endl;
        }*/

        if (filtered.size() == 1) {
            return move(filtered[0]);
        }

        return ALG_NO_MATCH;
    }
    
    void flip() {
        char temp_board[64];
        std::memcpy(temp_board, board, 64);

        for (int i = 0; i < 64; i++) {
            if (isupper(temp_board[i])) {
                board[CONVERT[i]] = tolower(temp_board[i]);
            } else {
                board[CONVERT[i]] = toupper(temp_board[i]);
            }
        }

        rights = {rights.bq, rights.bk, rights.wk, rights.wq};
        ep = (ep != -1) ? CONVERT[ep] : -1;
        whites_move = !whites_move;
    }

    void print_state() {
        std::cout << (whites_move ? "WHITE" : "BLACK") << " MOVE" << std::endl; 
        
        std::cout << "Castling: " << \
            (rights.wk ? "K" : "") << (rights.wq ? "Q" : "") << \
            (rights.bk ? "k" : "") << (rights.bq ? "q" : "") << std::endl;
        
        std::cout << "EP Index: " << ep << std::endl;
        std::cout << "Internal Position: " << std::endl;
        for (int i = 0; i < 8; i++) {
            std::cout << (whites_move ? 8-i : i+1) << " ";
            for (int j = 0; j < 8; j++) {
                std::cout << (whites_move ? board[i*8 + j] : board[i*8 + 7 - j]);
            }
            std::cout << std::endl;
        }
        std::cout << (whites_move?"\n  abcdefgh":"\n  hgfedcba") << std::endl;
    }

    Position(const Position& rhs) {
        assign_from(rhs);
    }

    Position& operator=(const Position& rhs) {
        assign_from(rhs);
        return *this;
    }

    char operator[](const char* str) {
        if (whites_move) {
            return board[alg_ind(str)];
        } else {
            return board[alg_ind_flip(str)];
        }
    }

    char operator[](const int index) {
        return board[index];
    }

};

namespace terminal {
    namespace code {
        const char *BG_DARK  = "\033[48;2;181;136;99m";
        const char *BG_LIGHT = "\033[48;2;216;195;164m";
        const char *BG_BLACK = "\033[48;2;0;0;0m";
        const char *BG_DARK_GREY  = "\033[48;2;38;38;38m";
        const char *BG_LIGHT_GREY = "\033[48;2;108;108;108m";
        const char *BG_WHITE = "\033[107m";

        const char *FG_BLACK  = "\033[30m";
        const char *FG_GREEN  = "\033[32m";
        const char *FG_ORANGE = "\033[38;2;255;127;0m";
        const char *FG_GREY   = "\033[38;2;108;108;108m";
        const char *FG_WHITE  = "\033[97m";

        const char *TM_RESET   = "\033[0m";

        const int INPUT_LN  = 15;
        const int INPUT_COL = 3;
    }

    void cursor_pos(int x, int y) {
        std::cout << "\033[" << y << ";" << x << "H";
    }

    void draw_board(Position& pos) {
        cursor_pos(1, 2);

        bool& w = pos.whites_move;

        std::cout << "      " << code::BG_WHITE << code::FG_BLACK;
        if (w) {
            std::cout << "      White to move     \n\n" << code::TM_RESET;
        } else {
            std::cout << "      Black to move     \n\n" << code::TM_RESET;
        }

        for (int i = 0; i < 8; i++) {
            std::cout << "    " << code::FG_GREY << (w ? 8-i : i+1) << " ";
            for (int j = 0; j < 8; j++) {
                bool dark = (((i+j) % 2) == 1)^w;
                char ch  = (w ? pos[i*8 + j] : pos[i*8+7 - j]);
                if (!w) {
                    if (islower(ch)) ch = toupper(ch);
                    else if (isupper(ch)) ch = tolower(ch);
                }
                std::cout << (dark ? code::BG_DARK : code::BG_LIGHT);

                if (isupper(ch)) std::cout << code::FG_WHITE;
                else if (islower(ch)) std::cout << code::FG_BLACK;
                else ch = ' ';

                std::cout << " " << ch << " ";
            }
            std::cout << code::TM_RESET << '\n';
        }

        std::cout << "       " << code::FG_GREY;
        if (w) {
            std::cout << "a  b  c  d  e  f  g  h \n" << code::TM_RESET;
        } else {
            std::cout << "h  g  f  e  d  c  b  a \n" << code::TM_RESET;
        }
    }

    void update_history(std::vector<Position>& history) {
        cursor_pos(40, 6);
        std::cout << code::FG_GREY;
        std::cout << "┌────────────────────┐";
        cursor_pos(40, 7);
        std::cout << "│ 1.                 │";
        cursor_pos(40, 8);
        std::cout << "│                    │";
        cursor_pos(40, 9);
        std::cout << "└────────────────────┘";
    }

    void update_players() {
        cursor_pos(38, 4);
        std::cout << code::FG_GREEN << "* ";
        std::cout << code::FG_ORANGE << "BOT ";
        std::cout << code::FG_WHITE << "badchess";
        cursor_pos(38, 11);
        std::cout << code::FG_GREEN << "* ";
        std::cout << code::FG_WHITE << "jonah";
    }

    void draw_your_move() {
        cursor_pos(1,13);
        std::cout << "\033[J";
        std::cout << "┌─ Your move ──────────────────┐\n";
        std::cout << "└ ";
    }

    void setup_bg(Position& pos, std::vector<Position>& history) {
        cursor_pos(1, 1);
        std::cout << "\033[2J" << code::TM_RESET << "\n";
        
        draw_board(pos);
        update_history(history);
        update_players();
        draw_your_move();
    }
}

namespace engine {}

int main(int argc, char* argv[]) {
    std::vector<Position> history;

    history.emplace_back(Position(
        OPENING, {true, true, true, true}, -1, true, 0
    ));

    terminal::setup_bg(history.back(), history);

    while (true) {
        terminal::draw_board(history.back());
        terminal::draw_your_move();
        history.back().generate_moves();
        
        if (history.back().move_list.size() == 0) {
            std::cout << "Checkmate! ";
            if (history.back().whites_move) {
                std::cout << "Black";
            } else {
                std::cout << "White";
            }
            std::cout << " is victorious.";
            break;
        }

        std::string input;
        std::cin >> input;
        
        MOVE_CODE code;
        while ((code = history.back().move(input)) != SUCCESS) {
            switch (code) {
                case ALG_MALFORM:
                    std::cout << "Malformed notation. Enter a move like e4. \n";
                    break;
                case ALG_INVALID_PROMO:
                    std::cout << "Invalid pawn promotion. Promote to B, N, R, or Q. \n";
                    break;
                case ALG_NO_TARGET:
                    std::cout << "Invalid target square. \n";
                    break;
                case ALG_NO_MATCH:
                    std::cout << "Move not found. Try disambiguating move.\n";
                    break;
                default:
                    std::cout << "Unknown error while parsing expression. \n";
                    break;
            }
            std::cin >> input;
        }

        history.emplace_back(history.back());
        history.back().flip();
        history.back().ply++;
    }

    return 0;
}