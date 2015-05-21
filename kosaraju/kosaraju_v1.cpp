#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// 

/* Author:  Hubert Karbowy, hk - at - hubertkarbowy.pl
   
   This program computes the number of Strongly Connected
   Components in a directed acyclic graph using the Kosaraju
   two-pass algorithm. See comments in the main loop
   for details on input file structure.

*/

using namespace std;

struct vertex
{
	int label; // This doesn't really do anything in my implementation,
			   // because vertices will be identified by their map key.
			   // However, I'm keeping it in case you'd prefer to 
			   // change the program's implementation.
			   
	vector<int> edges; 		// directed edges to other vertices
	bool explored; 			// have we explored this vertex yet?
	int f; 					// finishing time - how many vertices
							//   have we seen before this one?
	int leader;			 	// node where the 2nd pass started
};

int t=0; // global variable for keeping track of finishing times
int s; // global variable for keeping track of SCC leaders
int howmanysccs=0;
int number_of_nodes=0;

/* The two boolean parameters enable/disable displaying finishing
   times and node leaders */
   
void listAllEdges(map<int,vertex> &graph, bool showF, bool showL)
{
	map<int,vertex>::iterator it;
	vector<int>::iterator vit;
	
	it = graph.begin();
	while (it != graph.end())
	{
		cout << "Node " << it -> first << " has edges to: ";
		vit = it -> second.edges.begin();
		while (vit != it -> second.edges.end())
		{
			 cout << *vit << " ";
			 advance (vit,1);
		}
		if (showF) cout << " - f(" << it->first << ") = " << it->second.f;
		if (showL) cout << " - leader: " << it->second.leader;
		cout << "\n";
		advance(it,1);
	}
}

int dfs(map<int,vertex> &graph, int starting_node, int pass) // pass = 1 or 2
{
	map<int,vertex>::iterator it;
	vector<int>::iterator vit;
	int count=0;
	
	it = graph.find(starting_node);
	it -> second.explored = true;
	count++;
	if (pass==2) it->second.leader = s; // only relevant in the 2nd pass
	vit = it->second.edges.begin();
	while (vit != it->second.edges.end())
	{
		if (graph[*vit].explored==false)
		{
			count=count+dfs(graph, *vit, pass);
		}
		advance(vit, 1);
	}
	t++;
	if (pass==1) 
	{
		it -> second.f = t; // only relevant to the 1st pass
	}
	return count;
}


/*  Reverses a graph.
	
	C++ prohibits modifying map keys, so the result has to be written
	to a new graph. Not too elegant, and certainly memory-consuming,
	but works well unless you have gigabytes of data to process.

	Ensure that map<int,vertex> &reversed is empty before invoking this
	function.
*/
void reverseGraph(map<int,vertex> &graph, map<int,vertex> &reversed)
{
	map<int,vertex>::iterator it;	// iterator on the original graph
	vector<int>::iterator vit;		// iterator on edges in the original graph
	map<int,vertex>::iterator rit;  // iterator on the reversed graph
		
	it = graph.begin();
		
	while (it != graph.end())
	{
		vit=it->second.edges.begin();
		
		rit=reversed.find(it->first);
		if (rit==reversed.end()) // for sink vertices
		{
			reversed[it->first] = {it->first, {}, false, it->second.f, 0};	
		}
		
		while (vit != it->second.edges.end())
		{
			rit=reversed.find(*vit);
			if (rit==reversed.end()) // if this node doesn't exist yet in the reversed graph, we'd better create it
			{
				reversed[*vit] = {*vit, {it->first}, false, it->second.f, 0};	
			}
			else // otherwise update information in the edges vector
			{
				reversed[*vit].edges.push_back(it->first);
			}
			advance(vit, 1);
		}
		advance(it,1);
	}
}


