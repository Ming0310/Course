#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define maxn 10020
#define maxm 200020
//ͼ�Ķ���ʹ����ʽǰ���Ƿ�ʽ����ͼ
//��Ȼ��Ҳ�������Լ��ķ�������ͼ,���ٽӾ�������ٽӱ�
#define INF 2147483630
int ans[maxn];
struct Edge
{
    int start, end, weight;
    //�ñߣ�ͬһ������һ����edgs�д洢���±ꡣ��next��Ϊ��ʽѰ��ͬһ�������б�
    int next;
};

struct Graph
{
    struct Edge edegs[maxm];

    //head[i] ��¼������iΪ���ı߼��е�һ������edges�е�index,
    int head[maxn];

    //��ǰ�ڼ�����
    int tot;
};

struct Graph *init_graph()
{
    struct Graph *new_grapth = (struct Graph *)malloc(sizeof(struct Graph));
    new_grapth->tot = 0;
    memset(new_grapth->head, -1, sizeof(new_grapth->head));
    return new_grapth;
}

void add_edge(int start, int end, int weight, struct Graph *graph)
{
    //�ṩ���ִ洢�ӱ߷�ʽ
    graph->edegs[++graph->tot].start = start;
    graph->edegs[graph->tot].end = end;
    graph->edegs[graph->tot].weight = weight;

    //��ʽǰ���Ǵ洢��ÿ��һ���ߵ�ʱ��ͬһ���ı���next������ʽ�洢
    graph->edegs[graph->tot].next = graph->head[start];
    graph->head[start] = graph->tot;
}

void visit_graph(int n, struct Graph *graph)
{
    //debug �������ṩ���ִ洢��ʽ�ı������̣�����ͬѧ��debug
    for (int i = 1; i <= n; i++) //n�����
    {
        printf("start with %d\n", i);
        for (int j = graph->head[i]; j != -1; j = graph->edegs[j].next) //������iΪ���ı�
        {
            printf("edgs i is start:%d,end:%d,weight is:%d\n", graph->edegs[j].start, graph->edegs[j].end, graph->edegs[j].weight);
        }
    }

    //����ͼ��ͬһ������бߵķ���

    // int  index;
    // index = graph->head[startpoint];
    // while (index!=-1)
    // {
    //     /* code */
    //     //do something

    //     index = graph->edegs[index].next;
    // }


}

int get_min_time(int n, int startpoint, struct Graph *graph)
{
    //TODO��
    int LowCost[n+1];
    int Close[n+1];
    int k,min;
    for(int i = 1;i < n+1;i++){
            LowCost[i] = INFINITY;
            Visited[i] = 0;
    }
    for(int i = graph->head[startpoint];i != -1;i = graph->edegs[i].next){
            Visited[graph->edegs[i].end] = 0;
            LowCost[graph->edegs[i].end] = graph->edegs[i].weight;
    }
    Visited[startpoint] = 1;
    for(int i = 1;i < n+1;i++){
            min = INFINITY;
            for(int j = 1;j < n+1;j++){
                if(Visited[j] == 0 && min > LowCost[j]){
                    k = j;
                    min = LowCost[j];
                }
            }
    Visited[k] = 1;
    for(int j = graph->head[k];j != -1;j = graph->edegs[k].next){
        if(Visited[graph->edegs[j].end] == 0){
            int sum = LowCost[k]+graph->edegs[j].weight;
            if(sum < LowCost[j]){
                LowCost[j] = sum;
                Close[j] = k;
            }
        }
    }
    }
    k = 1;
    for(int i = 2;i<n+1;i++){
        if(LowCost[i] < LowCost[k]) k = i;
    }
    k = LowCost[k];
    if(k < INFINITY) return k;
    else return -1;
}

int main()
{
    int n, m, startpoint, min_time;

    //�ļ�����ķ�ʽ
    freopen("4_1_input.in", "r", stdin);
    while (scanf("%d%d%d", &n, &m, &startpoint) != EOF)
    {
        //scanf("%d%d%d", &n, &m, &startpoint);
        struct Graph *g = init_graph();
        int start, end, weight;
        for (int i = 1; i <= m; i++)
        {
            scanf("%d%d%d", &start, &end, &weight);
            add_edge(start, end, weight, g);
        }
        min_time = get_min_time(n, startpoint, g);
        printf("%d\n", min_time);
    }
    fclose(stdin);

    //�ն�����ķ�ʽ
    /*
    while(scanf("%d%d%d", &n, &m, &startpoint)!=EOF){
    //scanf("%d%d%d", &n, &m, &startpoint);
    struct Graph* g =init_graph();
    int start, end, weight;
    for (int i=1; i<=m; i++){
        scanf("%d%d%d", &start, &end, &weight);
        add_edge(start, end, weight,g);
    }

    min_time=get_min_time(n,startpoint,g);
    printf("%d\n",min_time);
    }
    */
    return 0;
    /*
    �������������ʽ��
���룺
4 6 1
1 2 2
2 3 2
2 4 1
1 3 5
3 4 3
1 4 4

�����
4
    */
}
