#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

vector<int> enemy_attack;
vector<int> enemy_hp;
vector<int> friendly_attack;
vector<int> friendly_hp;

int m;//敌方随从数量
int n;//我方随从数量

vector<int> target;//每个我方随从的目标
vector<int> order;//我方随从攻击的顺序



void compute()
{
    do
    {
        vector<int> t_enemy_hp=enemy_hp;
        vector<int> t_friendly_hp=friendly_hp;
        for(int i=0;i<n;i++)
        {
            if((target[order[i]]!=-1)&&(t_enemy_hp[target[order[i]]]>0))
            {
                t_enemy_hp[target[order[i]]]-=friendly_attack[order[i]];
                t_friendly_hp[order[i]]-=enemy_attack[target[order[i]]];
            }
        }
        cout<<endl;
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
    
    for(int i=0;i<n;i++) target.push_back(-1);
    for(int i=0;i<n;i++) order.push_back(i);

    cout<<endl;
    for(int i=1;i<=m;i++)
    {
        int x;
        cout<<"输入第"<<i<<"/"<<m<<"个敌方随从的攻击力";
        cin>>x;
        enemy_attack.push_back(x);
        cout<<"输入第"<<i<<"/"<<m<<"个敌方随从的生命值";
        cin>>x;
        enemy_hp.push_back(x);
        cout<<endl;
    }
    for(int i=1;i<=n;i++)
    {
        int x;
        cout<<"输入第"<<i<<"/"<<n<<"个我方随从的攻击力";
        cin>>x;
        friendly_attack.push_back(x);
        cout<<"输入第"<<i<<"/"<<n<<"个我方随从的生命值";
        cin>>x;
        friendly_hp.push_back(x);
        cout<<endl;
    }
    f(0);
    return 0;
}