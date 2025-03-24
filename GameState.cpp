#include "GameState.h"
#include <iostream>
#include <cassert>
#include <fstream>
using namespace std;

// public:

GameState::GameState()
{
    int m, n;
    cout << "输入敌方随从数量:";
    cin >> m;
    cout << "输入友方随从数量:";
    cin >> n;

    max_hp_ally = 0;
    max_hp_enemy = 0;

    for (int i = 0; i < m; i++)
    {
        minion &e = enemy.emplace_back();
        cout << "输入第[" << i + 1 << "]个敌方随从的信息:" << endl;
        cout << "atk:";
        cin >> e.atk;
        cout << "hp:";
        cin >> e.hp;
        e.locked = false;
        e.can_attack = false;
        if (e.hp > max_hp_enemy)
        {
            max_hp_enemy = e.hp;
        }
        assert((e.atk > 0 && e.hp > 0) && "Violation of: minion.atk>0 && minion.hp>0");
    }

    for (int i = 0; i < n; i++)
    {
        minion &a = ally.emplace_back();
        cout << "输入第[" << i + 1 << "]个友方随从的信息:" << endl;
        cout << "atk:";
        cin >> a.atk;
        cout << "hp:";
        cin >> a.hp;
        a.locked = false;
        a.can_attack = true;
        if (a.hp > max_hp_ally)
        {
            max_hp_ally = a.hp;
        }
        assert((a.atk > 0 && a.hp > 0) && "Violation of: minion.atk>0 && minion.hp>0");
    }

    is_hp_ally = new list<int>[max_hp_ally + 1];
    is_hp_enemy = new list<int>[max_hp_enemy + 1];

    for (int ally_id = 0; ally_id < get_ally_count(); ally_id++)
    {
        minion &a = ally[ally_id];
        a.it = is_hp_ally[a.hp].insert(is_hp_ally[a.hp].begin(), ally_id);
    }
    for (int enemy_id = 0; enemy_id < get_enemy_count(); enemy_id++)
    {
        minion &e = enemy[enemy_id];
        e.it = is_hp_enemy[e.hp].insert(is_hp_enemy[e.hp].begin(), enemy_id);
    }

    cout << endl;
}

GameState::GameState(string file)
{
    fstream fin(file);
    if (!fin.is_open())
    {
        cerr << "Error: cannot open file \"" << file << "\" ." << endl;
    }

    int m, n;
    fin >> m >> n;

    max_hp_ally = 0;
    max_hp_enemy = 0;

    for (int i = 0; i < m; i++)
    {
        minion &e = enemy.emplace_back();
        fin >> e.atk;
        fin >> e.hp;
        e.locked = false;
        e.can_attack = false;
        if (e.hp > max_hp_enemy)
        {
            max_hp_enemy = e.hp;
        }
        assert((e.atk > 0 && e.hp > 0) && "Violation of: minion.atk>0 && minion.hp>0");
    }

    for (int i = 0; i < n; i++)
    {
        minion &a = ally.emplace_back();
        fin >> a.atk;
        fin >> a.hp;
        a.locked = false;
        a.can_attack = true;
        if (a.hp > max_hp_ally)
        {
            max_hp_ally = a.hp;
        }
        assert((a.atk > 0 && a.hp > 0) && "Violation of: minion.atk>0 && minion.hp>0");
    }

    is_hp_ally = new list<int>[max_hp_ally + 1];
    is_hp_enemy = new list<int>[max_hp_enemy + 1];

    for (int ally_id = 0; ally_id < get_ally_count(); ally_id++)
    {
        minion &a = ally[ally_id];
        a.it = is_hp_ally[a.hp].insert(is_hp_ally[a.hp].begin(), ally_id);
    }
    for (int enemy_id = 0; enemy_id < get_enemy_count(); enemy_id++)
    {
        minion &e = enemy[enemy_id];
        e.it = is_hp_enemy[e.hp].insert(is_hp_enemy[e.hp].begin(), enemy_id);
    }

    fin.close();
}

GameState::~GameState()
{
    delete[] is_hp_ally;
    delete[] is_hp_enemy;
}

GameState::step::step() = default;

GameState::step::step(int ally_id, int enemy_id)
    : ally_id(ally_id), enemy_id(enemy_id) {}

int GameState::get_ally_count()
{
    return ally.size();
}

