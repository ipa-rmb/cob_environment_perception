#pragma once

#include <queue>

namespace DFS {

// This class represents a directed graph using adjacency
// list representation
template<class Ptr, class Data, class Value, class Input>
class Graph
{
	struct FrontData {
		Ptr p_;
		Data d_;
		
		FrontData(const Ptr &p) : p_(p) {}
	};
	
	struct Front {
		std::vector<FrontData> hist_;
		Value val_;
		
		inline bool operator<(const Front &o) const {
			return val_<o.val_;	//TODO: check this one
		}
	};
		
	typedef boost::shared_ptr<Front> FrontPtr;
	
	Ptr head_;
    std::priority_queue<FrontPtr> waves_;
    
    FrontPtr candidate() const {return waves_.size()>0?waves_.front() : FrontPtr();}
    
public:
    Graph(const Ptr &head)
	{
		head_ = head;
	}
	
	bool comp(Input inp) {
		if(inp.size()<1) return false;
		
		if(waves_.size()==0) {
			//init.
			waves_.push(new Front);
			waves_.front().hist_.push_back(FrontData(head_));
			waves_.front().val_ = waves_.front().hist_.front().p_->comp(inp[0], waves_.front().hist_.front().d_);
		}
		
		while(waves_.size()>0) {
			if(waves_.fron()->hist_.size()==inp.size()) {
				return true;
			}
			
			std::vector<Ptr> pre = waves_.fron()->hist_.back().get_candidates();
			for(size_t j=0; j<pre.size(); j++) {
				Front cur = *waves_.fron();
				cur.hist_.push_back(FrontData(pre[j]));
				const Value v = cur.hist_.back().p_->comp(inp[cur.hist_.size()], cur.hist_.back().d_)*0.99999f;
				assert(v<1);	//so it's only descending
				cur.val_ *= v;
				
				waves_.push(cur);
			}
			
		}
		
		return false;
	}
    
//    void addEdge(int v, int w); // to add an edge to graph
//    void DFS(int s);  // prints DFS from a given source s
};

/*void Graph::addEdge(int v, int w)
{
    adj[v].push_back(w); // Add w to v’s list.
}*/
 
// Function to print all vertices reachable from 's'
// using iterative DFS.
/*void Graph::DFS(int s)
{
    // Mark all the vertices as not visited
    bool *visited = new bool[V];
    for(int i = 0; i < V; i++)
        visited[i] = false;
 
    // Create a stack for DFS
    stack<int> stack;
 
    // Mark the current node as visited and push it
    visited[s] = true;
    stack.push(s);
 
    // 'i' will be used to get all adjacent vertices
    // of a vertex
    list<int>::iterator i;
 
    while (!stack.empty())
    {
        // Pop a vertex from stack and print it
        s = stack.top();
        cout << s << " ";
        stack.pop();
 
        // Get all adjacent vertices of the popped vertex s
        // If a adjacent has not been visited, then mark it
        // visited and push it to the stack
        for (i = adj[s].begin(); i != adj[s].end(); ++i)
        {
            if (!visited[*i])
            {
                visited[*i] = true;
                stack.push(*i);
            }
        }
    }
}*/

} //namespace DFS
