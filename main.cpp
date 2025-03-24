#include <iostream>
#include "step.h"
#include "GameState.h"
#include "ISolver.h"
#include "Fast_Guess.h"
#include "Guess.h"
#include <fstream>
#include <random>
using namespace std;

int main()
{
    string file = "test_input";

    GameState game(file);
    Fast_Guess s = Fast_Guess(game,true);
    // Guess s = Guess(game);
    s.register_incumbent_callback(
        [](int atk, const vector<step> steps) -> void
        {
            cout << endl;
            cout << "当前更优解:" << endl;
            cout << "敌方剩余场攻为:" << atk << endl;
            cout << "解：(友方随从id -> 敌方随从id)" << endl;
            for (step s : steps)
            {
                cout << s.ally_id << " -> " << s.enemy_id << endl;
            }
            cout << endl;
        });
    s.solve();

    game.undo_all();

    cout << "原始场面:" << endl;
    game.print_state();

    game.undo_all();

    cout << "------------------------------------------------------------" << endl;
    cout << "解:" << s.get_solution_atk() << endl;
    for (step s : s.get_solution())
    {
        game.attack(s);
    }
    game.print_state();
    cout << "------------------------------------------------------------" << endl;

    return 0;
}