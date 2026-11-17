/*
 * 实验三：图的遍历及应用—(一)六度空间问题
 * 必做：邻接表、无向图、建图、自定义队列、BFS六度空间统计
 * 选作1：自定义栈实现非递归DFS
 * 选作2：DFS/BFS生成森林及树形打印
 * 选作3：输出指定顶点到其他顶点的所有简单路径及长度
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================== 图的邻接表结构 ======================== */

typedef struct AdjNode {
    int vertex;
    struct AdjNode *next;
} AdjNode;

typedef struct {
    int vertexCount;
    AdjNode **heads;             /* heads[i] 为顶点 i 的边链表头指针 */
} Graph;

static int hasEdge(const Graph *g, int u, int v)
{
    AdjNode *p = g->heads[u];
    while (p != NULL) {
        if (p->vertex == v) return 1;
        p = p->next;
    }
    return 0;
}

static int initGraph(Graph *g, int n)
{
    g->vertexCount = n;
    g->heads = (AdjNode **)calloc((size_t)n + 1, sizeof(AdjNode *));
    return g->heads != NULL;
}

/* 添加无向边；重复边忽略，自环只存储一次 */
static int addUndirectedEdge(Graph *g, int u, int v)
{
    AdjNode *a, *b = NULL;

    if (u < 1 || u > g->vertexCount || v < 1 || v > g->vertexCount)
        return 0;

    if (hasEdge(g, u, v))
        return 1;                /* 重复边不重复存储 */

    a = (AdjNode *)malloc(sizeof(AdjNode));
    if (a == NULL) return 0;

    if (u != v) {
        b = (AdjNode *)malloc(sizeof(AdjNode));
        if (b == NULL) {
            free(a);
            return 0;
        }
    }

    a->vertex = v;
    a->next = g->heads[u];
    g->heads[u] = a;

    if (u != v) {
        b->vertex = u;
        b->next = g->heads[v];
        g->heads[v] = b;
    }
    return 1;
}

static void destroyGraph(Graph *g)
{
    int i;
    if (g->heads == NULL) return;

    for (i = 1; i <= g->vertexCount; ++i) {
        AdjNode *p = g->heads[i];
        while (p != NULL) {
            AdjNode *tmp = p;
            p = p->next;
            free(tmp);
        }
    }
    free(g->heads);
    g->heads = NULL;
    g->vertexCount = 0;
}

static void printGraph(const Graph *g)
{
    int i;
    printf("\n图的邻接表：\n");
    for (i = 1; i <= g->vertexCount; ++i) {
        AdjNode *p = g->heads[i];
        printf("%d:", i);
        while (p != NULL) {
            printf(" %d", p->vertex);
            p = p->next;
        }
        printf("\n");
    }
}

/* ======================== 自定义循环队列 ======================== */

typedef struct {
    int *data;
    int front;
    int rear;
    int size;
    int capacity;
} Queue;

static int initQueue(Queue *q, int capacity)
{
    q->data = (int *)malloc((size_t)capacity * sizeof(int));
    if (q->data == NULL) return 0;
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = capacity;
    return 1;
}

static int isQueueEmpty(const Queue *q)
{
    return q->size == 0;
}

static int enqueue(Queue *q, int value)
{
    if (q->size == q->capacity) return 0;
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
    return 1;
}

static int dequeue(Queue *q, int *value)
{
    if (isQueueEmpty(q)) return 0;
    *value = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return 1;
}

static void clearQueue(Queue *q)
{
    q->front = q->rear = q->size = 0;
}

static void destroyQueue(Queue *q)
{
    free(q->data);
    q->data = NULL;
    q->front = q->rear = q->size = q->capacity = 0;
}

/* ======================== 自定义 DFS 帧栈 ======================== */

/*
 * 每个栈帧保存当前顶点及下一条待检查的边。
 * 这种方式模拟递归 DFS 的调用现场，保证深度优先次序。
 */
typedef struct {
    int vertex;
    AdjNode *nextNeighbor;
} DFSFrame;

typedef struct {
    DFSFrame *data;
    int top;
    int capacity;
} DFSFrameStack;

static int initDFSFrameStack(DFSFrameStack *s, int capacity)
{
    s->data = (DFSFrame *)malloc((size_t)capacity * sizeof(DFSFrame));
    if (s->data == NULL) return 0;
    s->top = -1;
    s->capacity = capacity;
    return 1;
}

