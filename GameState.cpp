#include "GameState.h"
#include <istream>
#include <string>
#include <vector>
#include <cassert>

GameState::GameState(std::istream &in)
{
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
        enemy[i] = std::make_unique<minion>(minion_template[id]);
    }

    for (int i = 0; i < ally_count; i++)
    {
        int id = parse_minion(in);
        ally[i] = std::make_unique<minion>(minion_template[id]);
        if (initial_attack_chance[i] != -1)
        {
            ally[i]->attack_chance = initial_attack_chance[i];
        }
    }
}

void GameState::attack(int ally_pos, int enemy_pos)
{
    assert(ally[ally_pos] != nullptr && enemy[enemy_pos] != nullptr);
    assert(ally[ally_pos]->attack_chance > 0 && ally[ally_pos]->atk > 0);

    op_stack.emplace();

    const minion &a = *ally[ally_pos];
    const minion &e = *enemy[enemy_pos];

    modify_minion_attack_chance(ally_pos, SIDE_ALLY, a.attack_chance - 1);

    // 处理友方随从

    if (e.atk > 0 && !a.immune)
    {
        if (a.shield)
        {
            modify_minion_shield(ally_pos, SIDE_ALLY, false);
        }
        else
        {
            modify_minion_hp(ally_pos, SIDE_ALLY, a.hp - e.atk);
        }

        if (a.hp <= 0 || e.poisionous)
        {
            kill_minion(ally_pos, SIDE_ALLY);
            move_left(ally_pos, SIDE_ALLY);

            int child_count = a.child_id.size();
            if (MAX_MINION - ally_count < child_count)
            {
                child_count = MAX_MINION - ally_count;
            }

            if (child_count > 0)
            {
                move_right(ally_pos, SIDE_ALLY, child_count);
                for (int i = 0; i < child_count; i++)
                {
                    create_minion(ally_pos + i, a.child_id[i], SIDE_ALLY);
                }
            }

            if (a.reborn && ally_count < MAX_MINION)
            {
                move_right(ally_pos, SIDE_ALLY, 1);
                create_minion(ally_pos, a.id, SIDE_ALLY);
                ally[ally_pos]->reborn = false; // 应视为构造的一部分，不用接口化
                ally[ally_pos]->hp = 1;
            }
        }
    }

    // 处理敌方随从

    if (!e.immune)
    {
        if (e.shield)
        {
            modify_minion_shield(enemy_pos, SIDE_ENEMY, false);
        }
        else
        {
            modify_minion_hp(enemy_pos, SIDE_ENEMY, e.hp - a.atk);
        }

        if (e.hp <= 0 || a.poisionous)
        {
            kill_minion(enemy_pos, SIDE_ENEMY);
            move_left(enemy_pos, SIDE_ENEMY);

            int child_count = e.child_id.size();
            if (MAX_MINION - enemy_count < child_count)
            {
                child_count = MAX_MINION - enemy_count;
            }

            if (child_count > 0)
            {
                move_right(enemy_pos, SIDE_ENEMY, child_count);
                for (int i = 0; i < child_count; i++)
                {
                    create_minion(enemy_pos + i, e.child_id[i], SIDE_ENEMY);
                }
            }

            if (e.reborn && enemy_count < MAX_MINION)
            {
                move_right(enemy_pos, SIDE_ENEMY, 1);
                create_minion(enemy_pos, e.id, SIDE_ENEMY);
                enemy[enemy_pos]->reborn = false; // 应视为构造的一部分，不用接口化
                enemy[enemy_pos]->hp = 1;
            }
        }
    }
}

void GameState::undo_last_attack()
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

std::vector<int> GameState::get_enemy()
{
    std::vector<int> res;
    bool have_taunt = false;
    for (int i = 0; i < enemy_count; i++)
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

std::vector<int> GameState::get_ally()
{
    std::vector<int> res;
    for (int i = 0; i < ally_count; i++)
    {
        if (ally[i]->attack_chance > 0)
        {
            res.push_back(i);
        }
    }
    return res;
}

GameState::minion::minion(int id, int atk, int hp, bool shield,
                          bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
                          bool immune, bool reborn, std::vector<int> child)
    : id(id), atk(atk), hp(hp), shield(shield),
      windfury(windfury), charge_or_rush(charge_or_rush), taunt(taunt), poisionous(poisionous),
      immune(immune), reborn(reborn), child_id(std::move(child))
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

    std::vector<int> child_id;

    for (int i = 0; i < child_count; i++)
    {
        child_id.push_back(parse_minion(in));
    }

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
            minion_template[id].child_id == child_id)
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
            child_id);
    }

    return id;
}

GameState::operation::operation()
{
    type = OP_UNDEFINED;
    side = SIDE_UNDEFINED;
    pos = -1;
    old_hp = -1;
    attack_chance = -1;
    shield = false;
    from_pos = -1;
    to_pos = -1;
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
        assert(ally[pos] == nullptr);
        ally[pos] = std::make_unique<minion>(minion_template[id]);
        ally_count++;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(enemy[pos] == nullptr);
        enemy[pos] = std::make_unique<minion>(minion_template[id]);
        enemy_count++;
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
        assert(ally[pos] != nullptr);
        graveyard.push(std::move(ally[pos]));
        ally_count--;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(enemy[pos] != nullptr);
        graveyard.push(std::move(enemy[pos]));
        enemy_count--;
    }
}

