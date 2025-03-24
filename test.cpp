#include <iostream>
#include "GameState.h"
#include "Fast_Guess.h"
#include "Guess.h"
#include "step.h"
#include <fstream>
#include <random>
using namespace std;

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> minion_count(11, 11);
    std::uniform_int_distribution<> atk_hp(1, 10);

    string filename = "test_input";

    while (true)
    {
        fstream file(filename, std::ios::out | std::ios::trunc);

        int enemy_count = minion_count(gen);
        int ally_count = minion_count(gen);

        file << enemy_count << endl;
        file << ally_count << endl;

        for (int enemy_id = 0; enemy_id < enemy_count; enemy_id++)
        {
            file << atk_hp(gen) << endl;
            file << atk_hp(gen) << endl;
        }

        for (int ally_id = 0; ally_id < ally_count; ally_id++)
        {
            file << atk_hp(gen) << endl;
            file << atk_hp(gen) << endl;
        }


        GameState gs(filename);

        Guess g(gs);
        g.solve();

        gs.undo_all();

        Fast_Guess s(gs, true);
        s.solve();

        cout << g.get_elapsed_time() << " " << s.get_elapsed_time() << endl;

        if (g.get_solution_atk() != s.get_solution_atk())
        {
            cout << "答案不一致:" << endl;
            cout << "Guess:" << g.get_solution_atk();
            cout << "Solve:" << s.get_solution_atk();

            cout << "原始场面:" << endl;
            gs.undo_all();
            gs.print_state();

            cout << "Guess的解:" << endl;
            gs.undo_all();
            for (step s : g.get_solution())
            {
                gs.attack(s);
            }
            gs.print_state();

            cout << "Solve的解:" << endl;
            gs.undo_all();
            for (step s : s.get_solution())
            {
                gs.attack(s);
            }
            gs.print_state();
            break;
        }

        file.close();
    }

    return 0;
}