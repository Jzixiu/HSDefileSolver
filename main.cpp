#include "GameState.h"
#include <iostream>
#include <istream>
#include <fstream>
using namespace std;

int main()
{
    std::ifstream fin("test_input.txt");
    GameState gs(fin);
    gs.attack(0,0);
    gs.attack(0,0);
    gs.undo_last_attack();
    gs.undo_last_attack();
    fin.close();
    return 0;
}