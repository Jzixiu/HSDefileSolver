#include "GameState.h"

// Implicit include
#include <iostream>
#include <vector>
#include <stack>

GameState::GameState(std::istream &in)
{
    ally.reserve(MAX_MINION);
    enemy.reserve(MAX_MINION);
    minions.reserve(3 * MAX_MINION);

    int enemy_count, ally_count;
    in >> enemy_count >> ally_count;

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
        enemy.push_back(push_minion(id));
    }

    for (int i = 0; i < ally_count; i++)
    {
        int id = parse_minion(in);
        ally.push_back(push_minion(id));
        if (initial_attack_chance[i] != -1)
        {
            minions[ally[i]].attack_chance = initial_attack_chance[i];
        }
    }
}

void GameState::attack(int ally_pos, int enemy_pos)
{
    op_stack.emplace();

    int ally_instance_id = ally[ally_pos];
    int enemy_instance_id = enemy[enemy_pos];

    const minion &a = minions[ally_instance_id];
    const minion &e = minions[enemy_instance_id];

    decrement_minion_attack_chance(ally_instance_id);

    if (e.atk > 0 && !a.immune)
    {
        if (a.shield)
        {
            remove_minion_shield(ally_instance_id);
        }
        else
        {
            modify_minion_hp(ally_instance_id, a.hp - e.atk);
        }

        if (a.hp <= 0 || e.poisionous)
        {
            process_death(SIDE_ALLY, ally_pos);
        }
    }

    if (!e.immune)
    {
        if (e.shield)
        {
            remove_minion_shield(enemy_instance_id);
        }
        else
        {
            modify_minion_hp(enemy_instance_id, e.hp - a.atk);
        }

        if (e.hp <= 0 || a.poisionous)
        {
            process_death(SIDE_ENEMY, enemy_pos);
        }
    }
}

void GameState::undo_last_attack()
{
    while (!op_stack.top().empty())
    {
        undo_last_operation();
    }
    op_stack.pop();
}

std::vector<int> GameState::get_enemy() const
{
    std::vector<int> res;
    bool have_taunt = false;
    for (int i = 0; i < enemy.size(); i++)
    {
        bool this_have_taunt = minions[enemy[i]].taunt;
        if (!have_taunt)
        {
            if (this_have_taunt)
            {
                have_taunt = true;
                res.clear();
            }
            res.push_back(i);
        }
        else
        {
            if (this_have_taunt)
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
        if (minions[ally[i]].attack_chance > 0)
        {
            res.push_back(i);
        }
    }
    return res;
}

void GameState::print(std::ostream &out) const
{
    out << std::endl;
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
        minions[enemy[i]].print(out);
        out << "  |  ";
    }
    out << std::endl;
    out << std::endl;

    out << "友方:" << std::endl;
    for (int i = 0; i < ally.size(); i++)
    {
        minions[ally[i]].print(out);
        out << "  |  ";
    }
    out << std::endl;
    out << std::endl;

    out << "墓地:" << std::endl;
    for (int i = 0; i < graveyard.size(); i++)
    {
        minions[graveyard[i]].print(out);
        out << std::endl;
    }
    out << std::endl;
    out << std::endl;
}

int GameState::get_enemy_atk_after_Defile(bool exact)
{
    int defile_damage = 1;
    bool hp_exists[MAX_DEFILE_REPEAT + 1];

    for (int i = 0; i <= MAX_DEFILE_REPEAT; i++)
    {
        hp_exists[i] = false;
    }

    for (int i = 0; i < ally.size(); i++)
    {
        get_effective_hp(0, minions[ally[i]], hp_exists);
    }
    for (int i = 0; i < enemy.size(); i++)
    {
        get_effective_hp(0, minions[enemy[i]], hp_exists);
    }

    while (defile_damage <= MAX_DEFILE_REPEAT && hp_exists[defile_damage])
    {
        defile_damage++;
    }

    int sum = 0;
    for (int i = 0; i < enemy.size(); i++)
    {
        sum += get_atk(defile_damage, minions[enemy[i]]);
    }
    return sum;
}

