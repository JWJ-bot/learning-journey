/*
 * 实验三：图的遍历及应用-(二)家庭房产统计问题
 * 说明：
 * 1. 人员编号范围为 0～9999；不存在的父母编号用 -1 表示。
 * 2. 每个人的房产套数和房产总面积记录在其个人信息中。
 * 3. 父母、子女关系均按无向边处理；每个连通分量代表一个家庭。
 * 4. 输出按家庭最小成员编号升序排列。
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_ID 10000
#define MAX_CHILDREN 10

/* ======================== 数据结构定义 ======================== */

/* 邻接表边结点 */
typedef struct AdjNode {
    int vertex;
    struct AdjNode *next;
} AdjNode;

/* 图结构：邻接表 + 顶点及房产信息 */
typedef struct {
    int vertexCount;           /* 当前出现过的不同人员数量 */
    AdjNode **heads;           /* 邻接表头指针数组 */
    int *present;              /* present[id]：该编号是否出现在数据中 */
    long long *estateCount;    /* 该人员名下房产套数 */
    long long *totalArea;      /* 该人员名下房产总面积 */
} Graph;

/* 输入的一条人员记录 */
typedef struct {
    int id;
    int father;
    int mother;
    int childCount;
    int children[MAX_CHILDREN];
    long long estateCount;
    long long totalArea;
} PersonRecord;

/* 自定义顺序栈：DFS 使用，不使用系统栈容器 */
typedef struct {
    int *data;
    int top;
    int capacity;
} Stack;

/* 一个家庭的统计结果 */
typedef struct {
    int familyID;              /* 家庭中最小的人员编号 */
    int memberCount;           /* 家庭人口数 */
    long long estateCount;     /* 家庭房产套数总和 */
    long long totalArea;       /* 家庭房产面积总和 */
} Family;

/* ======================== 栈的基本操作 ======================== */

int initStack(Stack *s, int capacity)
{
    s->data = (int *)malloc((size_t)capacity * sizeof(int));
    if (s->data == NULL) {
        return 0;
    }

    s->top = -1;
    s->capacity = capacity;
    return 1;
}

int isStackEmpty(const Stack *s)
{
    return s->top == -1;
}

int push(Stack *s, int value)
{
    if (s->top + 1 >= s->capacity) {
        return 0;
    }

    s->data[++s->top] = value;
    return 1;
}

int pop(Stack *s, int *value)
{
    if (isStackEmpty(s)) {
        return 0;
    }

    *value = s->data[s->top--];
    return 1;
}

void destroyStack(Stack *s)
{
    free(s->data);
    s->data = NULL;
    s->top = -1;
    s->capacity = 0;
}

/* ======================== 图的基本操作 ======================== */

int initGraph(Graph *g)
{
    g->vertexCount = 0;
    g->heads = (AdjNode **)calloc(MAX_ID, sizeof(AdjNode *));
    g->present = (int *)calloc(MAX_ID, sizeof(int));
    g->estateCount = (long long *)calloc(MAX_ID, sizeof(long long));
    g->totalArea = (long long *)calloc(MAX_ID, sizeof(long long));

    if (g->heads == NULL || g->present == NULL ||
        g->estateCount == NULL || g->totalArea == NULL) {
        return 0;
    }

    return 1;
}

/* 判断顶点编号是否合法 */
int isValidID(int id)
{
    return id >= 0 && id < MAX_ID;
}

/* 标记一个人员顶点存在 */
void addVertex(Graph *g, int id)
{
    if (!isValidID(id)) {
        return;
    }

    if (!g->present[id]) {
        g->present[id] = 1;
        g->vertexCount++;
    }
}

/* 判断邻接表中是否已经存在边 u-v */
int hasEdge(const Graph *g, int u, int v)
{
    AdjNode *p = g->heads[u];

    while (p != NULL) {
        if (p->vertex == v) {
            return 1;
        }
        p = p->next;
    }

    return 0;
}

/* 添加一条无向边；重复边和自环不会造成重复存储 */
int addUndirectedEdge(Graph *g, int u, int v)
{
    AdjNode *node;

    if (!isValidID(u) || !isValidID(v)) {
        return 0;
    }

    addVertex(g, u);
    addVertex(g, v);

    if (u == v || hasEdge(g, u, v)) {
        return 1;
    }

    node = (AdjNode *)malloc(sizeof(AdjNode));
    if (node == NULL) {
        return 0;
    }
    node->vertex = v;
    node->next = g->heads[u];
    g->heads[u] = node;

    node = (AdjNode *)malloc(sizeof(AdjNode));
    if (node == NULL) {
        return 0;
    }
    node->vertex = u;
    node->next = g->heads[v];
    g->heads[v] = node;

    return 1;
}

