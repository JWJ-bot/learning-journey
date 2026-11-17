/*
 * 实验二：中序线索二叉树
 *
 * 功能：
 *   1. 按先序序列（# 表示空子树）创建二叉树
 *   2. 对二叉树进行中序线索化
 *   3. 求中序序列的第一个结点
 *   4. 求指定结点的中序后继
 *   5. 利用线索完成中序遍历，并输出结点值及左右指针信息
 *
 */

#include <stdio.h>
#include <stdlib.h>

#define EMPTY '#'

/* 线索标志：Link 表示指向孩子，Thread 表示指向前驱/后继 */
typedef enum {
    Link = 0,
    Thread = 1
} PointerTag;

/* 中序线索二叉树结点 */
typedef struct BiThrNode {
    char data;                    /* 结点值 */
    struct BiThrNode *lchild;     /* 左孩子或前驱 */
    struct BiThrNode *rchild;     /* 右孩子或后继 */
    PointerTag LTag;              /* 左指针类型 */
    PointerTag RTag;              /* 右指针类型 */
} BiThrNode, *BiThrTree;

/*
 * 读入一个有效字符，跳过空白字符。
 * 这样输入序列末尾的换行不会影响建树。
 */
static int ReadNodeChar(void)
{
    int ch;

    do {
        ch = getchar();
    } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

    return ch;
}

/*
 * 按先序序列递归创建二叉树。
 * 参数 T 是二级指针，因为需要修改调用者的根指针。
 */
void CreateBiThrTree(BiThrTree *T)
{
    int ch = ReadNodeChar();

    if (ch == EOF || ch == EMPTY) {
        *T = NULL;
        return;
    }

    *T = (BiThrTree)malloc(sizeof(BiThrNode));
    if (*T == NULL) {
        fprintf(stderr, "内存分配失败。\n");
        exit(EXIT_FAILURE);
    }

    (*T)->data = (char)ch;
    (*T)->LTag = Link;
    (*T)->RTag = Link;
    (*T)->lchild = NULL;
    (*T)->rchild = NULL;

    CreateBiThrTree(&((*T)->lchild));
    CreateBiThrTree(&((*T)->rchild));
}

/*
 * 递归完成中序线索化。
 * pre 始终指向当前访问结点的中序前驱。
 */
static void InThreading(BiThrTree p, BiThrTree *pre)
{
    if (p == NULL) {
        return;
    }

    InThreading(p->lchild, pre);        /* 线索化左子树 */

    if (p->lchild == NULL) {            /* 建立前驱线索 */
        p->LTag = Thread;
        p->lchild = *pre;
    }

    if (*pre != NULL && (*pre)->rchild == NULL) { /* 建立后继线索 */
        (*pre)->RTag = Thread;
        (*pre)->rchild = p;
    }

    *pre = p;

    InThreading(p->rchild, pre);        /* 线索化右子树 */
}

/*
 * 对整棵二叉树进行中序线索化。
 * 不增加头结点，最后一个结点的右线索为 NULL。
 */
void InOrderThreading(BiThrTree T)
{
    BiThrTree pre = NULL;

    InThreading(T, &pre);

    if (pre != NULL) {                  /* 处理中序最后一个结点 */
        pre->RTag = Thread;
        pre->rchild = NULL;
    }
}

/*
 * 求以 T 为根的子树在中序序列中的第一个结点。
 * 方法：沿着左孩子指针一直走到最左下结点。
 */
BiThrNode *FirstNode(BiThrTree T)
{
    BiThrNode *p = T;

    if (p == NULL) {
        return NULL;
    }

    while (p->LTag == Link && p->lchild != NULL) {
        p = p->lchild;
    }

    return p;
}

/*
 * 求结点 p 的中序后继。
 * 若右标志为 Thread，rchild 就是后继；
 * 否则后继是右子树的中序第一个结点。
 */
BiThrNode *NextNode(BiThrNode *p)
{
    if (p == NULL) {
        return NULL;
    }

    if (p->RTag == Thread) {
        return p->rchild;
    }

    return FirstNode(p->rchild);
}

/*
 * 输出一个指针域的信息：
 *   若为线索，输出其指向前驱/后继的值；
 *   若为孩子指针，输出其指向孩子的值；
 *   若为空，输出 NULL。
 */
static void PrintPointer(const BiThrNode *p, int isLeft)
{
    const BiThrNode *q;
    const char *fieldName;
    const char *role;

    if (isLeft) {
        q = p->lchild;
        fieldName = "左指针";
        role = (p->LTag == Thread) ? "前驱" : "孩子";
    } else {
        q = p->rchild;
        fieldName = "右指针";
        role = (p->RTag == Thread) ? "后继" : "孩子";
    }

    printf("%s=", fieldName);
    if (q != NULL) {
        printf("%c", q->data);
    } else {
        printf("NULL");
    }
    printf("(%s)", role);
}

/* 输出一个结点的值、左右标志以及左右指针信息 */
void PrintNode(const BiThrNode *p)
{
    if (p == NULL) {
        return;
    }

    printf("%c: 左标志=%d ", p->data, (int)p->LTag);
    PrintPointer(p, 1);
    printf("  右标志=%d ", (int)p->RTag);
    PrintPointer(p, 0);
    printf("\n");
}

/*
 * 利用中序线索遍历二叉树。
 * 先找到第一个结点，然后不断求后继即可，不需要递归和栈。
 */
void InOrderTraverse_Thr(BiThrTree T)
{
    BiThrNode *p = FirstNode(T);
    BiThrNode *q;

    if (p == NULL) {
        printf("空树。\n");
        return;
    }

    printf("中序遍历序列: ");
    for (q = p; q != NULL; ) {
        printf("%c", q->data);
        q = NextNode(q);
        if (q != NULL) {
            printf(" ");
        }
    }
    printf("\n");

    printf("结点指针信息 (0=孩子指针, 1=线索):\n");
    for (q = p; q != NULL; q = NextNode(q)) {
        PrintNode(q);
    }
}

int main(void)
{
    BiThrTree root = NULL;

    CreateBiThrTree(&root);     /* 先序建树 */
    InOrderThreading(root);     /* 中序线索化 */
    InOrderTraverse_Thr(root);  /* 中序遍历并输出 */

    return 0;
}
