#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <iostream>
#include <vector>

#define MAX_MINION 7
#define MAX_DEFILE_REPEAT 14 // 亵渎最多释放次数

class GameState
{
public:
    GameState(std::istream &in);

    void attack(int ally_pos, int enemy_pos);
    void undo_last_attack();

    void get_enemy(std::vector<int> &vec) const;
    void get_ally(std::vector<int> &vec) const;

    void print(std::ostream &out = std::cout) const;

    int get_enemy_atk_after_Defile();

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
    int push_minion(int id);

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
        SIDE_UNDEFINED,
        SIDE_ALLY,
        SIDE_ENEMY
    };

    struct operation
    {
        operation(Operation_Type type, Side side, int pos);
        operation(Operation_Type type, int instance_id, int old_hp);
        operation(Operation_Type type, int instance_id);
        Operation_Type type;

        Side side;
        int pos;
        int instance_id;

        int old_hp;
    };

    std::vector<operation> op_sequence;
    std::vector<int> step_size;

    void create_minion(Side side, int pos, int id);
    void move_minion_to_graveyard(Side side, int pos);

    void modify_minion_hp(int instance_id, int new_val);
    void decrement_minion_attack_chance(int instance_id);
    void remove_minion_shield(int instance_id);

    void undo_last_operation();

    void process_death(Side side, int pos);

    void get_effective_hp(int parent_effective_hp, const minion &m, bool *hp_exists);
    int get_atk(int damage, const minion &m);
};
#endif