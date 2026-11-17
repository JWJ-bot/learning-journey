/*
 * 实验内容二：哈夫曼编码判断
 * 功能：
 *   1. 读入 n 个字符、对应权值以及给定的 01 编码；
 *   2. 判断这些编码是否满足前缀编码；
 *   3. 用静态链表构造哈夫曼树，计算理论最优加权路径长度；
 *   4. 计算给定编码的加权路径长度，并与最优值比较；
 *   5. 输出该套编码是否为哈夫曼编码。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 1000
#define MAXNODE (2 * MAXN)
#define MAXL 128

/* ==================== 1. 静态链表结点 ==================== */
typedef struct {
    long long w;
    int p, l, r;
} HTNode;

typedef HTNode HT[MAXNODE];

/* 一套待判断的编码 */
typedef struct {
    char ch[8];
    long long w;
    char code[MAXL];
} Code;

/* ==================== 2. 初始化 ==================== */
void InitHuffmanTree(HT t, int n, Code c[])
{
    int i, m = 2 * n - 1;
    for (i = 0; i <= m; i++) {
        t[i].w = 0;
        t[i].p = t[i].l = t[i].r = 0;
    }
    for (i = 1; i <= n; i++)
        t[i].w = c[i - 1].w;
}

/* ==================== 3. 选择两个最小根 ==================== */
void SelectTwoMin(HT t, int m, int *a, int *b)
{
    int i;
    *a = *b = 0;
    for (i = 1; i <= m; i++) {
        if (t[i].p) continue;              /* 已有双亲，不是根 */
        if (*a == 0 || t[i].w < t[*a].w) {
            *b = *a;
            *a = i;
        } else if (*b == 0 || t[i].w < t[*b].w) {
            *b = i;
        }
    }
}

/* ==================== 4. 构造哈夫曼树 ==================== */
void CreateHuffmanTree(HT t, int n)
{
    int i, m = 2 * n - 1;
    for (i = n + 1; i <= m; i++) {
        int a, b;
        SelectTwoMin(t, i - 1, &a, &b);
        t[i].w = t[a].w + t[b].w;         /* 合并两个最小根 */
        t[i].l = a;
        t[i].r = b;
        t[a].p = t[b].p = i;
    }
}

/* ==================== 5. 计算最优 WPL ==================== */
long long ComputeHuffmanWPL(HT t, int n)
{
    long long s = 0;
    int i;
    for (i = 1; i <= n; i++) {
        int d = 0, p = i;
        while (t[p].p) {
            d++;
            p = t[p].p;
        }
        s += t[i].w * d;                  /* 权值 × 深度 */
    }
    return s;
}

/* ==================== 6. 计算给定编码的 WPL ==================== */
long long ComputeGivenWPL(Code c[], int n)
{
    long long s = 0;
    int i;
    for (i = 0; i < n; i++)
        s += c[i].w * (long long)strlen(c[i].code);
    return s;
}

static int Cmp(const void *a, const void *b)
{
    return strcmp(((const Code *)a)->code, ((const Code *)b)->code);
}

/* ==================== 7. 前缀编码判断 ==================== */
int CheckPrefixCodes(Code c[], int n)
{
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; c[i].code[j]; j++) {
            if (c[i].code[j] != '0' && c[i].code[j] != '1')
                return 0;
        }
    }

    qsort(c, n, sizeof(Code), Cmp);     /* 排序后只需比较相邻编码 */
    for (i = 0; i < n - 1; i++) {
        size_t len = strlen(c[i].code);
        if (strncmp(c[i].code, c[i + 1].code, len) == 0)
            return 0;
    }
    return 1;
}

/* ==================== 8. 综合判定 ==================== */
int CheckHuffmanCode(Code c[], int n, long long *g, long long *best, int *why)
{
    HT t;

    *g = ComputeGivenWPL(c, n);
    InitHuffmanTree(t, n, c);
    CreateHuffmanTree(t, n);
    *best = ComputeHuffmanWPL(t, n);

    if (n == 1)
        *best = c[0].w;             /* 单字符编码约定长度为 1 */

    if (!CheckPrefixCodes(c, n)) {
        *why = 1;
        return 0;
    }
    if (n == 1 && strlen(c[0].code) != 1) {
        *why = 2;
        return 0;
    }
    if (*g != *best) {
        *why = 3;
        return 0;
    }
    return 1;
}

/* ==================== 9. 主函数 ==================== */
int main(void)
{
    Code c[MAXN];
    int n, i, ok, why = 0;
    long long g, best;

    if (scanf("%d", &n) != 1 || n <= 0 || n > MAXN) {
        printf("输入格式错误。\n");
        return 1;
    }

    for (i = 0; i < n; i++) {
        if (scanf("%7s %lld %127s", c[i].ch, &c[i].w, c[i].code) != 3) {
            printf("输入格式错误。\n");
            return 1;
        }
        if (strlen(c[i].code) >= MAXL - 1) {
            printf("编码过长。\n");
            return 1;
        }
    }

    ok = CheckHuffmanCode(c, n, &g, &best, &why);
    printf("%s\n", ok ? "YES" : "NO");

    if (!ok) {
        if (why == 1)
            printf("原因：不满足前缀编码\n");
        else if (why == 2)
            printf("原因：单字符编码长度应为 1\n");
        else
            printf("原因：给定编码的 WPL 为 %lld，理论最优 WPL 为 %lld，"
                   "未达到最短\n", g, best);
    }

    printf("给定编码 WPL = %lld\n", g);
    printf("理论最优 WPL = %lld\n", best);
    return 0;
}