static int isDFSFrameStackEmpty(const DFSFrameStack *s)
{
    return s->top == -1;
}

static int pushDFSFrame(DFSFrameStack *s, int vertex, AdjNode *firstNeighbor)
{
    if (s->top + 1 >= s->capacity) return 0;
    s->top++;
    s->data[s->top].vertex = vertex;
    s->data[s->top].nextNeighbor = firstNeighbor;
    return 1;
}

static void popDFSFrame(DFSFrameStack *s)
{
    if (!isDFSFrameStackEmpty(s)) s->top--;
}

static void destroyDFSFrameStack(DFSFrameStack *s)
{
    free(s->data);
    s->data = NULL;
    s->top = -1;
    s->capacity = 0;
}

/* ======================== 必做：六度空间 BFS ======================== */

/*
 * 对每个顶点分别进行 BFS。
 * dist[v] 表示起点到 v 的最短边数；只扩展距离小于 6 的顶点。
 * 起点自身也计入“六步以内可达顶点数”。
 */
static void bfsSixDegrees(const Graph *g)
{
    int n = g->vertexCount;
    int *dist = (int *)malloc(((size_t)n + 1) * sizeof(int));
    Queue q;
    int start, i;

    if (dist == NULL || !initQueue(&q, n)) {
        printf("内存分配失败，无法执行六度空间统计。\n");
        free(dist);
        return;
    }

    printf("\n========== 六度空间统计（BFS） ==========\n");

    for (start = 1; start <= n; ++start) {
        int reachable = 0;

        for (i = 1; i <= n; ++i) dist[i] = -1;
        clearQueue(&q);

        dist[start] = 0;
        enqueue(&q, start);

        while (!isQueueEmpty(&q)) {
            int u;
            AdjNode *p;

            dequeue(&q, &u);
            if (dist[u] == 6) continue;

            for (p = g->heads[u]; p != NULL; p = p->next) {
                int v = p->vertex;
                if (dist[v] == -1) {
                    dist[v] = dist[u] + 1;
                    enqueue(&q, v);
                }
            }
        }

        for (i = 1; i <= n; ++i) {
            if (dist[i] >= 0 && dist[i] <= 6) reachable++;
        }

        printf("顶点 %d：6 步内可达 %d/%d 个顶点，占比 %.2f%%\n",
               start, reachable, n, 100.0 * reachable / n);
    }

    destroyQueue(&q);
    free(dist);
}

/* ======================== 选作1：非递归 DFS ======================== */

/* 从 start 出发，只遍历 start 所在的连通分量 */
static void dfsNonRecursive(const Graph *g, int start)
{
    int n = g->vertexCount;
    int *visited = (int *)calloc((size_t)n + 1, sizeof(int));
    DFSFrameStack s;

    if (visited == NULL || !initDFSFrameStack(&s, n)) {
        printf("内存分配失败，无法执行 DFS。\n");
        free(visited);
        return;
    }

    printf("非递归 DFS 遍历序列：%d", start);
    visited[start] = 1;
    pushDFSFrame(&s, start, g->heads[start]);

    while (!isDFSFrameStackEmpty(&s)) {
        DFSFrame *frame = &s.data[s.top];

        if (frame->nextNeighbor == NULL) {
            popDFSFrame(&s);
        } else {
            int v = frame->nextNeighbor->vertex;
            frame->nextNeighbor = frame->nextNeighbor->next;

            if (!visited[v]) {
                visited[v] = 1;
                printf(" -> %d", v);
                pushDFSFrame(&s, v, g->heads[v]);
            }
        }
    }

    printf("\n");
    destroyDFSFrameStack(&s);
    free(visited);
}

/* ======================== 选作2：DFS/BFS 生成森林 ======================== */

/*
 * parent[v]：
 *   -2 表示尚未访问
 *   -1 表示某棵生成树的根
 *   其他值表示 v 在生成树中的父节点
 *
 * 为兼容非连通图，从用户指定的 start 开始，
 * 再按顶点编号扫描剩余顶点，最终得到生成森林。
 */

