#ifndef SOLVER_H
#define SOLVER_H

#include "GameState.h"
#include <vector>

/**
 * @brief
 * 与GameState类配合的炉石传说“亵渎”最优解场（非纯暴力枚举)求解器。
 * 关于最优解的重要注释：
 * 最优解指使用亵渎解场后获得的敌方场攻是全局最低；
 * 如果该场面可以用纯解场（不用亵渎）的方式获得最低场攻，那么这不包含在上述最优解中。
 */
class Solver
{
public:
    /**
     * @brief 求解器的构造器
     *
     * @param game 要求解的游戏状态
     * @param mode 求解模式(0-近似最优/1-最优)
     */
    Solver(GameState &game, bool mode);

    /**
     * @brief 注册回调函数，会在求解过程中取得更优解时回调；
     * 在某些解求解过程过长时，获取当前进度。
     * 
     * @param callback 
     */
    void set_intermediate_solution_callback(std::function<void(int,const std::vector<GameState::step>)> callback);

    void solve();

    int get_solution_atk();
    std::vector<GameState::step> get_solution();
    long long get_elapsed_time();

private:
    GameState &gs;

    int solution_atk;
    std::vector<GameState::step> solution;
    long long elapsed_time;

    bool mode_is_optimal;
    std::vector<std::vector<int>> minion_exchange_targets; // 可以造成双方随从互换的步骤列表

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

    std::function<void(int, const std::vector<GameState::step>&)> intermediate_solution_callback;
};

#endif