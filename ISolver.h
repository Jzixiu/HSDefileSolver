#ifndef ISOLVER_H
#define ISOLVER_H

#include <vector>
#include "step.h"
#include "GameState.h"

class ISolver
{
public:
    void register_incumbent_callback(std::function<void(int, const std::vector<step> &)> function)
    {
        incumbent_callback = function;
    };

    virtual void solve() = 0;

    int get_solution_atk() const
    {
        return solution_atk;
    }
    int get_elapsed_time() const
    {
        return elapsed_time;
    }
    std::vector<step> get_solution() const
    {
        return solution;
    }

protected:
    ISolver(GameState &game)
        : solution_atk(-1), elapsed_time(-1LL),
          gs(game), incumbent_callback([](int, const std::vector<step> &) {})
    {
    }

    GameState &gs;

    int solution_atk;
    std::vector<step> solution;
    long long elapsed_time;

    // 用来输出现任解
    std::function<void(int, const std::vector<step> &)> incumbent_callback;
};

#endif