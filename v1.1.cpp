#include <iostream>
#include <vector>
using namespace std;

class minion
{
    public:
    int attack;
    int hp;
    int derivantAtk;//衍生物攻击力
    int derivantHp;//衍生物血量
};

int m;//敌方随从数量
int n;//我方随从数量
vector<minion> enemy;
vector<minion> friendly;
vector<int> target;//每个我方随从的目标
vector<int> order;//我方随从攻击的顺序

int enemy_atk_sum=-1;
int friendly_atk_sum=0;
vector<int> final_target;
vector<int> final_order;
vector<minion> final_enemy;
vector<minion> final_friendly;

void atk(minion &a,minion &b)//攻击,但顺序不重要
{
    a.hp-=b.attack;
    b.hp-=a.attack;
}

void check(vector<minion> &t_friendly,vector<minion> &t_enemy)
{
    vector<bool> flag;//flag[i]代表是否存在血量为i的随从
    flag.push_back(false);//第0个
    for(int i=0;i<m;i++)
    {
        if(t_enemy[i].hp>0)
        {
            while(t_enemy[i].derivantHp+t_enemy[i].hp>=flag.size()) flag.push_back(false);
            flag[t_enemy[i].hp]=true;
            flag[t_enemy[i].derivantHp+t_enemy[i].hp]=true;
        }
        else
        {
            while(t_enemy[i].derivantHp>=flag.size()) flag.push_back(false);
            flag[t_enemy[i].derivantHp]=true;
        }

    }
    for(int i=0;i<n;i++)
    {
        if(t_friendly[i].hp>0)
        {
            flag[t_friendly[i].hp]=true;
            flag[t_friendly[i].derivantHp+t_friendly[i].hp]=true;
        }
        else
        {
            flag[t_friendly[i].derivantHp]=true;
        }
    }
    bool accepted=true;
    for(int i=1;i<flag.size();i++) if(!flag[i]) accepted=false;
    if(accepted)
    {
        int x=flag.size();//最终亵渎打了x点伤害
        for(int i=0;i<m;i++)
        {
            if(t_enemy[i].hp>0)
            {
                if(x>t_enemy[i].hp)
                {
                    t_enemy[i].derivantHp+=t_enemy[i].hp;
                    t_enemy[i].derivantHp-=x;
                    t_enemy[i].hp=0;
                }
                else
                {
                    t_enemy[i].hp-=x;
                }
            }
            else
            {
                t_enemy[i].derivantHp-=x;
            }
        }

        for(int i=0;i<m;i++)
        {
            if(t_friendly[i].hp>0)
            {
                if(x>t_friendly[i].hp)
                {
                    t_friendly[i].derivantHp+=t_friendly[i].hp;
                    t_friendly[i].derivantHp-=x;
                    t_friendly[i].hp=0;
                }
                else
                {
                    t_friendly[i].hp-=x;
                }
            }
            else
            {
                t_friendly[i].derivantHp-=x;
            }
        }

        if(enemy_atk_sum!=0)
        {
            int temp=0;
            for(int i=0;i<m;i++)
            {
                if(t_enemy[i].hp>0)
                {
                    temp+=(t_enemy[i].hp*t_enemy[i].attack);
                }
                else if(t_enemy[i].derivantHp>0)
                {
                    temp+=(t_enemy[i].derivantHp*t_enemy[i].derivantAtk);
                }
            }
            if(temp<enemy_atk_sum || enemy_atk_sum==-1)
            {
                final_target=target;
                final_order=order;
                final_enemy=t_enemy;
                final_friendly=t_friendly;
                enemy_atk_sum=temp;
            }
        }
        else
        {
            int temp=0;
            for(int i=0;i<m;i++)
            {
                if(t_friendly[i].hp>0)
                {
                    temp+=(t_friendly[i].hp*t_friendly[i].attack);
                }
                else if(t_friendly[i].derivantHp>0)
                {
                    temp+=(t_friendly[i].derivantHp*t_friendly[i].derivantAtk);
                }
            }
            if(temp>friendly_atk_sum)
            {
                final_target=target;
                final_order=order;
                final_enemy=t_enemy;
                final_friendly=t_friendly;
                friendly_atk_sum=temp;
            }
        }
        for(int i:final_order)
        {
            if(final_target[i]!=-1)
            {    
                cout<<"我方第"<<i+1<<"个随从("<<friendly[i].attack<<"/"<<friendly[i].hp<<") --> 敌方第"
                <<final_target[i]+1<<"个随从("<<enemy[final_target[i]].attack<<"/"<<enemy[final_target[i]].hp<<")"<<endl;
            }
        }
        cout<<endl;
        cout<<"释放亵渎后的场面:"<<endl<<"敌方:";
        for(int i=0;i<m;i++) cout<<"("<<final_enemy[i].attack<<"/"<<final_enemy[i].hp<<") ";
        cout<<"总分数:"<<enemy_atk_sum<<endl<<"我方:";
        for(int i=0;i<n;i++) cout<<"("<<final_friendly[i].attack<<"/"<<final_friendly[i].hp<<") ";
        cout<<"总分数:"<<friendly_atk_sum<<endl;
    }
}

