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
int main()
{
    void *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }
    void *shm1;
    if ((shm1 = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }
    while (1)
    {
        // producer process will add nodes at interval of 50 sec
        addNodeEdges(graph, distanceMatrix);
        sleep(50);
    }
}