#ifndef _heavykeeper_H
#define _heavykeeper_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BOBHash32.h"
#include "params.h"
#include "BOBHash64.h"
#define HK_d 8
#define HK_b 1.08

using namespace std;
class heavykeeper
{
public:
    heavykeeper_node HK[HK_d][MAX_MEM + 10];
    BOBHash64 *bobhash;
    unsigned long long time_stamp;
    int update_i;
    int M2;
    int cycle;
    int field_num;
    unsigned max_k;
    unsigned period;
    unsigned memory_size;
    unsigned hash_num;
    
    /*
    heavykeeper(int M2, int cycle, int field_num, unsigned max_k) : M2(M2), cycle(cycle), field_num(field_num), max_k(max_k)
    {
        time_stamp = 0;
        for (int hk_i = 0; hk_i < HK_d; hk_i++)
        {
            for (int hk_j = 0; hk_j < MAX_MEM + 10; hk_j++)
            {
                HK[hk_i][hk_j].C = new int[field_num];
                memset(HK[hk_i][hk_j].C, 0, field_num * sizeof(int));
            }
        }
    }
    */
    heavykeeper(unsigned period, unsigned memory_size, unsigned hash_num, int field_num, unsigned max_k) 
    : period(period), memory_size(memory_size), hash_num(hash_num), field_num(field_num), max_k(max_k)
    {
        time_stamp = 0;
        update_i = 0;
        int hk_M;
        int single_size = 8 + (4 * field_num);
        M2 = memory_size * 1024 / (single_size * hash_num);
        for (int hk_i = 0; hk_i < hash_num; hk_i++)
        {
            for (int hk_j = 0; hk_j < M2 + 10; hk_j++)
            {
                HK[hk_i][hk_j].C = new int[field_num];
                memset(HK[hk_i][hk_j].C, 0, field_num * sizeof(int));
                memset(HK[hk_i][hk_j].FP, 0, sizeof(HK[hk_i][hk_j].FP));
            }
        }
    }
    ~heavykeeper()
    {
        for (int hk_i = 0; hk_i < hash_num; hk_i++) {
            for (int hk_j = 0; hk_j < MAX_MEM + 10; hk_j++) {
                delete HK[hk_i][hk_j].C;
            }
        }
    }
    
    // hash function
    unsigned long long Hash(int j, char *mem_s, unsigned size)
    {
        bobhash = new BOBHash64(j + 10);
        unsigned long long ans = (bobhash->run(mem_s, size));
        delete bobhash;

        return ans;
    }

    // update an item
    void Insert(char *mem_s, unsigned size)
    {
        char item[MAX_DATA_SIZE];
        memcpy(item, mem_s, size);
        time_stamp += 1;

        // 是否需要窗口滑动
        if (time_stamp % (period) == 0)
        {
            int time_i = 0;
            int hki = (update_i++) % hash_num;
            //cout << "Debug: zyq enter time_stamp: " << time_stamp << endl;
            for (int time_i = 0; time_i < M2; time_i++) {
                for (int nk = field_num - 1; nk > 0; nk--)
                {
                    HK[hki][time_i].C[nk] = HK[hki][time_i].C[nk - 1];
                }
                HK[hki][time_i].C[0] = 0;
            }
        }

        // calculate finger print
        for (int j = 0; j < hash_num; j++)
        {
            // calculate position
            unsigned long long H = Hash(j, item, size);
            int Hsh = H % (M2 - (2 * hash_num) + 2 * j + 3); //很神奇，这个准确率高
            //int Hsh = H % (M2 - 1);

            if (!memcmp(item, HK[j][Hsh].FP, size))
            {
                // If the finger print indicates that it is the same element
                HK[j][Hsh].C[0]++;
            }
            else {
                // Not the same element
                int prob = 0;
                for (int prob_i = 0; prob_i < field_num; prob_i++)
                {
                    prob = prob + HK[j][Hsh].C[prob_i];
                }
                // probability decay
                if (!(rand() % int(pow(HK_b, prob))))
                {
                    bool flag = false;
                    for (int decay_i = 0; decay_i < field_num; decay_i++)
                    {
                        if (HK[j][Hsh].C[decay_i] != 0)
                        {
                            HK[j][Hsh].C[decay_i]--;
                            flag = true;
                            break;
                        }
                    }
                    if (flag == false)
                    {
                        memcpy(HK[j][Hsh].FP, item, size);
                        HK[j][Hsh].C[0] = 1;
                    }
                }
            }
        }
    }
    // query an item
    bool num_query(const char *mem_s, unsigned size)
    {
        char item[MAX_DATA_SIZE];
        memcpy(item, mem_s, size);
        int maxv = 0;
        for (int j = 0; j < hash_num; j++)
        {
            unsigned long long H = Hash(j, item, size);
            int Hsh = H % (M2 - (2 * hash_num) + 2 * j + 3);
            if (!memcmp(HK[j][Hsh].FP, item, size))
            {
                int prob = 0;
                for (int prob_i = 0; prob_i < field_num; prob_i++)
                {
                    prob = prob + HK[j][Hsh].C[prob_i];
                }
                maxv = max(maxv, prob);
            }
        }
        // 和的最大值
        return maxv > max_k;
    }
};
#endif