void GameState::move_minion(int from, int to, Side side)
{
    assert(0 <= from && from < MAX_MINION && 0 <= to && to < MAX_MINION);

    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_MOVE;
    op.side = side;
    op.from_pos = from;
    op.to_pos = to;

    if (side == SIDE_ALLY)
    {
        assert(ally[from] != nullptr);
        assert(ally[to] == nullptr);
        ally[to] = std::move(ally[from]);
    }
    else if (side == SIDE_ENEMY)
    {
        assert(enemy[from] != nullptr);
        assert(enemy[to] == nullptr);
        enemy[to] = std::move(enemy[from]);
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
        assert(ally[pos] != nullptr);
        op.old_hp = ally[pos]->hp;
        ally[pos]->hp = new_val;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(enemy[pos] != nullptr);
        op.old_hp = enemy[pos]->hp;
        enemy[pos]->hp = new_val;
    }
}

void GameState::modify_minion_attack_chance(int pos, Side side, int new_val)
{
    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_MODIFY_ATTACK_CHANCE;
    op.side = side;
    op.pos = pos;

    if (side == SIDE_ALLY)
    {
        assert(ally[pos] != nullptr);
        op.attack_chance = ally[pos]->attack_chance;
        ally[pos]->attack_chance = new_val;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(enemy[pos] != nullptr);
        op.attack_chance = enemy[pos]->attack_chance;
        enemy[pos]->attack_chance = new_val;
    }
}

void GameState::modify_minion_shield(int pos, Side side, bool new_val)
{
    op_stack.top().emplace();
    operation &op = op_stack.top().top();
    op.type = OP_MODIFY_SHIELD;
    op.side = side;
    op.pos = pos;

    if (side == SIDE_ALLY)
    {
        assert(ally[pos] != nullptr);
        op.shield = ally[pos]->shield;
        ally[pos]->shield = new_val;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(enemy[pos] != nullptr);
        op.shield = enemy[pos]->shield;
        enemy[pos]->shield = new_val;
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
            ally_count--;
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy[op.pos].reset();
            enemy_count--;
        }
        break;

    case OP_KILL:
        assert(graveyard.size() != 0);
        if (op.side == SIDE_ALLY)
        {
            ally[op.pos] = std::move(graveyard.top());
            ally_count++;
            graveyard.pop();
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy[op.pos] = std::move(graveyard.top());
            enemy_count++;
            graveyard.pop();
        }
        break;

    case OP_MOVE:
        if (op.side == SIDE_ALLY)
        {
            assert(ally[op.from_pos] == nullptr);
            assert(ally[op.to_pos] != nullptr);
            ally[op.from_pos] = std::move(ally[op.to_pos]);
        }
        else if (op.side == SIDE_ENEMY)
        {
            assert(enemy[op.from_pos] == nullptr);
            assert(enemy[op.to_pos] != nullptr);
            enemy[op.from_pos] = std::move(enemy[op.to_pos]);
        }

        break;

    case OP_MODIFY_HP:
        if (op.side == SIDE_ALLY)
        {
            assert(ally[op.pos] != nullptr);
            ally[op.pos]->hp = op.old_hp;
        }
        else if (op.side == SIDE_ENEMY)
        {
            assert(enemy[op.pos] != nullptr);
            enemy[op.pos]->hp = op.old_hp;
        }
        break;

    case OP_MODIFY_ATTACK_CHANCE:
        if (op.side == SIDE_ALLY)
        {
            assert(ally[op.pos] != nullptr);
            ally[op.pos]->attack_chance = op.attack_chance;
        }
        else if (op.side == SIDE_ENEMY)
        {
            assert(enemy[op.pos] != nullptr);
            enemy[op.pos]->attack_chance = op.attack_chance;
        }
        break;

    case OP_MODIFY_SHIELD:
        if (op.side == SIDE_ALLY)
        {
            assert(ally[op.pos] != nullptr);
            ally[op.pos]->shield = op.shield;
        }
        else if (op.side == SIDE_ENEMY)
        {
            assert(enemy[op.pos] != nullptr);
            enemy[op.pos]->shield = op.shield;
        }
        break;

    default:
        break;
    }
}

void GameState::move_left(int pos, Side side)
{
    if (side == SIDE_ALLY)
    {
        for (int i = pos; i < ally_count; i++)
        {
            move_minion(i + 1, i, SIDE_ALLY);
        }
    }
    else if (side == SIDE_ENEMY)
    {
        for (int i = pos; i < enemy_count; i++)
        {
            move_minion(i + 1, i, SIDE_ENEMY);
        }
    }
}

void GameState::move_right(int pos, Side side, int offset)
{
    if (side == SIDE_ALLY)
    {
        for (int i = ally_count - 1; i >= pos; i--)
        {
            move_minion(i, i + offset, SIDE_ALLY);
        }
    }
    else if (side == SIDE_ENEMY)
    {
        for (int i = enemy_count - 1; i >= pos; i--)
        {
            move_minion(i, i + offset, SIDE_ENEMY);
        }
    }
}