#include "GameState.h"
#include <istream>
#include <string>
#include <vector>
#include <cassert>

GameState::GameState(std::istream &in)
{
    int enemy_count, ally_count;
    in >> enemy_count >> ally_count;

    assert(0 <= enemy_count && enemy_count <= MAX_MINION);
    assert(0 <= ally_count && ally_count <= MAX_MINION);

    std::vector<int> initial_attack_chance;

    for (int i = 0; i < ally_count; i++)
    {
        int temp;
        in >> temp;
        initial_attack_chance.push_back(temp);
    }

    for (int i = 0; i < enemy_count; i++)
    {
        int id = parse_minion(in);
        enemy.push_back(std::make_unique<minion>(minion_template[id]));
    }

    for (int i = 0; i < ally_count; i++)
    {
        int id = parse_minion(in);
        ally.push_back(std::make_unique<minion>(minion_template[id]));
        if (initial_attack_chance[i] != -1)
        {
            ally[i]->attack_chance = initial_attack_chance[i];
        }
    }
}

void GameState::attack(int ally_pos, int enemy_pos)
{
    assert(ally_pos < ally.size() && enemy_pos < enemy.size());
    assert(ally[ally_pos] != nullptr && enemy[enemy_pos] != nullptr);
    assert(ally[ally_pos]->attack_chance > 0 && ally[ally_pos]->atk > 0);

    op_stack.emplace();

    const minion &a = *ally[ally_pos];
    const minion &e = *enemy[enemy_pos];

    decrement_minion_attack_chance(ally_pos, SIDE_ALLY);

    // 处理友方随从

    if (e.atk > 0 && !a.immune)
    {
        if (a.shield)
        {
            remove_minion_shield(ally_pos, SIDE_ALLY);
        }
        else
        {
            modify_minion_hp(ally_pos, SIDE_ALLY, a.hp - e.atk);
        }

        if (a.hp <= 0 || e.poisionous)
        {
            process_death(ally_pos, SIDE_ALLY);
        }
    }

    // 处理敌方随从

    if (!e.immune)
    {
        if (e.shield)
        {
            remove_minion_shield(enemy_pos, SIDE_ENEMY);
        }
        else
        {
            modify_minion_hp(enemy_pos, SIDE_ENEMY, e.hp - a.atk);
        }

        if (e.hp <= 0 || a.poisionous)
        {
            process_death(enemy_pos, SIDE_ENEMY);
        }
    }
}

void GameState::undo()
{
    assert(op_stack.size() > 0);
    std::stack<operation> &ops = op_stack.top();
    while (!ops.empty())
    {
        undo_operation(ops.top());
        ops.pop();
    }
    op_stack.pop();
}

std::vector<int> GameState::get_enemy() const
{
    std::vector<int> res;
    bool have_taunt = false;
    for (int i = 0; i < enemy.size(); i++)
    {
        if (!have_taunt)
        {
            if (enemy[i]->taunt)
            {
                have_taunt = true;
                res.clear();
            }
            res.push_back(i);
        }
        else
        {
            if (enemy[i]->taunt)
            {
                res.push_back(i);
            }
        }
    }
    return res;
}

std::vector<int> GameState::get_ally() const
{
    std::vector<int> res;
    for (int i = 0; i < ally.size(); i++)
    {
        if (ally[i]->attack_chance > 0)
        {
            res.push_back(i);
        }
    }
    return res;
}

void GameState::print(std::ostream &out) const
{
    out << std::endl;
    out << "格式:[id] 攻击力/血量 (攻击次数)";
    out << std::endl;

    out << "模版:" << std::endl;
    for (int i = 0; i < minion_template.size(); i++)
    {
        minion_template[i].print(out);
        out << std::endl;
    }
    out << std::endl;
    out << std::endl;

    out << "敌方:" << std::endl;
    for (int i = 0; i < enemy.size(); i++)
    {
        enemy[i]->print(out);
        out << std::endl;
    }
    out << std::endl;
    out << std::endl;

    out << "友方:" << std::endl;
    for (int i = 0; i < ally.size(); i++)
    {
        ally[i]->print(out);
        out << std::endl;
    }
    out << std::endl;
    out << std::endl;

    out << "墓地:" << std::endl;
    for (int i = 0; i < graveyard.size(); i++)
    {
        graveyard[i]->print(out);
        out << std::endl;
    }
    out << std::endl;
    out << std::endl;
}

