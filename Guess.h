#ifndef GUESS_H
#define GUESS_H

#include <vector>

/**
 * @brief 亵渎求解器的参照实现。
 * 用暴力枚举实现，答案全局最优。
 * 为了体现速度的对比，完全脱离了GameState这个类，
 * 因为GameState实际上要管理hp_level，但这个类完全不需要这个功能。
 * 如果强行使用GameState，对这个暴力枚举并不公平。
 */
class Guess
{
public:
    Guess(std::string file);

    void solve();

    struct step
    {
        int ally_id;
        int enemy_id;
    };

    int get_solution_atk();
    std::vector<step> get_solution();
    long long get_elapsed_time();

private:
    int solution_atk;
    std::vector<step> solution;
    long long elapsed_time;

    /**
     * @brief 暴力枚举的求解器。为每个友方随从挑选一个攻击目标，或不攻击。
     * 默认从ally_id=0开始。
     *
     * @param ally_id 友方随从id
     * @return true 提前找到了最优解
     * @return false 正常
     */
    bool dfs(int ally_id = 0);

    /**
     * @brief 检查是否是一个更优解并替换。
     *
     * @return true 是一个最优解，可以提前终止程序
     * @return false 正常
     */
    bool check_solution();

    struct minion
    {
        int atk;
        int hp;
    };

    int enemy_count;
    int ally_count;
    int max_hp;
    std::vector<minion> ally;
    std::vector<minion> enemy;
    std::vector<step> steps;

    void atk(int ally_id, int enemy_id);
    step undo_last_attack();
};

#endif