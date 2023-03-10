#include <bits/stdc++.h>
using namespace std;

// declarations
const int first_node_id = 0, last_node_id = 37699, max_queue_size = 10340;
const int k = 10;
enum Action
{
    post,
    comment,
    like
};
typedef struct Action_
{
    int user_id;
    int action_id;
    Action action_type;
    time_t timestamp;
    Action_(int user_id, int action_id, Action action_type) // constructor for action class
    {
        this->user_id = user_id;
        this->action_id = action_id;
        this->action_type = action_type;
        time(&timestamp); // timestamp is set to current time
    }
    Action_(){};
} action;
typedef struct node_
{
    vector<action> wall_queue;
    vector<action> feed_queue;
    int priority;
    vector<int> neighbors;
    int action_id[3], feed_queue_in = 0, feed_queue_out = 0;
    pthread_mutex_t feed_queue_push_update_mutex, feed_queue_read_post_mutex;
    node_(int neighbour_id) // constructor to assign random priority to a node
    {
        this->priority = rand() % 2;
        this->neighbors.push_back(neighbour_id);
        int i;
        for (i = 0; i <= 2; i++)
            action_id[i] = 0;
        pthread_mutex_init(&this->feed_queue_push_update_mutex, NULL);
        pthread_mutex_init(&this->feed_queue_read_post_mutex, NULL);
        this->feed_queue.resize(max_queue_size);
    }
    node_(){}; // default constructor

} node;
unordered_map<int, node> graph;
vector<action> userS_pushU_shared_queue(max_queue_size); //
unordered_set<int> active_node;                          // will contain the node which have unread feed queue
int shared_queue_in = 0, shared_queue_out = 0;

