#ifndef SOLVER_H
#define SOLVER_H

#include "GameState.h"
#include <vector>

class Solver
{
public:
    struct step
    {
        int ally_pos;
        int enemy_pos;
    };

    Solver(GameState &game);
    void solve();
    void register_incumbent_callback(std::function<void(int, const std::vector<step> &)> function);

    int get_solution_atk() const;
    int get_elapsed_time() const;
    std::vector<step> get_solution() const;

private:
    GameState &gs;
    std::function<void(int, const std::vector<step> &)> incumbent_callback;

    std::vector<step> steps;
    int solution_atk;
    std::vector<step> solution;
    long long elapsed_time;

    void attack(int ally_pos, int enemy_pos);
    void undo();

    bool dfs();
    bool check_solution();
};

#endif