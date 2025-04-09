#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <iostream>
#include <vector>
#include <stack>

#define MAX_MINION 7
#define MAX_DEFILE_REPEAT 30 // 亵渎最多释放次数

class GameState
{
public:
    GameState(std::istream &in);

    void attack(int ally_pos, int enemy_pos);
    void undo_attack();

    std::vector<int> get_enemy() const;
    std::vector<int> get_ally() const;

    void print(std::ostream &out = std::cout) const;

    int get_enemy_atk_after_Defile(bool exact);

private:
    struct minion
    {
        minion(int id, int atk, int hp, bool shield,
               bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
               bool immune, bool reborn, std::vector<int> derivant_id);

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
        const bool reborn;

        const std::vector<int> derivant_id;

        void print(std::ostream &out) const;
    };

    std::vector<minion> minion_template;
    std::vector<minion> minions;

    std::vector<int> ally;
    std::vector<int> enemy;
    std::vector<int> graveyard;

    int get_id(int atk, int hp, bool shield,
               bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
               bool immune, bool reborn, std::vector<int> derivant_id);
    int parse_minion(std::istream &in);

    enum Operation_Type
    {
        OP_CREATE,
        OP_MOVE_TO_GRAVEYARD,
        OP_MODIFY_HP,
        OP_DECREMENT_ATTACK_CHANCE,
        OP_REMOVE_SHIELD
    };
    enum Side
    {
        SIDE_ALLY,
        SIDE_ENEMY
    };

    struct operation
    {
        Operation_Type type;
        
        Side side;
        int pos;

        int old_hp;
    };

    void create_minion(int pos, int id, Side side);
    void move_minion_to_graveyard(int pos, Side side);

    void modify_minion_hp(int pos, Side side, int new_val);
    void decrement_minion_attack_chance(int pos, Side side);
    void remove_minion_shield(int pos, Side side);

    void undo_last_operation();
    std::stack<std::stack<operation>> op_stack;

    void process_death(int pos, Side side);
};
#endif