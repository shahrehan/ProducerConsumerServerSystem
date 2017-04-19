#include <vector>
#include <iomanip>
#define INT_MAX 2147483647

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <algorithm>
using namespace std;

#define POINT_OF_INTERSECTION 1
#define POINT_OF_INTEREST 2

////////////////////////////////////// initializing edge /////////////////////////////////
struct edge
{
	int length;
	int speed;
	int direction;
	vector<int> events;

	edge()
	{
		length = 0;
		speed = 1;
		direction = 0;
	}

	float time()
	{
		return length / (speed + 0.0);
	}
};

////////////////////////////////////// initializing vertex /////////////////////////////////
struct Vertex
{
	int type;
	string label;
	Vertex()
	{
		type = POINT_OF_INTERSECTION;
		label = "";
	}
};

////////////////////////////////////// initializing road /////////////////////////////////
struct Road
{
	vector<edge> edges;
};

////////////////////////////////////// initializing class graph /////////////////////////////////
class Graph
{
private:
	int n;
	vector<Vertex> vert;
	vector<Road> roads;
	edge **adj;
	bool *visited;

public:
	Graph(string fileName); 
	Graph(int n);	
	~Graph(); 
	int add_edge(int origin, int destin, int direction, int speed, int length);
	void display();
	void retrieve(string fileName);
	int minDistance(float dist[], int sptSet[]);
	int trip(int,int,ostream& out);
	int routingTable(int src, int destination, ostream& out);
	int getSize();
	void edgeEvent(int origin, int destin, int event);
	void store(string fileName);
	void road(vector<edge> edges);
	Vertex vertex(int type, string label);
	void addVertex(int type, string label);
};

Graph::~Graph()
{
	delete visited;
	for (int i = 0; i < n; i++)
		delete [] adj[i];
	delete adj;
}

Graph::Graph(string fileName)
{
	retrieve(fileName);
}

////////////////////////////////////// retrieving graph /////////////////////////////////
void Graph::retrieve(string fileName)
{
	fstream fin;
	fin.open(fileName.c_str());
	int c2 = 0;
	int c1 = 0;
	int length = 0;
	int direction = 0;
	int speed = 0;
	int count = 0;
	int maxNodes = 0;
	string line;
	char temp;
	string label;
	int type = 0;

	if (fin.is_open() == false)
	{
		cout << "Err: cannot locate file " << fileName << endl;
		return;
	}

	getline(fin, line);
	n = atoi(line.c_str());

	visited = new bool[n];
	adj = new edge*[n];
	for (int i = 0; i < n; i++)
	{
		adj[i] = new edge[n];
	}

	for (int i = 0; i < n; i++)
	{
		getline(fin, line, ',');
		fin >> type;
		fin >> temp;
		fin >> label;
		addVertex(type, label);
	}

	while (fin.eof() == false)
	{
		getline(fin, line, ',');
		if (line.size() > 0)
		{
			c1 = atoi(line.c_str());
			fin >> c2;
			//ignore ,
			fin >> temp;
			fin >> direction;
			fin >> temp;
			fin >> speed;
			fin >> temp;
			fin >> length;
			add_edge(c1, c2, direction, speed, length);
			fin.ignore(1);
		}
		// ignore '\n'
	}
	fin.close();
}

////////////////////////////////////// adding vertex /////////////////////////////////
void Graph::addVertex(int type,string label)
{
	Vertex v;
	v.label = label;
	v.type = type;
	vert.push_back(v);
}

Graph::Graph(int n)
{
	this->n = n;
	visited = new bool[n];
	adj = new edge*[n];

	for (int i = 0; i < n; i++)
	{
		adj[i] = new edge[n];
		for (int j = 0; j < n; j++)
		{
			adj[i][j].length = 0;
			adj[i][j].speed = 1;
		}
	}
}

