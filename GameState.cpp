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
            process_death(ally_pos, SIDE_ALLY);
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

std::vector<int> GameState::get_ally() const
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
    for (int i = 0; i < enemy_count; i++)
    {
        enemy[i]->print(out);
        out << std::endl;
    }
    out << std::endl;
    out << std::endl;

    out << "友方:" << std::endl;
    for (int i = 0; i < ally_count; i++)
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
    if (child_id.size() > 0)
    {
        out << " [亡语:";
        for (int i : child_id)
        {
            out << " " << i;
        }
        out << " ";
        out << "]";
    }
}

int GameState::get_enemy_atk_after_Defile()
{
    op_stack.push({});

    bool some_minion_died;
    do
    {
        some_minion_died = false;
        for (int pos = 0; pos < ally_count; pos++)
        {
            modify_minion_hp(pos, SIDE_ALLY, ally[pos]->hp - 1);
        }
        for (int pos = 0; pos < enemy_count; pos++)
        {
            modify_minion_hp(pos, SIDE_ENEMY, enemy[pos]->hp - 1);
        }

        bool process_completed = true; // 结算是否完成
        do
        {
            for (int pos = 0; pos < ally_count; pos++)
            {
                if (ally[pos]->hp <= 0)
                {
                    some_minion_died = true;
                    process_completed = false;
                    process_death(pos, SIDE_ALLY);
                    break;
                }
            }
        } while (!process_completed);

        process_completed = true;
        do
        {
            for (int pos = 0; pos < enemy_count; pos++)
            {
                if (enemy[pos]->hp <= 0)
                {
                    some_minion_died = true;
                    process_completed = false;
                    process_death(pos, SIDE_ENEMY);
                    break;
                }
            }
        } while (!process_completed);

    } while (some_minion_died);

    int sum_atk = 0;
    for (int pos = 0; pos < enemy_count; pos++)
    {
        sum_atk += enemy[pos]->atk;
    }

    undo();

    return sum_atk;
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
        graveyard.push_back(std::move(ally[pos]));
        ally_count--;
    }
    else if (side == SIDE_ENEMY)
    {
        assert(enemy[pos] != nullptr);
        graveyard.push_back(std::move(enemy[pos]));
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
            ally[op.pos] = std::move(graveyard.back());
            ally_count++;
            graveyard.pop_back();
        }
        else if (op.side == SIDE_ENEMY)
        {
            enemy[op.pos] = std::move(graveyard.back());
            enemy_count++;
            graveyard.pop_back();
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

void GameState::process_death(int pos, Side side)
{
    if (side == SIDE_ALLY)
    {
        minion &a = *ally[pos];
        kill_minion(pos, SIDE_ALLY);
        fill_up(SIDE_ALLY);

        int child_count = a.child_id.size();
        if (MAX_MINION - ally_count < child_count)
        {
            child_count = MAX_MINION - ally_count;
        }

        if (child_count > 0)
        {
            reserve_space(pos, child_count, SIDE_ALLY);
            for (int i = 0; i < child_count; i++)
            {
                create_minion(pos + i, a.child_id[i], SIDE_ALLY);
            }
        }

        if (a.reborn && ally_count < MAX_MINION)
        {
            reserve_space(pos, 1, SIDE_ALLY);
            create_minion(pos, a.id, SIDE_ALLY);
            ally[pos]->reborn = false; // 应视为构造的一部分，不用接口化
            ally[pos]->hp = 1;
        }
    }
    else if (side == SIDE_ENEMY)
    {
        minion &e = *enemy[pos];
        kill_minion(pos, SIDE_ENEMY);
        fill_up(SIDE_ENEMY);

        int child_count = e.child_id.size();
        if (MAX_MINION - enemy_count < child_count)
        {
            child_count = MAX_MINION - enemy_count;
        }

        if (child_count > 0)
        {
            reserve_space(pos, child_count, SIDE_ENEMY);
            for (int i = 0; i < child_count; i++)
            {
                create_minion(pos + i, e.child_id[i], SIDE_ENEMY);
            }
        }

        if (e.reborn && enemy_count < MAX_MINION)
        {
            reserve_space(pos, 1, SIDE_ENEMY);
            create_minion(pos, e.id, SIDE_ENEMY);
            enemy[pos]->reborn = false; // 应视为构造的一部分，不用接口化
            enemy[pos]->hp = 1;
        }
    }
}

void GameState::fill_up(Side side)
{
    if (side == SIDE_ALLY)
    {
        for (int i = 0; i < ally_count; i++)
        {
            if (ally[i] == nullptr)
            {
                move_minion(i + 1, i, SIDE_ALLY);
            }
        }
    }
    else if (side == SIDE_ENEMY)
    {
        for (int i = 0; i < enemy_count; i++)
        {
            if (enemy[i] == nullptr)
            {
                move_minion(i + 1, i, SIDE_ENEMY);
            }
        }
    }
}

void GameState::reserve_space(int pos, int space, Side side)
{
    if (side == SIDE_ALLY)
    {
        assert(space <= MAX_MINION - ally_count);
        // -> ally_count+space<=MAX_MINION -> (ally_count-1)+space<MAX_MINION
        // 把[pos,ally_count)的随从每个都向右移动space格,是不会越界的
        for (int i = ally_count - 1; i >= pos; i--)
        {
            move_minion(i, i + space, SIDE_ALLY);
        }
    }
    else if (side == SIDE_ENEMY)
    {
        assert(space <= MAX_MINION - enemy_count);
        for (int i = enemy_count - 1; i >= pos; i--)
        {
            move_minion(i, i + space, SIDE_ENEMY);
        }
    }
}