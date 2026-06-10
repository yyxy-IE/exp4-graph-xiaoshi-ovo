/**
 * 地铁线路图查询器（学生版）
 * 实验任务：完成所有标记为 // TODO 的函数实现。
 * 编译：gcc -o metro metro_student.c -std=c99
 * 运行：./metro
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_NAME_LEN 32
#define MAX_LINE_NAME 20

// 邻接表边结点
typedef struct EdgeNode
{
    int adjVex; // 邻接站点编号
    int weight; // 权值（运行时间，分钟）
    struct EdgeNode *next;
} EdgeNode;

// 顶点结点（站点）
typedef struct VertexNode
{
    char name[MAX_NAME_LEN]; // 站点名称
    EdgeNode *firstEdge;     // 第一条边
    int *lineIds;            // 该站点所属的线路编号数组（动态分配）
    int lineCount;           // 所属线路数量
} VertexNode;

// 图结构
typedef struct
{
    VertexNode *vertices; // 顶点数组
    int vertexNum;        // 实际顶点数
    int vertexCapacity;   // 顶点数组容量
    int edgeNum;          // 边数
    int isDirected;       // 0:无向, 1:有向
} Graph;

// 辅助队列（用于BFS）
typedef struct Queue
{
    int *data;
    int front, rear, size, capacity;
} Queue;

// 函数声明
Graph *createGraph(int initCapacity, int isDirected);
void resizeGraph(Graph *g);
int addVertex(Graph *g, const char *name);
int findVertexIndex(Graph *g, const char *name);
void addEdge(Graph *g, int u, int v, int weight);
void addLineToStation(Graph *g, int stationIdx, int lineId);
void readMetroFile(const char *filename, Graph *g);
void printAdjList(Graph *g);

// 以下是需要实现的函数（TODO）
void DFSRecursive(Graph *g, int v, int *visited);
void DFSTraversal(Graph *g, int start);
void BFSTraversal(Graph *g, int start);
void connectivityAnalysis(Graph *g);
void dijkstra(Graph *g, int start, int *dist, int *prev);
void printPath(Graph *g, int *prev, int start, int end);
void shortestPathByTime(Graph *g, int start, int end);
void shortestPathByTransfer(Graph *g, int start, int end);
void freeGraph(Graph *g);

void printMenu();

// 队列操作
Queue *createQueue(int capacity);
void enqueue(Queue *q, int val);
int dequeue(Queue *q);
int isEmpty(Queue *q);
void freeQueue(Queue *q);

// ---------- 主函数 ----------
int main()
{
    Graph *g = createGraph(100, 0); // 无向图

    readMetroFile("metro.txt", g);

    int choice, start, end;
    char startName[MAX_NAME_LEN], endName[MAX_NAME_LEN];

    do
    {
        printMenu();
        printf("请输入选择：");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            printAdjList(g);
            break;
        case 2:
            printf("请输入起始站点名称：");
            fgets(startName, MAX_NAME_LEN, stdin);
            startName[strcspn(startName, "\n")] = '\0';
            start = findVertexIndex(g, startName);
            if (start == -1)
            {
                fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
            }
            else
            {
                printf("\nDFS 遍历序列（从 %s 开始）：\n", startName);
                DFSTraversal(g, start);
            }
            break;
        case 3:
            printf("请输入起始站点名称：");
            fgets(startName, MAX_NAME_LEN, stdin);
            startName[strcspn(startName, "\n")] = '\0';
            start = findVertexIndex(g, startName);
            if (start == -1)
            {
                fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
            }
            else
            {
                printf("\nBFS 遍历序列（从 %s 开始）：\n", startName);
                BFSTraversal(g, start);
            }
            break;
        case 4:
            connectivityAnalysis(g);
            break;
        case 5:
            printf("请输入起点站：");
            fgets(startName, MAX_NAME_LEN, stdin);
            startName[strcspn(startName, "\n")] = '\0';
            printf("请输入终点站：");
            fgets(endName, MAX_NAME_LEN, stdin);
            endName[strcspn(endName, "\n")] = '\0';
            start = findVertexIndex(g, startName);
            end = findVertexIndex(g, endName);
            if (start == -1)
            {
                fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
            }
            else if (end == -1)
            {
                fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
            }
            else
            {
                shortestPathByTime(g, start, end);
            }
            break;
        case 6:
            printf("请输入起点站：");
            fgets(startName, MAX_NAME_LEN, stdin);
            startName[strcspn(startName, "\n")] = '\0';
            printf("请输入终点站：");
            fgets(endName, MAX_NAME_LEN, stdin);
            endName[strcspn(endName, "\n")] = '\0';
            start = findVertexIndex(g, startName);
            end = findVertexIndex(g, endName);
            if (start == -1)
            {
                fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
            }
            else if (end == -1)
            {
                fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
            }
            else
            {
                shortestPathByTransfer(g, start, end);
            }
            break;
        case 0:
            printf("退出程序。\n");
            break;
        default:
            printf("无效选择，请重新输入。\n");
        }
        printf("\n");
    } while (choice != 0);

    freeGraph(g);
    return 0;
}

// ---------- 以下函数已完整实现（无需修改）----------

// 创建图
Graph *createGraph(int initCapacity, int isDirected)
{
    Graph *g = (Graph *)malloc(sizeof(Graph));
    g->vertexCapacity = initCapacity;
    g->vertexNum = 0;
    g->edgeNum = 0;
    g->isDirected = isDirected;
    g->vertices = (VertexNode *)malloc(sizeof(VertexNode) * initCapacity);
    for (int i = 0; i < initCapacity; i++)
    {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    return g;
}

// 动态扩容
void resizeGraph(Graph *g)
{
    int newCap = g->vertexCapacity * 2;
    g->vertices = (VertexNode *)realloc(g->vertices, sizeof(VertexNode) * newCap);
    for (int i = g->vertexCapacity; i < newCap; i++)
    {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    g->vertexCapacity = newCap;
}

// 添加站点，返回编号
int addVertex(Graph *g, const char *name)
{
    int idx = findVertexIndex(g, name);
    if (idx != -1)
        return idx;

    if (g->vertexNum >= g->vertexCapacity)
    {
        resizeGraph(g);
    }
    strcpy(g->vertices[g->vertexNum].name, name);
    g->vertices[g->vertexNum].firstEdge = NULL;
    g->vertices[g->vertexNum].lineIds = NULL;
    g->vertices[g->vertexNum].lineCount = 0;
    return g->vertexNum++;
}

// 查找站点编号
int findVertexIndex(Graph *g, const char *name)
{
    for (int i = 0; i < g->vertexNum; i++)
    {
        if (strcmp(g->vertices[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 添加边（无向图加双向）
void addEdge(Graph *g, int u, int v, int weight)
{
    if (u < 0 || u >= g->vertexNum || v < 0 || v >= g->vertexNum)
        return;

    EdgeNode *e = (EdgeNode *)malloc(sizeof(EdgeNode));
    e->adjVex = v;
    e->weight = weight;
    e->next = g->vertices[u].firstEdge;
    g->vertices[u].firstEdge = e;

    if (!g->isDirected)
    {
        e = (EdgeNode *)malloc(sizeof(EdgeNode));
        e->adjVex = u;
        e->weight = weight;
        e->next = g->vertices[v].firstEdge;
        g->vertices[v].firstEdge = e;
    }
    g->edgeNum++;
}

// 为站点添加所属线路编号
void addLineToStation(Graph *g, int stationIdx, int lineId)
{
    if (stationIdx < 0 || stationIdx >= g->vertexNum)
        return;
    for (int i = 0; i < g->vertices[stationIdx].lineCount; i++)
    {
        if (g->vertices[stationIdx].lineIds[i] == lineId)
            return;
    }
    g->vertices[stationIdx].lineCount++;
    g->vertices[stationIdx].lineIds = (int *)realloc(g->vertices[stationIdx].lineIds,
                                                     sizeof(int) * g->vertices[stationIdx].lineCount);
    g->vertices[stationIdx].lineIds[g->vertices[stationIdx].lineCount - 1] = lineId;
}

// 读取地铁文件
void readMetroFile(const char *filename, Graph *g)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr, "无法打开文件 %s\n", filename);
        exit(1);
    }

    char line[256];
    int routeCount = 0;
    // 跳过前两行（总站点数和线路数，这里简单处理：读取直到遇到第一条线路数据）
    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;
        sscanf(line, "%d", &routeCount);
        break;
    }
    // 再读一行（线路数）
    fgets(line, sizeof(line), fp);

    for (int rid = 0; rid < routeCount; rid++)
    {
        if (!fgets(line, sizeof(line), fp))
            break;
        if (line[0] == '#' || line[0] == '\n')
        {
            rid--;
            continue;
        }

        char lineName[MAX_LINE_NAME];
        int stationCount;
        char *token = strtok(line, " \t\n");
        if (!token)
            continue;
        strcpy(lineName, token);

        token = strtok(NULL, " \t\n");
        if (!token)
            continue;
        stationCount = atoi(token);

        int prevStation = -1;
        int timeVal = 1;
        for (int i = 0; i < stationCount; i++)
        {
            token = strtok(NULL, " \t\n");
            if (!token)
                break;

            // 判断是否为纯数字（时间）
            int isTime = 1;
            for (char *p = token; *p; p++)
            {
                if (!isdigit(*p))
                {
                    isTime = 0;
                    break;
                }
            }
            if (isTime && i > 0)
            {
                timeVal = atoi(token);
                continue;
            }

            int idx = addVertex(g, token);
            addLineToStation(g, idx, rid);

            if (prevStation != -1)
            {
                addEdge(g, prevStation, idx, timeVal);
                timeVal = 1;
            }
            prevStation = idx;
        }
    }
    fclose(fp);
    printf("成功读取地铁数据：共 %d 个站点，%d 条边。\n", g->vertexNum, g->edgeNum);
}

// 输出邻接表及换乘站
void printAdjList(Graph *g)
{
    printf("\n===== 邻接表 =====\n");
    for (int i = 0; i < g->vertexNum; i++)
    {
        printf("%s (%d条线路): ", g->vertices[i].name, g->vertices[i].lineCount);
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e)
        {
            printf("-> %s(%dmin) ", g->vertices[e->adjVex].name, e->weight);
            e = e->next;
        }
        printf("\n");
    }
    printf("\n===== 换乘站 =====\n");
    for (int i = 0; i < g->vertexNum; i++)
    {
        if (g->vertices[i].lineCount > 1)
        {
            printf("%s：%d 条线路\n", g->vertices[i].name, g->vertices[i].lineCount);
        }
    }
}

// 打印菜单
void printMenu()
{
    printf("\n====== 地铁查询系统 ======\n");
    printf("1. 输出邻接表和换乘站\n");
    printf("2. DFS 遍历（从指定站点）\n");
    printf("3. BFS 遍历（从指定站点）\n");
    printf("4. 连通分量分析\n");
    printf("5. 最短路径（最少时间）\n");
    printf("6. 最短路径（最少换乘）\n");
    printf("0. 退出\n");
}

// ---------- 队列实现（已提供）----------
Queue *createQueue(int capacity)
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->data = (int *)malloc(sizeof(int) * capacity);
    q->front = q->rear = q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int val)
{
    if (q->size == q->capacity)
        return;
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

int dequeue(Queue *q)
{
    if (q->size == 0)
        return -1;
    int val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return val;
}

int isEmpty(Queue *q)
{
    return q->size == 0;
}

void freeQueue(Queue *q)
{
    free(q->data);
    free(q);
}

// ---------- 以下为需要实现的函数（TODO）----------

void DFSRecursive(Graph *g, int v, int *visited)
{
    visited[v] = 1;
    printf("%s ", g->vertices[v].name);

    EdgeNode *e = g->vertices[v].firstEdge;
    while (e != NULL)
    {
        int adjVex = e->adjVex;
        if (!visited[adjVex])
        {
            DFSRecursive(g, adjVex, visited);
        }
        e = e->next;
    }
}

void DFSTraversal(Graph *g, int start)
{
    if (start < 0 || start >= g->vertexNum)
        return;

    int *visited = (int *)calloc(g->vertexNum, sizeof(int));
    if (visited == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        return;
    }

    DFSRecursive(g, start, visited);
    printf("\n");
    free(visited);
}

void BFSTraversal(Graph *g, int start)
{
    if (start < 0 || start >= g->vertexNum)
        return;

    int *visited = (int *)calloc(g->vertexNum, sizeof(int));
    if (visited == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        return;
    }

    Queue *q = createQueue(g->vertexNum);
    visited[start] = 1;
    enqueue(q, start);

    while (!isEmpty(q))
    {
        int u = dequeue(q);
        printf("%s ", g->vertices[u].name);

        EdgeNode *e = g->vertices[u].firstEdge;
        while (e != NULL)
        {
            int v = e->adjVex;
            if (!visited[v])
            {
                visited[v] = 1;
                enqueue(q, v);
            }
            e = e->next;
        }
    }

    printf("\n");
    freeQueue(q);
    free(visited);
}

void connectivityAnalysis(Graph *g)
{
    int *visited = (int *)calloc(g->vertexNum, sizeof(int));
    if (visited == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        return;
    }

    int componentCount = 0;
    printf("\n===== 连通分量分析 =====\n");

    for (int i = 0; i < g->vertexNum; i++)
    {
        if (!visited[i])
        {
            componentCount++;
            printf("连通分量 %d：", componentCount);

            Queue *q = createQueue(g->vertexNum);
            enqueue(q, i);
            visited[i] = 1;

            while (!isEmpty(q))
            {
                int u = dequeue(q);
                printf("%s ", g->vertices[u].name);

                EdgeNode *e = g->vertices[u].firstEdge;
                while (e != NULL)
                {
                    int v = e->adjVex;
                    if (!visited[v])
                    {
                        visited[v] = 1;
                        enqueue(q, v);
                    }
                    e = e->next;
                }
            }

            printf("\n");
            freeQueue(q);
        }
    }

    printf("总共有 %d 个连通分量\n", componentCount);
    free(visited);
}

void dijkstra(Graph *g, int start, int *dist, int *prev)
{
    int *visited = (int *)calloc(g->vertexNum, sizeof(int));
    if (visited == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        return;
    }

    // 初始化距离和前驱数组
    for (int i = 0; i < g->vertexNum; i++)
    {
        dist[i] = INT_MAX / 2; // 防止加法溢出
        prev[i] = -1;
    }
    dist[start] = 0;

    for (int i = 0; i < g->vertexNum; i++)
    {
        // 寻找当前距离最小的未访问节点
        int u = -1;
        int minDist = INT_MAX / 2;
        for (int j = 0; j < g->vertexNum; j++)
        {
            if (!visited[j] && dist[j] < minDist)
            {
                minDist = dist[j];
                u = j;
            }
        }

        if (u == -1)
            break; // 无可达节点，提前退出
        visited[u] = 1;

        // 松弛操作更新邻接节点距离
        EdgeNode *e = g->vertices[u].firstEdge;
        while (e != NULL)
        {
            int v = e->adjVex;
            if (!visited[v] && dist[v] > dist[u] + e->weight)
            {
                dist[v] = dist[u] + e->weight;
                prev[v] = u;
            }
            e = e->next;
        }
    }

    free(visited);
}

void printPath(Graph *g, int *prev, int start, int end)
{
    if (start == end)
    {
        printf("%s", g->vertices[start].name);
        return;
    }
    if (prev[end] == -1)
    {
        printf("不可达");
        return;
    }
    printPath(g, prev, start, prev[end]);
    printf(" -> %s", g->vertices[end].name);
}

void shortestPathByTime(Graph *g, int start, int end)
{
    int *dist = (int *)malloc(g->vertexNum * sizeof(int));
    int *prev = (int *)malloc(g->vertexNum * sizeof(int));
    if (dist == NULL || prev == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        free(dist);
        free(prev);
        return;
    }

    dijkstra(g, start, dist, prev);

    if (dist[end] == INT_MAX / 2)
    {
        printf("从 %s 到 %s 没有可达路径\n", g->vertices[start].name, g->vertices[end].name);
    }
    else
    {
        printf("最短时间路径（总时间 %d 分钟）：", dist[end]);
        printPath(g, prev, start, end);
        printf("\n");
    }

    free(dist);
    free(prev);
}

void shortestPathByTransfer(Graph *g, int start, int end)
{
    // 无向图邻接表中边结点总数为2*edgeNum
    int totalEdgeNodes = 2 * g->edgeNum;
    int *originalWeights = (int *)malloc(totalEdgeNodes * sizeof(int));
    if (originalWeights == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        return;
    }

    // 备份原始权值并将所有边权临时设为1
    int idx = 0;
    for (int i = 0; i < g->vertexNum; i++)
    {
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e != NULL)
        {
            originalWeights[idx++] = e->weight;
            e->weight = 1;
            e = e->next;
        }
    }

    // 调用Dijkstra计算边数最少的路径
    int *dist = (int *)malloc(g->vertexNum * sizeof(int));
    int *prev = (int *)malloc(g->vertexNum * sizeof(int));
    if (dist == NULL || prev == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        free(originalWeights);
        free(dist);
        free(prev);
        return;
    }

    dijkstra(g, start, dist, prev);

    // 恢复所有边的原始权值
    idx = 0;
    for (int i = 0; i < g->vertexNum; i++)
    {
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e != NULL)
        {
            e->weight = originalWeights[idx++];
            e = e->next;
        }
    }
    free(originalWeights);

    // 输出结果（换乘次数=路径边数-1）
    if (dist[end] == INT_MAX / 2)
    {
        printf("从 %s 到 %s 没有可达路径\n", g->vertices[start].name, g->vertices[end].name);
    }
    else
    {
        int transferCount = dist[end] - 1;
        printf("最少换乘路径（换乘 %d 次）：", transferCount);
        printPath(g, prev, start, end);
        printf("\n");
    }

    free(dist);
    free(prev);
}

void freeGraph(Graph *g)
{
    if (g == NULL)
        return;

    // 释放每个顶点的邻接表和线路编号数组
    for (int i = 0; i < g->vertexNum; i++)
    {
        // 释放邻接表边结点
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e != NULL)
        {
            EdgeNode *temp = e;
            e = e->next;
            free(temp);
        }
        // 释放线路编号数组
        free(g->vertices[i].lineIds);
    }

    // 释放顶点数组和图结构本身
    free(g->vertices);
    free(g);
}