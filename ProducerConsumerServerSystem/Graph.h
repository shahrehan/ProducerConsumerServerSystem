#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>
using namespace std;

/*
* Adjacency Matrix Class
*/

#define INTERSECTION 1
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
		type = INTERSECTION;
		label = "";
	}
};

////////////////////////////////////// initializing Road /////////////////////////////////
struct Road
{
	vector<edge> edges;
};

////////////////////////////// initializing graph functionality ///////////////////////////
class Graph
{
private:
	int n;
	vector<Vertex> vert;
	vector<Road> roads;
	edge **adj;
	bool *visited;

public:
	Graph(string fileName); //constructor reads from file
	Graph(int n);	//constructor , initializes matrix of size n
	~Graph(); //destructor
	/*
	* Adding Edge to Graph
	*/
	int add_edge(int origin, int destin, int direction, int speed, int length);
	/*
	* Print the graph
	*/
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
