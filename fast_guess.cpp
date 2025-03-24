#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <cassert>
using namespace std;

// #define DEBUG

int m;//敌方随从数量
int n;//友方随从数量

struct minion
{
    int atk;
    int hp;
    bool lock=false;
    bool can_attack=true;//攻击机会
};

struct step
{
    int ally_id;
    int enemy_id;
};

vector<minion> ally;
vector<minion> enemy;

map<int,vector< vector<int> > > is_atk_combo;
map<int,vector<int> > is_atk_enemy;
map<int,vector<int> > is_hp_ally;
map<int,vector<int> > is_hp_enemy;

vector<step> steps;
vector<step> solution;
int sol_atk=-1;//当前解对应的对方场攻

void move_hp_ally(int id, int from, int to)
{
    ally[id].hp=to;

    vector<int>::iterator vec_it;
    vec_it=find(is_hp_ally[from].begin(),is_hp_ally[from].end(),id);
    assert(vec_it!=is_hp_ally[from].end());
    is_hp_ally[from].erase(vec_it);
    is_hp_ally[to].push_back(id);
}

void move_hp_enemy(int id, int from, int to)
{
    enemy[id].hp=to;

    vector<int>::iterator vec_it;
    vec_it=find(is_hp_enemy[from].begin(),is_hp_enemy[from].end(),id);
    assert(vec_it!=is_hp_enemy[from].end());
    is_hp_enemy[from].erase(vec_it);
    is_hp_enemy[to].push_back(id);
}

void atk(int ally_id,int enemy_id)
{
    #ifdef DEBUG
    assert(!ally[ally_id].lock && ally[ally_id].can_attack);
    assert(!enemy[enemy_id].lock);
    assert(ally[ally_id].hp>0);
    assert(enemy[enemy_id].hp>0);
    #endif

    steps.push_back(step());
    steps.back().ally_id=ally_id;
    steps.back().enemy_id=enemy_id;
    move_hp_ally(ally_id,ally[ally_id].hp,ally[ally_id].hp-enemy[enemy_id].atk);
    move_hp_enemy(enemy_id,enemy[enemy_id].hp,enemy[enemy_id].hp-ally[ally_id].atk);
    ally[ally_id].can_attack=false;
}

void undo_atk()
{
    step s=steps.back();
    steps.pop_back();
    move_hp_ally(s.ally_id,ally[s.ally_id].hp,ally[s.ally_id].hp+enemy[s.enemy_id].atk);
    move_hp_enemy(s.enemy_id,enemy[s.enemy_id].hp,enemy[s.enemy_id].hp+ally[s.ally_id].atk);

    #ifdef DEBUG
    assert(!ally[s.ally_id].can_attack);
    #endif

    ally[s.ally_id].can_attack=true;
}

//检查某combo是否可用
bool check_combo(vector<int> combo)
{
    for(int i:combo)
    {
        if(ally[i].lock || !ally[i].can_attack) return false;
    }
    return true;
}