// Mutex and condition variables
pthread_mutex_t shared_queue_push_update_mutex, active_node_mutex, file_mutex;
pthread_cond_t cond_shared_queue_push_update, cond_feed_queue;
// thread processes
int rand_helper(int &poolSize, int unused_pool[])
{
    int idx = rand() % (poolSize + 1);
    int val = unused_pool[idx];
    swap(unused_pool[idx], unused_pool[poolSize]);
    poolSize--;
    return val;
}
fstream fout;
void *userSimulator(void *param)
{
    vector<int> selected_nodes(100);
    int itr = 1;
    fstream fp;
    fp.open("user_simulator.log");
    while (1)
    {
        int poolSize = last_node_id;
        int unused_pool[last_node_id + 1];
        for (int i = 0; i < poolSize; i++)
            unused_pool[i] = i;
        for (int i = 0; i < 100; i++)
        {
            selected_nodes[i] = rand_helper(poolSize, unused_pool);
        }
        fp << selected_nodes[0] << endl;
        // cout <<
        string s1, s2;
        s1 += "Following actions have been generated in itr" + to_string(itr) + '\n';
        s2 += "SELECTED LIST OF NODES IN ITERATION " + to_string(itr) + " ALONG WITH THEIR NUMBER OF ACTIONS AND DEGREE ARE FOLLOWING\n";

        for (auto node_id : selected_nodes)
        {
            fp << node_id << endl;
            node curr_node = graph[node_id];
            int num_actions = k * (1 + log2(curr_node.neighbors.size()));
            for (int i = 0; i < num_actions; i++)
            {
                int action_type = rand() % 3;
                graph[node_id].action_id[action_type] += 1;
                action rand_action = action(node_id, graph[node_id].action_id[action_type], (Action)action_type);
                graph[node_id].wall_queue.push_back(rand_action);
                // pthread_mutex_lock(&shared_queue_push_update_mutex);
                while (((shared_queue_in + 1) % max_queue_size) == shared_queue_out)
                {
                    cout << "BUFFER FULL FOR SHARED QUEUE" << endl;

                }; // buffer full wait
                userS_pushU_shared_queue[shared_queue_in] = rand_action;
                shared_queue_in = (shared_queue_in + 1) % max_queue_size;
                pthread_cond_broadcast(&cond_shared_queue_push_update);
                s1 += "action_id:   " + to_string(rand_action.action_id) + "     type:     " + to_string(rand_action.action_type) + "     user_id:       " + to_string(rand_action.user_id) + "        timestamp:      " + to_string(rand_action.timestamp) + '\n';
            }
            s2 += to_string(node_id) + "        " + to_string(num_actions) + "           " + to_string(curr_node.neighbors.size()); // also write to log file
            s2 += '\n';
        }
        pthread_mutex_lock(&file_mutex);
        cout << s2 << s1 << endl;
        fout << s2 << s1 << endl;
        pthread_mutex_unlock(&file_mutex);

        cout << "I went to sleep";
        sleep(120);
        itr++;
    }
}
int total_action_handled = 0;
int total_pushed = 0;
pthread_mutex_t temp;
void *pushUpdate(void *param)
{
    fstream fout1;
    fout1.open("push_update.log");

    // try
    // {
    while (1)
    {
        pthread_mutex_lock(&shared_queue_push_update_mutex);
        while (shared_queue_in == shared_queue_out) // wait as queue is empty;
        {
            if (pthread_cond_wait(&cond_shared_queue_push_update, &shared_queue_push_update_mutex) > 0)
            {
                // cout<<"Waiting "
            }
        }
        action curr_action = userS_pushU_shared_queue[shared_queue_out];
        shared_queue_out = (shared_queue_out + 1) % max_queue_size;
        pthread_mutex_unlock(&shared_queue_push_update_mutex);
        for (auto neighbour_id : graph[curr_action.user_id].neighbors)
        {
            // apply a lock here also broadcast to all readPost thread(maybe pass the id of then node which feed have been updated)
            // graph[neighbour_id].feed_queue.push(curr_action);
            // release the lock
            pthread_mutex_lock(&active_node_mutex);
            node curr_node = graph[neighbour_id];
            while (((curr_node.feed_queue_in + 1) % max_queue_size) == curr_node.feed_queue_out)
            {
                cout << "BUFFER FULL FOR FEED QUEUE!!!" << endl;
            }
            pthread_mutex_lock(&curr_node.feed_queue_push_update_mutex);
            graph[neighbour_id].feed_queue[graph[neighbour_id].feed_queue_in] = curr_action;
            graph[neighbour_id].feed_queue_in = (graph[neighbour_id].feed_queue_in + 1) % max_queue_size;
            pthread_mutex_unlock(&curr_node.feed_queue_push_update_mutex);
            active_node.insert(neighbour_id);
            fout1 << neighbour_id << endl;
            pthread_mutex_unlock(&active_node_mutex);
            pthread_cond_signal(&cond_feed_queue);
        }
        pthread_mutex_lock(&file_mutex);
        cout << "action with following details have been successfully handled by push_update     "
             << "user_id: " << curr_action.user_id << "       "
             << "action_id: " << curr_action.action_id << "action_type: " << curr_action.action_type << "          "
             << "timestamp:      " << curr_action.timestamp << " and have been posted to feed_queue of of : " << graph[curr_action.user_id].neighbors.size() << "neighbors" << endl;
        fout << "action with following details have been successfully handled by push_update     "
             << "user_id: " << curr_action.user_id << "       "
             << "action_id: " << curr_action.action_id << "action_type: " << curr_action.action_type << "          "
             << "timestamp:      " << curr_action.timestamp << "posted to feed_queue of of : " << graph[curr_action.user_id].neighbors.size() << "neighbors" << endl;
        pthread_mutex_unlock(&file_mutex);
    }
}
map<pair<int, int>, int> mutual_friends;
bool comp(action A, action B)
{
    return A.timestamp > B.timestamp;
}
void *readPost(void *param)
{
    fstream fout1;
    fout1.open("read_post.log");
    // cout<<"I am working"<<endl;
    while (1)
    {
        pthread_mutex_lock(&active_node_mutex);
        while (active_node.size() == 0)
        {
            pthread_cond_wait(&cond_feed_queue, &active_node_mutex);
        }
        int node_id = *active_node.begin();
        pthread_mutex_lock(&graph[node_id].feed_queue_read_post_mutex);
        // fout1<<node_id<<endl;
        // fout1<<active_node.size()<<endl;
        active_node.erase(node_id);
        pthread_mutex_unlock(&active_node_mutex);
        string s = "";
        if (graph[node_id].priority)
        {
            // sort in chronological order
            sort(graph[node_id].feed_queue.begin(), graph[node_id].feed_queue.end(), comp);
        }
        else
        {
            auto comp2 = [&](action A, action B) -> bool
            {
                return mutual_friends[{A.user_id, node_id}] > mutual_friends[{B.user_id, node_id}];
            };
            sort(graph[node_id].feed_queue.begin(), graph[node_id].feed_queue.end(), comp2);
        }
        s += "I am reading the feed queue of the user_id:   " + to_string(node_id) + " and\n";
        while (graph[node_id].feed_queue_out < graph[node_id].feed_queue_in)
        {
            action curr_action = graph[node_id].feed_queue[graph[node_id].feed_queue_out];
            graph[node_id].feed_queue_out = (graph[node_id].feed_queue_out + 1) % max_queue_size;
            if (curr_action.timestamp == 0)
                continue;
            s += "I read action number: " + to_string(curr_action.action_id) + " Of type: " + to_string(curr_action.action_type) + " By user_id: " + to_string(curr_action.user_id) + " time: " + to_string(curr_action.timestamp) + "\n";
        }
        pthread_mutex_unlock(&graph[node_id].feed_queue_read_post_mutex);
        pthread_mutex_lock(&file_mutex);
        cout << s << endl;
        // fout1 << s;
        fout1 << s << endl;
        fout << s << endl;
        pthread_mutex_unlock(&file_mutex);
    }

    // to be implemented
}
void calculate_mutuals()
{
    cout << "Calculating mutuals please wait....." << endl;

    for (int i = first_node_id; i <= last_node_id; i++)
    {

        for (auto neighbour_id : graph[i].neighbors)
        {
            if (mutual_friends.find({neighbour_id, i}) != mutual_friends.end())
            {
                continue;
            }
            set<int> s;
            for (auto id :graph[i].neighbors) s.insert(id);
            for(auto id:graph[neighbour_id].neighbors)s.insert(id);
            mutual_friends[{i, neighbour_id}] = mutual_friends[{neighbour_id, i}] = graph[i].neighbors.size() + graph[neighbour_id].neighbors.size()-s.size();
        }
    }
    cout << "Completed! starting the other threads" << endl;
}
int main()
{
    fout.open("sns.log");
    // Main thread and loading of graph from file
    srand(time(NULL));
    ifstream fin;
    fin.open("musae_git_edges.csv");
    int line = 0;
    while (fin && ++line)
    {
        string s;
        getline(fin, s);
        if (line == 1 || s == "")
            continue;
        size_t pos = s.find(',');
        if (pos == string::npos)
        {
            cout << "Wrong format for line: " << line << "string: " << s << endl;
            continue;
        }
        try
        {
            int node1_id = stoi(s.substr(0, pos)), node2_id = stoi(s.substr(pos + 1, s.length() - pos));
            if (graph.find(node1_id) == graph.end())
                graph[node1_id] = node(node2_id);
            else
                graph[node1_id].neighbors.push_back(node2_id);
            if (graph.find(node2_id) == graph.end())
                graph[node2_id] = node(node1_id);
            else
                graph[node2_id].neighbors.push_back(node1_id);
        }
        catch (const std::exception &e)
        {
            cout << "Some error in parsing line: " << line << "string: " << s << endl;
            std::cerr << e.what() << '\n';
        }
    }
    calculate_mutuals();
    pthread_mutex_init(&shared_queue_push_update_mutex, NULL);
    pthread_mutex_init(&active_node_mutex, NULL);
    pthread_mutex_init(&temp, NULL);
    pthread_mutex_init(&file_mutex, NULL);
    pthread_cond_init(&cond_shared_queue_push_update, NULL);
    pthread_cond_init(&cond_feed_queue, NULL);
    // user_simulator thread
    pthread_t user_simulator;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&user_simulator, &attr, userSimulator, NULL);
    pthread_t push_update[25];
    for (int i = 0; i < 25; i++)
    {
        pthread_create(&push_update[i], NULL, pushUpdate, NULL);
    }
    pthread_t read_post[25];
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&read_post[i], NULL, readPost, NULL);
    }
    pthread_join(user_simulator, NULL);
    for (int i = 0; i < 25; i++)
    {
        pthread_join(push_update[i], NULL);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(read_post[i], NULL);
    }

    return 0;
}