////////////////////////////////////// adding edge /////////////////////////////////
int Graph::add_edge(int origin, int destin, int direction, int speed, int length)
{
	if (origin > n || destin > n || origin < 0 || destin < 0)
	{
		cout << "Invalid edge!\n";
		return 0;
	}
	else
	{
		if (!direction)
		{
			//add for both since edge is undirected
			adj[origin - 1][destin - 1].length = length;
			adj[origin - 1][destin - 1].speed = speed;

			adj[destin - 1][origin - 1].length = length;
			adj[destin - 1][origin - 1].speed = speed;
		}
		else
		{
			adj[origin - 1][destin - 1].length = length;
			adj[origin - 1][destin - 1].speed = speed;
		}
		return 1;
	}
}

////////////////////////////////////// graph display /////////////////////////////////
void  Graph::display()
{
	int i, j;
	for (i = 0; i < n; i++)
	{
		// display time as weight
		for (j = 0; j < n; j++)
		{
			cout << (int)adj[i][j].time() << " ";
		}
		cout << endl;
	}
}

////////////////////////////////////// initializing distance /////////////////////////////////
int Graph::minDistance(float dist[], int sptSet[])
{
	int v = 0;
	/* Initialize min value */
	int min = INT_MAX, min_index = 0;

	for (v = 0; v < n; v++)
	{
		if (sptSet[v] == 0 && dist[v] <= min && dist[v] != INT_MAX)
			min = dist[v], min_index = v;
	}
	return min_index;

}

////////////////////////////////////// initializing trip /////////////////////////////////
int Graph::trip(int source,int destination,ostream& out)
{
	int i = 0;
	int count = 0;
	int u = 0;
	int p = 0;
	int v = 0;
	int distance = 0;
	float *dist = new float[n];     /* The output array.  dist[i] will hold the shortest */
	int *parent = new int[n]; /* Array to store constructed MST */
	int minTrips = 0;

	int *sptSet = new int[n]; /* sptSet[i] will true if vertex i is included in shortest */
	/* path tree or shortest distance from src to i is finalized */

	/* Initialize all distances as INFINITE and stpSet[] as false */
	for (i = 0; i < n; i++)
		dist[i] = INT_MAX, sptSet[i] = 0;

	/* Distance of source vertex from itself is always 0 */
	dist[source - 1] = 0;
	parent[source - 1] = -1;

	/* Find shortest path for all vertices */
	for (count = 0; count < n - 1; count++)
	{
		/* Pick the minimum distance vertex from the set of vertices not
		yet processed. u is always equal to src in first iteration. */
		u = minDistance(dist, sptSet);
		/* Mark the picked vertex as processed */
		sptSet[u] = 1;

		/* Update dist value of the adjacent vertices of the picked vertex. */
		for (v = 0; v < n; v++)
		{
			/* Update dist[v] only if is not in sptSet, there is an edge from
			u to v, and total weight of path from src to  v through u is
			smaller than current value of dist[v] */
			if (!sptSet[v] && adj[u][v].time() && dist[u] != INT_MAX)
			{
				if ((adj[u][v].time() + dist[u]) < dist[v])
				{
					parent[v] = u;
					dist[v] = adj[u][v].time() + dist[u];
				}
			}
		}
	}

	/* printing the path */
	out << "Source: " << source << endl;
	out << "Destination: " << destination << endl;
	out << "Path: ";
	p = destination - 1;
	vector<int> path;
	path.push_back(destination);
	
	while (p != source - 1)
	{
		path.push_back(parent[p]+1);
		p = parent[p];
	}

	for (int i = path.size() - 1; i >= 0; i--)
	{
		out << path[i] << " ";
	}

	out << "\n";
	out << "time(length/speed): " << dist[destination - 1];
	distance = dist[destination - 1];

	delete dist;
	delete sptSet;
	delete parent;

	return distance;
}

////////////////////////////////////// initializing routing table /////////////////////////////////