/* Swaps vertex names with their finishing times.
   Ensure that map<int,vertex> &renamed_graph is empty before invoking this function.
*/
void renameNodes(map<int,vertex> &graph, map<int,vertex> &renamed_graph)
{
	map<int,vertex>::iterator it;
	vector<int>::iterator vit;
	
	it=graph.begin();
	
	while (it != graph.end())
	{
		renamed_graph[it->second.f] = {it->second.f, {}, false, it->second.f, -1}; // leave the set of edges empty for handling in the inner while loop below
		vit = it->second.edges.begin(); // edges must be re-indexed too to point to vertices with new names
		while (vit != it->second.edges.end())
		{
			renamed_graph[it->second.f].edges.push_back(graph[*vit].f);
			advance(vit, 1);
		}
		advance(it,1);
	}
}

int main()
{
	map<int,vertex> graph; // Key: node name (in this example nodes are integers).
						   // Value: see the vertext struct description.
	map<int,vertex>::iterator it;
	map<int,vertex> reversed_graph; // We'll need at least two structures for our transformations
	
	ifstream graph_file("graphfile.txt");
	string single_edge;
	size_t line_space_separator;
	int thisnode;
	int points_to;
	
	/* Pre-processing: load the graph from file and get it into the map:
	
	The structure of graphfile.txt should be:
	
	node_id edge_to_which_the_node_points
	
	where both values are integers separated by a space. For example:
	
	1 5
	9 5
	
	means that there are three nodes: 1, 9 and 5. There is an edge from
	1 to 5 and another one from 9 to 5.
	  
	*/
	
	while (getline(graph_file,single_edge))
	{
		line_space_separator=single_edge.find(' ',0); // At this point the node id and the node towards which it points are separated
		thisnode = stoi(single_edge.substr(0, line_space_separator));
		points_to = stoi(single_edge.substr(line_space_separator));
		
		// If this is the first time we see this node, then we create a new map entry for it,
		// otherwise we update edges information in the vector
		it=graph.find(thisnode);
		if (it==graph.end()) graph[thisnode] = {thisnode, {points_to}, false, -1, 0};
		else graph[thisnode].edges.push_back(points_to);
		
		// We must also ensure that no sink vertices are left behind,
		// so we'd better create a map entry for every node pointed to.
		it=graph.find(points_to); 
		if (it==graph.end()) graph[points_to] = {points_to, {}, false, -1, 0};
	}
	
	cout << "Input graph: \n";
	// listAllEdges(graph, false, false);
	it = graph.end(); it--;
	cout << "Final node is # " << it->second.label << "\n";
	reverseGraph(graph, reversed_graph);
	graph.clear(); // After the graph is reversed, we no longer need
				   // the input one. This should free up some RAM.
	
	// First pass: Find finishing times on the reverse graph
	it = reversed_graph.end(); it--;
	
	t=0;
	
	while (it != reversed_graph.begin())
	{
		if (it -> second.explored == false)
		{
			s=it->first;
			number_of_nodes=dfs(reversed_graph,it->first,1);
		}
		advance(it,-1);
	}
	
	cout << "Reversed graph after 1st pass of Kosaraju algorithm with finishing times: \n";
	listAllEdges(reversed_graph, true, false);
	
	// Bookkeeping before the 2nd pass:
		
	graph.clear();
	renameNodes(reversed_graph, graph); // We swap vertex indices for the finishing times computed in the 1st pass
	reversed_graph.clear(); // Now we can do away with the reversed graph
	reverseGraph(graph, reversed_graph); // Uh-oh! Those names are getting somewhat confusing.
										//  The reversed_graph now holds the graph ready
										//  for the 2nd pass of the Kosaraju algorithm
	
	cout << "Input graph ready for the 2nd pass of Kosaraju algorithm: \n";
	listAllEdges(reversed_graph, false, false);
	
	
	// Second pass:
	s=0;
	it=reversed_graph.end();
	
	do
	{
		advance(it,-1);
		if (it->second.explored == false)
		{
			howmanysccs++;
			number_of_nodes=0;
			s=it->first;
			// cout << "Processing node # " <<s << "\n";
			number_of_nodes=dfs(reversed_graph,it->first,2);
			cout << "Graph " << howmanysccs << " has " << number_of_nodes << " SCCs\n";
			
		}
		
	} while (it != reversed_graph.begin());
	
	cout << "Leader vertices after the 2nd pass:\n";
	listAllEdges(reversed_graph, false, true);
	cout << "There are " << howmanysccs << " SCCs in the graph";
	
}
