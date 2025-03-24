#include <iostream>
#include <vector>
using namespace std;

const int atk[10]={1,2,3,4,5,6,7,8,9,0};

//实际情况下返回根据lock/can_attack筛选的友方随从
vector<int> get_avaliable_ally()
{
    vector<int> temp;
    temp.push_back(0);
    temp.push_back(2);
    temp.push_back(3);
    temp.push_back(5);
    temp.push_back(7);
    temp.push_back(8);
    return temp;
}

//k:当前需要凑到的攻击力
//id:可用友方随从id列表
vector<vector<int>> get_combo(int k,const vector<int> &id)
{
    vector<vector<vector<int>>> dp(k+1);
    dp[0].push_back({});

    for(int ally_id:id)
    {
        int ally_atk=atk[ally_id];
        for(int i=k-ally_atk;i>=0;i--)
        {
            for(const vector<int> comb:dp[i])
            {
                vector<int> new_comb=comb;
                new_comb.push_back(ally_id);
                dp[i+ally_atk].push_back(new_comb);
            }

        }
    }

    return dp[k];
}

int main()
{
    vector<vector<int>> res=get_combo(15,get_avaliable_ally());
    for(vector<int> v:res)
    {
        for(int id:v)
        {
            cout<<id<<" ";
        }
        cout<<endl;
    }
    return 0;
}