#include <iostream>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

class minions
{
    public:
    int atk;
    int hp;
    bool alive;
    bool lock=false;//是否被锁定
    bool attack_chance=false;//攻击机会
    int father;
    vector<int> derivant;//衍生物
};

struct step
{
    int ally_id;
    int ally_hp_before;//发生攻击前的血量
    int ally_hp_after;//发生攻击后的血量
    int enemy_id;
    int enemy_hp_before;
    int enemy_hp_after;
};

int m;//敌方随从数量
int n;//友方随从数量
vector<vector<step>> final_steps;
map<int,vector<vector<int>>> combo;//combo[k]中存储了所有打k点伤害的友方随从组合
map<int,vector<int>> is_atk;//is_atk[k] 中保存了攻击力为k的敌方随从


vector<minions> minion;
vector<step> steps;//i -> j
map<int,vector<int>> is_hp_enemy;// is_hp[hp] = vec  (id为vec[k]的随从的血量是hp)
map<int,vector<int>> is_hp_ally;

bool can_attack(int id)
{
    return minion[id].alive && !minion[id].lock && minion[id].attack_chance;
}

bool can_be_attacked(int id)
{
    return minion[id].alive && !minion[id].lock;
}

bool check_combo(vector<int> &vec)//检查combo中的随从是否全部都是可攻击状态
{
    for(int i:vec) if(!can_attack(i)) return false;
    return true;
}

void move_hp_ally(int id, int from, int to)
{
    map<int,vector<int>>::iterator map_it;
    vector<int>::iterator vec_it;
    vec_it=find(is_hp_ally[from].begin(),is_hp_ally[from].begin(),id);
    is_hp_ally[from].erase(vec_it);
    if(is_hp_ally[from].size()==0)
    {
        map_it=is_hp_ally.find(from);
        is_hp_ally.erase(map_it);
    }
    is_hp_ally[to].push_back(id);
}

void move_hp_enemy(int id, int from, int to)
{
    map<int,vector<int>>::iterator map_it;
    vector<int>::iterator vec_it;
    vec_it=find(is_hp_enemy[from].begin(),is_hp_enemy[from].begin(),id);
    is_hp_enemy[from].erase(vec_it);
    if(is_hp_enemy[from].size()==0)
    {
        map_it=is_hp_enemy.find(from);
        is_hp_enemy.erase(map_it);
    }
    is_hp_enemy[to].push_back(id);
}

void atk(int ally,int enemy)
{
    step s;
    s.ally_id=ally;
    s.enemy_id=enemy;

    int hp_before,hp_after;

    hp_before=minion[ally].hp;
    hp_after=hp_before-minion[enemy].atk;
    if(hp_after<=0)//死亡结算
    {
        minion[ally].alive=false;
        for(int i:minion[ally].derivant) minion[i].alive=true;
    }
    minion[ally].hp=hp_after;
    minion[ally].attack_chance=false;
    move_hp_ally(ally,hp_before,hp_after);
    s.ally_hp_before=hp_before;
    s.ally_hp_after=hp_after;

    hp_before=minion[enemy].hp;
    hp_after=hp_before-minion[ally].atk;
    if(hp_after<=0)//死亡结算
    {
        minion[enemy].alive=false;
        for(int i:minion[enemy].derivant) minion[i].alive=true;
    }
    minion[enemy].hp=hp_after;
    move_hp_enemy(enemy,hp_before,hp_after);
    s.enemy_hp_before=hp_before;
    s.enemy_hp_after=hp_after;

    steps.push_back(s);
}

void undo_atk()
{
    step s=steps[steps.size()-1];
    minion[s.ally_id].alive=true;
    minion[s.ally_id].attack_chance=true;
    minion[s.enemy_id].alive=true;
    for(int i:minion[s.ally_id].derivant) minion[i].alive=false;
    for(int i:minion[s.enemy_id].derivant) minion[i].alive=false;

    move_hp_ally(s.ally_id,s.ally_hp_after,s.ally_hp_before);
    move_hp_enemy(s.enemy_id,s.enemy_hp_after,s.enemy_hp_before);

}

void use_combo(vector<int> &vec,int enemy_id)
{
    for(int ally_id:vec)
    {
        atk(ally_id,enemy_id);
    }
}

