#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <map>
#include <vector>
#include <list>

class GameState
{
public:
    GameState();
    GameState(std::string file);
    ~GameState();

    struct step
    {
        step();
        step(int ally_id, int enemy_id);
        int ally_id;
        int enemy_id;
    };

    int max_hp_ally;
    int max_hp_enemy;

    int get_ally_count();
    int get_enemy_count();
    std::vector<step> get_current_step_history();

    int get_ally_atk(int id);
    int get_enemy_atk(int id);

    int get_ally_hp(int id);
    int get_enemy_hp(int id);

    bool getlock_ally(int id);
    bool getlock_enemy(int id);
    void setlock_ally(int id, bool locked);
    void setlock_enemy(int id, bool locked);

    bool get_can_attack(int id); // 注意只有友方随从的can_attack才有意义

    /**
     * @brief 获取那些可以使双方随从互换的步骤
     *
     * @return std::vector<std::vector<int>> 步骤列表
     */
    std::vector<std::vector<int>> get_minion_exchange_targets();

    /**
     * @brief 获取一份当前血量值为hp的友方随从的列表的快照
     *
     * @param hp 血量值
     * @return std::vector<int> id列表
     */
    std::vector<int> get_ally_of_hp(int hp);

    /**
     * @brief 获取一份当前血量值为hp的敌方随从的列表的快照
     *
     * @param hp 血量值
     * @return std::vector<int> id列表
     */
    std::vector<int> get_enemy_of_hp(int hp);

    /**
     * @brief 获取所有血量大于hp的友方随从的id列表
     *
     * @param hp 血量下限（不包含）
     * @return std::vector<int> 友方随从列表
     */
    std::vector<int> get_ally_gt_hp(int hp);

    /**
     * @brief 获取攻击力为attack的敌方随从的一份列表
     *
     * @param attack 攻击力
     * @return std::vector<int> 列表
     */
    std::vector<int> get_enemy_of_atk(int attack);

    /**
     * @brief 获取所有血量大于hp的敌方随从的id列表
     *
     * @param hp 血量下限（不包含）
     * @return std::vector<int> 敌方随从列表
     */
    std::vector<int> get_enemy_gt_hp(int hp);

    /**
     * @brief 获取所有当前状态下可以造成总共total_damage点伤害的友方随从组合。
     * 保证返回结果中的友方随从都是!locked且can_attack的。
     *
     * @param total_damage 要造成多少伤害
     * @return vector<vector<int>> 友方随从组合的列表
     */
    std::vector<std::vector<int>> get_combo_of_atk(int total_damage);

    void attack(step s);
    step undo_last_attack();
    void undo_all();

    void print_ally(int id);
    void print_enemy(int id);
    void print_state();

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

    std::vector<step> step_history;

    int move_hp_ally(int id, int new_hp);
    int move_hp_enemy(int id, int new_hp);
};

#endif