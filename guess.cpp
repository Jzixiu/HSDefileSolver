#include "Guess.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <fstream>
using namespace std;

Guess::Guess(string file)
{
    solution_atk=-1;

    fstream fin(file);
    if (!fin.is_open())
    {
        cerr << "Error: cannot open file \"" << file << "\" ." << endl;
    }

    fin >> enemy_count >> ally_count;

    max_hp=0;

    for (int i = 0; i < enemy_count; i++)
    {
        minion &e = enemy.emplace_back();
        fin >> e.atk;
        fin >> e.hp;
        if (e.hp > max_hp)
        {
            max_hp = e.hp;
        }
        assert((e.atk > 0 && e.hp > 0) && "Violation of: minion.atk>0 && minion.hp>0");
    }

    for (int i = 0; i < ally_count; i++)
    {
        minion &a = ally.emplace_back();
        fin >> a.atk;
        fin >> a.hp;
        if (a.hp > max_hp)
        {
            max_hp = a.hp;
        }
        assert((a.atk > 0 && a.hp > 0) && "Violation of: minion.atk>0 && minion.hp>0");
    }

    fin.close();
}

void Guess::solve()
{
    auto start = chrono::high_resolution_clock::now();
    dfs();
    auto end = chrono::high_resolution_clock::now();
    elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int Guess::get_solution_atk()
{
    return solution_atk;
}

vector<Guess::step> Guess::get_solution()
{
    return solution;
}

long long Guess::get_elapsed_time()
{
    return elapsed_time;
}

bool Guess::dfs(int ally_id)
{
    if(ally_id==ally_count)
    {
        if(check_solution())
        {
            return true;
        }
        return false;
    }

    for(int enemy_id=0;enemy_id<=enemy_count;enemy_id++)
    {
        if(enemy_id<enemy_count && enemy[enemy_id].hp<=0) continue;
        if(enemy_id!= enemy_count) //如果id==enemy_count代表不攻击
        {
            atk(ally_id,enemy_id);
        }
        if(dfs(ally_id+1))
        {
            return true;
        }
        if(enemy_id!=enemy_count) 
        {
            undo_last_attack();
        }
    }
    return false;
}

bool Guess::check_solution()
{
    int damage=1;//亵渎造成的伤害
    int sum_atk=0;//总敌方场攻
    bool hp_exists[max_hp+1];
    for(int i=0;i<=max_hp;i++)
    {
        hp_exists[i]=false;
    }

    for(int i=0;i<enemy_count;i++)
    {
        hp_exists[enemy[i].hp]=true;
    }
    for(int i=0;i<ally_count;i++)
    {
        hp_exists[ally[i].hp]=true;
    }

    //模拟亵渎
    while(hp_exists[damage])
    {
        damage++;
    }

    for(int i=0;i<enemy_count;i++)
    {
        if(enemy[i].hp>damage)
        {
            sum_atk+=enemy[i].atk;
        }
    }

    if(sum_atk < solution_atk || solution_atk == -1)//更优解
    {
        solution_atk=sum_atk;
        solution=steps;

        if(solution_atk==0)
        {
            return true;
        }
    }
    return false;
}

void Guess::atk(int ally_id,int enemy_id)
{
    steps.push_back(step());
    steps.back().ally_id=ally_id;
    steps.back().enemy_id=enemy_id;
    enemy[enemy_id].hp-=ally[ally_id].atk;
    ally[ally_id].hp-=enemy[enemy_id].atk;
}

Guess::step Guess::undo_last_attack()
{
    step s=steps.back();
    steps.pop_back();
    enemy[s.enemy_id].hp+=ally[s.ally_id].atk;
    ally[s.ally_id].hp+=enemy[s.enemy_id].atk;
    return s;
}