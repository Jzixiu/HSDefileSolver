#include <iostream>
#include <vector>
#include <map>
using namespace std;

class minions
{
    public:
    int atk;
    int hp;
    bool side;//0-我方 1-敌方
    bool active;//1-可攻击或可被攻击的
    vector<int> derivant;
    bool lock;
    minions()
    {
        lock=false;
    };
};

struct step
{
    int friend_id;
    int enemy_id;
};

int m;//敌方随从数量
int n;//我方随从数量
vector<minions> minion;
vector<step> steps;//i -> j
map<int,vector<int>> is_hp;// is_hp[hp] = vec  (id为vec[k]的随从的血量是hp)
vector<vector<step>> final_steps;
map<int,vector<vector<int>>> combo;//combo[k]中存储了所有打k点伤害的友方随从组合

bool check()//检查当前情况能否全解所有敌方随从
{
    int highest_hp=-1;//最高的敌方hp
    int Damage=1;//亵渎打的伤害
    for(int i=0;i<m;i++) highest_hp=max(highest_hp,minion[i].hp);
    while(is_hp[Damage].size()!=0) Damage++;
    return Damage>=highest_hp;
}

void f(int k)//锁定hp=k的随从
{
    if(check())
    {
        final_steps.push_back(steps);
        return;
    }
    
    for(int i:is_hp[k])
    {
        minion[i].lock=true;
        f(k+1);
        minion[i].lock=false;
    }

    for(int id=0;id<minion.size();id++)
    {
        if(minion[id].lock || !minion[id].active) continue;
        if(minion[id].side==0)//是我方随从
        {
            
        }
        else//是敌方随从
        {

        }
    }

}

void generate_combo()
{
    vector<int> active_friendly;
    for(int i=m;i<minion.size();i++) if(minion[i].active) active_friendly.push_back(i);
    unsigned int k=1;
    while(k<pow(2,active_friendly.size()))
    {
        int dmg=0;
        vector<int> temp;
        for(int i=0;i<active_friendly.size();i++)
        {
            if((k>>i)%2)
            {
                dmg+=minion[active_friendly[i]].atk;
                temp.push_back(active_friendly[i]);
            }
        }
        combo[dmg].push_back(temp);
        k++;
    }
}

int main()
{
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
        minion[id].side=1;

        cout<<"攻击力:";
        cin>>minion[id].atk;

        cout<<"生命值:";
        cin>>minion[id].hp;

        cout<<"如果是衍生物,它所属的随从id(非衍生物则填-1):";
        int x;
        cin>>x;
        if(x==-1) 
        {
            minion[id].active=true;
        }
        else
        {
            minion[id].active=false;            
            minion[x].derivant.push_back(id);
        }
        cout<<endl;
    }

    cout<<"输入友方随从信息:"<<endl<<endl;
    for(int i=0;i<n;i++)
    {
        int id=minion.size();
        cout<<"此友方随从id为["<<id<<"]。"<<endl;
        minion.push_back(minions());
        minion[id].side=0;

        cout<<"攻击力:";
        cin>>minion[id].atk;

        cout<<"生命值:";
        cin>>minion[id].hp;

        cout<<"如果是衍生物,它所属的随从id(非衍生物则填-1):";
        int x;
        cin>>x;
        if(x==-1) 
        {
            minion[id].active=true;
        }
        else
        {
            minion[id].active=false;            
            minion[x].derivant.push_back(id);
        }
        cout<<endl;
    }

    for(int i=0;i<minion.size();i++) is_hp[minion[i].hp].push_back(i);

    generate_combo();

    f(1);

    return 0;
}