#include <iostream>
#include "GameState.h"
#include "Solver.h"
#include "Guess.h"
#include <fstream>
#include <random>
using namespace std;

int main()
{
    // bool mode;
    // cout << "求解模式: 0-近似最优 1-最优(极耗时)" << endl;
    // cout << "选择(0/1):";
    // cin>>mode;

    string file="order_correctness";

    GameState game(file);
    Solver s = Solver(game, true);
    s.set_intermediate_solution_callback(
        [](int atk, const vector<GameState::step> steps) -> void
        {
            cout << endl;
            cout << "当前更优解:" << endl;
            cout << "敌方剩余场攻为:" << atk << endl;
            cout << "解：(友方随从id -> 敌方随从id)" << endl;
            for (GameState::step s : steps)
            {
                cout << s.ally_id << " -> " << s.enemy_id << endl;
            }
            cout << endl;
        });
    s.solve();

    cout << "原始场面:" << endl;
    GameState gs(file);
    gs.print_state();

    cout << "------------------------------------------------------------" << endl;
    cout << "Solve的解:" << endl;
    GameState solve_solution(file);
    for (GameState::step s : s.get_solution())
    {
        solve_solution.attack(s);
    }
    solve_solution.print_state();
    cout << "------------------------------------------------------------" << endl;

    // std::random_device rd;
    // std::mt19937 gen(rd());

    // std::uniform_int_distribution<> minion_count(3, 10);
    // std::uniform_int_distribution<> atk_hp(1, 10);

    // string filename="test_input";

    // while(true)
    // {
    //     fstream file(filename,std::ios::out | std::ios::trunc);

    //     int enemy_count = minion_count(gen);
    //     int ally_count = minion_count(gen);

    //     file << enemy_count << endl;
    //     file << ally_count << endl;

    //     for (int enemy_id = 0; enemy_id < enemy_count; enemy_id++)
    //     {
    //         file << atk_hp(gen) << endl;
    //         file << atk_hp(gen) << endl;
    //     }

    //     for (int ally_id = 0; ally_id < ally_count; ally_id++)
    //     {
    //         file << atk_hp(gen) << endl;
    //         file << atk_hp(gen) << endl;
    //     }

    //     Guess g(filename);
    //     g.solve();

    //     GameState gs(filename);
    //     Solver s(gs, true);
    //     s.solve();

    //     cout<<g.get_elapsed_time()<<" "<<s.get_elapsed_time()<<endl;

    //     if(g.get_solution_atk()!=s.get_solution_atk())
    //     {
    //         cout<<"答案不一致:"<<endl;
    //         cout<<"Guess:"<<g.get_solution_atk();
    //         cout<<"Solve"<<s.get_solution_atk();

    //         cout<<"原始场面:"<<endl;
    //         GameState gs(filename);
    //         gs.print_state();

    //         cout<<"Guess的解:"<<endl;
    //         GameState guess_solution(filename);
    //         for(Guess::step s:g.get_solution())
    //         {
    //             guess_solution.attack(GameState::step(s.ally_id,s.enemy_id));
    //         }
    //         guess_solution.print_state();

    //         cout<<"Solve的解:"<<endl;
    //         GameState solve_solution(filename);
    //         for(GameState::step s:s.get_solution())
    //         {
    //             solve_solution.attack(s);
    //         }
    //         solve_solution.print_state();
    //         break;
    //     }

    //     file.close();
    // }

    return 0;
}