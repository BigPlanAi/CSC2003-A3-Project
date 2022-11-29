/*
A mapping and navigation algorithm to 


*/



#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 12
#define V 10


int nodeCounter = 1;
int count = 0;
int arr[][2];

struct node {
  int vertex;
  struct node* next;
};

struct node* createNode(int);

struct Graph {
  int numVertices;
  struct node** adjLists;
};

// Create a node
struct node* createNode(int v) {
  struct node* newNode = malloc(sizeof(struct node));
  newNode->vertex = v;
  newNode->next = NULL;
  return newNode;
}

// Create a graph
struct Graph* createAGraph(int vertices) {
  struct Graph* graph = malloc(sizeof(struct Graph));
  graph->numVertices = vertices;

  graph->adjLists = malloc(vertices * sizeof(struct node*));

  int i;
  for (i = 0; i < vertices; i++)
    graph->adjLists[i] = NULL;

  return graph;
}

// Add edge
void addEdge(struct Graph* graph, int s, int d) {
  // Add edge from s to d
  struct node* newNode = createNode(d);
  newNode->next = graph->adjLists[s];
  graph->adjLists[s] = newNode;
  

  // Add edge from d to s
  newNode = createNode(s);
  printf("d: ", newNode);
  newNode->next = graph->adjLists[d];
  graph->adjLists[d] = newNode;

}

// Print the graph
void printGraph(struct Graph* graph) {
  int v;
  for (v = 1; v < graph->numVertices; v++) {
    struct node* temp = graph->adjLists[v];
    printf("\n Vertex %d: ", v);
    while (temp) {
      printf("%d -> ", temp->vertex);
      temp = temp->next;
    }
    printf("\n");
  }
}

// Creating a stack
struct stack {
  int items[MAX];
  int top;
};
typedef struct stack st;

void createEmptyStack(st *s) {
  s->top = -1;
}

// Check if the stack is full
int isfull(st *s) {
  if (s->top == MAX - 1)
    return 1;
  else
    return 0;
}

// Check if the stack is empty
int isempty(st *s) {
  if (s->top == -1)
    return 1;
  else
    return 0;
}

// Add elements into stack
void push(st *s, int newitem) {
  if (isfull(s)) {
    printf("STACK FULL");
  } else {
    s->top++;
    s->items[s->top] = newitem;
  }
  count++;
}

// Remove element from stack
void pop(st *s) {
  if (isempty(s)) {
    printf("\n STACK EMPTY \n");
  } else {
    // printf("\nItem popped = %d", s->items[s->top]);
    s->top--;
  }
  count--;
  printf("\n");
}

int peekToVar(st* s) {
  return s->items[s->top];
}

// Print elements of stack
void printStack(st *s) {
  printf("Stack: ");
  for (int i = 0; i < count; i++) {
    printf("%d ", s->items[i]);
  }
  printf("\n");
}

void navigate () {
  /* Dummy function to replicate wall following navigation algorithm */
  int dist;

  // for (int i = 0; i < 10; i++)
  // {
  //   nodeCounter++; // simulation, assumes that the car iteratively traversed through 10 nodes 
  // }

  dist > 5 ? moving_forward : moving_right 

  

}

void goBackToStart(st* s, st* t) {

  int latestAction;

  /* Turns back */
  void moving_left();
  void moving_left();

  while (isempty(s) == 0)
  {
    latestAction = s->items[s->top];
    switch (latestAction)
    {
    case 1:
      printf("Going straight for %d seconds", peekToVar(t));
      void moving_forward();
      break;
    case 2:
      printf("Going straight for %d seconds, then turning left", peekToVar(t));
      void moving_forward();
      void moving_left();
      break;
    case 3:
      printf("Going straight for %d seconds, then turning right", peekToVar(t));
      void moving_forward();
      void moving_right();
      break; 
    }
    pop(s);
    pop(t);
  }
  
}

void getHCSR04Distance() {
  // placeholder function to simulate integration
}

void moving_forward() {
  // placeholder function to simulate integration
}

void moving_left() {
  // placeholder function to simulate integration
}

void moving_right() {
  // placeholder function to simulate integration
}

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

int main() {
  
  int ch;
  st *s = (st *)malloc(sizeof(st)); // stack for direction
  st *t = (st *)malloc(sizeof(st)); // stack for time

  createEmptyStack(s);
  createEmptyStack(t);

  for (int i = 0; i < 10; i++)
  {
    nodeCounter++; // simulation, assumes that the car iteratively traversed through 10 nodes 
  }

  /*
    Stack codes:
    1: moving_forward()
    2: moving_right(), moving_forward()
    3: moving_left(), moving_forward()
  */

  struct Graph* graph = createAGraph(nodeCounter); // craete graph for the 
  addEdge(graph, 1, 2);
  push(s, 1); // pushes the direction number
  push(t, 8); // pushes the time
  addEdge(graph, 2, 3);
  push(s, 1);
  push(t, 4);
  addEdge(graph, 3, 4);
  push(s, 2);
  push(t, 15);
  addEdge(graph, 4, 5);
  push(s, 2);
  push(t, 12);
  addEdge(graph, 5, 6);
  push(s, 1);
  push(t, 3);
  addEdge(graph, 6, 7);
  push(s, 1);
  push(t, 3);
  // addEdge(graph, 6, 10); 
  // push(s, 2);
  // push(t, 8);
  // addEdge(graph, 10, 2); 
  // push(s, 3);
  // push(t, 9);
  // addEdge(graph, 1, 7);
  // push(s, 2);
  // push(t, 6);
  // addEdge(graph, 7, 8);
  // push(s, 3);
  // push(t, 2);
  // addEdge(graph, 8, 9);
  // push(s, 3);
  // push(t, 2);

  // printf("Size of directional stack: %d", sizeof(s));
  // printf("Size of time stack: %d", sizeof(t));
  // printf("Size of graph: %d", sizeof(graph));

  printGraph(graph);
  //printf("Element: %d\n", peekToVar(t));
  goBackToStart(s, t);
  // printGraph(graph);
  // printStack(t);
   
  return 0;
}