#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "step.h"
#include <map>
#include <vector>
#include <list>

/**
 * @brief 可以高速管理炉石场面数据的类。
 * 复杂度:
 * m:敌方随从数量
 * n:友方随从数量
 * 构造函数:O(m+n)
 * 析构函数:O(1)
 * 所有的getter/setter: O(1)
 * atk/undo_atk: O(1)
 * 搜索函数均与结果大小有关。
 *
 */
class GameState
{
public:
    /**
     * @brief 默认无参构造函数，从stdin读入场面数据。
     * O(m+n)
     */
    GameState();

    /**
     * @brief 构造函数，从文件读入数据。
     * O(m+n)
     *
     * @param file 文件名
     */
    GameState(std::string file);
    /**
     * @brief 析构函数。
     * O(1)
     *
     */
    ~GameState();

    int max_hp_ally;
    int max_hp_enemy;

    // 全部getter和setter为O(1)

    int get_ally_count() const;
    int get_enemy_count() const;
    std::vector<step> get_current_step_history() const;

    int get_ally_atk(int id) const;
    int get_enemy_atk(int id) const;

    int get_ally_hp(int id) const;
    int get_enemy_hp(int id) const;

    int get_enemy_full_hp(int id) const;

    bool getlock_ally(int id) const;
    bool getlock_enemy(int id) const;
    void setlock_ally(int id, bool locked);
    void setlock_enemy(int id, bool locked);

    bool get_can_attack(int id) const; // 注意只有友方随从的can_attack才有意义

    /**
     * @brief 获取与每个随从互换的敌方随从的id。
     * O(m*n)
     *
     * @return vector<vector<int>> id列表
     */
    std::vector<std::vector<int>> get_minion_exchange_targets() const;

    /**
     * @brief 获取一份当前血量值为hp的友方随从的列表的快照
     * 远小于O(n)
     *
     * @param hp 血量值
     * @return vector<int> id列表
     */
    std::vector<int> get_ally_of_hp(int hp) const;

    /**
     * @brief 获取一份当前血量值为hp的敌方随从的列表的快照
     * 远小于O(m)
     *
     * @param hp 血量值
     * @return vector<int> id列表
     */
    std::vector<int> get_enemy_of_hp(int hp) const;

    /**
     * @brief 获取所有血量大于hp的友方随从的id列表
     * 远小于O(max_hp_ally+n)
     *
     * @param hp 血量下限（不包含）
     * @return vector<int> 友方随从列表
     */
    std::vector<int> get_ally_gt_hp(int hp) const;

    /**
     * @brief 获取攻击力为attack的敌方随从的一份列表
     * O(m)
     *
     * @param attack 攻击力
     * @return vector<int> 列表
     */
    std::vector<int> get_enemy_of_atk(int attack) const;

    /**
     * @brief 获取所有血量大于hp的敌方随从的id列表
     * 远小于O(max_hp_enemy+m)
     *
     * @param hp 血量下限（不包含）
     * @return vector<int> 敌方随从列表
     */
    std::vector<int> get_enemy_gt_hp(int hp) const;

    /**
     * @brief 获取所有当前状态下可以造成总共total_damage点伤害的友方随从组合。
     * 保证返回结果中的友方随从都是!locked且can_attack的。
     * 远小于O(m*total_damage*(2^m))
     *
     * @param total_damage 要造成多少伤害
     * @return vector<vector<int>> 友方随从组合的列表
     */
    std::vector<std::vector<int>> get_combo_of_atk(int total_damage) const;

    /**
     * @brief 执行s步骤中的攻击指令。
     * O(1)
     *
     * @param s 步骤指令
     */
    void attack(step s);

    /**
     * @brief 撤销上一个攻击指令
     * O(1)
     *
     * @return step
     */
    step undo_last_attack();

    void undo_all();

    void print_ally(int id) const;
    void print_enemy(int id) const;
    void print_state() const;

private:
    struct minion
    {
        int atk;
        int hp;
        bool locked;
        bool can_attack;
        std::list<int>::iterator it;
    };

    std::list<int> *is_hp_ally;
    std::list<int> *is_hp_enemy;

    std::vector<minion> ally;
    std::vector<minion> enemy;

    std::vector<int> enemy_full_hp;

    std::vector<step> step_history;

    /**
     * @brief 更新某友方随从的血量。
     * O(1)
     *
     * @param id 被更新随从的id
     * @param new_hp 要更新到的hp
     * @return int 旧hp值
     */
    int move_hp_ally(int id, int new_hp);

    /**
     * @brief 更新某敌方随从的血量。
     * O(1)
     *
     * @param id 被更新随从的id
     * @param new_hp 要更新到的hp
     * @return int 旧hp值
     */
    int move_hp_enemy(int id, int new_hp);
};

#endif