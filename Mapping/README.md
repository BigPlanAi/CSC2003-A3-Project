# Mapping and Navigation (Team A3)

## Team Members:

| SIT ID  |     Github ID       |       Module      |
| ------  |     ---------       |    ------         |
| 2101303 |   nazirahmed-sit    |	Mapping & Navi  |
| 2101612 | 	Owen-Teo	    |   Mapping & Navi  |
| 2100913 | 	JinThong	    |   Mapping & Navi  |


## Overview

This mapping and navigation README consists of the following:

1. Mapping algorithm 
2. Navigation algorithm 
3. Data structures
4. Constraints

## Mapping algorithm
Our team decided to implement the Dijkstra algorithm in this project. 
We decided on implementing Dijkstra's algorithm due to:

 1. **High ease of use**, which is almost linear. 
 2.  Lack of negative weights in our algorithm

A brief pseudocode of our Dijkstra code is as follows:
```
Program start

Initialise sptSet = 0
Initialise min = INF
Initialise dist[src] = 0

While sptSet is not empty
	Pick a vertex (i.e. x) which is not in sptSet and has a minimum distance value.
    Include x to sptSet
    Update distance value of all adjacent vertices of x
	    To update, iterate through all adjacent vertices of x
	    If the sum of the distance value of u (from source) and the weight of edge u-v is less 
	    than the distance value of v, then the distance value of v is updated.
    
```


## Navigation algorithm

A flowchart of our algorithm is shown below:

**![](https://lh3.googleusercontent.com/JYL8CpkxpLjWmRulYSC9XZASPLo0jTsJGRrE0K6K0amMY7HI79IwVXHnTYT8RmUoLOx84nOzZlkkS2SxctJ5oNsAQGWL0u4Rj01gwwFCz2f4rnD0zp0s-Ek8l8SaDBC71ZNIoMZk-SFkReOnPt1LWYX723E1yHeBuppN5mlOc6v4v-UFna8g5obRoZXFqt12)**
## Data structures
For mapping and navigation to work, we implemented the following data structures:
1. **Adjacency List** - store the set of neighbors of a particular vertex
2. **Stack** - LIFO data structure that is used for the back tracking of the robot to it's starting location
3. **Adjacency Matrix** - store the set of neighbors of a particular vertex, but in a matrix form. This is used for determining shortest path via the Dijkstra code.


## Constraints
Some constrains of our program are:
1. The navigation algorithm will not work as well if the maze is not connected to the wall
2. Lack of heuristics mean that Dijkstra can be ineffective (can use A* for this)
3. No coordinate or positioning data structure