static int buildDfsForest(const Graph *g, int start, int *parent)
{
    int n = g->vertexCount;
    int i;
    DFSFrameStack s;

    for (i = 1; i <= n; ++i) parent[i] = -2;

    if (!initDFSFrameStack(&s, n)) return 0;

    for (i = 0; i <= n; ++i) {
        int root = (i == 0) ? start : i;

        if (parent[root] != -2) continue;

        parent[root] = -1;
        pushDFSFrame(&s, root, g->heads[root]);

        while (!isDFSFrameStackEmpty(&s)) {
            DFSFrame *frame = &s.data[s.top];

            if (frame->nextNeighbor == NULL) {
                popDFSFrame(&s);
            } else {
                int u = frame->vertex;
                int v = frame->nextNeighbor->vertex;
                frame->nextNeighbor = frame->nextNeighbor->next;

                if (parent[v] == -2) {
                    parent[v] = u;
                    pushDFSFrame(&s, v, g->heads[v]);
                }
            }
        }
    }

    destroyDFSFrameStack(&s);
    return 1;
}
static int buildBfsForest(const Graph *g, int start, int *parent)
{
    int n = g->vertexCount;
    int i;
    Queue q;

    for (i = 1; i <= n; ++i) parent[i] = -2;

    if (!initQueue(&q, n)) return 0;

    for (i = 0; i <= n; ++i) {
        int root;

        if (i == 0) root = start;
        else root = i;

        if (parent[root] != -2) continue;

        parent[root] = -1;
        clearQueue(&q);
        enqueue(&q, root);

        while (!isQueueEmpty(&q)) {
            int u;
            AdjNode *p;
            dequeue(&q, &u);

            for (p = g->heads[u]; p != NULL; p = p->next) {
                int v = p->vertex;
                if (parent[v] == -2) {
                    parent[v] = u;
                    enqueue(&q, v);
                }
            }
        }
    }

    destroyQueue(&q);
    return 1;
}

/* 用缩进方式打印以 node 为根的树 */
static void printTreeChildren(const int *parent, int n, int node, int depth)
{
    int i;
    for (i = 1; i <= n; ++i) {
        if (parent[i] == node) {
            int k;
            for (k = 0; k < depth; ++k) printf("    ");
            printf("|-- %d\n", i);
            printTreeChildren(parent, n, i, depth + 1);
        }
    }
}

static void printForest(const int *parent, int n, int preferredRoot)
{
    int i;

    printf("根节点：%d\n", preferredRoot);
    printTreeChildren(parent, n, preferredRoot, 1);

    for (i = 1; i <= n; ++i) {
        if (parent[i] == -1 && i != preferredRoot) {
            printf("根节点：%d（其他连通分量）\n", i);
            printTreeChildren(parent, n, i, 1);
        }
    }
}

static void showSpanningForests(const Graph *g, int start)
{
    int n = g->vertexCount;
    int *parent = (int *)malloc(((size_t)n + 1) * sizeof(int));

    if (parent == NULL) {
        printf("内存分配失败，无法建立生成森林。\n");
        return;
    }

    printf("\n========== DFS 生成森林（非递归） ==========\n");
    if (buildDfsForest(g, start, parent))
        printForest(parent, n, start);
    else
        printf("DFS 生成森林建立失败。\n");

    printf("\n========== BFS 生成森林 ==========\n");
    if (buildBfsForest(g, start, parent))
        printForest(parent, n, start);
    else
        printf("BFS 生成森林建立失败。\n");

    free(parent);
}

/* ======================== 选作3：所有简单路径 ======================== */

/*
 * 从 source 出发，枚举到所有其他可达顶点的所有简单路径。
 * 每扩展到一个新顶点，就输出当前路径；随后继续向下搜索，
 * 因而既能输出到中间顶点的路径，也能输出更长的路径。
 *
 * visited 保证一条路径中顶点不重复。
 * 简单路径数量可能非常大，图较大时输出会很多。
 */
static void enumerateSimplePaths(const Graph *g,
                                 int current,
                                 int *visited,
                                 int *path,
                                 int depth,
                                 unsigned long long *pathCount)
{
    AdjNode *p;

    for (p = g->heads[current]; p != NULL; p = p->next) {
        int v = p->vertex;

        if (!visited[v]) {
            int i;

            visited[v] = 1;
            path[depth + 1] = v;

            printf("路径：");
            for (i = 0; i <= depth + 1; ++i) {
                if (i > 0) printf(" -> ");
                printf("%d", path[i]);
            }
            printf("，长度：%d\n", depth + 1);

            (*pathCount)++;

            enumerateSimplePaths(g, v, visited, path,
                                 depth + 1, pathCount);

            visited[v] = 0;      /* 回溯，允许其他路径再次经过 v */
        }
    }
}