int GameState::get_enemy_count()
{
    return enemy.size();
}

vector<GameState::step> GameState::get_current_step_history()
{
    return step_history;
}

int GameState::get_ally_atk(int id)
{
    return ally[id].atk;
}

int GameState::get_enemy_atk(int id)
{
    return enemy[id].atk;
}

int GameState::get_ally_hp(int id)
{
    return ally[id].hp;
}

int GameState::get_enemy_hp(int id)
{
    return enemy[id].hp;
}

bool GameState::getlock_ally(int id)
{
    return ally[id].locked;
}

bool GameState::getlock_enemy(int id)
{
    return enemy[id].locked;
}

void GameState::setlock_ally(int id, bool locked)
{
    ally[id].locked = locked;
}

void GameState::setlock_enemy(int id, bool locked)
{
    enemy[id].locked = locked;
}

bool GameState::get_can_attack(int id)
{
    return ally[id].can_attack;
}

std::vector<std::vector<int>> GameState::get_minion_exchange_targets()
{
    assert(step_history.size() == 0 && "Violation of: step_history.size()==0");

    std::vector<std::vector<int>> res;

    for (int ally_id = 0; ally_id < get_ally_count(); ally_id++)
    {
        std::vector<int> &target = res.emplace_back();
        for (int enemy_id = 0; enemy_id < get_enemy_count(); enemy_id++)
        {
            if (ally[ally_id].atk >= enemy[enemy_id].hp && enemy[enemy_id].atk >= ally[ally_id].hp)
            {
                target.push_back(enemy_id);
            }
        }
    }

    return res;
}

vector<int> GameState::get_ally_of_hp(int hp)
{
    assert((0 < hp) && (hp <= max_hp_ally) && "Violation of: (0<hp<=max_hp_ally)");

    vector<int> snapshot;
    snapshot.reserve(is_hp_ally[hp].size());

    for (int id : is_hp_ally[hp])
    {
        snapshot.push_back(id);
    }

    return snapshot;
}

vector<int> GameState::get_enemy_of_hp(int hp)
{
    assert((0 < hp) && (hp <= max_hp_enemy) && "Violation of: (0<hp<=max_hp_enemy)");

    vector<int> snapshot;
    snapshot.reserve(is_hp_enemy[hp].size());

    for (int id : is_hp_enemy[hp])
    {
        snapshot.push_back(id);
    }

    return snapshot;
}

vector<int> GameState::get_ally_gt_hp(int hp)
{
    assert((0 < hp) && (hp <= max_hp_ally) && "Violation of: (0<hp<=max_hp_ally)");

    vector<int> res;
    for (int i = hp + 1; i <= max_hp_ally; i++)
    {
        for (int id : is_hp_ally[i])
        {
            res.push_back(id);
        }
    }
    return res;
}

vector<int> GameState::get_enemy_of_atk(int attack)
{
    vector<int> vec;
    for (int enemy_id = 0; enemy_id < get_enemy_count(); enemy_id++)
    {
        if (enemy[enemy_id].atk == attack)
        {
            vec.push_back(enemy_id);
        }
    }
    return vec;
}

vector<int> GameState::get_enemy_gt_hp(int hp)
{
    assert((0 < hp) && (hp <= max_hp_enemy) && "Violation of: (0<hp<=max_hp_enemy)");
    vector<int> res;
    for (int i = hp + 1; i <= max_hp_enemy; i++)
    {
        for (int id : is_hp_enemy[i])
        {
            res.push_back(id);
        }
    }
    return res;
}

vector<vector<int>> GameState::get_combo_of_atk(int damage)
{
    vector<int> avaliable_ally;
    for (int id = 0; id < get_ally_count(); id++)
    {
        if (ally[id].can_attack && !ally[id].locked)
        {
            avaliable_ally.push_back(id);
        }
    }

    vector<vector<vector<int>>> dp(damage + 1);
    dp[0].push_back({});

    for (int ally_id : avaliable_ally)
    {
        int ally_atk = ally[ally_id].atk;
        for (int i = damage - ally_atk; i >= 0; i--)
        {
            for (const vector<int> &comb : dp[i])
            {
                dp[i + ally_atk].push_back(comb);
                dp[i + ally_atk].back().push_back(ally_id);
            }
        }
    }
    return dp[damage];
}