/* 释放图中所有动态申请的空间 */
void destroyGraph(Graph *g)
{
    int i;

    if (g->heads != NULL) {
        for (i = 0; i < MAX_ID; ++i) {
            AdjNode *p = g->heads[i];

            while (p != NULL) {
                AdjNode *temp = p;
                p = p->next;
                free(temp);
            }
        }
    }

    free(g->heads);
    free(g->present);
    free(g->estateCount);
    free(g->totalArea);

    g->heads = NULL;
    g->present = NULL;
    g->estateCount = NULL;
    g->totalArea = NULL;
    g->vertexCount = 0;
}

/* ======================== 建图函数======================== */

/*
 * 根据人员记录建立家庭关系无向图。
 * 每个人、父母和子女均作为顶点；家庭成员关系作为无向边。
 */
int buildGraph(Graph *g, const PersonRecord records[], int n)
{
    int i, j;

    /* 第一遍：登记所有有个人记录的成员及其房产信息 */
    for (i = 0; i < n; ++i) {
        int id = records[i].id;

        if (!isValidID(id)) {
            printf("错误：人员编号 %d 超出范围 0～9999。\n", id);
            return 0;
        }

        addVertex(g, id);
        g->estateCount[id] = records[i].estateCount;
        g->totalArea[id] = records[i].totalArea;
    }

    /* 第二遍：根据父母、子女关系建立无向边 */
    for (i = 0; i < n; ++i) {
        int id = records[i].id;
        int father = records[i].father;
        int mother = records[i].mother;

        if (father != -1) {
            if (!isValidID(father) || !addUndirectedEdge(g, id, father)) {
                printf("建立父子关系边失败。\n");
                return 0;
            }
        }

        if (mother != -1) {
            if (!isValidID(mother) || !addUndirectedEdge(g, id, mother)) {
                printf("建立母子关系边失败。\n");
                return 0;
            }
        }

        for (j = 0; j < records[i].childCount; ++j) {
            int child = records[i].children[j];

            if (!isValidID(child) || !addUndirectedEdge(g, id, child)) {
                printf("建立亲子关系边失败。\n");
                return 0;
            }
        }
    }

    return 1;
}

/* ======================== DFS 遍历函数 ======================== */

/*
 * 从 start 出发，使用自定义栈遍历一个连通分量。
 * 将该家庭成员的数量、房产套数、房产面积累加到 family 中。
 * visited 在入栈时标记，避免同一顶点重复入栈和重复统计。
 */
void dfsFamily(const Graph *g, int start, int visited[], Family *family)
{
    Stack s;
    int current;

    if (!initStack(&s, g->vertexCount)) {
        printf("错误：DFS 栈内存分配失败。\n");
        return;
    }

    visited[start] = 1;
    push(&s, start);

    while (pop(&s, &current)) {
        AdjNode *p;

        family->memberCount++;
        if (current < family->familyID) {
            family->familyID = current;
        }

        family->estateCount += g->estateCount[current];
        family->totalArea += g->totalArea[current];

        for (p = g->heads[current]; p != NULL; p = p->next) {
            int next = p->vertex;

            if (!visited[next]) {
                visited[next] = 1;
                push(&s, next);
            }
        }
    }

    destroyStack(&s);
}

/* ======================== 家庭统计函数 ======================== */

/*
 * 扫描所有出现过的人员顶点。
 * 每遇到一个未访问顶点，就调用 DFS 得到一个完整家庭。
 * 返回家庭总数。
 */
int collectFamilies(const Graph *g, Family families[])
{
    int *visited;
    int id;
    int familyCount = 0;

    visited = (int *)calloc(MAX_ID, sizeof(int));
    if (visited == NULL) {
        printf("错误：visited 数组内存分配失败。\n");
        return -1;
    }

    for (id = 0; id < MAX_ID; ++id) {
        if (g->present[id] && !visited[id]) {
            Family family;

            family.familyID = id;
            family.memberCount = 0;
            family.estateCount = 0;
            family.totalArea = 0;

            dfsFamily(g, id, visited, &family);
            families[familyCount++] = family;
        }
    }

    free(visited);
    return familyCount;
}

/* 家庭按最小成员编号升序排列 */
int compareFamily(const void *a, const void *b)
{
    const Family *fa = (const Family *)a;
    const Family *fb = (const Family *)b;

    if (fa->familyID < fb->familyID) return -1;
    if (fa->familyID > fb->familyID) return 1;
    return 0;
}

/* ======================== 房产信息输出函数 ======================== */

void sortFamilies(Family families[], int familyCount)
{
    qsort(families, (size_t)familyCount, sizeof(Family), compareFamily);
}

/*
 * 按 PTA 常见格式输出：
 * 家庭编号 人口数 人均房产套数 人均房产面积
 * 两项平均值均保留三位小数。
 */
