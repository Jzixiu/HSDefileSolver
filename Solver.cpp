#include "Solver.h"

Solver::Solver(GameState &game)
    : gs(game), solution_atk(-1), elapsed_time(-1LL),
      incumbent_callback([](int, const std::vector<step> &) {})
{
}

void Solver::solve()
{
    auto start = std::chrono::high_resolution_clock::now();
    dfs();
    auto end = std::chrono::high_resolution_clock::now();
    elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void Solver::register_incumbent_callback(std::function<void(int, const std::vector<step> &)> function)
{
    incumbent_callback = function;
};

int Solver::get_solution_atk() const
{
    return solution_atk;
}

int Solver::get_elapsed_time() const
{
    return elapsed_time;
}

std::vector<Solver::step> Solver::get_solution() const
{
    return solution;
}

void Solver::attack(int ally_pos, int enemy_pos)
{
    step s;
    s.ally_pos = ally_pos;
    s.enemy_pos = enemy_pos;
    steps.push_back(s);

    gs.attack(ally_pos, enemy_pos);
}

void Solver::undo()
{
    steps.pop_back();
    gs.undo_last_attack();
}

bool Solver::dfs()
{
    if (check_solution())
    {
        return true;
    }

    const std::vector<int> &enemy = gs.get_enemy();
    const std::vector<int> &ally = gs.get_ally();

    for (int ally_pos : ally)
    {
        for (int enemy_pos : enemy)
        {
            attack(ally_pos, enemy_pos);
            if (dfs())
            {
                undo();
                return true;
            }
            undo();
        }
    }

    return false;
}

bool Solver::check_solution()
{
    int atk = gs.get_enemy_atk_after_Defile(false);
    if (atk < solution_atk || solution_atk == -1)
    {
        // 有更优解
        solution_atk = atk;
        solution = steps;
        incumbent_callback(solution_atk, solution);
        if (atk == 0)
        {
            // 有最优解
            return true;
        }
    }
    return false;
}