int GameState::get_enemy_atk_after_Defile()
{
    int final_enemy_atk = 0;

    std::vector<int> ally_of_effective_hp[MAX_DEFILE_REPEAT + 1];
    std::vector<int> enemy_of_effective_hp[MAX_DEFILE_REPEAT + 1];

    auto get_effective_hp = [&](int parent_effective_hp, const minion &m, Side s)
    {
        int effective_hp = m.hp;
        if (m.shield)
        {
            effective_hp++;
        }

        effective_hp += parent_effective_hp;

        if (s == SIDE_ALLY)
        {
            if (m.immune || effective_hp > MAX_DEFILE_REPEAT)
            {
            }
            else
            {
                ally_of_effective_hp[effective_hp].push_back(m.id);
            }
        }
        else if (s == SIDE_ENEMY)
        {
            if (m.immune || effective_hp > MAX_DEFILE_REPEAT)
            {
                final_enemy_atk += m.atk;
            }
            else
            {
                enemy_of_effective_hp[effective_hp].push_back(m.id);
            }
        }
    };

    for (int pos = 0; pos < ally.size(); pos++)
    {
        get_effective_hp(0, *ally[pos], SIDE_ALLY);
    }

    for (int pos = 0; pos < enemy.size(); pos++)
    {
        get_effective_hp(0, *enemy[pos], SIDE_ENEMY);
    }

    int ally_free_space = MAX_MINION - ally.size();
    int enemy_free_space = MAX_MINION - enemy.size();
    int defile_damage;

    for (defile_damage = 1; defile_damage <= MAX_DEFILE_REPEAT; defile_damage++)
    {
        if (ally_of_effective_hp[defile_damage].size() != 0)
        {
            for (int minion_id : ally_of_effective_hp[defile_damage])
            {
                ally_free_space++; // 让这个随从死
                for (int child_id : minion_template[minion_id].derivant_id)
                {
                    if (ally_free_space > 0)
                    {
                        get_effective_hp(defile_damage, minion_template[child_id], SIDE_ALLY);
                        ally_free_space--;
                    }
                }
            }
        }
        if (enemy_of_effective_hp[defile_damage].size() != 0)
        {
            for (int minion_id : enemy_of_effective_hp[defile_damage])
            {
                enemy_free_space++; // 让这个随从死
                for (int child_id : minion_template[minion_id].derivant_id)
                {
                    if (enemy_free_space > 0)
                    {
                        get_effective_hp(defile_damage, minion_template[child_id], SIDE_ENEMY);
                        enemy_free_space--;
                    }
                }
            }
        }
        if (ally_of_effective_hp[defile_damage].size() == 0 && enemy_of_effective_hp[defile_damage].size() == 0)
        {
            break;
        }
    }

    for (int i = defile_damage; i < MAX_DEFILE_REPEAT; i++)
    {
        for (int id : enemy_of_effective_hp[i])
        {
            final_enemy_atk += minion_template[id].atk;
        }
    }

    return final_enemy_atk;
}

GameState::minion::minion(int id, int atk, int hp, bool shield,
                          bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
                          bool immune, bool reborn, std::vector<int> derivant_id)
    : id(id), atk(atk), hp(hp), shield(shield),
      windfury(windfury), charge_or_rush(charge_or_rush), taunt(taunt), poisionous(poisionous),
      immune(immune), reborn(reborn), derivant_id(std::move(derivant_id))
{
    if (charge_or_rush && atk > 0)
    {
        attack_chance = 1;
        if (windfury)
        {
            attack_chance = 2;
        }
    }
}

void GameState::minion::print(std::ostream &out) const
{
    out << "[" << id << "] ";
    out << atk << "/" << hp;
    out << " (" << attack_chance << ")";
    if (shield)
    {
        out << " [圣盾]";
    }
    if (windfury)
    {
        out << " [风怒]";
    }
    if (charge_or_rush)
    {
        out << " [冲锋/突袭]";
    }
    if (taunt)
    {
        out << " [嘲讽]";
    }
    if (poisionous)
    {
        out << " [剧毒]";
    }
    if (immune)
    {
        out << " [免疫]";
    }
    if (reborn)
    {
        out << " [复生]";
    }
    if (derivant_id.size() > 0)
    {
        out << " [亡语:";
        for (int i : derivant_id)
        {
            out << " " << i;
        }
        out << " ";
        out << "]";
    }
}