static void printAllSimplePaths(const Graph *g, int start)
{
    int n = g->vertexCount;
    int *visited = (int *)calloc((size_t)n + 1, sizeof(int));
    int *path = (int *)malloc(((size_t)n + 1) * sizeof(int));
    unsigned long long pathCount = 0;

    if (visited == NULL || path == NULL) {
        printf("内存分配失败，无法枚举简单路径。\n");
        free(visited);
        free(path);
        return;
    }

    printf("\n========== 从顶点 %d 出发的所有简单路径 ==========\n", start);
    printf("说明：路径长度按边数计算；不重复经过同一顶点。\n");

    visited[start] = 1;
    path[0] = start;

    enumerateSimplePaths(g, start, visited, path, 0, &pathCount);

    if (pathCount == 0)
        printf("该顶点没有到其他顶点的简单路径。\n");
    else
        printf("共输出 %llu 条简单路径。\n", pathCount);

    free(visited);
    free(path);
}

/* ======================== 主函数及菜单 ======================== */

static int readStartVertex(const Graph *g)
{
    int start;
    printf("请输入起始顶点（1～%d）：", g->vertexCount);

    if (scanf("%d", &start) != 1) {
        printf("输入格式错误。\n");
        return -1;
    }

    if (start < 1 || start > g->vertexCount) {
        printf("顶点编号超出范围。\n");
        return -1;
    }
    return start;
}

int main(void)
{
    Graph graph;
    int n, m, i;
    int choice;

    graph.vertexCount = 0;
    graph.heads = NULL;

    printf("========== 实验三：图的遍历及应用 ==========\n");
    printf("请输入顶点数 n 和边数 m：");

    if (scanf("%d%d", &n, &m) != 2 || n <= 0 || m < 0) {
        printf("输入错误：顶点数必须大于 0，边数不能小于 0。\n");
        return 1;
    }

    if (!initGraph(&graph, n)) {
        printf("图的邻接表初始化失败。\n");
        return 1;
    }

    printf("请依次输入 %d 条无向边（每行两个顶点编号）：\n", m);
    for (i = 0; i < m; ++i) {
        int u, v;

        if (scanf("%d%d", &u, &v) != 2) {
            printf("边输入格式错误。\n");
            destroyGraph(&graph);
            return 1;
        }

        if (u < 1 || u > n || v < 1 || v > n) {
            printf("第 %d 条边的顶点编号无效，已跳过。\n", i + 1);
            continue;
        }

        if (!addUndirectedEdge(&graph, u, v)) {
            printf("添加边时内存分配失败。\n");
            destroyGraph(&graph);
            return 1;
        }
    }

    printGraph(&graph);

    do {
        printf("\n================ 功能菜单 ================\n");
        printf("1. 六度空间统计（必做：BFS）\n");
        printf("2. 非递归 DFS 遍历（选作一：自定义栈）\n");
        printf("3. 建立并打印 DFS、BFS 生成森林（选作二）\n");
        printf("4. 输出指定顶点到其他顶点的所有简单路径（选作三）\n");
        printf("0. 退出程序\n");
        printf("请选择：");

        if (scanf("%d", &choice) != 1) {
            printf("输入格式错误，程序结束。\n");
            break;
        }

        switch (choice) {
            case 1:
                bfsSixDegrees(&graph);
                break;

            case 2: {
                int start = readStartVertex(&graph);
                if (start != -1) dfsNonRecursive(&graph, start);
                break;
            }

            case 3: {
                int start = readStartVertex(&graph);
                if (start != -1) showSpanningForests(&graph, start);
                break;
            }

            case 4: {
                int start = readStartVertex(&graph);
                if (start != -1) printAllSimplePaths(&graph, start);
                break;
            }

            case 0:
                printf("程序已退出。\n");
                break;

            default:
                printf("无效选项，请重新输入。\n");
        }
    } while (choice != 0);

    destroyGraph(&graph);
    return 0;
}

/*