//锁定一个生命值为k的随从
void f(int k)
{
    map<int,vector<int>>::iterator map_it;
    vector<int> temp_vec;
    bool leaf=true;//仅在叶子节点结算

    //找正好k血量的友方随从
    map_it=is_hp_ally.find(k);
    if(map_it!=is_hp_ally.end())
    {
        temp_vec=map_it->second;
        for(int ally_id:temp_vec)
        {
            ally[ally_id].lock=true;
            leaf=false;
            f(k+1);
            ally[ally_id].lock=false;
        }
    }
    
    //找正好k血量的敌方随从
    map_it=is_hp_enemy.find(k);
    if(map_it!=is_hp_enemy.end())
    {
        temp_vec=map_it->second;
        for(int enemy_id:temp_vec)
        {
            enemy[enemy_id].lock=true;
            leaf=false;
            f(k+1);
            enemy[enemy_id].lock=false;
        }
    }

    //找hp>k的友方随从，找atk=diff的敌方随从修血
    for(map_it=is_hp_ally.upper_bound(k);map_it!=is_hp_ally.end();map_it++)
    {
        int hp=map_it->first;//随从的血量
        int diff=hp-k;//血量差值
        temp_vec=map_it->second;
        for(int ally_id:temp_vec)
        {
            if(ally[ally_id].lock || !ally[ally_id].can_attack) continue;
            for(int enemy_id:is_atk_enemy[diff])
            {
                if(enemy[enemy_id].lock || enemy[enemy_id].hp<=0) continue;
                atk(ally_id,enemy_id);
                ally[ally_id].lock=true;
                leaf=false;
                f(k+1);
                ally[ally_id].lock=false;
                undo_atk();
            }
        }
    }

    //找hp>k的敌方随从，找combo攻击力=hp-k的修血量
    for(map_it=is_hp_enemy.upper_bound(k);map_it!=is_hp_enemy.end();map_it++)
    {
        int hp=map_it->first;//随从的血量
        int diff=hp-k;//血量差值
        temp_vec=map_it->second;
        for(int enemy_id:temp_vec)
        {
            if(enemy[enemy_id].lock) continue;
            for(vector<int> chosen_combo:is_atk_combo[diff])
            {
                if(!check_combo(chosen_combo)) continue;
                for(int ally_id:chosen_combo) atk(ally_id,enemy_id);
                enemy[enemy_id].lock=true;
                leaf=false;
                f(k+1);
                enemy[enemy_id].lock=false;
                for(int i=0;i<chosen_combo.size();i++) undo_atk();
            }
        }
    }

    if(leaf)
    {
        //未找到k血量的随从，那么亵渎就打了k点伤害
        int sum_atk=0;
        for(int i=0;i<m;i++)
        {
            if(enemy[i].hp>k) sum_atk+=enemy[i].atk;
        }
        if(sum_atk<sol_atk || sol_atk==-1)
        {
            sol_atk=sum_atk;
            solution=steps;
        }
    }

}

void init()
{
    cout<<"输入敌方随从数量:";
    cin>>m;
    cout<<"输入友方随从数量:";
    cin>>n;

    for(int i=0;i<m;i++)
    {
        enemy.push_back(minion());
        cout<<"输入第["<<i+1<<"]个敌方随从的信息:"<<endl;
        cout<<"atk:";
        cin>>enemy.back().atk;
        cout<<"hp:";
        cin>>enemy.back().hp;
    }

    for(int i=0;i<n;i++)
    {
        ally.push_back(minion());
        cout<<"输入第["<<i+1<<"]个友方随从的信息:"<<endl;
        cout<<"atk:";
        cin>>ally.back().atk;
        cout<<"hp:";
        cin>>ally.back().hp;
    }

    
    for(unsigned k=0;k< (1u << ally.size()) ;k++)
    {
        int dmg=0;
        vector<int> temp;
        for(unsigned i=0;i<ally.size();i++)
        {
            if((k>>i)%2)
            {
                dmg += ally[i].atk;
                temp.push_back(i);
            }
        }
        is_atk_combo[dmg].push_back(temp);
    }
    

    for(int i=0;i<enemy.size();i++)
    {
        is_atk_enemy[enemy[i].atk].push_back(i);
    }

    for(int i=0;i<ally.size();i++)
    {
        is_hp_ally[ally[i].hp].push_back(i);
    }

    for(int i=0;i<enemy.size();i++)
    {
        is_hp_enemy[enemy[i].hp].push_back(i);
    }

    #ifdef DEBUG
    assert(m==enemy.size());
    assert(n==ally.size());
    #endif

}

int main()
{
    #ifdef DEBUG
    ifstream infile("/Users/venoflame/desktop/code/HSDefile/input");
    ofstream outfile("/Users/venoflame/desktop/code/HSDefile/res_fast_guess");
    std::ofstream null_stream;
    streambuf* cinbuf = cin.rdbuf();
    streambuf* coutbuf = cout.rdbuf();
    cin.rdbuf(infile.rdbuf());
    cout.rdbuf(null_stream.rdbuf());
    #endif

    init();

    f(1);

    cout<<endl<<endl<<"解:敌方最低场攻为"<<sol_atk<<endl;
    for(step s:solution)
    {
        cout<<"友方["<<s.ally_id+1<<"]->敌方["<<s.enemy_id+1<<"]"<<endl;
    }

    #ifdef DEBUG
    outfile<<sol_atk;
    //恢复流指针，防止全局析构出问题
    cin.rdbuf(cinbuf);
    cout.rdbuf(coutbuf);
    infile.close();
    outfile.close();
    #endif
    return 0;
}