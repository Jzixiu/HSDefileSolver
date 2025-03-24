#include <iostream>
#include <vector>
#include <map>
using namespace std;

class minions
{
    public:
    int atk;
    int hp;
    bool taunt;//嘲讽
    bool alive;
    bool attack_chance;//攻击机会
    bool lock=false;//是否被锁定
    vector<int> derivant;//衍生物
};

struct step
{
    int ally_id;
    int enemy_id;
};

//静止的初始化过的数据
struct Config {
    int m; //敌方随从数量
    int n; //友方随从数量
    map<int, vector<int> > is_atk; //攻击力为k的敌方随从
    map<int, vector<vector<int> > > combo; //打k点伤害的友方随从组合
}config;

//需要动态维护的游戏状态
struct GameState {
    vector<minions> enemy;
    vector<minions> ally;
    map<int, vector<int> > is_hp_enemy; //敌方随从生命值映射
    map<int, vector<int> > is_hp_ally;  //友方随从生命值映射
    vector<step> steps;
}game;

void input()
{
    int temp;

    cout<<"敌方随从数量(包含衍生物):";
    cin>>config.m;
    cout<<"我方随从数量(包含衍生物):";
    cin>>config.n;
    cout<<endl;

    cout<<"请按照亡语先后顺序输入所有随从。先输入某随从,再输入其衍生物。例：先输入机械蛋，再输入机械暴龙。"<<endl<<endl;
    cout<<"输入敌方随从的信息:"<<endl;
    for(int i=0;i<config.m;i++)
    {
        int id=game.enemy.size();
        cout<<"此敌方随从id为["<<id<<"]。"<<endl;
        game.enemy.push_back(minions());

        cout<<"攻击力:";
        cin>>game.enemy[id].atk;

        cout<<"生命值:";
        cin>>game.enemy[id].hp;

        cout<<"具有嘲讽(1-是/0-否):";
        cin>>game.enemy[id].taunt;

        cout<<"如果本随从是某随从的衍生物,输入其所属的随从的id(否则输入-1):";
        cin>>temp;
        if(temp==-1)//是一个主随从(非衍生物)
        {
            game.enemy[i].alive=true;
            game.enemy[i].attack_chance=true;
        }
        else//是某随从(主随从/衍生物)的衍生物
        {
            game.enemy[i].alive=false;
            game.enemy[i].attack_chance=false;
            game.enemy[temp].derivant.push_back(i);//i是temp的衍生物
        }
    }
}

void init()
{
    input();
}

int main()
{
    init();

}