void GameState::attack(step s)
{
    assert(s.ally_id < get_ally_count() && "Violation of: s.ally_id < get_ally_count()");
    assert(s.enemy_id < get_enemy_count() && "Violation of: s.enemy_id < get_enemy_count()");
    assert(ally[s.ally_id].can_attack && "Violation of: ally[s.ally_id].can_attack");
    assert(!ally[s.ally_id].locked && "Violation of: !ally[s.ally_id].locked");
    assert(!enemy[s.enemy_id].locked && "Violation of: !enemy[s.enemy_id].locked");

    step_history.push_back(s);

    move_hp_ally(s.ally_id, ally[s.ally_id].hp - enemy[s.enemy_id].atk);
    move_hp_enemy(s.enemy_id, enemy[s.enemy_id].hp - ally[s.ally_id].atk);
    ally[s.ally_id].can_attack = false;
}

GameState::step GameState::undo_last_attack()
{
    assert((step_history.size() > 0) && "Violation of: step_history.size>0");

    step s = step_history.back();
    step_history.pop_back();

    move_hp_ally(s.ally_id, ally[s.ally_id].hp + enemy[s.enemy_id].atk);
    move_hp_enemy(s.enemy_id, enemy[s.enemy_id].hp + ally[s.ally_id].atk);
    ally[s.ally_id].can_attack = true;

    return s;
}

void GameState::undo_all()
{
    while(step_history.size()>0)
    {
        undo_last_attack();
    }
}

void GameState::print_ally(int id)
{
    cout << id << ":(" << ally[id].atk << "/" << ally[id].hp << ") ";
}

void GameState::print_enemy(int id)
{
    cout << id << ":(" << enemy[id].atk << "/" << enemy[id].hp << ") ";
}

void GameState::print_state()
{
    cout << endl;

    for (int enemy_id = 0; enemy_id < get_enemy_count(); enemy_id++)
    {
        print_enemy(enemy_id);
    }

    cout << endl
         << endl;

    for (int ally_id = 0; ally_id < get_ally_count(); ally_id++)
    {
        print_ally(ally_id);
    }

    cout << endl
         << endl;

    cout << "Enemy count:" << get_enemy_count() << endl;
    cout << "Ally count:" << get_ally_count() << endl;

    cout << endl;

    cout << "Steps taken:" << endl;
    if (get_current_step_history().size() == 0)
    {
        cout << "No steps." << endl;
    }
    else
    {
        for (const step &s : step_history)
        {
            cout << s.ally_id << " -> " << s.enemy_id << endl;
        }
    }

    cout << endl;

    cout << "Hp levels:" << endl;

    cout << endl;

    cout << "Ally:" << endl;

    for (int hp = 0; hp <= max_hp_ally; hp++)
    {
        if (is_hp_ally[hp].size() > 0)
        {
            cout << "Hp=" << hp << ": ";
            for (list<int>::iterator it = is_hp_ally[hp].begin(); it != is_hp_ally[hp].end(); ++it)
            {
                print_ally(*it);
            }
            cout << endl;
        }
    }

    cout << endl;

    cout << "Enemy:" << endl;

    for (int hp = 0; hp <= max_hp_enemy; hp++)
    {
        if (is_hp_enemy[hp].size() > 0)
        {
            cout << "Hp=" << hp << ": ";
            for (list<int>::iterator it = is_hp_enemy[hp].begin(); it != is_hp_enemy[hp].end(); ++it)
            {
                print_enemy(*it);
            }
            cout << endl;
        }
    }

    cout << endl;
}

// private:

int GameState::move_hp_ally(int id, int new_hp)
{
    minion &a = ally[id];
    int old_hp = a.hp;
    if (old_hp > 0)
    {
        is_hp_ally[old_hp].erase(a.it);
    }

    a.hp = new_hp;

    if (new_hp > 0)
    {
        a.it = is_hp_ally[new_hp].insert(is_hp_ally[new_hp].begin(), id);
    }

    return old_hp;
}

int GameState::move_hp_enemy(int id, int new_hp)
{
    minion &e = enemy[id];
    int old_hp = e.hp;
    if (old_hp > 0)
    {
        is_hp_enemy[old_hp].erase(e.it);
    }

    e.hp = new_hp;

    if (new_hp > 0)
    {
        e.it = is_hp_enemy[new_hp].insert(is_hp_enemy[new_hp].begin(), id);
    }

    return old_hp;
}