#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

using namespace std;
typedef struct g {
    int vertices;
    //declaring a vector for adjacency list of  matrix
    vector<vector<int>> adjList;
} Graph;
typedef struct GraphT2 {
    int vertices;
    int **adjList;
} GraphT2;

int main()
{
    FILE *fp;
    fp = fopen("facebook_combined.txt", "r");
    int u, v;
    Graph *g;
    g=(Graph*)malloc(sizeof(Graph));
    vector<pair<int,int>> edges;
    int max=0;
    while (fscanf(fp, "%d %d", &u, &v) != EOF)
    {
        edges.push_back(make_pair(u, v));
        if(u>max)
            max=u;
        if(v>max)
            max=v;
    }
    fclose(fp);
    g->vertices=max+1;
    g->adjList.resize(g->vertices);
    for(int i=0;i<edges.size();i++)
    {
        g->adjList[edges[i].first].push_back(edges[i].second);
        g->adjList[edges[i].second].push_back(edges[i].first);
    }
    
    int size = 1 + (edges.size()*2) + g->vertices;         //initial size of a integer to store number of vertices
    // size=1000;

    int shmid;
    if((shmid=shmget(IPC_PRIVATE,size*sizeof(int),IPC_CREAT|0666))<0)
    {
        perror("shmget");
        exit(1);
    }
    void *shm;
    if((shm=shmat(shmid,NULL,0))==(void*)-1)
    {
        perror("shmat");
        exit(1);
    }
    int *list = (int*)shm;

    list[0] = g->vertices;
    list[size-1] = edges.size();
    int pos=0;
    // cout<<size<<endl;
    for(int i=0;i<g->vertices;i++)
    {
        list[++pos] = g->adjList[i].size();
        // pos++;
        for(int j=0;j<g->adjList[i].size();j++)
        {
            list[++pos] = g->adjList[i][j];
            // pos++;
        }
    }

    // for(int i=0;i<5;i++)
    // {
    //     cout<<g->adjList[2][i]<<" ";
    // }
    // cout<<endl;

    free(g);

    GraphT2 *graph;
    graph=(GraphT2*)malloc(sizeof(GraphT2));
    graph->vertices = list[0];
    graph->adjList = (int**)malloc((graph->vertices)*sizeof(int*));
    pos=1;
    
    for(int i=0;i<graph->vertices;i++)
    {
        // cout<<i<<endl;
        graph->adjList[i] = list + pos;
        pos += list[pos] + 1;
    }
    
    // for(int i=1;i<=5;i++)
    // {
    //     cout<<graph->adjList[2][i]<<" ";
    // }

    // cout<<graph->vertices<<endl;

    return 0;
}