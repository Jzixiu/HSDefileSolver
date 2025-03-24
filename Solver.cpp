#include "Solver.h"
#include <chrono>
#include <iostream>
using namespace std;

Solver::Solver(GameState &game, bool mode)
    : gs(game), mode_is_optimal(mode)
{
    if (mode_is_optimal)
    {
        minion_exchange_targets = gs.get_minion_exchange_targets();
    }

    solution_atk = -1;

    intermediate_solution_callback=
    [](int,const std::vector<GameState::step>&)
    {
        //默认回调函数
    };
}

void Solver::set_intermediate_solution_callback(std::function<void(int,const std::vector<GameState::step>)> callback)
{
    intermediate_solution_callback=callback;
}

void Solver::solve()
{
    auto start = chrono::high_resolution_clock::now();
    if (mode_is_optimal)
    {
        minion_exchange();
    }
    else
    {
        dfs();
    }
    auto end = chrono::high_resolution_clock::now();
    elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int Solver::get_solution_atk()
{
    return solution_atk;
}

vector<GameState::step> Solver::get_solution()
{
    return solution;
}

long long Solver::get_elapsed_time()
{
    return elapsed_time;
}

bool Solver::minion_exchange(int ally_id)
{
    if (ally_id == gs.get_ally_count())
    {
        if (dfs())
        {
            return true;
        }
    }
    else
    {
        if (minion_exchange(ally_id + 1))
        {
            return true;
        } // 这个随从不进行互换的情况
        for (int enemy_id : minion_exchange_targets[ally_id])
        {
            gs.attack(GameState::step(ally_id, enemy_id));
            if (minion_exchange(ally_id + 1))
            {
                return true;
            }
            gs.undo_last_attack();
        }
    }
    return false;
}

bool Solver::dfs(int hp_level)
{
    bool leaf = true; // 记录是否为叶子节点，仅在叶子节点结算

    if (hp_level <= gs.max_hp_ally)
    {
        /*
        Part I: 找一个生命值刚好为k的友方随从，然后直接锁定。
        */

        const vector<int> &ally_of_hp = gs.get_ally_of_hp(hp_level);
        for (int ally_id : ally_of_hp)
        {
            leaf = false; // 如果进入了某个分支则不是叶子节点

            gs.setlock_ally(ally_id, true);
            if (dfs(hp_level + 1))
            {
                return true;
            }
            gs.setlock_ally(ally_id, false);
        }

        /*
        Part II: 找hp>k的友方随从，找atk=diff的敌方随从修血
        */

        const vector<int> &ally_gt_hp = gs.get_ally_gt_hp(hp_level);
        for (int ally_id : ally_gt_hp)
        {
            if (gs.getlock_ally(ally_id) || !gs.get_can_attack(ally_id))
            {
                // 如果这个友方随从被锁定了，或已不能攻击了，就跳过
                continue;
            }
            int diff = gs.get_ally_hp(ally_id) - hp_level; // 需要补的伤害
            const vector<int> &enemy_of_atk = gs.get_enemy_of_atk(diff);
            for (int enemy_id : enemy_of_atk)
            {
                if (gs.getlock_enemy(enemy_id) || gs.get_enemy_hp(enemy_id) <= 0)
                {
                    // 如果这个敌方随从已被锁定或死亡，则跳过
                    continue;
                }
                leaf = false;

                gs.attack(GameState::step(ally_id, enemy_id));
                gs.setlock_ally(ally_id, true);
                if (dfs(hp_level + 1))
                {
                    return true;
                }
                gs.setlock_ally(ally_id, false);
                gs.undo_last_attack();
            }
        }
    }

    if (hp_level <= gs.max_hp_enemy)
    {
        /*
        Part III: 找一个生命值刚好为k的敌方随从，然后直接锁定。
        */

        const vector<int> &enemy_of_hp = gs.get_enemy_of_hp(hp_level);
        for (int enemy_id : enemy_of_hp)
        {
            leaf = false;

            gs.setlock_enemy(enemy_id, true);
            if (dfs(hp_level + 1))
            {
                return true;
            }
            gs.setlock_enemy(enemy_id, false);
        }

        /*
        Part IV: 找hp>k的敌方随从，找combo攻击力=hp-k的修血量
        */

        const vector<int> &enemy_gt_hp = gs.get_enemy_gt_hp(hp_level);
        for (int enemy_id : enemy_gt_hp)
        {
            if (gs.getlock_enemy(enemy_id))
            {
                continue;
            }
            int diff = gs.get_enemy_hp(enemy_id) - hp_level;
            const vector<vector<int>> &combo_of_atk = gs.get_combo_of_atk(diff);
            for (const vector<int> &combo : combo_of_atk)
            {
                leaf = false;

                for (int ally_id : combo)
                {
                    gs.attack(GameState::step(ally_id, enemy_id));
                }
                gs.setlock_enemy(enemy_id, true);
                if (dfs(hp_level + 1))
                {
                    return true;
                }
                gs.setlock_enemy(enemy_id, false);
                for (int i = 0; i < combo.size(); i++)
                {
                    gs.undo_last_attack();
                }
            }
        }
    }

    /*
    Part V: 如果到达了叶子节点，则结算
    */

    if (leaf)
    {
        // 当前未找到hp_level血量的随从，那么亵渎就打了hp_level点伤害
        if (check_solution(hp_level))
        {
            return true;
        }
    }
    return false;
}

bool Solver::check_solution(int damage)
{
    int enemy_sum_atk = 0;
    for (int id = 0; id < gs.get_enemy_count(); id++)
    {
        if (gs.get_enemy_hp(id) > damage)
        {
            enemy_sum_atk += gs.get_enemy_atk(id);
        }
    }

    if (enemy_sum_atk < solution_atk || solution_atk == -1)
    {
        solution_atk = enemy_sum_atk;
        solution = gs.get_current_step_history();

        if (solution_atk == 0)
        {
            return true;
        }
        else
        {
            intermediate_solution_callback(solution_atk,solution);
        }
    }
    return false;
}