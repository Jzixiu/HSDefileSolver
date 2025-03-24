#include <iostream>
#include <random>
#include <fstream>
#include <chrono>
using namespace std;

int main()
{
    ofstream runlog("log",ios::out | ios::trunc);

    for(int x=0;x<100;x++)
    {
        random_device rd;
        unsigned int seed=rd();
        // unsigned int seed=2421727352;
        mt19937 eng(seed);
        uniform_int_distribution<int> getrd(1, 10);
        // uniform_int_distribution<int> getrd_2(5, 10);

        // runlog<<"Seed:"<<seed<<endl;;

        int fast_diff_count=0;
        int fast_slower_count=0;

        int optimal_fast_diff_count=0;
        int optimal_fast_slower_count=0;

        int count=100;

        chrono::nanoseconds Total_guess_time;
        chrono::nanoseconds Total_fast_guess_time;
        chrono::nanoseconds Total_optimal_fast_guess_time;

        for(int i=0;i<count;i++)
        {
            fstream input("input",ios::in | ios::out | ios::trunc);
            int m=7;
            int n=7;
            input<<m<<endl;
            input<<n<<endl;
            for(int i=0;i<m;i++)//敌方随从的
            {
                input<<getrd(eng)<<endl;//攻击力
                input<<getrd(eng)<<endl;//生命值
            }
            for(int i=0;i<n;i++)//友方随从的
            {
                input<<getrd(eng)<<endl;//攻击力
                input<<getrd(eng)<<endl;//生命值
            }

            auto start = chrono::high_resolution_clock::now(); //开始时间
            system("./build/guess");
            auto end = chrono::high_resolution_clock::now(); //结束时间
            auto guess_time=end-start;
            Total_guess_time+=guess_time;

            start = chrono::high_resolution_clock::now();
            system("./build/fast_guess");
            end = chrono::high_resolution_clock::now();
            auto fast_guess_time=end-start;
            Total_fast_guess_time+=fast_guess_time;

            start = chrono::high_resolution_clock::now();
            system("./build/optimal_fast_guess");
            end = chrono::high_resolution_clock::now();
            auto optimal_fast_guess_time=end-start;
            Total_optimal_fast_guess_time+=optimal_fast_guess_time;

            if(system("diff res_fast_guess res_guess"))
            {
                fast_diff_count++;
                // runlog<<endl;
                // runlog<<"Different answer on input:"<<endl;
                // string line;
                // input.seekg(0);
                // while (getline(input, line)) {
                // runlog << line << endl;
                // }
            }
            if(guess_time<fast_guess_time)
            {
                fast_slower_count++;
                // runlog<<endl;
                // runlog<<"Guess faster on this input, with "<<guess_time.count()<<" and "<<fast_time.count()<<endl;
                // string line;
                // input.seekg(0);
                // while (getline(input, line)) {
                // runlog << line << endl;
                // }
            }

            if(system("diff res_optimal_fast_guess res_guess"))
            {
                optimal_fast_diff_count++;
            }
            if(guess_time<optimal_fast_guess_time)
            {
                optimal_fast_slower_count++;
            }
            
            input.close();
        }
        
        // runlog<<"Results for fast_guess:"<<endl;
        // runlog<<"Different results: "<<fast_diff_count<<endl;
        // runlog<<"Slower Runtime: "<<fast_slower_count<<endl;

        // runlog<<"Results for optimal_fast_guess:"<<endl;
        // runlog<<"Different results: "<<optimal_fast_diff_count<<endl;
        // runlog<<"Slower Runtime: "<<optimal_fast_slower_count<<endl;

        runlog<<fast_diff_count<<" "<<fast_slower_count<<" "<<optimal_fast_diff_count<<" "<<optimal_fast_slower_count<<" ";
        runlog<<(static_cast<double>(Total_guess_time.count()) / Total_fast_guess_time.count())<<" "
                <<(static_cast<double>(Total_guess_time.count()) / Total_optimal_fast_guess_time.count())<<endl;


        // runlog<<endl;

        cout<<"Done "<<x<<"."<<endl;
    }

    runlog.close();
    return 0;
}