void f(int k)//锁定一个血量为k的随从
{
    map<int,vector<int>>::iterator map_it;
    vector<int>::iterator vec_it;
    map_it=is_hp_enemy.end();
    map_it--;
    if(k > map_it->first)
    {
        final_steps.push_back(steps);
        return;
    }

    map_it=is_hp_enemy.find(k);
    if(map_it!=is_hp_enemy.end())
    {
        for(vec_it=map_it->second.begin();vec_it<map_it->second.end();vec_it++)
        {
            int i=*vec_it;
            minion[i].lock=true;
            f(k+1);
            minion[i].lock=false;
        }
    }

    map_it=is_hp_ally.find(k);
    if(map_it!=is_hp_ally.end())
    {
        for(vec_it=map_it->second.begin();vec_it<map_it->second.end();vec_it++)
        {
            int i=*vec_it;
            minion[i].lock=true;
            f(k+1);
            minion[i].lock=false;
        }
    }

    map_it=is_hp_enemy.upper_bound(k);
    while(map_it!=is_hp_enemy.end())
    {
        int hp=map_it->first;//随从的血量
        int diff=hp-k;//血量差值
        for(vec_it=map_it->second.begin();vec_it<map_it->second.end();vec_it++)
        {
            int enemy_id=*vec_it;
            if(!can_be_attacked(enemy_id)) continue;
            for(vector<int> vec:combo[diff])
            {
                if(!check_combo(vec)) continue;

                use_combo(vec,enemy_id);
                minion[enemy_id].lock=true;
                f(k+1);
                minion[enemy_id].lock=false;
                for(int i=0;i<vec.size();i++) undo_atk();
            }
        }
        map_it++;
    }

    map_it=is_hp_ally.upper_bound(k);
    while(map_it!=is_hp_ally.end())
    {
        int hp=map_it->first;//随从的血量
        int diff=hp-k;//血量差值
        for(vec_it=map_it->second.begin();vec_it<map_it->second.end();vec_it++)
        {
            int ally_id=*vec_it;
            if(!can_attack(ally_id)) continue;
            for(int enemy_id:is_atk[diff])
            {
                if(!can_be_attacked(enemy_id)) continue;
                
                atk(ally_id,enemy_id);
                minion[ally_id].lock=true;
                f(k+1);
                minion[ally_id].lock=false;
                undo_atk();
            }
        }
        map_it++;
    }
}

void generate_combo()
{
    vector<int> alive_friendly;
    for(int i=m;i<minion.size();i++) if(minion[i].alive) alive_friendly.push_back(i);
    unsigned int k=1;
    while(k<pow(2,alive_friendly.size()))
    {
        int dmg=0;
        vector<int> temp;
        for(int i=0;i<alive_friendly.size();i++)
        {
            if((k>>i)%2)
            {
                dmg+=minion[alive_friendly[i]].atk;
                temp.push_back(alive_friendly[i]);
            }
        }
        combo[dmg].push_back(temp);
        k++;
    }
}

int main()
{
    ifstream inputFile("input");
    cin.rdbuf(inputFile.rdbuf());
    cout<<"敌方随从数量(包含衍生物):";
    cin>>m;
    cout<<"我方随从数量(包含衍生物):";
    cin>>n;

    cout<<endl;

    cout<<"输入敌方随从信息:"<<endl<<endl;
    for(int i=0;i<m;i++)
    {
        int id=minion.size();
        cout<<"此敌方随从id为["<<id<<"]。"<<endl;
        minion.push_back(minions());

        cout<<"攻击力:";
        cin>>minion[id].atk;

        cout<<"生命值:";
        cin>>minion[id].hp;

        cout<<"如果是衍生物,它所属的随从id(非衍生物则填-1):";
        int x;
        cin>>x;
        if(x==-1) 
        {
            minion[id].alive=true;
            minion[id].father=-1;
        }
        else
        {
            minion[id].father=x;
            minion[id].alive=false;            
            minion[x].derivant.push_back(id);
        }

        is_atk[minion[id].atk].push_back(id);
        is_hp_enemy[minion[id].hp].push_back(id);
        cout<<endl;
    }

    cout<<"输入友方随从信息:"<<endl<<endl;
    for(int i=0;i<n;i++)
    {
        int id=minion.size();
        cout<<"此友方随从id为["<<id<<"]。"<<endl;
        minion.push_back(minions());

        cout<<"攻击力:";
        cin>>minion[id].atk;

        cout<<"生命值:";
        cin>>minion[id].hp;

        cout<<"如果是衍生物,它所属的随从id(非衍生物则填-1):";
        int x;
        cin>>x;
        if(x==-1) 
        {
            minion[id].attack_chance=true;
            minion[id].alive=true;
            minion[id].father=-1;
        }
        else
        {
            minion[id].father=x;
            minion[id].alive=false;            
            minion[x].derivant.push_back(id);
        }

        is_hp_ally[minion[id].hp].push_back(id);

        cout<<endl;
    }
    
    generate_combo();

    f(1);
    inputFile.close();
    return 0;
}