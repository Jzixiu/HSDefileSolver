#include "Fast_Guess.h"
using namespace std;

Fast_Guess::Fast_Guess(GameState &game, bool mode)
    : ISolver(game), mode_is_optimal(mode)
{
    if (mode_is_optimal)
    {
        minion_exchange_targets = gs.get_minion_exchange_targets();
    }

    attackers.reserve(gs.get_enemy_count());
    for (int i = 0; i < gs.get_enemy_count(); i++)
    {
        attackers.emplace_back(CompareByAttack(&gs));
    }

    sum_of_attackers.resize(gs.get_enemy_count(), 0);
}

void Fast_Guess::solve()
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

Fast_Guess::CompareByAttack::CompareByAttack(const GameState *gs_ptr)
    : gs_ptr(gs_ptr)
{
}

bool Fast_Guess::CompareByAttack::operator()(int left_id, int right_id) const
{
    int left_atk = gs_ptr->get_ally_atk(left_id);
    int right_atk = gs_ptr->get_ally_atk(right_id);
    return (left_atk == right_atk) ? (left_id < right_id) : (left_atk < right_atk);
}

void Fast_Guess::attack(int ally_id, int enemy_id)
{
    gs.attack(step(ally_id, enemy_id));
    attackers[enemy_id].insert(ally_id);
    sum_of_attackers[enemy_id] += gs.get_ally_atk(ally_id);
}

void Fast_Guess::undo_last_attack()
{
    step s = gs.undo_last_attack();
    attackers[s.enemy_id].erase(s.ally_id);
    sum_of_attackers[s.enemy_id] -= gs.get_ally_atk(s.ally_id);
}

bool Fast_Guess::minion_exchange(int ally_id)
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
            attack(ally_id, enemy_id);
            if (minion_exchange(ally_id + 1))
            {
                return true;
            }
            undo_last_attack();
        }
    }
    return false;
}

bool Fast_Guess::dfs(int hp_level)
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
            if(gs.getlock_ally(ally_id))
            {
                continue;
            }
            
            leaf = false; // 如果进入了某个分支则不是叶子节点

            gs.setlock_ally(ally_id, true);
            if (dfs(hp_level))
            {
                return true;
            }
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
                if (gs.getlock_enemy(enemy_id))
                {
                    // 如果这个敌方随从已被锁定则跳过
                    continue;
                }

                if (gs.get_enemy_hp(enemy_id) <= 0)
                {
                    int unleathal_damage = sum_of_attackers[enemy_id];
                    unleathal_damage -= gs.get_ally_atk(*attackers[enemy_id].rbegin());
                    unleathal_damage += gs.get_ally_atk(ally_id);
                    if (unleathal_damage >= gs.get_enemy_full_hp(enemy_id))
                    {
                        continue;
                    }
                }

                leaf = false;
                attack(ally_id, enemy_id);
                gs.setlock_ally(ally_id, true);
                if (dfs(hp_level))
                {
                    return true;
                }
                if (dfs(hp_level + 1))
                {
                    return true;
                }
                gs.setlock_ally(ally_id, false);
                undo_last_attack();
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
            if(gs.getlock_enemy(enemy_id))
            {
                continue;
            }

            leaf = false;

            gs.setlock_enemy(enemy_id, true);
            if (dfs(hp_level))
            {
                return true;
            }
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
                    attack(ally_id, enemy_id);
                }
                gs.setlock_enemy(enemy_id, true);
                if (dfs(hp_level))
                {
                    return true;
                }
                if (dfs(hp_level + 1))
                {
                    return true;
                }
                gs.setlock_enemy(enemy_id, false);
                for (int i = 0; i < combo.size(); i++)
                {
                    undo_last_attack();
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

bool Fast_Guess::check_solution(int damage)
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
        vector<step> s;
        for (int enemy_id = 0; enemy_id < gs.get_enemy_count(); enemy_id++)
        {
            for (int ally_id : attackers[enemy_id])
            {
                s.push_back(step(ally_id, enemy_id));
            }
        }
        solution = s;

        if (solution_atk == 0)
        {
            return true;
        }
        else
        {
            incumbent_callback(solution_atk, solution);
        }
    }
    return false;
}