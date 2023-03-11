#include <iostream>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <vector>
#include <random>
#include <time.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <ctime>
using namespace std;
const int INF = 100000000;
auto start = chrono::system_clock::now();
vector<vector<int>> createGraph(int *graph)
{
    vector<vector<int>> adjList(graph[0]);
    int edges = graph[1];
    for (int i = 2; i <= 2 * edges + 1; i += 2)
    {
        adjList[graph[i]].push_back(graph[i + 1]);
        adjList[graph[i + 1]].push_back(graph[i]);
    }
    return adjList;
}
void addNodeEdges(int *graph, int *distanceMatrix)
{
    auto end = chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    cout << "TIME ELAPSED PRODUCER PROCESS: " << elapsed_seconds.count() << endl;
    // graph[1] contains edges and graph[0] contains vertices
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(10, 30);
    int m = distr(gen); // generate a random number between 10 to 30
    uniform_int_distribution<> distr2(1, 20);
    int k = distr2(gen); // generate a random number between to 1 to 20
    vector<vector<int>> adjList = createGraph(graph);
    cout << "PRODUCER PROCESS: ADDING " << m << " NODES, k=" << k << " AND " << 2 * m * k << " EDGES\n";
    vector<bool> alreadyTaken(graph[0], false);
    int currEmptyIdx = 2 + graph[1] * 2; // find the empty pointer in graph shared memory
    for (int nodeNo = graph[0]; nodeNo < graph[0] + m; nodeNo++)
    {
        for (int j = 0; j < k; j++)
        {
            int total_probability = 2 * graph[1];
            int chosen_node = -1;
            for (int i = 0; i < graph[0]; i++)
            {

                float random_prob = (rand() / static_cast<float>(RAND_MAX)) * total_probability;
                if (random_prob < adjList[i].size() && !alreadyTaken[i] && i != nodeNo)
                {
                    chosen_node = i;
                    break;
                }
            }
            if (chosen_node == -1) // if none of the node have probability greater than random_prob then take the first available one
            {
                for (int i = 0; i < graph[0]; i++)
                    if (!alreadyTaken[i] && i != nodeNo)
                        chosen_node = i;
            }

            cout << "ADDING (" << nodeNo << ", " << chosen_node << ") TO THE GRAPH" << endl;
            graph[currEmptyIdx] = nodeNo;
            graph[currEmptyIdx + 1] = chosen_node;

            graph[currEmptyIdx] = chosen_node;
            graph[currEmptyIdx + 1] = nodeNo;
            currEmptyIdx += 4;
        }
    }

    graph[0] += m;                 // update no of vertices
    graph[1] += 2 * m * k;         // update edge
    distanceMatrix[0] = 2 * m * k; // new edges added
}
vector<int> dijkstra(int src, vector<vector<int>> &graph,int *distanceMatrix)
{
    int n = graph.size();
    vector<int> parent(n, -1), dist(n, INF);
    vector<bool> taken(n, false);
    dist[src] = 0;
    for (int i = 0; i < n; i++)
    {
        int v = -1;
        for (int j = 0; j < n; j++)
        {
            if (!taken[j] && (v == -1 || dist[j] < dist[v]))
                v = j;
        }

        taken[v] = true;
        for (auto node : graph[v])
        {
            if (dist[v] + 1 < dist[node])
            {
                dist[node] = dist[v] + 1;
                parent[node] = v;
            }
        }

    }
    //store the distance matrix in memory
    int curr_pointer=2;
    for (int i=0;i<dist.size();i++)
    {
        distanceMatrix[curr_pointer]=src;
        distanceMatrix[curr_pointer+1] = i;
        distanceMatrix[curr_pointer + 2] = dist[i];
        curr_pointer+=3;
    }
    distanceMatrix[1]+=dist.size();
    return parent; // return path
}
vector<int> makePath(int src, int dest, vector<int> parent)
{
    vector<int> path;
    for (int v = dest; v != src; v = parent[v])
        path.push_back(v);
    path.push_back(src);
    reverse(path.begin(), path.end());
    return path;
}
void findShortestPathAndWriteToFile(int *graph, int i)
{
    if (i == 1)
    {
        auto end = chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        cout << "TIME ELAPSED CONSUMER PROCESS: " << elapsed_seconds.count() << endl;
    }
    fstream file;
    string file_name = "consumer" + to_string(i) + "output.txt";
    file.open(file_name, ios::trunc | ios::in | ios::out);
    cout << "CONSUMER #" << i << " IS WRITING PATH TO FILE " << file_name << endl;
    if (!file)
    {
        cout << "UNABLE TO OPEN FILE" << endl;
        exit(EXIT_FAILURE);
    }
    vector<vector<int>> adjList = createGraph(graph);
    int noOfMappedNode = (graph[0] + 9) / 10;
    if (i < 9)
    {
        for (int j = (i - 1) * noOfMappedNode; j < (i - 1) * noOfMappedNode + noOfMappedNode; j++)
        {
            vector<int> parent = dijkstra(j, adjList);

            for (int k = 0; k < graph[0]; k++)
            {
                if (k != j)
                {
                    vector<int> path = makePath(j, k, parent);
                    for (auto node : path)
                        file << node << ' ';
                    file << endl;
                }
            }
        }
    }
    else // for the last consumer
    {
        for (int j = (i - 1) * noOfMappedNode; j < graph[0]; j++)
        {
            vector<int> parent = dijkstra(j, adjList);

            for (int k = 0; k < graph[0]; k++)
            {
                if (k != j)
                {
                    vector<int> path = makePath(j, k, parent);
                    for (auto node : path)
                        file << node << ' ';
                    file << endl;
                }
            }
        }
    }
    cout << "WRITING OF CONSUMER #" << i << " FINISHED" << endl;
}
void optimizedCalculation(int *graph, int *distanceMatrix)
{
    // do the optimized calculation
    int newEdges=graph[1]-distanceMatrix[0];
    
}
int main(int argc, char **argv)
{
    bool optimize = false;
    if (argc > 1 && argv[1] == "-optimize")
    {
        optimize = true;
    }
    int size = 1000000; // total size of the graph(assuming no more than 10^6 edges are to be added)
    int shmid;
    if ((shmid = shmget(IPC_PRIVATE, size * sizeof(int), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    void *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }
    // creating a shared memory for storing the distance matrix and new edges
    if ((shmid = shmget(3245, 1000 * sizeof(int), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    void *shm1;
    if ((shm1 = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }
    int *distanceMatrix = (int *)shm1;
    distanceMatrix[1]=0;
    int *graph = (int *)shm;
    // populate the shared memory(will contain the edges only)
    FILE *fp;
    fp = fopen("facebook_combined.txt", "r");
    int max = 0;
    int u, v;
    int i = 2, edges = 0;
    while (fscanf(fp, "%d %d", &u, &v) != EOF)
    {
        graph[i] = u, graph[i + 1] = v;
        if (u > max)
            max = u;
        if (v > max)
            max = v;
        i += 2;
        edges++;
    }
    fclose(fp);
    graph[0] = max + 1; // set the number of vertices
    graph[1] = edges;   // set the number of edges
    if (fork() == 0)
    {
        while (1)
        {
            // producer process will add nodes at interval of 50 sec
            addNodeEdges(graph, distanceMatrix);
            sleep(50);
        }
    }
    else
    {
        // spawn 10 childs to do the shortest path computation

        for (int i = 0; i < 10; i++)
        {

            if (fork() == 0)
            {
                while (1)
                {
                    findShortestPathAndWriteToFile(graph, i + 1);
                    sleep(30);
                }
                exit(0);
            }
        }
    }
    wait(NULL);
    return 0;
}