int Graph::routingTable(int src, int destination, ostream& out)
{
	int i = 0;
	int count = 0;
	int u = 0;
	int p = 0;
	int v = 0;
	float *dist = new float[n];     /* The output array.  dist[i] will hold the shortest */
	int *parent = new int[n]; /* Array to store constructed MST */
	int minTrips = 0;
	int distance = 0;
	int *sptSet = new int[n]; /* sptSet[i] will true if vertex i is included in shortest */
	/* path tree or shortest distance from src to i is finalized */

	/* Initialize all distances as INFINITE and stpSet[] as false */
	for (i = 0; i < n; i++)
		dist[i] = INT_MAX, sptSet[i] = 0;

	/* Distance of source vertex from itself is always 0 */
	dist[src - 1] = 0;
	parent[src - 1] = -1;

	/* Find shortest path for all vertices */
	for (count = 0; count < n - 1; count++)
	{
		/* Pick the minimum distance vertex from the set of vertices not
		yet processed. u is always equal to src in first iteration. */
		u = minDistance(dist, sptSet);
		/* Mark the picked vertex as processed */
		sptSet[u] = 1;

		/* Update dist value of the adjacent vertices of the picked vertex. */
		for (v = 0; v < n; v++)
		{
			/* Update dist[v] only if is not in sptSet, there is an edge from
			u to v, and total weight of path from src to  v through u is
			smaller than current value of dist[v] */
			if (!sptSet[v] && adj[u][v].time() && dist[u] != INT_MAX)
			{
				if ((adj[u][v].time() + dist[u]) < dist[v])
				{
					parent[v] = u;
					dist[v] = adj[u][v].time() + dist[u];
				}
			}
		}
	}

	/* printing the path */
	

	vector<int> path;
	p = destination - 1;
	path.push_back(destination);

	while (p != src - 1)
	{
		path.push_back(parent[p] + 1);
		p = parent[p];
	}


	out << destination << "\t\t";
	if (path.size() >= 2)
		out << path[path.size()-2] << "\t\t";
	else
		out << "error occured\n";
	out << dist[destination - 1] << "\t\t\t";
	for (int i = path.size() - 1; i >= 0; i--)
	{
		out << path[i] << " ";
	}
	out << endl;

	distance = dist[destination - 1];
	delete dist;
	delete sptSet;
	delete parent;

	return distance;
}

int Graph::getSize()
{
	return n;
}

void Graph::edgeEvent(int origin, int destin, int event)
{
	if (origin > n || destin > n || origin < 0 || destin < 0)
	{
		cout << "Invalid edge!\n";
	}
	else
	{
		if (!adj[origin - 1][destin - 1].direction)
		{
			adj[origin - 1][destin - 1].events.push_back(event);
		}
		else
		{
			adj[origin - 1][destin - 1].events.push_back(event);
			adj[destin - 1][origin - 1].events.push_back(event);
		}
	}
}

void Graph::store(string fileName)
{
	ofstream fout;
	fout.open(fileName.c_str());

	fout << n;
	for (int i = 0; i < n; i++)
	{
		fout << "Label,";
		fout << vert[i].type;
		fout << ",";
		fout << vert[i].label;
		fout << endl;
	}
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			fout << i+1 << "," << j+1 << "," << 0 << "," << adj[i][j].speed << "," << adj[i][j].length << endl;
		}
	}

	fout.close();
}

/////////////////////// initializing road ////////////////////////////////////
void Graph::road(vector<edge> edges)
{
	Road r;
	r.edges = edges;
	roads.push_back(r);
}
/////////////////////////////// vertex functionality /////////////////////////////
Vertex Graph::vertex(int type, string label)
{
	Vertex temp;
	for (int i = 0; i < vert.size(); i++)
	{
		if (vert[i].type == type && vert[i].label == label)
		{
			return vert[i];
		}
	}
	cout << "No such vertex exists\n";
	return temp;
}