void printFamilyStatistics(Family families[], int familyCount)
{
    int i;

    sortFamilies(families, familyCount);

    printf("\n========== 家庭房产统计结果 ==========\n");
    printf("家庭总数：%d\n\n", familyCount);
    printf("%-12s %-12s %-18s %-18s\n",
           "家庭编号", "家庭人口", "人均房产套数", "人均房产面积");
    printf("--------------------------------------------------------------\n");

    for (i = 0; i < familyCount; ++i) {
        double averageEstate =
            (double)families[i].estateCount / families[i].memberCount;
        double averageArea =
            (double)families[i].totalArea / families[i].memberCount;

        printf("%-12.4d %-12d %-18.3f %-18.3f\n",
               families[i].familyID,
               families[i].memberCount,
               averageEstate,
               averageArea);
    }

    printf("--------------------------------------------------------------\n");
}

/* ======================== 主函数 ======================== */

int main(void)
{
    int n, i;
    PersonRecord *records;
    Family *families;
    Graph graph;
    int familyCount;

    printf("========== 家庭房产统计系统 ==========\n");
    printf("本程序使用邻接表存储家庭关系，使用自定义栈实现非递归 DFS。\n");
    printf("每个连通分量代表一个家庭。\n");
    printf("说明：父亲编号或母亲编号为 -1，表示该成员信息未知。\n\n");

    printf("请输入家庭成员记录人数：");
    if (scanf("%d", &n) != 1 || n < 0 || n > MAX_ID) {
        printf("输入错误：记录人数 N 应在 0～%d 之间。\n", MAX_ID);
        return 1;
    }

    records = (PersonRecord *)calloc((size_t)n, sizeof(PersonRecord));
    families = (Family *)calloc(MAX_ID, sizeof(Family));

    if ((n > 0 && records == NULL) || families == NULL) {
        printf("内存分配失败。\n");
        free(records);
        free(families);
        return 1;
    }

    printf("\n请依次输入每位成员的信息。\n");
    printf("每条记录格式：成员编号 父亲编号 母亲编号 子女数量 [子女编号...] 房产套数 房产总面积\n");
    printf("例如：1001 1000 -1 2 1002 1003 2 300\n\n");

    /* 读取每个人的家庭成员关系和房产信息 */
    for (i = 0; i < n; ++i) {
        int j;

        printf("请输入第 %d 位成员的信息（编号 父亲 母亲 子女数 子女编号... 房产套数 总面积）：\\n", i + 1);

        if (scanf("%d%d%d%d",
                  &records[i].id,
                  &records[i].father,
                  &records[i].mother,
                  &records[i].childCount) != 4) {
            printf("第 %d 条人员记录输入错误。\n", i + 1);
            free(records);
            free(families);
            return 1;
        }

        if (records[i].childCount < 0 ||
            records[i].childCount > MAX_CHILDREN) {
            printf("第 %d 条记录的子女数量超出范围（0～%d）。\n",
                   i + 1, MAX_CHILDREN);
            free(records);
            free(families);
            return 1;
        }

        for (j = 0; j < records[i].childCount; ++j) {
            if (scanf("%d", &records[i].children[j]) != 1) {
                printf("子女编号输入错误。\n");
                free(records);
                free(families);
                return 1;
            }
        }

        if (scanf("%lld%lld",
                  &records[i].estateCount,
                  &records[i].totalArea) != 2) {
            printf("房产信息输入错误。\n");
            free(records);
            free(families);
            return 1;
        }

        if (records[i].estateCount < 0 || records[i].totalArea < 0) {
            printf("房产套数和房产面积不能为负数。\n");
            free(records);
            free(families);
            return 1;
        }
    }

    graph.vertexCount = 0;
    graph.heads = NULL;
    graph.present = NULL;
    graph.estateCount = NULL;
    graph.totalArea = NULL;

    printf("\n正在初始化图结构并建立家庭关系……\n");
    if (!initGraph(&graph)) {
        printf("图初始化失败。\n");
        free(records);
        free(families);
        destroyGraph(&graph);
        return 1;
    }

    /* 模块化调用：建图 -> DFS 连通分量统计 -> 房产信息输出 */
    if (!buildGraph(&graph, records, n)) {
        destroyGraph(&graph);
        free(records);
        free(families);
        return 1;
    }

    printf("家庭关系图建立完成，共识别到 %d 个不同成员。\n",
           graph.vertexCount);
    printf("正在使用 DFS 统计各个家庭……\n");

    familyCount = collectFamilies(&graph, families);
    if (familyCount < 0) {
        destroyGraph(&graph);
        free(records);
        free(families);
        return 1;
    }

    printFamilyStatistics(families, familyCount);

    printf("\n统计完成，程序结束。\n");

    destroyGraph(&graph);
    free(records);
    free(families);

    return 0;
}
