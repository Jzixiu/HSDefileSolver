#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <stack>
#include <istream>
#include <memory>

#define MAX_MINION 7

/**
 * @brief 高速可回溯的炉石场面模拟类
 *
 */
class GameState
{
public:
    GameState(std::istream &in);

    void attack(int ally_pos, int enemy_pos);
    void undo_last_attack();

    //获取能被攻击的敌方随从列表
    std::vector<int> get_enemy();

    //获取能发动攻击的友方随从列表
    std::vector<int> get_ally();

private:
    struct minion
    {
        minion(int id, int atk, int hp, bool shield,
               bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
               bool immune, bool reborn, std::vector<int> child);

        const int id;
        const int atk;
        int hp;
        int attack_chance;
        bool shield;
        const bool windfury;
        const bool charge_or_rush;
        const bool taunt;
        const bool poisionous;
        const bool immune;
        bool reborn;

        const std::vector<int> child_id;
    };

    std::vector<minion> minion_template;

    std::unique_ptr<minion> ally[MAX_MINION];
    std::unique_ptr<minion> enemy[MAX_MINION];

    int ally_count;
    int enemy_count;

    std::stack<std::unique_ptr<minion>> graveyard;

    int parse_minion(std::istream &in);

    enum Operation_Type
    {
        OP_UNDEFINED,
        OP_CREATE,
        OP_KILL,
        OP_MOVE,
        OP_MODIFY_HP,
        OP_MODIFY_ATTACK_CHANCE,
        OP_MODIFY_SHIELD
    };

    enum Side
    {
        SIDE_UNDEFINED,
        SIDE_ALLY,
        SIDE_ENEMY
    };

    struct operation
    {
        operation();
        Operation_Type type;
        Side side;

        // create,kill,modify
        int pos;

        // modify
        int old_hp;
        int attack_chance;
        bool shield;

        // move;
        int from_pos;
        int to_pos;
    };

    void create_minion(int pos, int id, Side side);
    void kill_minion(int pos, Side side);
    void move_minion(int from, int to, Side side);

    void modify_minion_hp(int pos, Side side, int new_val);
    void modify_minion_attack_chance(int pos, Side side, int new_val);
    void modify_minion_shield(int pos, Side side, bool new_val);

    void undo_operation(const operation &op);

    std::stack<std::stack<operation>> op_stack;

    // 把pos右边的所有随从都向左移动1格
    void move_left(int pos, Side side);

    // 把pos及右边的随从都向右移动offset格
    void move_right(int pos, Side side, int offset);
};

#endif