#include "Guess.h"
#include "step.h"
using namespace std;

Guess::Guess(GameState &game)
    : ISolver(game)
{
    for(int i=0;i<gs.get_ally_count();i++)
    {
        attack_order.push_back(i);
    }
}

void Guess::solve()
{
    auto start = chrono::high_resolution_clock::now();
    do
    {
        if(dfs())
        {
            break;
        }
    } while (next_permutation(attack_order.begin(),attack_order.end()));
    
    auto end = chrono::high_resolution_clock::now();
    elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

bool Guess::dfs(int order)
{
    if(order==gs.get_ally_count())
    {
        if(check_solution())
        {
            return true;
        }
        return false;
    }
    int ally_id=attack_order[order];
    
    for(int enemy_id=0;enemy_id<=gs.get_enemy_count();enemy_id++)
    {
        if(enemy_id<gs.get_enemy_count() && gs.get_enemy_hp(enemy_id)<=0) continue;
        if(enemy_id!= gs.get_enemy_count()) //如果id==enemy_count代表不攻击
        {
            gs.attack(step(ally_id,enemy_id));
        }
        if(dfs(order+1))
        {
            return true;
        }
        if(enemy_id!=gs.get_enemy_count()) 
        {
            gs.undo_last_attack();
        }
    }
    return false;
}

bool Guess::check_solution()
{
    int damage = 1;  // 亵渎造成的伤害
    int sum_atk = 0; // 总敌方场攻
    int max_hp = (gs.max_hp_ally > gs.max_hp_enemy) ? gs.max_hp_ally : gs.max_hp_enemy;
    bool hp_exists[max_hp + 1];
    for (int i = 0; i <= max_hp; i++)
    {
        hp_exists[i] = false;
    }

    for (int i = 0; i < gs.get_enemy_count(); i++)
    {
        hp_exists[gs.get_enemy_hp(i)] = true;
    }
    for (int i = 0; i < gs.get_ally_count(); i++)
    {
        hp_exists[gs.get_ally_hp(i)] = true;
    }

    // 模拟亵渎
    while (hp_exists[damage])
    {
        damage++;
    }

    for (int i = 0; i < gs.get_enemy_count(); i++)
    {
        if (gs.get_enemy_hp(i) > damage)
        {
            sum_atk += gs.get_enemy_atk(i);
        }
    }

    if (sum_atk < solution_atk || solution_atk == -1) // 更优解
    {
        solution_atk = sum_atk;
        solution = gs.get_current_step_history();

        if (solution_atk == 0)
        {
            return true;
        }
    }
    return false;
}