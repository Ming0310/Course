#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define maxn 10020
#define maxm 200020
//ͼ�Ķ���ʹ����ʽǰ���Ƿ�ʽ����ͼ
//��Ȼ��Ҳ�������Լ��ķ�������ͼ,���ٽӾ�������ٽӱ�
#define INF 2147483647
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

int Panduan(int* Flag,int n){
    int flag = 0;
    for(int i = 1;i<n+1;i++){
        if(Flag[i] != 0) flag = 1;
    }
    return flag; //ȫ��Ϊ0����0 ����
}


void forward_propagation(int n_total, int n_input, int n_output, int *input_array, struct Graph *graph)
{
    //���ս���洢��ȫ�ֱ���ans��
    int Flag[n_total];
    for(int i = 0;i < n_total+1;i++) Flag[i] = 0;
    for(int i = 1;i < n_input+1;i++){
        for(int j = graph->head[i];j!=-1;j = graph->edegs[j].next){
            int m = graph->edegs[j].end;
            ans[m] += input_array[i]*graph->edegs[j].weight;
            Flag[m] = 1;
        }
    }
    while(Panduan(Flag,n_total)){
    for(int i = 1;i < n_total+1;i++){
        if(Flag[i]){
                Flag[i] = 0;
            for(int j = graph->head[i];j!=-1;j = graph->edegs[j].next){
                int m = graph->edegs[j].end;
                Flag[m] = 1;
                ans[m] += ans[i]*graph->edegs[j].weight;
            }
        }
    }
    }
}
int main()
{

    int input[maxn];
    int n_total, m_total, n_input, n_output;

    //�ļ�����ķ�ʽ
   /* freopen("4_2_input.in", "r", stdin);
    while (scanf("%d%d%d%d", &n_total, &m_total, &n_input, &n_output) != EOF)
    {

        struct Graph *g = init_graph();
        int x, y, z;
        int start, end, weight;

        for (int i = 1; i <= m_total; i++)
        {
            scanf("%d%d%d", &start, &end, &weight);
            add_edge(start, end, weight, g);
        }
        for (int i = 1; i <= n_input; i++)
        {
            scanf("%d", &input[i]);
        }
        //���������ȫ�ֱ���ans������
        forward_propagation(n_total, n_input, n_output, input, g);

        for (int i = n_total - n_output + 1; i <= n_total; i++)
        {
            printf(i < n_total ? "%d " : "%d\n", ans[i]);
        }
    }
    fclose(stdin);

    */
    //�ն�����ķ�ʽ����io
    while (scanf("%d%d%d%d", &n_total, &m_total, &n_input, &n_output) != EOF)
    {

        struct Graph *g = init_graph();
        int x, y, z;
        int start, end, weight;

        for (int i = 1; i <= m_total; i++)
        {
            scanf("%d%d%d", &start, &end, &weight);
            add_edge(start, end, weight, g);
        }
        for (int i = 1; i <= n_input; i++)
        {
            scanf("%d", &input[i]);
        }
        //���������ȫ�ֱ���ans������
        forward_propagation(n_total, n_input, n_output, input, g);

        for (int i = n_total - n_output + 1; i <= n_total; i++)
        {
            printf(i < n_total ? "%d " : "%d\n", ans[i]);
        }
    }
/*
�����������
���룺
8 10 2 1
1 3 2
1 4 1
2 4 3
2 5 3
3 6 4
4 6 2
4 7 2
5 7 7
6 8 2
7 8 1
5 10
�����
500
*/
    return 0;
}
