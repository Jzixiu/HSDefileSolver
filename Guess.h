#ifndef GUESS_H
#define GUESS_H

#include <vector>
#include "GameState.h"
#include "ISolver.h"

class Guess : public ISolver
{
public:
    Guess(GameState &game);
    void solve() override;

private:
    std::vector<int> attack_order;
    bool dfs(int order = 0);
    bool check_solution();
};

#endif