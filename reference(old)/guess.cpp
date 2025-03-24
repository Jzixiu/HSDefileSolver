#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
using namespace std;

#define DEBUG

int m;//敌方随从数量
int n;//友方随从数量

struct minion
{
    int atk;
    int hp;
};

struct step
{
    int ally_id;
    int enemy_id;
};

vector<minion> ally;
vector<minion> enemy;
int max_hp=0;

vector<step> steps;
vector<step> solution;
int sol_atk=-1;//当前解对应的对方场攻


//检查是否是一个解
void check()
{
    int damage;//亵渎造成的伤害
    int sum_atk=0;//总敌方场攻
    bool flag[max_hp+1];
    memset(flag,0,sizeof(flag));

    for(int i=0;i<m;i++)
    {
        flag[enemy[i].hp]=true;
    }
    for(int i=0;i<n;i++)
    {
        flag[ally[i].hp]=true;
    }

    for(int i=1;i<=max_hp;i++)
    {
        if(!flag[i])
        {
            damage=i;
            break;
        }
    }

    for(int i=0;i<m;i++)
    {
        if(enemy[i].hp>damage)
        {
            sum_atk+=enemy[i].atk;
        }
    }

    if(sum_atk < sol_atk || sol_atk == -1)//更优解
    {
        sol_atk=sum_atk;
        solution=steps;
    }
}

void atk(int ally_id,int enemy_id)
{
    steps.push_back(step());
    steps.back().ally_id=ally_id;
    steps.back().enemy_id=enemy_id;
    enemy[enemy_id].hp-=ally[ally_id].atk;
    ally[ally_id].hp-=enemy[enemy_id].atk;
}

void undo_atk()
{
    step s=steps.back();
    steps.pop_back();
    enemy[s.enemy_id].hp+=ally[s.ally_id].atk;
    ally[s.ally_id].hp+=enemy[s.enemy_id].atk;
}

//为第k个友方随从挑选目标
void f(int ally_id)
{
    if(ally_id==n)
    {
        check();
        return;
    }

    for(int enemy_id=0;enemy_id<=m;enemy_id++)
    {
        if(enemy_id<m && enemy[enemy_id].hp<=0) continue;
        if(enemy_id!=m) atk(ally_id,enemy_id);//如果i==m代表不攻击
        f(ally_id+1);
        if(enemy_id!=m) undo_atk();
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
        if(enemy.back().hp>max_hp) max_hp=enemy.back().hp;
    }

    for(int i=0;i<n;i++)
    {
        ally.push_back(minion());
        cout<<"输入第["<<i+1<<"]个友方随从的信息:"<<endl;
        cout<<"atk:";
        cin>>ally.back().atk;
        cout<<"hp:";
        cin>>ally.back().hp;
        if(ally.back().hp>max_hp) max_hp=ally.back().hp;
    }
}

int main()
{
    #ifdef DEBUG
    ifstream infile("/Users/venoflame/desktop/code/HSDefile/input");
    ofstream outfile("/Users/venoflame/desktop/code/HSDefile/res_guess");
    std::ofstream null_stream;
    streambuf* cinbuf = cin.rdbuf();
    streambuf* coutbuf = cout.rdbuf();
    cin.rdbuf(infile.rdbuf());
    cout.rdbuf(null_stream.rdbuf());
    #endif

    init();

    f(0);

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