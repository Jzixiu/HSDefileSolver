#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include <stack>
#include <iostream>
#include <istream>
#include <ostream>
#include <memory>

#define MAX_MINION 7
#define MAX_DEFILE_REPEAT 30 // 亵渎最多释放次数

/**
 * @brief 高速可回溯的炉石场面模拟类
 *
 */
class GameState
{
public:
    GameState(std::istream &in);

    /**
     * @brief 使位于ally_pos的友方随从
     * 攻击位于enemy_pos的敌方随从
     *
     * @param ally_pos 友方随从在场上的位置
     * @param enemy_pos 敌方随从在场上的位置
     */
    void attack(int ally_pos, int enemy_pos);
    /**
     * @brief 回溯上一次攻击
     *
     */
    void undo();

    /**
     * @brief 获取能被攻击的敌方随从的位置列表
     *
     * @return 一个位置列表，
     * 每个数字代表敌方场上处于该位置的敌方随从可以被当作攻击目标
     */
    std::vector<int> get_enemy() const;

    /**
     * @brief 获取能发起攻击的友方随从的位置列表
     *
     * @return 一个位置列表，
     * 每个数字代表我方场上处于该位置的友方随从可以发起攻击
     */
    std::vector<int> get_ally() const;

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

    enum Operation_Type
    {
        OP_UNDEFINED,
        OP_CREATE,
        OP_KILL,
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
        operation();
        Operation_Type type;
        Side side;

        // create,kill,modify
        int pos;

        // modify
        int old_hp;
    };

    std::vector<minion> minion_template;
    std::vector<std::unique_ptr<minion>> ally;
    std::vector<std::unique_ptr<minion>> enemy;
    std::vector<std::unique_ptr<minion>> graveyard;

    int get_id(int atk, int hp, bool shield,
               bool windfury, bool charge_or_rush, bool taunt, bool poisionous,
               bool immune, bool reborn, std::vector<int> derivant_id);
    int parse_minion(std::istream &in);

    /**
     * @brief 在指定位置创建一个指定id的随从
     *
     * @param pos 随从被创建的位置
     * @param id minion_template中的id
     * @param side 友方:SIDE_ALLY 敌方:SIDE_ENEMY
     */
    void create_minion(int pos, int id, Side side);
    /**
     * @brief 将指定随从移到坟场
     *
     * @param pos 随从在场上的位置
     * @param side 友方:SIDE_ALLY 敌方:SIDE_ENEMY
     */
    void kill_minion(int pos, Side side);

    /**
     * @brief 改变指定随从的hp
     *
     * @param pos 随从在场上的位置
     * @param side 友方:SIDE_ALLY 敌方:SIDE_ENEMY
     * @param new_val 该随从的新hp值
     */
    void modify_minion_hp(int pos, Side side, int new_val);
    /**
     * @brief 使指定随从的attack_chance减少1
     *
     * @param pos 随从在场上的位置
     * @param side 友方:SIDE_ALLY 敌方:SIDE_ENEMY
     */
    void decrement_minion_attack_chance(int pos, Side side);
    /**
     * @brief 移除指定随从的圣盾
     *
     * @param pos 随从在场上的位置
     * @param side 友方:SIDE_ALLY 敌方:SIDE_ENEMY
     */
    void remove_minion_shield(int pos, Side side);

    /**
     * @brief 回溯一个操作
     *
     *
     * @param op 被回溯的操作，必须是最新的操作(op_stack.top().top())
     */
    void undo_operation(const operation &op);

    /**
     * @brief 操作栈，每个子栈代表一个攻击行为，
     * 倒序回溯最新子栈的所有内容即可回溯整个攻击行为
     *
     */
    std::stack<std::stack<operation>> op_stack;

    /**
     * @brief 处理一个随从的死亡,
     * 移到墓地、根据情况生成衍生物
     *
     * @param pos 被处理的随从在场上的位置
     * @param side 友方:SIDE_ALLY 敌方:SIDE_ENEMY
     */
    void process_death(int pos, Side side);
};

#endif