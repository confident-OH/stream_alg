#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <map>
#include <math.h>
#include <fstream>
#include <iomanip>
#include "BOBHash32.h"
#include "params.h"
#include "heavykeeper.h"

using namespace std;
map <string ,int> B;
map <string, int> map_hk;
struct node {string x;int y;} p[10000005];

void Read(int read_length, ifstream& fin, char *data)
{
    fin.read(data, read_length);
    data[read_length] = '\0';
}
int cmp(node i,node j) {return i.y>j.y;}

//argv[1]:period,  update/instert 时间窗口大小
//argv[2]:memory_size 总体使用的内存大小(单位KB)
//argv[3]:hash_num  所使用的哈希个数
//argv[4]:field_num  新旧桶的个数
//argv[5]:K  阈值

int main(int argc, char* argv[])
{
    int K = 100;
    int field_num = 2;
    unsigned period = 1000000;
    unsigned memory_size = 2000;
    unsigned hash_num = 8;

    if (argc < 6) {
        printf("Sketch will use default config\n");
    }
    else {
        period = atoi(argv[1]);
        memory_size = atoi(argv[2]);
        hash_num = atoi(argv[3]);
        field_num = atoi(argv[4]);
        K = atoi(argv[5]);
        printf("period: %u, memory_size: %u, hash_num: %u, field_num: %u, K: %u\n", 
                period, memory_size, hash_num, field_num, K);
    }
    
    // preparing heavykeeper
    heavykeeper *hk; 
    hk = new heavykeeper(period, memory_size, hash_num, field_num, K); // 构造函数用来init

    // 测试数据初始化
    ifstream fin("../../../../data/formatted00.dat",ios::in|ios::binary);
    int single_size = 8 + (4 * field_num);
    int hk_M = memory_size * 1024 / (single_size * hash_num);
    double average_cr = 0;
    double average_rr = 0;
    int average_heavy = 0;
    int out_num = 0;
    double are = 0;
    int read_length = 4;
    int m = 4000000;

    // Inserting
    for (int i=1; i<=m; i++)
	{
        char s[MAX_DATA_SIZE];
		Read(read_length, fin, s);
		B[string(s)]++;
		hk->Insert(s, read_length);  // Insert使用范例

        if(i%period == 0){
            out_num ++;
            // preparing true flow
            double recall = 0;
            double recall_ = 0;
            double real = 0;
            double real_ = 0;
            for(int qi = 0; qi < hash_num; qi++){
                for(int qj = 0; qj < hk_M; qj++){
                    if(map_hk.find(hk->HK[qi][qj].FP) == map_hk.end()){
                        // 标记已查询
                        map_hk[hk->HK[qi][qj].FP] = 1;
                        if(hk->num_query(hk->HK[qi][qj].FP, read_length)){ // query使用范例
                            for (int j = 0; j < read_length; j++) {
                                printf("%x ", (unsigned int)(unsigned char)hk->HK[qi][qj].FP[j]);
                            }
                            printf("\n");
                            real = real + 1;
                            if(B[string(hk->HK[qi][qj].FP)] > K){
                                real_ = real_ + 1;
                            }
                        }
                    }
                }
            }
            average_cr = average_cr + (real_/real);

            for(map <string ,int>::iterator sit = B.begin();sit != B.end(); sit++){
                if(sit->second > K){
                    recall = recall + 1;
                    if(hk->num_query((sit->first).c_str(), read_length)){
                        recall_ = recall_ + 1;
                    }
                }
            }

            //cout << "recall rate:" << recall_/recall << endl;
            average_rr = average_rr + (recall_/recall);
            average_heavy = average_heavy + recall;

            cout << "Sl-HK,"<<"Arrivals:"<<i<<" SL_recall: "<<recall_ <<" Real_recall: " <<recall<<",Recall Rate:"<<recall_/recall << endl;
            cout << "Sl-HK,"<<"Arrivals:"<<i<<" SL_real: "<<real_<<" Real_real: " << real << ",Precision Rate:"<<real_/real << endl;

            map_hk.clear();
        }
	}

    cout << "out_num: " << out_num << " average_heavy: " << average_heavy << " average_cr: " << average_cr << " average_rr: " << average_rr << endl;
    cout << "total average heavy hitter:" << average_heavy / out_num << endl;
    cout << "average precision rate:" << average_cr / out_num << endl;
    cout <<"average recall rate:" << average_rr / out_num << endl;
    
    delete hk;
    return 0;
}