void compute()
{
    do//枚举攻击顺序
    {
        vector<minion> t_enemy=enemy;
        vector<minion> t_friendly=friendly;
        for(int i=0;i<n;i++)
        {
            if((target[order[i]]!=-1)&&(t_enemy[target[order[i]]].hp>0))
            {
                atk(t_friendly[order[i]],t_enemy[target[order[i]]]);
            }
        }
        check(t_friendly,t_enemy);
    } while (next_permutation(order.begin(),order.end()));
}

void f(int i)//枚举第i个我随从的攻击目标
{
    if(i==n)
    {
        compute();
        return;
    }
    for(int k=-1;k<m;k++)
    {
        target[i]=k;
        f(i+1);
    }
}

int main()
{
    cout<<"敌方随从数量:";
    cin>>m;
    cout<<"我方随从数量:";
    cin>>n;
    cout<<endl;
    for(int i=0;i<n;i++) target.push_back(-1);
    for(int i=0;i<n;i++) order.push_back(i);
    for(int i=0;i<m;i++) enemy.push_back(minion());
    for(int i=0;i<n;i++) friendly.push_back(minion());

    for(int i=1;i<=m;i++)
    {
        cout<<"输入第"<<i<<"/"<<m<<"个敌方随从的攻击力:";
        cin>>enemy[i-1].attack;
        cout<<"输入第"<<i<<"/"<<m<<"个敌方随从的生命值:";
        cin>>enemy[i-1].hp;
        cout<<"输入第"<<i<<"/"<<m<<"个敌方随从的亡语衍生物攻击力(若无则填0):";
        cin>>enemy[i-1].derivantAtk;
        cout<<"输入第"<<i<<"/"<<m<<"个敌方随从的亡语衍生物生命值(若无则填0):";
        cin>>enemy[i-1].derivantHp;
        cout<<endl;
    }
    for(int i=1;i<=n;i++)
    {
        cout<<"输入第"<<i<<"/"<<n<<"个我方随从的攻击力:";
        cin>>friendly[i-1].attack;
        cout<<"输入第"<<i<<"/"<<n<<"个我方随从的生命值:";
        cin>>friendly[i-1].hp;
        cout<<"输入第"<<i<<"/"<<n<<"个我方随从的亡语衍生物攻击力(若无则填0):";
        cin>>friendly[i-1].derivantAtk;
        cout<<"输入第"<<i<<"/"<<n<<"个我方随从的亡语衍生物生命值(若无则填0):";
        cin>>friendly[i-1].derivantHp;
        cout<<endl;
    }
    f(0);


    
    for(int i:final_order)
    {
        if(final_target[i]!=-1)
        {    
            cout<<"我方第"<<i+1<<"个随从("<<friendly[i].attack<<"/"<<friendly[i].hp<<") --> 敌方第"
            <<final_target[i]+1<<"个随从("<<enemy[final_target[i]].attack<<"/"<<enemy[final_target[i]].hp<<")"<<endl;
        }
    }
    cout<<endl;
    cout<<"释放亵渎后的场面:"<<endl<<"敌方:";
    for(int i=0;i<m;i++) cout<<"("<<final_enemy[i].attack<<"/"<<final_enemy[i].hp<<") ";
    cout<<"总分数:"<<enemy_atk_sum<<endl<<"我方:";
    for(int i=0;i<n;i++) cout<<"("<<final_friendly[i].attack<<"/"<<final_friendly[i].hp<<") ";
    cout<<"总分数:"<<friendly_atk_sum<<endl;
    return 0;
}