GameState::operation::operation()
{
    type = OP_UNDEFINED;
    side = SIDE_UNDEFINED;
    pos = -1;
    old_hp = -1;
}

int GameState::get_id(int atk, int hp, bool shield,
                      bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
                      bool immune, bool reborn, std::vector<int> derivant_id)
{
    int id = 0;
    bool repeated = false;
    while (id < minion_template.size())
    {
        if (
            minion_template[id].atk == atk &&
            minion_template[id].hp == hp &&
            minion_template[id].shield == shield &&
            minion_template[id].windfury == windfury &&
            minion_template[id].charge_or_rush == charge_or_rush &&
            minion_template[id].taunt == taunt &&
            minion_template[id].poisionous == poisionous &&
            minion_template[id].immune == immune &&
            minion_template[id].reborn == reborn &&
            minion_template[id].derivant_id == derivant_id)
        {
            repeated = true;
            break;
        }
        id++;
    }

    if (!repeated)
    {
        id = minion_template.size();
        minion_template.emplace_back(
            id,
            atk,
            hp,
            shield,
            windfury,
            charge_or_rush,
            taunt,
            poisionous,
            immune,
            reborn,
            derivant_id);
    }

    return id;
}

int GameState::parse_minion(std::istream &in)
{
    std::string atk_slash_hp;
    in >> atk_slash_hp;
    int pos = atk_slash_hp.find('/');
    int atk = std::stoi(atk_slash_hp.substr(0, pos));
    int hp = std::stoi(atk_slash_hp.substr(pos + 1));

    bool shield = false;
    bool windfury = false;
    bool charge_or_rush = false;
    bool taunt = false;
    bool poisionous = false;
    bool immune = false;
    bool reborn = false;

    int child_count;

    std::string temp;

    while (true)
    {
        in >> temp;
        if (temp == "圣盾")
        {
            shield = true;
        }
        else if (temp == "风怒")
        {
            windfury = true;
        }
        else if (temp == "冲锋" || temp == "突袭")
        {
            charge_or_rush = true;
        }
        else if (temp == "嘲讽")
        {
            taunt = true;
        }
        else if (temp == "剧毒")
        {
            poisionous = true;
        }
        else if (temp == "免疫")
        {
            immune = true;
        }
        else if (temp == "复生")
        {
            reborn = true;
        }
        else
        {
            child_count = std::stoi(temp);
            break;
        }
    }

    std::vector<int> derivant_id;

    for (int i = 0; i < child_count; i++)
    {
        derivant_id.push_back(parse_minion(in));
    }

    if (reborn)
    {
        int reborn_id = get_id(
            atk,
            1, // 1血
            shield,
            windfury,
            charge_or_rush,
            taunt,
            poisionous,
            immune,
            false,                        // 无复生
            derivant_id);                 // 此时derivant_id里只有亡语衍生物，还没有它本身
        derivant_id.push_back(reborn_id); // 然后把这个复生衍生物插入本体的衍生物里（最后）
    }

    return get_id(atk, hp, shield, windfury, charge_or_rush, taunt, poisionous, immune, reborn, derivant_id);
}

void GameState::create_minion(int pos, int id, Side side)
{
    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_CREATE;
    op.side = side;
    op.pos = pos;

    if (side == SIDE_ALLY)
    {
        assert(0 <= pos && pos <= ally.size());
        ally.insert(ally.begin() + pos, std::make_unique<minion>(minion_template[id]));
    }
    else if (side == SIDE_ENEMY)
    {
        assert(0 <= pos && pos <= enemy.size());
        enemy.insert(enemy.begin() + pos, std::make_unique<minion>(minion_template[id]));
    }
}

void GameState::kill_minion(int pos, Side side)
{
    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_KILL;
    op.side = side;
    op.pos = pos;

    if (side == SIDE_ALLY)
    {
        assert(0 <= pos && pos < ally.size());
        assert(ally[pos] != nullptr);
        graveyard.push_back(std::move(ally[pos]));
        ally.erase(ally.begin() + pos);
    }
    else if (side == SIDE_ENEMY)
    {
        assert(0 <= pos && pos < enemy.size());
        assert(enemy[pos] != nullptr);
        graveyard.push_back(std::move(enemy[pos]));
        enemy.erase(enemy.begin() + pos);
    }
}

