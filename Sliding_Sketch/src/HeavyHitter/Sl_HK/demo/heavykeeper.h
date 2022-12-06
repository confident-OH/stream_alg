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
    int M2;
    int cycle;
    int field_num;
    unsigned max_k;
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
    void clear()
    {
        for (int i = 0; i < HK_d; i++)
            for (int j = 0; j <= M2 + 5; j++) {
                for (int k = 0; k < field_num; k++) {
                    HK[i][j].C[k] = 0;
                }
                memset(HK[i][j].FP, 0, sizeof(HK[i][j].FP));
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
        if (time_stamp % (M2 * HK_d) == 0)
        {
            int time_i = 0;
            int hki = (time_stamp / M2 - 1) % HK_d;
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
        for (int j = 0; j < HK_d; j++)
        {
            // calculate position
            unsigned long long H = Hash(j, item, size);
            int Hsh = H % (M2 - (2 * HK_d) + 2 * j + 3);

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
        for (int j = 0; j < HK_d; j++)
        {
            unsigned long long H = Hash(j, item, size);
            int Hsh = H % (M2 - (2 * HK_d) + 2 * j + 3);
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