GameState::minion::minion(int id, int atk, int hp, bool shield,
                          bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
                          bool immune, bool reborn, std::vector<int> derivant_id)
    : id(id), atk(atk), hp(hp), shield(shield),
      windfury(windfury), charge_or_rush(charge_or_rush), taunt(taunt), poisionous(poisionous),
      immune(immune), reborn(reborn), derivant_id(std::move(derivant_id))
{
    attack_chance = 0;
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

int GameState::push_minion(int id)
{
    int instance_id = minions.size();
    minions.emplace_back(minion_template[id]);
    return instance_id;
}

GameState::operation::operation(Operation_Type type, Side side, int pos)
    : type(type), side(side), pos(pos),
      instance_id(-1), old_hp(-1)
{
}

GameState::operation::operation(Operation_Type type, int instance_id, int old_hp)
    : type(type), instance_id(instance_id), old_hp(old_hp),
      side(SIDE_UNDEFINED), pos(-1)
{
}

GameState::operation::operation(Operation_Type type, int instance_id)
    : type(type), instance_id(instance_id),
      side(SIDE_UNDEFINED), pos(-1), old_hp(-1)
{
}

void GameState::create_minion(Side side, int pos, int id)
{
    op_stack.top().emplace(
        OP_CREATE,
        side,
        pos);

    int instance_id = push_minion(id);
    if (side == SIDE_ALLY)
    {
        ally.insert(ally.begin() + pos, instance_id);
    }
    else if (side == SIDE_ENEMY)
    {
        enemy.insert(enemy.begin() + pos, instance_id);
    }
}

void GameState::move_minion_to_graveyard(Side side, int pos)
{
    op_stack.top().emplace(
        OP_MOVE_TO_GRAVEYARD,
        side,
        pos);

    int instance_id;
    if (side == SIDE_ALLY)
    {
        instance_id = ally[pos];
        ally.erase(ally.begin() + pos);
    }
    else if (side == SIDE_ENEMY)
    {
        instance_id = enemy[pos];
        enemy.erase(enemy.begin() + pos);
    }
    graveyard.push_back(instance_id);
}

void GameState::modify_minion_hp(int instance_id, int new_val)
{
    op_stack.top().emplace(
        OP_MODIFY_HP,
        instance_id,
        minions[instance_id].hp);

    minions[instance_id].hp = new_val;
}

void GameState::decrement_minion_attack_chance(int instance_id)
{
    op_stack.top().emplace(
        OP_DECREMENT_ATTACK_CHANCE,
        instance_id);

    minions[instance_id].attack_chance--;
}

void GameState::remove_minion_shield(int instance_id)
{
    op_stack.top().emplace(
        OP_REMOVE_SHIELD,
        instance_id);

    minions[instance_id].shield = false;
}

void GameState::undo_last_operation()
{
    const operation &op = op_stack.top().top();
    switch (op.type)
    {
    case OP_CREATE:
        minions.pop_back();
        if (op.side == SIDE_ALLY)
        {
            ally.erase(ally.begin() + op.pos);
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy.erase(enemy.begin() + op.pos);
        }
        break;

    case OP_MOVE_TO_GRAVEYARD:
        int instance_id = graveyard.back();
        graveyard.pop_back();
        if (op.side == SIDE_ALLY)
        {
            ally.insert(ally.begin() + op.pos, instance_id);
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy.insert(enemy.begin() + op.pos, instance_id);
        }
        break;

    case OP_MODIFY_HP:
        minions[op.instance_id].hp = op.old_hp;
        break;
    case OP_DECREMENT_ATTACK_CHANCE:
        minions[op.instance_id].attack_chance++;
        break;
    case OP_REMOVE_SHIELD:
        minions[op.instance_id].shield = true;
        break;
    default:
        break;
    }
    op_stack.top().pop();
}

void GameState::process_death(Side side, int pos)
{
    if (side == SIDE_ALLY)
    {
        int instance_id = ally[pos];
        move_minion_to_graveyard(SIDE_ALLY, pos);

        int child_count = minions[instance_id].derivant_id.size();
        int avaliable_space = MAX_MINION - ally.size();
        if (avaliable_space < child_count)
        {
            child_count = avaliable_space;
        }

        for (int i = 0; i < child_count; i++)
        {
            create_minion(SIDE_ALLY, pos + i, minions[instance_id].derivant_id[i]);
        }
    }
    else if (side == SIDE_ENEMY)
    {
        int instance_id = enemy[pos];
        move_minion_to_graveyard(SIDE_ENEMY, pos);

        int child_count = minions[instance_id].derivant_id.size();
        int avaliable_space = MAX_MINION - enemy.size();
        if (avaliable_space < child_count)
        {
            child_count = avaliable_space;
        }

        for (int i = 0; i < child_count; i++)
        {
            create_minion(SIDE_ENEMY, pos + i, minions[instance_id].derivant_id[i]);
        }
    }
}

void GameState::get_effective_hp(int parent_effective_hp, const minion &m, bool *hp_exists)
{
    int effective_hp = parent_effective_hp + m.hp;
    if (m.shield)
    {
        effective_hp++;
    }
    if (effective_hp <= MAX_DEFILE_REPEAT)
    {
        hp_exists[effective_hp] = true;
    }
    for (int child_id : m.derivant_id)
    {
        get_effective_hp(effective_hp, minion_template[child_id], hp_exists);
    }
}

int GameState::get_atk(int damage, const minion &m)
{
    int effective_hp = m.hp;
    if (m.shield)
    {
        effective_hp++;
    }

    if (damage < effective_hp)
    {
        return m.atk;
    }
    else
    {
        int sum = 0;
        for (int child_id : m.derivant_id)
        {
            sum += get_atk(damage - effective_hp, minion_template[child_id]);
        }
        return sum;
    }
}