void GameState::modify_minion_hp(int pos, Side side, int new_val)
{
    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_MODIFY_HP;
    op.side = side;
    op.pos = pos;

    if (side == SIDE_ALLY)
    {
        assert(0 <= pos && pos < ally.size());
        assert(ally[pos] != nullptr);
        op.old_hp = ally[pos]->hp;
        ally[pos]->hp = new_val;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(0 <= pos && pos < enemy.size());
        assert(enemy[pos] != nullptr);
        op.old_hp = enemy[pos]->hp;
        enemy[pos]->hp = new_val;
    }
}

void GameState::decrement_minion_attack_chance(int pos, Side side)
{
    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_DECREMENT_ATTACK_CHANCE;
    op.side = side;
    op.pos = pos;

    if (side == SIDE_ALLY)
    {
        assert(0 <= pos && pos < ally.size());
        assert(ally[pos] != nullptr);
        ally[pos]->attack_chance--;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(0 <= pos && pos < enemy.size());
        assert(enemy[pos] != nullptr);
        enemy[pos]->attack_chance--;
    }
}

void GameState::remove_minion_shield(int pos, Side side)
{
    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_REMOVE_SHIELD;
    op.side = side;
    op.pos = pos;

    if (side == SIDE_ALLY)
    {
        assert(0 <= pos && pos < ally.size());
        assert(ally[pos] != nullptr);
        ally[pos]->shield = false;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(0 <= pos && pos < enemy.size());
        assert(enemy[pos] != nullptr);
        enemy[pos]->shield = false;
    }
}

void GameState::undo_operation(const operation &op)
{
    switch (op.type)
    {
    case OP_CREATE:
        if (op.side == SIDE_ALLY)
        {
            ally[op.pos].reset();
            ally.erase(ally.begin() + op.pos);
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy[op.pos].reset();
            enemy.erase(enemy.begin() + op.pos);
        }
        break;

    case OP_KILL:
        if (op.side == SIDE_ALLY)
        {
            ally.insert(ally.begin() + op.pos, std::move(graveyard.back()));
            graveyard.pop_back();
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy.insert(enemy.begin() + op.pos, std::move(graveyard.back()));
            graveyard.pop_back();
        }
        break;

    case OP_MODIFY_HP:
        if (op.side == SIDE_ALLY)
        {
            ally[op.pos]->hp = op.old_hp;
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy[op.pos]->hp = op.old_hp;
        }
        break;

    case OP_DECREMENT_ATTACK_CHANCE:
        if (op.side == SIDE_ALLY)
        {
            ally[op.pos]->attack_chance++;
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy[op.pos]->attack_chance++;
        }
        break;

    case OP_REMOVE_SHIELD:
        if (op.side == SIDE_ALLY)
        {
            assert(ally[op.pos] != nullptr);
            ally[op.pos]->shield = true;
        }
        else if (op.side == SIDE_ENEMY)
        {
            assert(enemy[op.pos] != nullptr);
            enemy[op.pos]->shield = true;
        }
        break;

    default:
        break;
    }
}

void GameState::process_death(int pos, Side side)
{
    if (side == SIDE_ALLY)
    {
        minion &a = *ally[pos];
        kill_minion(pos, SIDE_ALLY);

        int child_count = a.derivant_id.size();
        int avaliable_space = MAX_MINION - ally.size();
        if (avaliable_space < child_count)
        {
            child_count = avaliable_space;
        }

        for (int i = 0; i < child_count; i++)
        {
            create_minion(pos + i, a.derivant_id[i], SIDE_ALLY);
        }
    }
    else if (side == SIDE_ENEMY)
    {
        minion &e = *enemy[pos];
        kill_minion(pos, SIDE_ENEMY);

        int child_count = e.derivant_id.size();
        int avaliable_space = MAX_MINION - enemy.size();
        if (avaliable_space < child_count)
        {
            child_count = avaliable_space;
        }

        for (int i = 0; i < child_count; i++)
        {
            create_minion(pos + i, e.derivant_id[i], SIDE_ENEMY);
        }
    }
}