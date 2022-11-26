#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
 
// Number of vertices in the graph
#define V 10
 
// compare distance between nodes and choose the node with lesser distance
int minDistance(int dist[], bool sptSet[])
{
    // Initialize min value
    int min = INT_MAX, min_index;
 
    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;
 
    return min_index;
}
 
// Dijkstra code, takes in the adjacency matrix and the source node
void dijkstra(int graph[V][V], int src)
{
    int dist[V]; // output array which will store the node shortest from source
 
    bool sptSet[V]; // sptSet[i] will be true if vertex i is
                    // included in shortest
    // path tree or shortest distance from src to i is
    // finalized
 
    // initialise all distances to be INF
    for (int i = 0; i < V; i++)
        dist[i] = INT_MAX, sptSet[i] = false;
 
    // initialise distance of source to be 0; this always holds true for source node
    dist[src] = 0;
 
    // Find shortest path for all nodes
    for (int count = 0; count < V - 1; count++) {
        // Pick the minimum distance vertex from the set of
        // vertices not yet processed. u is always equal to
        // src in the first iteration.
        int u = minDistance(dist, sptSet);
 
        // Mark the node as iterated through
        sptSet[u] = true;
 
        // Update dist value of the adjacent vertices of the
        // picked vertex.
        for (int v = 0; v < V; v++)
 
            // Update dist[v] only if is not in sptSet,
            // there is an edge from u to v, and total
            // weight of path from src to  v through u is
            // smaller than current value of dist[v]
            if (!sptSet[v] && graph[u][v]
                && dist[u] != INT_MAX
                && dist[u] + graph[u][v] < dist[v])
                dist[v] = dist[u] + graph[u][v];
    }
 
    // print the constructed distance array
    printSolution(dist);
}

// print all node distance
void printSolution(int dist[])
{
    printf("Node \t\t Distance from Source\n");
    for (int i = 0; i < V; i++)
        printf("%d \t\t\t\t %d\n", i + 1, dist[i]);
}

// driver's code
int main()
{
    
    int graph[V][V] = { { 0, 8, 0, 0, 0, 0, 6, 0, 0, 0 }, // node 1
                        { 8, 0, 4, 0, 0, 0, 0, 0, 0, 10 }, // node 2
                        { 0, 4, 0, 15, 0, 0, 0, 0, 0, 0 }, // node 3
                        { 0, 0, 15, 0, 12, 0, 0, 0, 0, 0 }, // node 4
                        { 0, 0, 0, 12, 0, 3, 0, 0, 0, 0 }, // node 5
                        { 0, 0, 0, 0, 3, 0, 3, 0, 0, 8 }, // node 6
                        { 6, 0, 0, 0, 0, 3, 0, 2, 0, 0 }, // node 7
                        { 0, 0, 0, 0, 0, 0, 2, 0, 2, 0 }, // node 8
                        { 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 }, // node 9
                        { 0, 9, 0, 0, 0, 8, 0, 0, 0, 0 } }; // node 10


    // Function call
    dijkstra(graph, 0);
 
    return 0;
}