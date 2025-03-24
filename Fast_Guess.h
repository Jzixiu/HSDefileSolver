#ifndef FAST_GUESS_H
#define FAST_GUESS_H

#include <vector>
#include <set>
#include "step.h"
#include "GameState.h"
#include "ISolver.h"

class Fast_Guess : public ISolver
{
public:
    /**
     * @brief 求解器的构造器
     *
     * @param game 要求解的游戏状态
     * @param mode 求解模式(0-近似最优/1-最优)
     */
    Fast_Guess(GameState &game, bool mode);

    void solve() override;

private:
    bool mode_is_optimal;
    std::vector<std::vector<int>> minion_exchange_targets; // 某友方随从和哪些敌方随从可以造成互换

    struct CompareByAttack
    {
        const GameState *gs_ptr;

        CompareByAttack(const GameState *gs_ptr);

        bool operator()(int left_id, int right_id) const;
    };

    /**
     * @brief 记录某敌方随从被哪些友方随从攻击过，用来处理真正的攻击顺序。
     * 与gs.history中记录的不同:gs中记录的是模拟顺序，
     * gs中是可以出现“对死亡的随从进行攻击”的情况的。
     *
     */
    std::vector<std::set<int, CompareByAttack>> attackers;

    /**
     * @brief 保存每个attackers中的id所对应的攻击力的和。
     *
     */
    std::vector<int> sum_of_attackers;

    /**
     * @brief 尝试进行一次ally_id对enemy_id的攻击，
     * 对于已死亡的敌方随从尝试通过逆转步骤的方式来进行这次攻击，
     * 并正确记录在attackers中。
     *
     * @param ally_id 发起攻击的友方随从id
     * @param enemy_id 被攻击的敌方随从id
     */
    void attack(int ally_id, int enemy_id);

    /**
     * @brief 撤销上次攻击,在attackers和gs中更新对应状态。
     *
     */
    void undo_last_attack();

    /**
     * @brief 带有随从互换预处理的求解器，答案是最优。
     * 递归地为第ally_id个友方随从选择互换目标(包括不互换),
     * 使问题退化成一个更小的问题,
     * 然后在叶子节点调用dfs()进行求解。
     * 默认从0开始。
     *
     * @param ally_id 哪个随从进行互换
     * @return true 提前找到了最优解
     * @return false 正常
     */
    bool minion_exchange(int ally_id = 0);

    /**
     * @brief 不带预处理的求解器，速度快但答案不保证最优(近最优)。
     * 尝试凑一个血量为hp_level的随从。
     * 默认从1开始。
     *
     * @param hp_level 要凑的血量。
     * @return true 提前找到了最优解
     * @return false 正常
     */
    bool dfs(int hp_level = 1);

    /**
     * @brief 检查是否是一个更优解
     *
     * @param damage 当前亵渎能造成的伤害
     * @return true 是一个最优解，可以提前终止程序
     * @return false 正常
     */
    bool check_solution(int damage);
};

#endif