#include <string>
#include <cstring>
#include <cassert>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include<limits>
#include "../include/GraphManager.h"
#include "../include/GraphManagerNew.h"
#include "../include/XMLParser.h"
#include "../include/macros.h"
//#include "../include/template_datatypes.h"
//#include "../include/template_scanner.tab.h"
#include "../include/rules_parser_datatypes.h"
#include "../include/rules_parser.tab.h"
//#include "../include/Rules.h"
#include <ctime>
#include <unistd.h>
#include <math.h> // required for 'sleep' on gcc version >= 4.7
#include <vector>
#include <queue>


using namespace std;

ofstream debug_log("mylog.txt");



// Rudrajit's min-cut code -- edited by sukanya -- starts here



//Gusfield algorithm obatined from TopCoder


/* Returns true if there is a path from source 's' to sink 't' in
  residual graph. Also fills parent[] to store the path */

int V,E;

bool bfs(vector< vector<int> >& rGraph, int s, int t, vector<int>& parent) //for both directed and un-directed graphs
    {
        // Create a visited array and mark all vertices as not visited
        vector<bool> visited(V);
        int i;
        for(i=0;i<V;i++){
        visited[i]=0;
        }

        // Create a queue, enqueue source vertex and mark source vertex
        // as visited
        queue <int> q;
        q.push(s);
        visited[s] = true;
        parent[s] = -1;

        // Standard BFS Loop
        while (!q.empty())
        {
            int u = q.front();
            q.pop();

            for (int v=0; v<V; v++)
            {
                if (visited[v]==false && rGraph[u][v] > 0)
                {
                    q.push(v);
                    parent[v] = u;
                    visited[v] = true;
                }
            }
        }

        // If we reached sink in BFS starting from source, then return
        // true, else false
        return (visited[t] == true);
    }



void dfs(vector< vector<int> >& rGraph, int s, vector<bool>& visited)
{
    visited[s] = true;
    for (int i = 0; i < V; i++)
       if (rGraph[s][i] && !visited[i])
           dfs(rGraph, i, visited);
}

    void fordFulkersonTree(vector< vector<int> >& tree, int s , int t, vector< vector<int> >& min_cut)
    {
        vector<int> parent(V);
        bool b=bfs(tree,s,t,parent);
        int u,v,ct=0;
         int path_flow = INT_MAX;
            for (v=t; v!=s; v=parent[v]) // OR v=u
            {
                u = parent[v];
                if(tree[u][v]<=path_flow)
                {
                    path_flow=tree[u][v];
                    min_cut[ct][0]=u;
                    min_cut[ct][1]=v;
                    ct++;
                }
            }
          // cout<<ct;
           min_cut[ct][0]=min_cut[ct][1]=-2;
    }



    // Returns the maximum flow from s to t in the given graph
    int fordFulkerson(vector< vector<int> >& graph, int s, int t,vector< vector<int> >& rGraph, vector< vector<int> >& min_cut) //for undirected graphs
    {
        int u, v,ct=0;

        // Create a residual graph and fill the residual graph with
        // given capacities in the original graph as residual capacities
        // in residual graph
         // Residual graph where rGraph[i][j] indicates
                         // residual capacity of edge from i to j (if there
                         // is an edge. If rGraph[i][j] is 0, then there is not)
        for (u = 0; u < V; u++)
            for (v = 0; v < V; v++)
                 rGraph[u][v] = graph[u][v];

        vector<int> parent(V);  // This array is filled by BFS and to store path
        int max_flow = 0;  // There is no flow initially
        // Augment the flow while there is path from source to sink
        while (bfs(rGraph, s, t, parent))
        {
            // Find minimum residual capacity of the edges along the
            // path filled by BFS. Or we can say find the maximum flow
            // through the path found.
            int path_flow = INT_MAX;
            for (v=t; v!=s; v=parent[v]) // OR v=u
            {
                u = parent[v];
                path_flow = min(path_flow, rGraph[u][v]);
            }

            // update residual capacities of the edges and reverse edges
            // along the path
            for (v=t; v!= s; v=parent[v]) // OR v=u
            {
                u = parent[v];
                rGraph[u][v] -= path_flow;
                rGraph[v][u] -= path_flow;
            }

            // Add path flow to overall flow
            max_flow += path_flow;
        }

    for(u=0;u<V;u++)
    {
        for(v=u+1;v<V;v++)
        {
            if(graph[u][v]>0 && rGraph[u][v]==0)
            {
                min_cut[ct][0]=u;
                min_cut[ct][1]=v;
                ct++;
            }

        }
    }
     min_cut[ct][0]=min_cut[ct][1]=-2;
     for(u=0;u<ct;u++)
     {
        rGraph[min_cut[u][0]][min_cut[u][1]]= rGraph[min_cut[u][1]][min_cut[u][0]]= graph[min_cut[u][0]][min_cut[u][1]];
        if(bfs(rGraph,s,t,parent))
        {
           rGraph[min_cut[u][0]][min_cut[u][1]]=rGraph[min_cut[u][1]][min_cut[u][0]]=0;
        }
        else
        {
            min_cut[u][0]=min_cut[u][1]=-1;
        }
     }

        // Return the overall flow
        return max_flow;
    }

int myfordFulkerson(GraphNew * graph_ptr, vector< vector<int> >& graph, vector< vector<int> >& EdgeID, int s, int t,vector< vector<int> >& rGraph, vector< vector<int> >& min_cut, vector< vector <set<int> > >& minCutEdgeIds) //for undirected graphs
    {
        int u, v,ct=0;
        set<int> cut_edges;
        // Create a residual graph and fill the residual graph with
        // given capacities in the original graph as residual capacities
        // in residual graph
         // Residual graph where rGraph[i][j] indicates
                         // residual capacity of edge from i to j (if there
                         // is an edge. If rGraph[i][j] is 0, then there is not)
        for (u = 0; u < V; u++)
            for (v = 0; v < V; v++)
                 rGraph[u][v] = graph[u][v];

        vector<int> parent(V);  // This array is filled by BFS and to store path
        int max_flow = 0;  // There is no flow initially
        // Augment the flow while there is path from source to sink
        while (bfs(rGraph, s, t, parent))
        {
            // Find minimum residual capacity of the edges along the
            // path filled by BFS. Or we can say find the maximum flow
            // through the path found.
            int path_flow = INT_MAX;
            for (v=t; v!=s; v=parent[v]) // OR v=u
            {
                u = parent[v];
                path_flow = min(path_flow, rGraph[u][v]);
            }

            // update residual capacities of the edges and reverse edges
            // along the path
            for (v=t; v!= s; v=parent[v]) // OR v=u
            {
                u = parent[v];
                rGraph[u][v] -= path_flow;
                rGraph[v][u] -= path_flow;
            }

            // Add path flow to overall flow
            max_flow += path_flow;
        }
        

    vector<bool> visited(V, false);
    dfs(rGraph, s, visited);
 
    // Print all edges that are from a reachable vertex to
    // non-reachable vertex in the original graph
    
    
    for (int i = 0; i < V; i++) {
      for (int j = 0; j < V; j++) { 
         //cout << i  << "--" << j << endl; 
         if (visited[i] && !visited[j] && graph[i][j]) {
             if (EdgeID[i][j] != 0)
                cut_edges.insert(EdgeID[i][j]);
             if (EdgeID[j][i] != 0)
                cut_edges.insert(EdgeID[j][i]);
             
         }
         
      }
      
    }
    
    
    
        minCutEdgeIds[s][t] = cut_edges;
//    for(set<int>::iterator set_itr = cut_edges.begin(); set_itr != cut_edges.end(); set_itr++) {
//        minCutEdgeIds[s][t].push_back(*set_itr);
//    }
    
//    for(u=0;u<V;u++)
//    {
//        for(v=u+1;v<V;v++)
//        {
//            if(graph[u][v]>0 && rGraph[u][v]==0)
//            {
//                min_cut[ct][0]=u;
//                min_cut[ct][1]=v;
//                ct++;
//            }
//
//        }
//    }
//     min_cut[ct][0]=min_cut[ct][1]=-2;

//    int src, tgt;
//    vector<int>::iterator cut_edge_iter;
//    for(cut_edge_iter = cut_edges.begin(); cut_edge_iter != cut_edges.end(); cut_edge_iter++)
//    {
//        int curr_edge_id = *cut_edge_iter;
//        src = graph_ptr->get_source_node(curr_edge_id) - 1;
//        tgt = graph_ptr->get_target_node(curr_edge_id) - 1;
//    
//        rGraph[src][tgt] = rGraph[tgt][src] = graph[src][tgt];
//        if(bfs(rGraph,s,t,parent))
//        {
//           rGraph[src][tgt]=rGraph[tgt][src]=0;
//           minCutEdgeIds[s][t].push_back(curr_edge_id);
//           minCutEdgeIds[t][s].push_back(curr_edge_id);
//        }
//    }

    // Return the overall flow
    return max_flow;
}


    
    
    void Gusfield(vector< vector<int> >& graph, vector< vector<int> >& Gh, vector< vector<int> >& Gh2, vector< vector< vector<int> > >& min_cut, vector<int>& parent)
    { //for undirected graphs

    int i,j,F,ct=0;
    for(i=0;i<V;i++)
    {
        parent[i]=0; //initialized to 0
    }


    for(i=0;i<V;i++)
    for(j=0;j<V;j++)
    Gh[i][j]=Gh2[i][j]=0;

    vector<int> parent2(V);
    vector< vector<int> > graph2(V, vector<int>(V));

    //Replace graph everywhere with undirectedGraph and later on check whether the edges are directed from source to target
    for(i=1;i<V;i++){
    	//Compute the minimum cut between i and parent[i].
    	//Let the i-side of the min cut be S, and the value of the min-cut be F
            F=fordFulkerson(graph,i,parent[i],graph2,min_cut[0]);
            //min_cut[0][0][0]=min_cut[0][0][1]=-2;
            	Gh[i][parent[i]]=Gh[parent[i]][i]=F;
    	for (j=i+1;j<V;j++)
    		{
    		    if (bfs(graph2,i,j,parent2) && parent[j]==parent[i])
    			parent[j]=i;
    		}

    //	printGraph(Gh);
//    	print_v(parent);


    	for (j=0;j<i;j++)
         if(Gh[i][j]==0)
    		Gh[j][i]=Gh[i][j]=min(F,Gh[parent[i]][j]);



    //	printGraph(Gh);
    //	cout<<"----------"<<endl;
    }

    for(i=0;i<V;i++)
    {
        for(j=i+1;j<V;j++)
        {
            if (j!=parent[i] && i!=parent[j])
            {
                Gh[i][j]=Gh[j][i]=0;
            }
        }
    }
    for(i=0;i<V;i++)
    {
        for(j=i+1;j<V;j++)
        {
            if (Gh[i][j]!=0)
            {
                Gh2[i][j]=Gh2[j][i]=ct;
                F=fordFulkerson(graph,i,j,graph2,min_cut[ct]);
                ct++;
            }
        }
    }
    for(i=ct;i<V-1;i++)
    min_cut[i][0][0]=min_cut[i][0][1]=-2;
}

void myGusfield(GraphNew * graph_ptr, vector< vector<int> >& graph, vector< vector<int> >& EdgeID, vector< vector<int> >& Gh, vector< vector<int> >& Gh2, vector< vector< vector<int> > >& min_cut, vector<int>& parent, vector< vector <set<int> > >& minCutEdgeIds)
    { //for undirected graphs

    int i,j,F,ct=0;
    
//    for(i=0;i<V;i++) {
//        for(j=0;j<V;j++) {
//                Gh[i][j]=Gh2[i][j]=0;
//        }
//    }

    vector<int> parent2(V);
    vector< vector<int> > graph2(V, vector<int>(V));

    for (int t=1;t<V;t++) {
        if(EdgeID[0][t] != 0)
                minCutEdgeIds[0][t].insert(EdgeID[0][t]);
    }
    
    //Replace graph everywhere with undirectedGraph and later on check whether the edges are directed from source to target
    for(i=1;i<V;i++){
        
    	//Compute the minimum cut between i and parent[i].
    	//Let the i-side of the min cut be S, and the value of the min-cut be F
        F=myfordFulkerson(graph_ptr, graph, EdgeID, i,parent[i],graph2,min_cut[0], minCutEdgeIds);
        

    	for (j=i+1;j<V;j++)
        {
            if (bfs(graph2,i,j,parent2) && parent[j]==parent[i]){
                parent[j]=i;
                //minCutEdgeIds[j][parent[i]] = minCutEdgeIds[i][parent[i]];
                }
        }
        
        Gh[i][parent[i]]=Gh[parent[i]][i]=F;
        
    	for (j=0;j<i;j++) {
            Gh[j][i]=Gh[i][j]=min(F,Gh[parent[i]][j]);
            
            set<int>cut_edges;
            if(F < Gh[parent[i]][j]) {
                cut_edges = minCutEdgeIds[i][parent[i]];
                
            }
            else {
                cut_edges = minCutEdgeIds[parent[i]][j];
                
            }

            for(set<int>::iterator itr = cut_edges.begin(); itr != cut_edges.end(); itr++) {
                minCutEdgeIds[j][i].insert(*itr);
                minCutEdgeIds[i][j].insert(*itr);
                
            }
            
            
        }

    }

}
    
    
void minCut(vector< vector<int> >& graph, vector< vector<int> >& EdgeID, vector< vector< vector<int> > >& minCutPointer, vector< vector<int> >& Gh)
{  //for both directed and undirected graphs

    vector< vector<int> > undirectedGraph(V, vector<int>(V));
    int i,j;
    for(i=0;i<V;i++)
    {
        for(j=i+1;j<V;j++)
        {
            if(graph[i][j]==graph[j][i])
            {
                if(graph[i][j]==0)
                undirectedGraph[i][j]=undirectedGraph[j][i]=0;
                if(graph[i][j]!=0)
                undirectedGraph[i][j]=undirectedGraph[j][i]=graph[i][j];
            }
            else
            {
               if(graph[i][j]!=0)
               {
                  undirectedGraph[i][j]=undirectedGraph[j][i]=graph[i][j];
               }
                if(graph[j][i]!=0)
               {
                  undirectedGraph[i][j]=undirectedGraph[j][i]=graph[j][i];
               }

            }
        }
       undirectedGraph[i][i]=0;
       //minCutPointer[i][i][0]=-2;
    }
    vector<int> parent(V),parent2(V);
    vector< vector< vector<int> > > min_cut(V-1, vector< vector<int> >(E, vector<int>(2)));
    vector< vector<int> > Gh2(V, vector<int>(V)),graph2(V, vector<int>(V));
    int k,v,u,n1,n2,ct,prevct;
    int a,b,m,n;
    vector<int> minCutPointerTemp(E);
    vector< vector<int> > temp(V, vector<int>(2));
    Gusfield(undirectedGraph,Gh,Gh2,min_cut,parent); //Gh is the Gomory Hu Tree for the Undirected Case
    
    for(i=0;i<V;i++)
    {
        for(j=0;j<V;j++)
        {
             if((i!=j) && bfs(graph,i,j,parent2))
            {
                fordFulkersonTree(Gh,i,j,temp);
                prevct=INT_MAX;
                minCutPointer[i][j][0]=-2;
                for(m=0;m>-1;m++)
                 {
                     if(temp[m][0]!=-2)
                     {
                         ct=0;
                         u=temp[m][0];
                         v=temp[m][1];
                         for(a=0;a<V;a++)
                         for(b=0;b<V;b++)
                         graph2[a][b]=graph[a][b];

                         for(k=0;k>-1;k++)
                         {
                           if(min_cut[Gh2[u][v]][k][0]>-1)
                           {
                             graph2[min_cut[Gh2[u][v]][k][0]][min_cut[Gh2[u][v]][k][1]]=graph2[min_cut[Gh2[u][v]][k][1]][min_cut[Gh2[u][v]][k][0]]=0;
                           }
                            if(min_cut[Gh2[u][v]][k][0]==-2)
                            k=-2;
                         }

                        ct=0;
                        for(k=0;k>-1;k++)
                        {
                          if(min_cut[Gh2[u][v]][k][0]>-1)
                          {
                            n1=min_cut[Gh2[u][v]][k][0];
                            n2=min_cut[Gh2[u][v]][k][1];
                             if(graph[n1][n2]!=0)
                               {
                                  graph2[n1][n2]=graph[n1][n2];
                                  if(bfs(graph2,i,n1,parent2) && bfs(graph2,n2,j,parent2)) //NOT absolutely sure of this part
                                  {
                                    minCutPointerTemp[ct]=EdgeID[n1][n2];
                                    ct++;
                                  }
                                graph2[n1][n2]=0;
                               }
                             if(graph[n2][n1]!=0)
                              {
                                 graph2[n2][n1]=graph[n2][n1];
                                 if(bfs(graph2,i,n2,parent2) && bfs(graph2,n1,j,parent2)) //NOT absolutely sure of this part
                                 {
                                   minCutPointerTemp[ct]=EdgeID[n2][n1];
                                   ct++;
                                 }
                               graph2[n2][n1]=0;
                             }
                            }
                            if(min_cut[Gh2[u][v]][k][0]==-2)
                             {
                               k=-2;
                             }
                           }
                           minCutPointerTemp[ct]=-2;
                           if(ct<prevct && ct!=0)
                           {
                               minCutPointer[i][j].resize(ct+1);
                               for(n=0;n<=ct;n++)
                               {
                                   minCutPointer[i][j][n]=minCutPointerTemp[n];
                               }
                               prevct=ct;
                           }
                        }
                    else
                    m=-2;
             }
          }
        else
        minCutPointer[i][j][0]=-2;

        }

    }
  }


void myminCut(GraphNew * graph_ptr, vector< vector<int> >& graph, vector< vector<int> >& EdgeID, vector< vector< vector<int> > >& minCutPointer, vector< vector<int> >& Gh, vector< vector <set<int> > >& minCutEdgeIds)
{  //for both directed and undirected graphs

    vector< vector<int> > undirectedGraph(V, vector<int>(V,0));
    int i,j;
    for(i=0; i<V; i++) {
        for(j=i+1; j<V; j++) {
            undirectedGraph[i][j] = undirectedGraph[j][i] = max(graph[i][j], graph[j][i]);

        }
    }
    vector<int> parent(V,0),parent2(V,0);
    vector< vector< vector<int> > > min_cut(V-1, vector< vector<int> >(E, vector<int>(2)));
    vector< vector<int> > Gh2(V, vector<int>(V,0));
    
    myGusfield(graph_ptr, undirectedGraph, EdgeID, Gh,Gh2,min_cut,parent, minCutEdgeIds); //Gh is the Gomory Hu Tree for the Undirected Case
    
  }




void GHTree(GraphNew * graph, vector< vector <vector<int> > >& minCutPointer) //presumably V defined as 2000
 {
        /*minCutPointer stores the Edge Ids of the Min-Cut Edges between all the pairs of nodes and has to be defined as:
        vector< vector <vector<int> > > minCutPointer(V, vector< vector<int> >(V, vector<int>(1))); in the calling function*/
        std::vector<int> node_id_set = graph->get_node_ids();
        V=node_id_set.size();
        std::vector<int> edge_id_set = graph->get_edge_ids();
        E=2*edge_id_set.size();
        std::vector<int>::iterator i;
        int a,b,j,k;
        vector < vector<int> > G(V, vector<int>(V)),EdgeID(V, vector<int>(V));
        for(j=0;j<V;j++)
        for(k=0;k<V;k++)
        G[j][k]=0;
	for(i = edge_id_set.begin(); i != edge_id_set.end(); i++) 
        {
            EdgeNew * e=graph->get_edge_from_eid(*i);
            a=e->get_source();
            b=e->get_target();
            G[a-1][b-1] = 1; 
            EdgeID[a-1][b-1] = *i;
        }
        vector< vector<int> > Gh(V, vector<int>(V));
        minCut(G,EdgeID,minCutPointer,Gh);
        //Note that Gh is the Gomory Hu Tree for the UNDIRECTED graph
}


void myGHTree(GraphNew * graph, vector< vector <vector<int> > >& minCutPointer, vector< vector <set<int> > >& minCutEdgeIds) //presumably V defined as 2000
 {
        /*minCutPointer stores the Edge Ids of the Min-Cut Edges between all the pairs of nodes and has to be defined as:
        vector< vector <vector<int> > > minCutPointer(V, vector< vector<int> >(V, vector<int>(1))); in the calling function*/
        std::vector<int> node_id_set = graph->get_node_ids();
        V=node_id_set.size();
        std::vector<int> edge_id_set = graph->get_edge_ids();
        E=2*edge_id_set.size();
        std::vector<int>::iterator i;
        int a,b,j,k;
        vector < vector<int> > G(V, vector<int>(V,0));
        vector < vector<int> > EdgeID(V, vector<int>(V));
        
	for(i = edge_id_set.begin(); i != edge_id_set.end(); i++) 
        {
            EdgeNew * e=graph->get_edge_from_eid(*i);
            a=e->get_source();
            b=e->get_target();
            G[a-1][b-1] = 1; 
            EdgeID[a-1][b-1] = *i;
        }
#ifdef DEBUG_FLAG
        debug_log << "EdgeID: " << endl;
        for (int i=0; i<V; i++) {
            for (int j=0; j<V; j++) {
                debug_log << "Edge Id(s) from " << i << " to " << j << ": " << EdgeID[i][j] << " " << endl;
            }
            debug_log << endl;
        }
        
#endif         
        vector< vector<int> > Gh(V, vector<int>(V,inf));
        myminCut(graph,G,EdgeID,minCutPointer,Gh, minCutEdgeIds);
        //Note that Gh is the Gomory Hu Tree for the UNDIRECTED graph
        
#ifdef DEBUG_FLAG
        debug_log << "minCutEdgeIds: " << endl;
        for (int i=0; i<V; i++) {
            for (int j=0; j<V; j++) {
                debug_log << "minCutEdgeIds(s) from " << i << " to " << j << ": ";
                debug_log << "{ ";
                set<int> curr_min_cut_edges = minCutEdgeIds[i][j];
                for (set<int>::iterator set_iter = curr_min_cut_edges.begin(); set_iter != curr_min_cut_edges.end(); set_iter++) {
                        debug_log << *set_iter << " ";
                }
                debug_log << "}" << endl;
            }
            debug_log << endl;
        }
        
#endif        
        
}
// Rudrajit's min-cut code -- edited by sukanya -- ends here



 
 
 void getMinCutMatrix_dummy(GraphNew * graph, vector< vector <vector<int> > >& minCutMatrix) {

        int v = graph->get_node_ids().size();
        int e = graph->get_edge_ids().size();
        
        for(int i=0; i<v; i++){
            for(int j=0; j<v; j++){
                int outlist_size, inlist_size;
                outlist_size = graph->get_outlist(i+1).size();
                inlist_size = graph->get_inlist(j+1).size();
                if (inlist_size < outlist_size)
                    minCutMatrix[i][j] = graph->get_inlist(j+1);
                else
                    minCutMatrix[i][j] = graph->get_outlist(i+1);
            }
        }
        
}


//constraint solver CNF related global data
int idx = 1, clauses = 0; //clauses is num of clauses, idx stores the number that is to be given to the next variable, thus idx-1 is the total variables
map<string, t_Expression *> getExpressionMap;//maps variable name as string to expression as t_Expression*
map<t_Expression*, pair<string,int> > getCNFIndexMap;//maps expression as t_Expression* to a pair<variable name as string, internal variable index in CNF format>

void read_z3_output(string inp_filename,string out_filename);
vector<int> add_cnf(vector<int> u,vector<int> v,int& idx,int & clauses,ofstream& fout_CNF_file);
string buildRelationalExpression(string reader, t_ExpressionManager* em, t_Expression* &expr);
string buildArithmaticExpression(string reader, t_ExpressionManager* em, t_Expression* &expr);
string buildSelectExpression(string reader,t_ExpressionManager* em,t_Expression* &expr);
string buildConcatExpression(string reader, t_ExpressionManager *em, t_Expression* &expr);
string buildBracketExpression(string reader, t_ExpressionManager *em, t_Expression* &expr);
string buildVariableExpression(string reader, t_ExpressionManager *em, t_Expression* &expr);

t_Expression* build_OR_constraints(string s1,string s2,int a1,int a2, t_ExpressionManager *em);
t_Expression* build_AND_constraints(string s1,string s2,int a1,int a2, t_ExpressionManager *em);
t_Expression* build_OR_constraints(t_Expression* e1,t_Expression* e2,int a1,int a2, t_ExpressionManager *em);
t_Expression* build_AND_constraints(t_Expression* e1,t_Expression* e2,int a1,int a2, t_ExpressionManager *em);


bool nonempty_intersection_files(string file1, string file2){
	string command1 = "sort -u " + file1 + " > temp-in-1";
	string command2 = "sort -u " + file2 + " > temp-in-2";
	string command3 = "sort temp-in-1 temp-in-2 | uniq -d | wc -l > temp-in-3";

	string command = command1 + " ; " + command2 + " ; " + command3;
	system(command.c_str());

	ifstream  fin("temp-in-3");

	if(!fin){
		cerr << "Error: can't open file temp-in-3 file for reading, exiting ..." << endl;
		exit(1);
	}
	else{
		int count;
		fin >> count;
		fin.close();
		system("rm temp-in-1 temp-in-2 temp-in-3");
		if(count == 0){
			return false;
		}
		return true;
	}
}

void read_z3_output(string inp_filename,string out_filename) {
	ifstream fin(inp_filename.c_str());
	ofstream fout(out_filename.c_str(),ofstream::out);

	string s;

	fin>>s;

	if(s!="sat") {
		fout<<"Solution is not sat\n";
		return;
	}

	int num;

	map<int,bool> soln_map;

	while(fin>>num) {
		if(num>0)
			soln_map[num]=true;
		else
			soln_map[-num]=false;
	}

	for(map<t_Expression *,pair<string,int> >::iterator it=getCNFIndexMap.begin();it!=getCNFIndexMap.end();it++) {
		int var_idx=(it->second).second;
		if(var_idx!=-1) {		
			fout<<(it->second).first;
			if(soln_map[var_idx]==true)
				fout<<" true\n";
			else
				fout<<" false\n";
		}
	}

}

//the bit sequences represented by u and v are added and there sum is represented by the returned vector
//this summing up of u and v and its outcome is written as constraints in the fout_CNF_file
vector<int> add_cnf(vector<int> u,vector<int> v,int& idx,int &clauses,ofstream& fout_CNF_file) {
	vector<int> r;
	fout_CNF_file<<-idx<<" 0\n";
	clauses++;

	int carry=idx;
	idx++;

	for(int i=0;i<u.size();i++) {
		vector<int> v1;
		v1.push_back(u[i]);
		v1.push_back(v[i]);
		v1.push_back(carry);
		r.push_back(idx++);
		carry=idx++;
		for(int a=0;a<8;a++) {
			int mask=a;
			// fout<<mask<<" --- mask\n";
			int sum=0;
			for(int j=0;j<3;j++) {
				sum+=mask%2;
				if(mask%2==0)
					fout_CNF_file<<v1[j]<<" ";
				else
					fout_CNF_file<<-v1[j]<<" ";

				mask/=2;
			}

			if(sum%2==0)
				fout_CNF_file<<-r[i]<<" 0\n";
			else
				fout_CNF_file<<r[i]<<" 0\n";
			clauses++;

			mask=a;
			sum=0;
			for(int j=0;j<3;j++) {
				sum+=mask%2;
				if(mask%2==0)
					fout_CNF_file<<v1[j]<<" ";
				else
					fout_CNF_file<<-v1[j]<<" ";

				mask/=2;
			}

			if(sum>1)
				fout_CNF_file<<carry<<" 0\n";
			else
				fout_CNF_file<<-carry<<" 0\n";
			clauses++;

		}
	}
	r.push_back(carry);

	/*	fout<<"bro\n";
	for(int i=0;i<r.size();i++)
		fout<<r[i]<<" ";
	fout<<endl;
	 */

	return r;
}


string buildExpression(string reader, t_ExpressionManager* em, t_Expression* &expr)
{
	//check for relational operators
	t_Expression* first;
	trimInPlace(reader);
	//assert( ! reader.empty());
	if(reader.empty())
	{
		cout<<"empty reader"<<endl;
		expr = NULL;
		return reader;
	}
	reader = buildRelationalExpression(reader, em, first);

	trimInPlace(reader);

	if (reader.empty())
	{
		expr = first;
		return reader;
	}
	string oper = reader.substr(0, 2);
	if (oper == "&&" || oper == "||")
		reader = reader.substr(2);
	else
	{
		expr = first;
		return reader;
	}

	while (oper == "&&" || oper == "||")
	{
		t_Expression* second;
		reader = buildRelationalExpression(reader, em, second);
		if (second == NULL)
			break;
		string label;
		if (oper == "&&")
		{
			label = em->m_operatorLabelLogAND;
		} else if (oper == "||")
			label = em->m_operatorLabelLogOR;

		first = em->createOneBitExpressionWithTwoOperands(label, first, second);
		trimInPlace(reader);

		if (reader.empty())
			break;
		oper = reader.substr(0, 2);
		reader = reader.substr(2);
	}

	expr = first;
	return reader;
}

string buildRelationalExpression(string reader, t_ExpressionManager* em, t_Expression* &expr)
{
	t_Expression* first;
	reader = buildArithmaticExpression(reader, em, first);

	trimInPlace(reader);

	if (reader.empty())
	{
		expr = first;
		return reader;
	}
	string oper = reader.substr(0, 2);
	string label;
	if (oper == ">=")
	{
		label = em->m_operatorLabelGreaterThanOrEqual;
		reader = reader.substr(2);
	} else if (oper == "<=")
	{
		label = em->m_operatorLabelLessThanOrEqual;
		reader = reader.substr(2);
	} else if (oper == "==")
	{
		label = em->m_operatorLabelLogicalEquality;
		reader = reader.substr(2);
	} else
	{
		oper = reader.substr(0, 1);
		if (oper == "<")
		{
			label = em->m_operatorLabelLessThan;
			reader = reader.substr(1);
		} else if (oper == ">")
		{
			label = em->m_operatorLabelGreaterThan;
			reader = reader.substr(1);
		} else //no relational operator
		{
			expr = first;
			return reader;
		}
	}

	trimInPlace(reader);

	t_Expression* second;
	reader = buildArithmaticExpression(reader, em, second);
	if (second == NULL)
	{
		expr = first;
		//cout<<"No second operand found."<<endl;
		return reader;
	}
	//    if(0)
	//    {
	//        ofstream out("ParsedExpression", ios::app);
	//        out<<"Testing: operands of "<<label<<endl;
	//        string name = "first";
	//        em->printExpressionToFileAsDAG(name, first, &out);
	//        name = "second";
	//        em->printExpressionToFileAsDAG(name, second, &out);
	//        out<<endl;
	//    }

	expr = em->createOneBitExpressionWithTwoOperands(label, first, second);

	//TypeOfExpressionTuple type_info = {TYPE_BOOL, 1};
	//vector<t_Expression*> vec_operands = t_ExpressionManager::buildVector(first,second);
	//expr = em->createExpression(label, vec_operands, type_info);

	return reader;
}

string buildArithmaticExpression(string reader, t_ExpressionManager* em, t_Expression* &expr)
{
	t_Expression* first;
	reader = buildSelectExpression(reader, em, first);

	trimInPlace(reader);
	if (reader.empty())
	{
		expr = first;
		return reader;
	}
	string oper = reader.substr(0, 1);
	if (oper == "+" || oper == "-" || oper == "*" || oper == "/" || oper == "%")
	{
		//reader = reader.substr(1);
	} else
	{
		expr = first;
		return reader;
	}

	while (oper == "+" || oper == "-" || oper == "*" || oper == "/" || oper == "%")
	{
		reader = reader.substr(1); //skip the operator
		string label;
		if (oper == "+")
			label = em->m_operatorLabelADD;
		else if (oper == "-")
			label = em->m_operatorLabelSUB;
		else if (oper == "*")
			label = em->m_operatorLabelMultiply;
		else if (oper == "/")
			label = em->m_operatorLabelDivide;
		else if (oper == "%")
			label = em->m_operatorLabelModulus;

		t_Expression* second;
		reader = buildSelectExpression(reader, em, second);
		if (second == NULL)
			break;

		vector<t_Expression*> vec_operands = t_ExpressionManager::buildVector(first, second);
		int wid = max( em->getWidth(first), em->getWidth(second) );
		//TypeOfExpressionTuple operatorType = {TYPE_UNSIGNED_INTEGER, -1};
		TypeOfExpressionTuple operatorType = {TYPE_UNSIGNED_BITVECTOR, wid};
		first = em->createExpression(label, vec_operands, operatorType);

		trimInPlace(reader);
		if (reader.empty())
			break;
		oper = reader.substr(0,1);
	}

	expr = first;
	return reader;
}

string buildSelectExpression(string reader,t_ExpressionManager* em,t_Expression* &expr)
{
	t_Expression* first;
	reader = buildConcatExpression(reader, em, first);
	trimInPlace(reader);

	if(reader.empty())
	{
		expr = first;
		return reader;
	}
	string oper = reader.substr(0, 1);
	bool foundSelect = false;
	if (oper == "[")
	{
		foundSelect = true;
	} else
	{
		expr = first;
		return reader;
	}

#ifdef OLD
	if(foundSelect)
	{
		reader = reader.substr(1);
		t_Expression* ub_expr;
		t_Expression* lb_expr;
		reader = buildConcatExpression(reader, em, ub_expr);
		trimInPlace(reader);

		assert(ub_expr != NULL);
		assert(!reader.empty() && reader[0]==':');

		reader = reader.substr(1);
		int ub = em->getConstantValuePresentInExpression(ub_expr);


		reader = buildConcatExpression(reader,em,lb_expr);
		assert(lb_expr != NULL);
		int lb = em->getConstantValuePresentInExpression(lb_expr);

		int width = ub-lb+1;
		vector<t_Expression*> operands = t_ExpressionManager::buildVector(first,lb_expr,ub_expr);
		string label = "select";

		TypeOfExpressionTuple operatorType = {TYPE_UNSIGNED_BITVECTOR, width};
		expr = em->createExpression(label,operands,operatorType);

		assert(reader[0] == ']');
		reader = reader.substr(1);

		return reader;
	}
	else
	{
		assert(false);
	}
#else
	int openidx = reader.find('[');
	int mididx = reader.find(':');
	int closeidx = reader.find(']');

	assert(openidx>=0 && openidx<mididx);
	assert(mididx < closeidx);
	assert(closeidx <= reader.length());

	string ubStr = reader.substr(openidx+1, mididx-openidx-1);
	string lbStr = reader.substr(mididx+1, closeidx-mididx-1);

	TypeOfExpressionTuple constIntType = {TYPE_UNSIGNED_INTEGER, -1};
	vector<t_Expression*> operands(3,NULL);
	operands[0] = first;
	operands[1] = em->createConstant(lbStr, constIntType);
	operands[2] = em->createConstant(ubStr, constIntType);

	assert(operands[1]!= NULL);
	assert(operands[2]!= NULL);

	int ub = stringToInteger(ubStr);
	int lb = stringToInteger(lbStr);
	assert(lb>=0);
	assert(ub>=lb);
	TypeOfExpressionTuple typ = {TYPE_UNSIGNED_BITVECTOR, ub-lb+1};
	string label = "select";
	expr = em->createExpression(label, operands, typ);

	return reader.substr(closeidx+1);   //done forget to eat away the expression just parsed
#endif
}

string buildConcatExpression(string reader, t_ExpressionManager *em, t_Expression* &expr)
{
	trimInPlace(reader);
	assert(! reader.empty());
	if(reader[0] != '{')
	{
		return buildBracketExpression(reader, em, expr);
	}

	vector<t_Expression*> operands;
	int width = 0;
	while(reader[0] != '}')
	{
		reader = reader.substr(1);      //eat away the '{' ',' '}'
		t_Expression *e;
		reader = buildBracketExpression(reader, em, e);
		assert(e != NULL);
		assert( ! reader.empty());      //'}' should be left in the string
		operands.push_back(e);
		width += em->getWidth(e);
		assert(reader[0] ==',' || reader[0] == '}');
	}

	assert(operands.empty() == false);
	{
		string label = "concat";
		TypeOfExpressionTuple type = {TYPE_UNSIGNED_BITVECTOR, width};
		expr = em-> createExpression(label, operands, type);
	}

	return reader.substr(1);    //eat away the '}'
}

string buildBracketExpression(string reader, t_ExpressionManager *em, t_Expression* &expr)
{
	trimInPlace(reader);
	bool negate = false;
	if (reader[0] == '!')
	{
		negate = true;
		reader = reader.substr(1);
		trimInPlace(reader);
	}

	t_Expression* first;

	if (reader[0] == '(')
	{
		//cout << "'(' found.:" << reader << endl;

		int brackCount = 1, i;
		for (i = 1; i < reader.size() && brackCount > 0; ++i)
		{
			if (reader[i] == ')')
				brackCount--;
			else if (reader[i] == '(')
				brackCount++;

			if (brackCount == 0)
				break;
		}
		//Now i is the position of corresponding ')'
		string insideBracket = reader.substr(1, i - 1);
		reader = reader.substr(i + 1);

		//cout << "InsideBracket:" << insideBracket << endl;
		insideBracket = buildExpression(insideBracket, em, first);
		trimInPlace(insideBracket);

		if (insideBracket.empty() == false)
		{
			cout << "ERROR: Incomplete expression found:"<<insideBracket << endl;
		}
		//cout << "Expression with bracket parsed. Remaining:" << reader << endl;

		trimInPlace(reader);

	} else
	{
		reader = buildVariableExpression(reader, em, first);
	}

	if (negate)
	{
		expr = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, first);
	} else
		expr = first;

	return reader;
}

string buildVariableExpression(string reader, t_ExpressionManager *em, t_Expression* &expr)
{
	trimInPlace(reader);
	//cout << "BuildVariable:" << reader << endl;
	if (reader[0] == ')')
	{
		//cout << "Closing parenthesis found" << endl;
		expr = NULL;
		return reader;
	}
	string operand;
	int i = 0;
	static string specialChars("_$.#@");
	for (int i = 0; i < reader.size(); ++i)
	{
		char c = reader[i];
		if (isalnum(c) || specialChars.find(c)!=string::npos)
		{
			operand += c;
		} else
			break;
	}

	//const char* strPtr = operand.c_str();
	//char* end;
	//long val = strtol(strPtr, &end, 0);
	//if (end == strPtr)
	reader = reader.substr(operand.size() );
	if( ! isdigit((char)operand[0]) )
	{
		//Not a number'
		int width, ub, lb;
		string symName = operand;
		cout<<"Processing "<<operand<<endl;
		operand = extractNumberAtEnd(operand, lb);
		operand = extractNumberAtEnd(operand, ub);
		width = ub-lb+1;
		assert(width>0);
		TypeOfExpressionTuple symbolType = {TYPE_UNSIGNED_BITVECTOR, width};
		//cout << "Creating symbol:" << symName << " of size " << width << endl;
		expr = em->createSymbol(symName, symbolType);
	} else
	{
		//cout << "Creating constant:" << operand << " ";

		{
			if(operand.find("0b") != 0) //not starts with 0b
			{
				//integer
				int op;
				if(operand.find("0x") == 0)
				{
					istringstream ss(operand);
					ss>>op;
				}
				else
				{
					op= stringToInteger(operand);
				}

				string bitv = integerToBinaryString(op);
				TypeOfExpressionTuple constantType = {TYPE_UNSIGNED_BITVECTOR, (int)bitv.size()}; // .size() returns size_t - typecasted to int by sukanya on 26 Feb 2016
				expr = em->createConstant(bitv, constantType);
				//cout<<"type integer to "<<bitv.size()<<" bit bitvector"<<endl;
			}
			else{
				operand = operand.substr(2);
				TypeOfExpressionTuple constantType = {TYPE_UNSIGNED_BITVECTOR, (int)operand.size()}; // .size() returns size_t - typecasted to int by sukanya on 26 Feb 2016
				expr = em->createConstant(operand, constantType);
				//cout<<"type bitvector"<<endl;
			}
		}
		//TypeOfExpressionTuple constantType = {TYPE_UNSIGNED_BITVECTOR, operand.size()};
		//expr = em->createConstant(operand, constantType);
	}

	//reader = reader.substr(operand.size()); //skip the operand
	//cout<<"  Remaining:"<<reader<<endl;
	return reader;
}

// function added by sai krishna
// s1 = (not e1) if a1==1
// s2 = (not e2) if a2==1
// retuns the expression (or e1 e2)

t_Expression* build_OR_constraints(t_Expression* e1,t_Expression* e2,int a1,int a2, t_ExpressionManager *em) {
	TypeOfExpressionTuple te= {TYPE_UNSIGNED_BITVECTOR, 1};


	vector<t_Expression *> v;

	if(a1==1)
		e1 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, e1);

	if(a2==1)
		e2 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, e2);

	v.push_back(e1);
	v.push_back(e2);

	return em->createExpression(em->m_operatorLabelLogOR, v,te);

}

// function added by sai krishna
// s1 = (not e1) if a1==1
// s2 = (not e2) if a2==1
// retuns the expression (and e1 e2)

t_Expression* build_AND_constraints(t_Expression* e1,t_Expression* e2,int a1,int a2, t_ExpressionManager *em) {
	TypeOfExpressionTuple te= {TYPE_UNSIGNED_BITVECTOR, 1};


	vector<t_Expression *> v;

	if(a1==1)
		e1 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, e1);

	if(a2==1)
		e2 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, e2);

	v.push_back(e1);
	v.push_back(e2);


	return em->createExpression(em->m_operatorLabelLogAND, v,te);

}


// function added by sai krishna
// s1 = (not s1) if a1==1
// s2 = (not s2) if a2==1
// retuns the expression (or s1 s2)

t_Expression* build_OR_constraints(string s1,string s2,int a1,int a2, t_ExpressionManager *em) {
	TypeOfExpressionTuple te= {TYPE_UNSIGNED_BITVECTOR, 1};

	if(getExpressionMap.find(s1)==getExpressionMap.end() || getExpressionMap.find(s2)==getExpressionMap.end() ) {
		cout<<"no....!!! "<<s1<< " "<<s2<<endl;
		// shoudl add some cerror here later
		return NULL;
	}

	t_Expression *expr1,*expr2;
	vector<t_Expression *> v;

	expr1=getExpressionMap[s1];
	expr2=getExpressionMap[s2];

	if(a1==1)
		expr1 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, expr1);

	if(a2==1)
		expr2 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, expr2);

	v.push_back(expr1);
	v.push_back(expr2);


	return em->createExpression(em->m_operatorLabelLogOR, v,te);

}

// function added by sai krishna
// s1 = (not s1) if a1==1
// s2 = (not s2) if a2==1
// returns the expression (and s1 s2)

t_Expression* build_AND_constraints(string s1,string s2,int a1,int a2, t_ExpressionManager *em) {
	TypeOfExpressionTuple te= {TYPE_UNSIGNED_BITVECTOR, 1};

	if(getExpressionMap.find(s1)==getExpressionMap.end() || getExpressionMap.find(s2)==getExpressionMap.end() ) {
		cout<<"no....!!! "<<s1<< " "<<s2<<endl;
		// shoudl add some cerror here later
		return NULL;
	}

	t_Expression *expr1,*expr2;
	vector<t_Expression *> v;

	expr1=getExpressionMap[s1];
	expr2=getExpressionMap[s2];

	if(a1==1)
		expr1 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, expr1);

	if(a2==1)
		expr2 = em->createOneBitExpressionWithOneOperand(em->m_operatorLabelLogNOT, expr2);

	v.push_back(expr1);
	v.push_back(expr2);


	return em->createExpression(em->m_operatorLabelLogAND, v,te);

}

t_Expression* build_XOR_constraints(t_Expression* a,t_Expression* b, t_ExpressionManager *em){
	//a XOR b = ab' + a'b
	t_Expression* a_and_b_prime = build_AND_constraints(a, b, 0, 1, em);
	t_Expression* a_prime_and_b = build_AND_constraints(a, b, 1, 0, em);
	return build_OR_constraints(a_and_b_prime, a_prime_and_b, 0, 0, em);
}

t_Expression* build_EQUIV_constraint(t_Expression* a,t_Expression* b, t_ExpressionManager *em){
	// a EQUIV b = a implies b AND b implies A
	t_Expression* a_implies_b = build_OR_constraints(a, b, 1, 0, em);
	t_Expression* b_implies_a = build_OR_constraints(b, a, 1, 0, em);
	return build_AND_constraints(a_implies_b, b_implies_a, 0, 0, em);
}

t_Expression* build_AND_constraints_from_vector(vector<t_Expression*>& vec, t_ExpressionManager *em){
	t_Expression* to_return = NULL;
	TypeOfExpressionTuple te= {TYPE_UNSIGNED_BITVECTOR, 1};
	if(!vec.empty()){
		to_return = em->createExpression(em->m_operatorLabelLogAND, vec, te);
	}

	/*if(!vec.empty()){
		to_return = vec[0];
		for(vector<t_Expression*>::iterator itr = vec.begin()+1; itr != vec.end(); ++itr){
			t_Expression* temp = build_AND_constraints(to_return, *itr, 0, 0, em);
			to_return = temp;
		}
	}
	else{
		cerr << "vec is empty" << endl;
	}
	if(!to_return){
		cerr << "Error: returning NULL" << endl;
	}*/
	return to_return;
}

t_Expression* build_OR_constraints_from_vector(vector<t_Expression*>& vec, t_ExpressionManager *em){
	t_Expression* to_return = NULL;
	TypeOfExpressionTuple te= {TYPE_UNSIGNED_BITVECTOR, 1};
	if(!vec.empty()){
		to_return = em->createExpression(em->m_operatorLabelLogOR, vec, te);
	}

	/*if(!vec.empty()){
		to_return = vec[0];
		for(vector<t_Expression*>::iterator itr = vec.begin()+1; itr != vec.end(); ++itr){
			t_Expression* temp = build_OR_constraints(to_return, *itr, 0, 0, em);
			to_return = temp;
		}
	}
	else{
		cerr << "vec is empty" << endl;
	}
	if(!to_return){
		cerr << "Error: returning NULL" << endl;
	}*/
	return to_return;
}

t_Expression* get_expression_sequence_positive_constraint(vector<t_Expression*>& nid_dist_variable_seq, t_ExpressionManager *em){
	t_Expression* to_return = build_OR_constraints_from_vector(nid_dist_variable_seq, em);
	//if the value is non zero then atleast one bit should be True, thus ORing all bits must give True
	return to_return;
}


#define SUBTRACT_MODE 0
#define INTERSECT_MODE 1
#define MERGE_MODE 2
//#define DOTPATHSTRING "/usr/local/bin/dot"
#define NUM_KEGG_FILES 39

int Node::id_counter = 1;
int Graph::graph_id_counter = 1;

extern int yyparse();
extern FILE *yyin;
extern list<rule_t* > list_of_rules;

void display_commands(){
	cout << endl;
	cout << "Commands and usage: " << endl;
	cout << endl;
	cout << "help \t(or h) " << endl;
	cout << "read_graph_xml \t(or rgx) \t<filename>" << endl;
        cout << "read_graph_sbml (or rgs) \t<filename>" << endl;
	cout << "read_graph_ppin (or rgp) \t<filename>" << endl;
        cout << "read_graph_htri (or rgh) \t<filename>" << endl;
        cout << "read_graph_reactome_all (or rgr) \t<filename>" << endl;
        cout << "read_phenotypes (or pheno)" << endl;
	cout << "size \t(or sz) \t<graph-id>" << endl;
	cout << "find_intersecting_nodes (or fin) \t<graph-id1> \t<graph-id2>" << endl;
	cout << "find_node \t(or fn)" << endl;
	cout << "display \t(or dis) \t<graph-id>" << endl;
	cout << "reach \t(or rch) \t<graph-id>" << endl;
	cout << "print_graph_info \t(or pgi) \t<graph-id>" << endl;
	cout << "print_graph_man_info \t(or pgmi) " << endl;
	cout << "merge \t(or mg)" << endl;
	cout << "merge_from_file (or mff) \t<filename>" << endl;
	cout << "genesis \t(or gen) \t<graph-id>" << endl;
	cout << "write_graph_xml (or wgx) \t<graph-id>" << endl;
	cout << "read_rules \t(or rr) \t<rule_file>" << endl;
	cout << "generate_constraints \t(or gencn) " << endl;
        cout << "solve" << endl;
	cout << "print_GErels \t(or pgrel) \t<graph-id>" << endl;
	cout << "subgraph \t\t(or sg) \t<graph-id> \t<file_name>" << endl;
        cout << "select \t(or sel) \t<graph-id> \t<file_name>" << endl;
	cout << "get_assignments_z3 \t(or get_assg_z3) \t<z3_cnf_result_file> \t<output_file>" << endl;
	cout << "get_all_solutions \t(or get_all_solns) " << endl;
        cout << "call_ApproxMC \t\t(or approxmc)" << endl;
	cout << "process_microarr_data \t(or pmd) \t<graph-id>" << endl;
        cout << "add_projected_variables \t(or proj)" << endl;
	cout << "clear_expression_maps \t\t(or clxm)" << endl;
	cout << "fwd_bkwd_rch \t\t\t(or fb_rch) \t<graph-id>" << endl;
	cout << "kegg_consistency_check \t\t(or kcc) \t<graph-id>" << endl;
	cout << "list_pathways_with_nodes \t(or lpwn ) \t<filename>" << endl;
        cout << "mincut \t(or mc) \t<graoh-id>" << endl;
        cout << "connect \t<graph-id>" << endl;
	cout << "clr" << endl;
	cout << "exit" << endl;
	cout << endl;
}

//print a generic map ---useful as there are many maps in the code
template<typename Key, typename Val>
void print_map(map<Key, Val>& map2print){
	for(typename map<Key, Val>::iterator itr = map2print.begin(); itr != map2print.end(); ++itr){
		cout << itr->first << " ==> " << itr->second << endl;
	}
}

std::string concatenate_strings(std::vector<std::string> vec_strings, std::string delim){
	if(vec_strings.empty()){
		return "";
	}
	std::string to_return = *vec_strings.begin();
	for(std::vector<std::string>::iterator itr = vec_strings.begin()+1; itr != vec_strings.end(); itr++){
		to_return = to_return + delim + *itr;
	}
	return to_return;
}

//split string based on a delimiter
set<string> split_string(string name_str, string delim)  {
	set<string> splitted_strings;
	unsigned start_pos = 0;
	while (start_pos < name_str.length()) {
		std::size_t found = name_str.find(delim, start_pos);
		if (found != std::string::npos) {
			splitted_strings.insert(name_str.substr(start_pos, found-start_pos));
			start_pos = found + delim.length();
		}
		else {
			splitted_strings.insert(name_str.substr(start_pos));
			break;
		}
	}
	return splitted_strings;
}

list<string> split_string_into_list(string name_str, string delim)  {
	list<string> list_of_strings;
	unsigned start_pos = 0;
	while (start_pos < name_str.length()) {
		std::size_t found = name_str.find(delim, start_pos);
		if (found != std::string::npos) {
			(list_of_strings).push_back(name_str.substr(start_pos, found-start_pos));
			start_pos = found + delim.length();
		}
		else {
			(list_of_strings).push_back(name_str.substr(start_pos));
			break;
		}
	}
	return list_of_strings;
}

string remove_substr(string base_string, string substr_to_remove){
    std::size_t n = substr_to_remove.length();
    for (std::size_t i = base_string.find(substr_to_remove); i != std::string::npos; i = base_string.find(substr_to_remove)){
        base_string.erase(i, n);
    } 
    return base_string;
}

string IntToString (int num) {
	ostringstream ss;
	ss << num;
	return ss.str();
}

int main()
{
    
        //display_commands();
	const string prompt = "sysbio> ";
	//GraphManager *gm = new GraphManager;
	GraphManagerNew* graph_man = new GraphManagerNew;
	//graph_man->read_edge_type_subtype_map();
	t_ConfigurationOptions config_options("../supporting_files/config.txt");
	t_ExpressionManager *em = new t_ExpressionManager(config_options);

	//Rules* rules_ptr = new Rules; //for storing rules index wise starting at 1
	std::map<unsigned int, std::set<Node*> > map_potential_conflict_nodes;
        
        time_t time1, time2, time3, time4;
	double time_in_sec;
        string command;
	
	//testing--starts
	//graph_man->testing_printexpr_iter(em);
	//testing--ends

        // testing mincut starts
//        int graphnum = graph_man->read_graph_xml_file("../test/test3.xml", false);
//        
//        GraphNew * graph = graph_man->get_graph(graphnum);
//                        
//        vector<int> gomoryhu_parents;
//        vector<int> node_ids = graph->get_node_ids();
//        for (vector<int>::iterator vec_itr = node_ids.begin()+1; vec_itr != node_ids.end(); vec_itr++) { // not setting parent of first node
//            gomoryhu_parents[*vec_itr] = 1;
//        }
//        vector<set<int> > cut_edges;
//
//        time(&time1);
//
//        graph_man->compute_gh_tree(graphnum, gomoryhu_parents, cut_edges);
//
//        time(&time2);
//        cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;
//        
//        
        
        
////        GraphNew * graph = graph_man->get_graph(graphnum);
////        V = graph->get_node_ids().size();
////        E = graph->get_edge_ids().size();
////        cout << "V = " << V << " E = " << E << endl;
////        time(&time1);
////        vector< vector <vector<int> > > minCutPointer(V, vector< vector <int> >(V, vector<int>(1,inf)));
////        vector< vector <set<int> > > minCutEdgeIds(V, vector< set <int> >(V, set<int>()));
////        myGHTree(graph, minCutPointer, minCutEdgeIds);
////        time(&time2);
////        cout << "Computed min-cuts for Graph " << graphnum << endl;
////        cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;        
        // testing mincut ends
        
        // testing -- profiling starts
////        int graphnum = graph_man->read_graph_xml_file("../test/test5.xml", false);
////        GraphNew * graph = graph_man->get_graph(graphnum);
////        V = graph->get_node_ids().size();
////        E = graph->get_edge_ids().size();
////        cout << "V = " << V << " E = " << E << endl;
////        time(&time1);
////        //vector< vector <vector<int> > > minCutPointer(V, vector< vector<int> >(V, vector<int>(1)));
////        vector< vector <vector<int> > > minCutPointer(V, vector< vector <int> >(V, vector<int>(1)));
////        //GHTree(graph, minCutPointer);
////        vector< vector <int> > minCutValues(V, vector<int>(V, inf));
////        vector< vector <vector<int> > > minCutEdgeIds(V, vector< vector <int> >(V, vector<int>(0)));
////        GHTree(graph, minCutEdgeIds);
////        time(&time2);
////        cout << "Computed min-cuts for Graph " << graphnum << endl;
////        cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;
        //set<int> present_edges;
        //graph_man->connect(graph, present_edges, minCutEdgeIds, 1, 6);
////            time(&time1);
////            int graphnum = graph_man->read_graph_from_sbml("apop.sbml");
////            GraphNew * graph = graph_man->get_graph(graphnum);
////            cout << "V = " << graph->get_node_ids().size() << " E = " << graph->get_edge_ids().size() << endl;
////            time(&time2);
////            cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;
        
//        string mapfilename; std::list<int> glist;
//        list<unordered_map<string, set<string> > > map_list;
//        unordered_map<string, set<string> > mapname;
//        mapfilename = "map1.txt";
//        graph_man->read_naming_map(mapfilename, mapname);
//        map_list.push_back(mapname);
//        mapfilename = "map2.txt";
//        graph_man->read_naming_map(mapfilename, mapname);
//        map_list.push_back(mapname);
//        
//        int graph_id = graph_man->read_graph_xml_file("merged_164.xml", true);
//        cout << "Read graph " << graph_id << endl;
        
        
        //string filename = "/home/sysbio/sukanya-cse-git/network_tool/data/Reactome/all_pathways/all_list.txt";
        //string filename = "/home/sysbio/sukanya-cse-git/network_tool/data/Reactome/all_pathways/list100.txt";
        //string filename = "/home/sysbio/harshit-cse-git/network_tool/data/all_hsa_graphs_kegg/hsa_all_path_minus_metabol_path_list.txt";
////        string filename = "test_list.txt";
////        
////        ifstream ifs(filename.c_str());
////        string file;
////        int graph_id;
////        while (getline(ifs, file)) {
////            graph_id = graph_man->read_graph_xml_file(file, true);
////            cout << "Read graph " << graph_id << endl;
////            
////            if (graph_id != -1)
////                    glist.push_back(graph_id);
////        }
////        int merged_graph_id = graph_man->merge_graphs(glist, map_list);
////        if (merged_graph_id == -1)
////                cerr << "No merged graph produced" << endl;
////        else {
////                std::string oper = "Merged graphs from file " + filename;
////                std::list<int> oper_graph_ids;
////                oper_graph_ids.empty();
////
////                graph_man->add_graph_genesis(merged_graph_id, oper, oper_graph_ids);
////                cout << "New graph: " << merged_graph_id << endl;
////                GraphNew * merged_graph = graph_man->get_graph(merged_graph_id);
////                cout << "Total nodes: " << merged_graph->get_node_ids().size() << endl;
////                cout << "Total edges: " << merged_graph->get_edge_ids().size() << endl;
////        }
        
                
////        
////        int graphnum1 = graph_man->read_graph_xml_file("apop.xml", false);
////        int graphnum2 = graph_man->read_graph_from_sbml("apop.sbml");
////        list<int> glist;
////        
////        glist.push_back(graphnum1);
////        glist.push_back(graphnum2);
////        int graph_merged = graph_man->merge_graphs(glist);
////        if (graph_merged == -1)
////                cerr << "No merged graph produced" << endl;
////        else
////                cout << "New graph: " << graph_merged << endl;
//        GraphNew * graph = graph_man->get_graph(graphnum);
//        V = graph->get_node_ids().size();
//        E = graph->get_edge_ids().size();
//        cout << "V = " << V << " E = " << E << endl;
//        time(&time1);
//        vector< vector <vector<int> > > minCutMatrix(V, vector< vector<int> >(V, vector<int>(E,0)));
//        GHTree(graph, minCutMatrix);
//        time(&time2);
//        cout << "Computed min-cuts for Graph " << graphnum << endl;
//        cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;
//        cout << "bfs calls made = " << bfs_call_counter << endl;
        // profiling ends
        
	while(1) {
		cout << prompt;
		cin >> command;

                
                if (command == "read_graph_xml" || command == "rgx") {
			string filename;
			cin >> filename;
                        			
			bool tool_written = false;
			char tool_written_ch;
			cout << "Are you reading a graph that was earlier written by this tool?, 'y' or 'Y' for yes, otherwise any other key: ";
			cin >> tool_written_ch;
			if(tool_written_ch == 'y' || tool_written_ch == 'Y'){
				tool_written = true;
                        }
                        time(&time1);
			int gid = graph_man->read_graph_xml_file(filename, tool_written);
                        time(&time2);
			if(gid != -1){
				cout << "New graph id: " << gid << endl;
			}
                        time_in_sec = difftime(time2,time1);
			cout << "Time taken (in sec): " << time_in_sec << endl;
		}

                else if (command == "create_reactome_map" || command == "rmap"){
                        string filename;
		 	cin >> filename;
		 	graph_man->create_map_from_sbml(filename);
                }
                
                else if (command == "read_graph_sbml" || command == "rgs") {
		 	string filename;
		 	cin >> filename;
		 	int gid = graph_man->read_graph_from_sbml(filename);
                        if(gid != -1){
                                cout << "New graph id: " << gid << endl;
                        }
			
		 }
		
                
		else if (command == "read_graph_ppin" || command == "rgp") {

			string filename;
			cin >> filename;
			time(&time1);
			int gid = graph_man->create_graph_PPIN(filename);
			time(&time2);
			if (gid != -1) {
				cout << "New graph id: " << gid << endl;
			}
			else{
				cerr << "Could not create PPIN" << endl;
			}
			time_in_sec = difftime(time2,time1);
			cout << "Time taken (in sec): " << time_in_sec << endl;
		}

                else if (command == "read_graph_htri" || command == "rgh") {

			string filename;
			cin >> filename;
			int gid = graph_man->create_graph_HTRI(filename);
			if (gid != -1) {
				cout << "New graph id: " << gid << endl;
			}
			else{
				cerr << "Could not create HTRI" << endl;
			}
			
		}
                
                else if (command == "read_graph_reactome_all" || command == "rgr") {

			string filename;
			cin >> filename;
			int gid = graph_man->create_graph_reactome_all(filename);
			if (gid != -1) {
				cout << "New graph id: " << gid << endl;
			}
			else{
				cerr << "Could not create Reactome all interactions network" << endl;
			}
			
		}
                
                else if (command == "read_phenotypes" || command == "pheno") {
                        cout << "Enter filename containing phenotype edges: ";
                        string filename;
			cin >> filename;
			int gid = graph_man->add_phenotype_edges(filename);
			if (gid != -1) {
				cout << "New graph id: " << gid << endl;
			}
			else{
				cerr << "Could not create graph with phenotype info" << endl;
			}
			
		}
                
                
		else if (command == "size" || command == "sz"){
			int graphnum;
			cin >> graphnum;
			GraphNew * graph = graph_man->get_graph(graphnum);
			if(graph == NULL){
				cerr << "No such graph" << endl;
			}
			else{
				cout << "Number of nodes: " << graph->get_node_ids().size() << endl;
				cout << "Number of edges: " << graph->get_edge_ids().size() << endl;
			}
		}

		else if (command == "display" || command == "dis") {
			int graphnum;
			cin >> graphnum;
			GraphNew * graph = graph_man->get_graph(graphnum);
			if(graph != NULL){
				std::string color_map_filename;
				cout << "Enter color map filename (0 to skip): ";
				cin >> color_map_filename;
				graph->display_graph(color_map_filename, graph_man);
			}
			else{
				cerr << "Graph id " << graphnum << " is not present" << endl;
			}
		}
                else if (command == "display_rich" || command == "dr") {
			int graphnum;
			cin >> graphnum;
			GraphNew * graph = graph_man->get_graph(graphnum);
			if(graph != NULL){
				std::string color_map_filename;
				cout << "Enter color map filename (0 to skip): ";
				cin >> color_map_filename;
				graph->display_graph_richer(color_map_filename, graph_man);
			}
			else{
				cerr << "Graph id " << graphnum << " is not present" << endl;
			}
		}

		else if (command == "find_intersecting_nodes" || command == "fin"){
			int graphnum1, graphnum2;
			cin >> graphnum1;
			cin >> graphnum2;
			GraphNew * graph1 = graph_man->get_graph(graphnum1);
			GraphNew * graph2 = graph_man->get_graph(graphnum2);
			if(graph1 == NULL)
				cerr << "Invalid graph id " << graphnum1 << endl;
			else if(graph2 == NULL)
				cerr << "Invalid graph id " << graphnum2 << endl;
			else{
				std::vector<int> nids_g1 = graph1->get_node_ids();
				std::vector<int> nids_g2 = graph2->get_node_ids();
				std::set<string> rep_ids_g1, rep_ids_g2, rep_ids_intersection;
				std::vector<int>::iterator itr;
				std::set<string>::iterator itr_str;
				for(itr = nids_g1.begin(); itr != nids_g1.end(); itr++){
					std::string rep_id = graph1->get_rep_id_from_nid(*itr);
					rep_ids_g1.insert(rep_id);
				}
				for(itr = nids_g2.begin(); itr != nids_g2.end(); itr++){
					std::string rep_id = graph2->get_rep_id_from_nid(*itr);
					rep_ids_g2.insert(rep_id);
				}
				set_intersection(rep_ids_g1.begin(), rep_ids_g1.end(), rep_ids_g2.begin(), rep_ids_g2.end(), inserter(rep_ids_intersection, rep_ids_intersection.begin()));
				string filename;
				cout << "Enter filename to store intersecting rep-ids: ";
				cin >> filename;
				ofstream fout(filename.c_str());
				if(fout.is_open()){
					for(itr_str = rep_ids_intersection.begin(); itr_str != rep_ids_intersection.end(); itr_str++){
						fout << (*itr_str) << " ";
					}
					fout << endl;
				}
				else{
					cerr << "File " << filename << " could not be opened" << endl;
				}

			}

		}

		else if (command == "find_node" || command == "fn"){
			int graphnum;
			string node_id;
			cout << "Enter graph number: ";
			cin >> graphnum;
			GraphNew* graph = graph_man->get_graph(graphnum);
			if(graph == NULL){
				cerr << "No graph with id " << graphnum << endl;
			}
			else{
                                string node_name;
                                set<string> nodes_set;
                                cout << "Enter ids of nodes to find ending with -1, e.g. hsa100 hsa200 -1 : ";
                                cin >> node_name;
                                ofstream fout("find_nodes_result.txt");
                                while (node_name != "-1") {
                                        nodes_set.insert(node_name);
                                        cin >> node_name;
                                }
                                for (set<string>::iterator itr = nodes_set.begin(); itr != nodes_set.end(); itr++) {
                                        string curr_id = (*itr);
                                        string rep_id = graph->get_rep_id_from_id(curr_id);
                                        int nid = graph->get_nid_from_rep_id(rep_id);
                                        if (nid == -1){
                                                //fout << (curr_id) << "\t" << graph->get_node_from_nid(nid)->get_disp_ids().front() << "\t" << "Not found in graph" << endl;
                                                fout << graph_man->kegg_hsa_id_to_display_name_map[curr_id] << "\t" << "Not found in graph" << endl;
                                        }
                                        else{
                                                //fout << (curr_id) << "\t" << graph->get_node_from_nid(nid)->get_disp_ids().front() << "\t" <<  "Found in graph" << endl;
                                                fout << graph_man->kegg_hsa_id_to_display_name_map[curr_id] << "\t" << "Found in graph" << endl;
                                        }
                                                
                                }
				
//				else{
//
//					string color_file = "color_file_for_find";
//					ofstream fout(color_file.c_str());
//					if(fout.is_open()){
//						fout << node_id << " 128 128 255";
//						fout.close();
//						graph->display_graph(color_file, graph_man);
//					}
//					else{
//						cerr << "Error: couldn't open file " << color_file << endl;
//					}
//
//				}
			}
		}

		else if (command == "kegg_consistency_check" || command == "kcc"){
			int graphnum;
			cin >> graphnum;
			GraphNew * graph = graph_man->get_graph(graphnum);
			if(graph == NULL){
				cerr << "No such graph" << endl;
			}
			else{
				graph->perform_kegg_consistency_check();
			}
		}

		else if(command == "reach" || command == "rch"){

			int gid;
			cin >> gid;
			int direction;
			cout << "Direction (2 for undirected, 1 for back, 0 for fwd (default)): ";
			cin >> direction;

			if (direction == 2) {
				direction = FORWARD_BACKWARD;
			}
			else if (direction == 1) {
				direction = BACKWARD;
			}

			else {
				if (direction != 0) {
					cerr << "Invalid direction " << direction << ". Using default: fwd\n";
				}
				direction = FORWARD;
			}

			string node_name;
			vector<string> source_nodes_set;
			cout << "Source node ids(any id from equiv class of that node) list ending with -1, e.g. hsa100 hsa200 -1 : ";
			cin >> node_name;

			while (node_name != "-1") {
				source_nodes_set.push_back(node_name);
				cin >> node_name;
			}

			vector<string> excluded_nodes_set;
			cout << "Similarly, excluded node list (end by -1), e.g. hsa500 hsa600 -1: ";
			cin >> node_name;
			while (node_name != "-1") {
				excluded_nodes_set.push_back(node_name);
				cin >> node_name;
			}

			cout << "Bound: ";
			int bound;
			cin >> bound;
                        ofstream fout("nodes_created");
			int new_gid = graph_man->bounded_reach(gid, direction, source_nodes_set, excluded_nodes_set, bound);
			if(new_gid != -1){
				cout << "Reachability Graph: " << new_gid << endl;
                                GraphNew * graph = graph_man->get_graph(new_gid);
                                vector<int> all_nids = graph->get_node_ids();
                                for (vector<int>::iterator itr = all_nids.begin(); itr != all_nids.end(); itr++) {
                                    fout << graph->get_rep_id_from_nid(*itr) << endl;
                                }
                                fout << all_nids.size() << endl;
			}
			else{
				cerr << "Error: reachability didn't work" << endl;
			}
		}

		else if(command == "print_graph_info" || command == "pgi"){
			int gid;
			cin >> gid;
			GraphNew* graph = graph_man->get_graph(gid);
			if(graph == NULL){
				cerr << "No graph with id " << gid << "is present" << endl;
			}
			else{
				graph->print_all_graph_info();
			}
		}

		else if(command == "print_graph_man_info" || command == "pgmi"){
			graph_man->print_all_graph_manager_info();
		}

		else if (command == "merge" || command == "mg") {
			int graphnum;
			std::list<int> glist;
			cout << "Enter graphs (end by -1): ";
			cin >> graphnum;
			while (graphnum != -1) {
				glist.push_back(graphnum);
				cin >> graphnum;
			}
			if (glist.size() == 0) {
				cerr << "No graphs to merge" << endl;
			}

			else {
                                string filename;
                                list<unordered_map<string, set<string> > > map_list;
                                cout << "Enter map file names (end by -1): ";
                                cin >> filename;
                                while (filename != "-1") {
                                    unordered_map<string, set<string> > mapname;
                                    graph_man->read_naming_map(filename, mapname);
                                    map_list.push_back(mapname);
                                    cin >> filename;
                                    
                                }
                            
				//int merged_graph_id = graph_man->merge_graphs(glist);
                                int merged_graph_id = graph_man->merge_graphs(glist, map_list);
				//int merged_graph_id = graph_man->merge_graphs_divide_and_conquer(glist);
				if (merged_graph_id == -1)
					cerr << "No merged graph produced" << endl;
				else
					cout << "New graph: " << merged_graph_id << endl;
			}
		}

		else if (command == "merge_from_file" || command == "mff") {
                    
			string filename;
			cin >> filename;
                        
                        string format;
                        cout << "Enter file format (xml/sbml): ";
                        cin >> format;
			
                        std::list<int> glist;
                        
			time(&time3);
			ifstream ifs(filename.c_str());
			string file;
			int graph_id;

			bool tool_written = false;
			char tool_written_ch;
			cout << "Were these graph earlier written by this tool?, 'y' or 'Y' for yes, otherwise any other key: ";
			cin >> tool_written_ch;
			if(tool_written_ch == 'y' || tool_written_ch == 'Y'){
				tool_written = true;
			}

                        // read map files
                        string mapfilename;
                        list<unordered_map<string, set<string> > > map_list;
                        cout << "Enter map file names (end by -1): ";
                        cin >> mapfilename;
                        while (mapfilename != "-1") {
                            unordered_map<string, set<string> > mapname;
                            graph_man->read_naming_map(mapfilename, mapname);
                            cout << "Read map with " << mapname.size() << " entries" << endl;
                            map_list.push_back(mapname);
                            cin >> mapfilename;

                        }
                        string pause;
                        cin >> pause;
                        if(format == "xml"){
                            while (getline(ifs, file)) {
				time(&time1);
				graph_id = graph_man->read_graph_xml_file(file, tool_written);
				time(&time2);
				time_in_sec = difftime(time2,time1);
				//cout << "Time taken (in sec): " << time_in_sec << endl;

				if (graph_id != -1)
					glist.push_back(graph_id);
                            }
			}
                        
                        else if (format == "sbml"){
                            while (getline(ifs, file)) {
				time(&time1);
				graph_id = graph_man->read_graph_from_sbml(file);
				time(&time2);
				time_in_sec = difftime(time2,time1);
				cout << "Time taken (in sec): " << time_in_sec << endl;

				if (graph_id != -1)
					glist.push_back(graph_id);
                            }
			}
                        
                        else{
                            cerr << "Unknown format\n";
                        }
			

			if (glist.size() == 0) {
				cerr << "No graphs to merge" << endl;
			}
			else {
                                
                                
				int merged_graph_id = graph_man->merge_graphs(glist, map_list);
				if (merged_graph_id == -1)
					cerr << "No merged graph produced" << endl;
				else {
					std::string oper = "Merged graphs from file " + filename;
					std::list<int> oper_graph_ids;
					oper_graph_ids.empty();

					graph_man->add_graph_genesis(merged_graph_id, oper, oper_graph_ids);
					cout << "New graph: " << merged_graph_id << endl;

				}
			}
			time(&time4);
			cout << "Total time taken (in sec): " << difftime(time4, time3) << endl;
		}


		else if(command == "list_pathways_with_nodes" || command == "lpwn"){
			string filename;
			cin >> filename;
			ifstream ifs(filename.c_str());
			string file;
			int graph_id;

			bool tool_written = false;
			char tool_written_ch;
			cout << "Were these graph earlier written by this tool?, 'y' or 'Y' for yes, otherwise any other key: ";
			cin >> tool_written_ch;
			if(tool_written_ch == 'y' || tool_written_ch == 'Y'){
				tool_written = true;
			}

			string diff_nodes_file, curr_node;
			cout << "\nEnter the nodes file that contains the differentially nodes to be looked for: ";
			cin >> diff_nodes_file;
			ifstream fin_nodes_diff(diff_nodes_file.c_str());
			set<string> diff_ids_to_look_for;

			if(fin_nodes_diff.is_open()){
				while(fin_nodes_diff >> curr_node){
					diff_ids_to_look_for.insert(curr_node);
				}
			}
			else{
				cerr << "\nError: Couldn't open file " + diff_nodes_file << endl;
			}

			string perturbed_node;
			cout << "\nEnter the id of the perturbed node: ";
			cin >> perturbed_node;

			string selected_pathways_file;
			cout << "\nEnter the file path to which to write the list of pathways that contain any node from the list of nodes: ";
			cin >> selected_pathways_file;
			ofstream fout_select_path(selected_pathways_file.c_str());

			if(fout_select_path.is_open()){
				while (getline(ifs, file)) {
					graph_id = graph_man->read_graph_xml_file(file, tool_written);
					bool graph_selected = false;
					if (graph_id != -1){
						GraphNew* graph = graph_man->get_graph(graph_id);
						for(set<string>::iterator ids_itr = diff_ids_to_look_for.begin(); ids_itr != diff_ids_to_look_for.end(); ids_itr++){
							string rid = graph->get_rep_id_from_id(*ids_itr);
							if(rid != ""){
								int nid = graph->get_nid_from_rep_id(rid);
								NodeNew* node = graph->get_node_from_nid(nid);
								if(node->is_target_of_GErel_edge(graph)){
									fout_select_path << file << endl;
									graph_selected = true;
									break;
								}
							}
						}
						if(!graph_selected){
							string rid = graph->get_rep_id_from_id(perturbed_node);
							if(rid != ""){
								fout_select_path << file << endl;
							}
						}
					}
				}
			}
			else{
				cerr << "\nError: Couldn't open file " + selected_pathways_file << endl;
			}
		}

		else if (command == "genesis" || command == "gen") {
			int graphnum;
			cin >> graphnum;
			graph_man->print_genesis_new(graphnum, cout);

		}

		else if(command == "write_graph_xml" || command == "wgx") {
			int graphnum;
			cin >> graphnum;
			GraphNew* graph_ptr = graph_man->get_graph(graphnum);

			if(graph_ptr != NULL){
				string filename;
				cout << "Enter the file path to store the graph in xml format: ";
				cin >> filename;
				ofstream fout(filename.c_str());
				if(fout.is_open()){
					graph_ptr->write_to_xml_file(fout);
					fout.close();
				}
				else{
					cerr << "Error: couldn't open file " << filename << endl;
				}
			}
			else{
				cerr << "Graph id " << graphnum << " is not present" << endl;
			}

		}

		else if (command == "generate_constraints" || command == "gencn"){
			int gid;
			cout << "Enter graph number on which rules are to be applied: ";
			cin >> gid;
			graph_man->generate_constraints_using_rules_parser(gid, list_of_rules, em);
		}
                
                else if (command == "solve"){
                        int graphnum;
                        string cnf_file, mapping_filename, z3_out_file_name;
			string soln, soln_file;
                        
                        cout << "\nFor graph number: ";
                        cin >> graphnum;
                        cout << "\nEnter DIMACS format file: ";
			cin >> cnf_file;
			cout << "\nEnter z3 output file name: ";
			cin >> z3_out_file_name;

                        string systemCommand = "z3 -dimacs " + cnf_file + " > " + z3_out_file_name;
			//cout << systemCommand << endl;
			soln_file = z3_out_file_name;
			system(systemCommand.c_str());
			sleep(2);
			ifstream fin(soln_file.c_str());
			fin>>soln;

			if (soln == "unsat")
				cout << "\nNo solutions";

			else if (soln == "sat"){

                                string output_assigns = z3_out_file_name + "_assignment";
                                read_z3_output(z3_out_file_name, output_assigns);

                                GraphNew * graph = graph_man->get_graph(graphnum);
                                GraphNew * subgraph = graph_man->get_subgraph_with_edges_retained(graph, output_assigns);

                                cout << "Solution subgraph is Graph " << subgraph->get_graph_id() << endl;
                        }
                }

		else if (command == "print_GErels" || command == "pgrel"){
			int graphnum;
			cin >> graphnum;
			GraphNew* graph_ptr = graph_man->get_graph(graphnum);
			if(graph_ptr != NULL){
				char input;
				cout << "\nEnter one of the options:\n'a' for getting GErel information of all edges\n'b' for getting all GErel target node ids\n";
				cin >> input;
				switch(input){
				case 'a':
					graph_ptr->print_GErel_edges();
					break;

				case 'b':
					graph_ptr->print_GErel_targets();
					break;

				}
			}
			else{
				cerr << "Graph id " << graphnum << " is not present" << endl;
			}
		}


		else if (command == "subgraph" || command == "sg") {
			int graphnum;
			string filename;
			cin >> graphnum;
			cin >> filename;

			GraphNew* graph = graph_man->get_graph(graphnum);
			GraphNew* subgraph = graph_man->get_subgraph_with_edges_retained(graph, filename);
			if(subgraph != NULL){
				cout << "New subgraph id: " << subgraph->get_graph_id() << endl;
			}
			else{
				cerr << "Subgraph not created" << endl;
			}

			string file_visible_edges;
			cout << "Enter the file name to store the edges that are visible (to be colored red) but not present (i.e. not doing their job)" << endl;
			cin >> file_visible_edges;

			ofstream fout(file_visible_edges.c_str());

			if(fout.is_open()){
				graph_man->print_visible_but_not_present_edges_from_z3_assignment(filename, fout);
				fout.close();
			}
			else{
				cerr << "Error: file " + file_visible_edges + " couldn't be opened" << endl;
			}
		}
                
                else if (command == "select" || command == "sel") {
			int gid;
			cin >> gid;
			std::string input_file, line;
                        std::cin >> input_file;
                        set<string> nodes_set;
                        std::ifstream fin;
                        fin.open(input_file.c_str(), std::ifstream::in);
                        if (fin.is_open()) {
                            while (getline(fin, line)) {
                                std::stringstream s(line);
                                std::string id;
                                s >> id;
                                nodes_set.insert(id);
                            }
                            fin.close();

                            
                        }
                        else{
                            std::cerr << "\nUnable to open the file " + input_file << std::endl;
                        }

			int new_gid = graph_man->select_subgraph(gid, nodes_set);
			
			if(new_gid != -1){
				cout << "Graph: " << new_gid << endl;
			}
			else{
				cerr << "Error in select" << endl;
			}
		}

		else if (command == "process_microarr_data" || command == "pmd"){
			int gid;
			cin >> gid;
			GraphNew* graph = graph_man->get_graph(gid);
			graph->write_microarr_decl_to_file();
			graph->write_microarr_assert_to_file();
		}
                
                else if (command == "add_projected_variables" || command == "proj"){
			string cnf_file, mapping_filename, cnf_out_file;
                        cout << "\nEnter DIMACS format file: ";
			cin >> cnf_file;
			cout << "\nEnter mapping file name: ";
			cin >> mapping_filename;
			cout << "\nEnter output CNF file name: ";
			cin >> cnf_out_file;
                        
			string systemCommand = "../supporting_scripts/add_proj_vars_to_cnf_file.py " + cnf_file + " " + mapping_filename + " " + cnf_out_file;
                        cout << systemCommand << endl;
			system(systemCommand.c_str());
			sleep(2);

                        cout << "Written to CNF file " << cnf_out_file << endl;
		}

		else if (command == "get_all_solutions" || command == "get_all_solns"){
			int graphnum;
			string cnf_file, mapping_filename, z3_out_file_prefix;
			string soln, soln_file;
                        list<int> resultant_subgraphs_to_merge;
                        int merged_graphnum = graphnum;
                        
                        string xdot_path_name = XDOT_PATH_NAME;
                        string merged_result = "merged_result.dot";
                        string merged_vis_file = "merged_vis";
                        
                        cout << "\nFor graph number: ";
			cin >> graphnum;
			cout << "\nEnter DIMACS format file: ";
			cin >> cnf_file;
			cout << "\nEnter mapping filename: ";
			cin >> mapping_filename;
			cout << "\nEnter z3 output file prefix: ";
			cin >> z3_out_file_prefix;

                        string file_diff_nodes;
                        cout << "Enter the file path with the list differentially expressed nodes : ";
                        cin >> file_diff_nodes;
                        
                        string color_map_filename;
                        cout << "\nEnter color map file name: ";
                        cin >> color_map_filename;
                        //string vis_edges_filename;
                        //cout << "\nEnter the file containing visible edges list: ";
                        //cin >> vis_edges_filename;
                        string dis_name;
                        cout << "\nEnter a file prefix for the graph display: ";
                        cin >> dis_name;
                        
			int soln_counter = 0;
			string systemCommand = "z3 -dimacs " + cnf_file + " > " + z3_out_file_prefix + (soln_counter + 1);
			//cout << systemCommand << endl;
			soln_file = z3_out_file_prefix + (soln_counter + 1);
			system(systemCommand.c_str());
			sleep(2);
			ifstream fin(soln_file.c_str());
			fin>>soln;

			if (soln == "unsat")
				cout << "\nNo solutions";

			else if (soln == "sat"){  // got a new sat solution

                            string project_option; bool project = false;
                            int num_soln_option = 1;
                            cout << "How many do you want to see? (default is 1): ";
                            cin >> num_soln_option;
                            cout << "Count solutions projected on present edges? (Y/n): ";
                            cin >> project_option;
                            if (project_option == "y" || project_option == "Y")
                                project = true;
                            
				// copying original CNF file - intermediate copies will be deleted
				string systemCommand2 = "cp " + cnf_file + " " + cnf_file + (soln_counter + 1);
				//cout << systemCommand2 << endl;
				system(systemCommand2.c_str());
                                
				while (1){
					soln_counter++;
					cout << "Solutions counted: " << soln_counter << endl;

					// display first n solutions
					if (soln_counter <= num_soln_option) {

					////cout << "Displaying solution " << z3_out_file_prefix << soln_counter << endl;
					string z3_result = z3_out_file_prefix + IntToString(soln_counter);
					string output_assigns = z3_result + "_assignment";
					read_z3_output(z3_result, output_assigns);

					GraphNew * graph = graph_man->get_graph(graphnum);
					GraphNew * subgraph = graph_man->get_subgraph_with_edges_retained(graph, output_assigns);
                                        
                                        vector<string> diif_exp_nodes_set;
                                        vector<string> excl_exp_nodes_set;
                                        excl_exp_nodes_set.clear();
                                        
                                        
                                        ifstream fin(file_diff_nodes.c_str());
                                        string node_name;
                                        while(fin >> node_name){
                                                string diff_rid = subgraph->get_rep_id_from_id(node_name);
                                                if(diff_rid != ""){
                                                        //from the diff_exp_nodes_set pick only those that are actually targets of GErel edges in the graph fwd_gid
                                                         int diff_nid = subgraph->get_nid_from_rep_id(diff_rid);
                                                        vector<int> inlist_diff_node = subgraph->get_inlist(diff_nid);
                                                        for(vector<int>::iterator itr = inlist_diff_node.begin(); itr != inlist_diff_node.end(); ++itr){
								diif_exp_nodes_set.push_back(node_name);
								
							}
						}
					}
					
				
                                        int breach_on_sg_id = graph_man->bounded_reach(subgraph->get_graph_id(),BACKWARD, diif_exp_nodes_set, excl_exp_nodes_set,25);
                                        GraphNew * breach_on_sg = graph_man->get_graph(breach_on_sg_id);
                                        string file_visible_edges = "empty";
                                        //cout << "Enter the file name to store the edges that are visible (to be colored red) but not present (i.e. not doing their job)" << endl;
                                        //cin >> file_visible_edges;

////                                        string file_visible_edges = output_assigns + "_vis";
////                                        ofstream fout(file_visible_edges.c_str());
////                                        if(fout.is_open()){
////                                                graph_man->print_visible_but_not_present_edges_from_z3_assignment(output_assigns, fout);
////                                                fout.close();
////                                        }
////                                        else{
////                                                cerr << "Error: file " + file_visible_edges + " couldn't be opened" << endl;
////                                        }
                                        
                                        /** Merging solutions begins **/
//                                        string output_assigns_prev;
//                                        if(soln_counter == 0 || soln_counter == 1)
//                                            output_assigns_prev = z3_out_file_prefix + "1" + "_assignment";
//                                        else
//                                            output_assigns_prev = z3_out_file_prefix + gm->toString(soln_counter-1) + "_assignment";
//                                        
//                                        ofstream vis_fout(merged_vis_file.c_str());
//                                        if(vis_fout.is_open()) {
//                                            graph_man->merge_vis_files(output_assigns_prev, output_assigns, vis_fout);
//                                            vis_fout.close();
//                                        }
//                                        else{
//                                            cerr << "Error: File " + merged_vis_file + " could not be opened" << endl;
//                                        }
//                                        
//                                        resultant_subgraphs_to_merge.push_back(subgraph->get_graph_id());
//                                        GraphNew * merged_result_graph = graph_man->get_graph(graph_man->merge_graphs(resultant_subgraphs_to_merge));
//                                        merged_graphnum = merged_result_graph->get_graph_id();
//                                        
                                        /** Merging solutions ends **/
					//string color_map_filename = "0";
					//string dot_filename = "graph_" + gm->toString(soln_counter) + ".dot";
                                        string dot_filename = dis_name + "_" + IntToString(soln_counter) + ".dot";
					////subgraph->display_graph_silent(color_map_filename, file_visible_edges, dot_filename, graph_man);
                                        breach_on_sg->display_graph_silent(color_map_filename, file_visible_edges, dot_filename, graph_man);


//					string xdot_path_name = XDOT_PATH_NAME;
					string systemCommand_xdot =  xdot_path_name + dot_filename + " & ";
					system(systemCommand_xdot.c_str());
                                        
                                        
					}
                                        
//                                        string tps = " -Tps ";
//                                        string dotCommand = DOT_PATH_NAME + tps + result_graph_name + " -o MERGED_RESULT.pdf &";
//                                        system(dotCommand.c_str());
//                                        string pdf = " MERGED_RESULT.pdf &";
//                                        string evinceCommand = PDF_VIEWER_PATH_NAME + pdf;
//                                        system(evinceCommand.c_str());
                                        
                                        //string systemCommand_xdot_result =  xdot_path_name + result_graph_name + " & ";
					//system(systemCommand_xdot_result.c_str());
                                        
					int temp = soln_counter + 1;
					//string systemCommand3 = "../supporting_scripts/negate_z3_cnf_solution.py " + cnf_file + soln_counter + " " + std::string("mapping") + " " + z3_out_file_prefix + soln_counter + " " + cnf_file + temp;
					string systemCommand3;
                                        set<int> eid_set;
                                        set<int>::iterator eid_itr;
                                        if (project == true){
                                             
                                            systemCommand3 = "../supporting_scripts/negate_z3_cnf_solution_only_black_edges.py " + cnf_file + soln_counter + " " + mapping_filename + " " + z3_out_file_prefix + soln_counter + " " + cnf_file + temp;
                                        }
                                        else
                                                systemCommand3 = "../supporting_scripts/negate_z3_cnf_solution2.py " + cnf_file + soln_counter + " " + mapping_filename + " " + z3_out_file_prefix + soln_counter + " " + cnf_file + temp;
					//cout << systemCommand3 << endl;
					system(systemCommand3.c_str());
					sleep(2);

					string systemCommand5 = "rm " + cnf_file + soln_counter;
					//cout << systemCommand5 << endl;
					system(systemCommand5.c_str());

					string systemCommand6 = "z3 -dimacs " + cnf_file + temp + " > " + z3_out_file_prefix + temp;
					//cout << systemCommand6 << endl;
					system(systemCommand6.c_str());
					sleep(2);

					string systemCommand4 = "rm " + z3_out_file_prefix + soln_counter;
					//cout << systemCommand4 << endl;
					system(systemCommand4.c_str());

					string soln_filename = z3_out_file_prefix + IntToString(temp);

					ifstream infile(soln_filename.c_str());
					string soln;
					infile>>soln;
					//cout << soln << endl;
					if (soln == "unsat") {
						cout << "\nNo more solutions" << endl;
						break;
					}
					else if (soln != "sat") {
						cout << "\nError in z3 output" << endl;
					}


				}
			}

			cout << "Total solutions: " << soln_counter << endl;
                        cout << "Merged vis file written to " << merged_vis_file << endl;
                        cout << "Merged graph id: " << merged_graphnum << endl;
		}

                
                else if (command == "call_ApproxMC" || command == "approxmc"){
			string cnf_file, delta, epsilon, logging;
                        string logfile="log/logging/" + cnf_file + ".txt";
                        cout << "\nEnter DIMACS format file: ";
			cin >> cnf_file;
			cout << "\nEnter delta parameter: ";
			cin >> delta;
			cout << "\nEnter epsilon parameter: ";
			cin >> epsilon;
                        cout << "\nEnable logging (0/1): ";
                        cin >> logging;
                        if (logging == "1"){
                                cout << "\nEnter destination for logfile: ";
                                cin >> logfile;
                        }
                        string approxmc = APPROXMC_PATH_NAME;
			string systemCommand = "python " + approxmc + " -delta=" + delta + " -epsilon=" + epsilon + " " + cnf_file;
                        cout << systemCommand << endl;
			system(systemCommand.c_str());
			sleep(2);

		}
                
		else if(command == "clear_expression_maps" || command == "clxm"){
			//empty the internal maps
			getCNFIndexMap.clear();
			getExpressionMap.clear();
                        idx = 1; //added by Sukanya
			clauses = 0; // added by Sukanya
			//more maps need to be cleared - not complete
		}

		else if(command == "clr"){
			system("clear");
		}

		//"fwd_bkwd_rch \t<graph-id>"
		else if(command == "fwd_bkwd_rch" || command == "fb_rch"){
			int gid;
			cin >> gid;
			GraphNew* graph = graph_man->get_graph(gid);
			string perturbed_node;
			cout << "Enter the id of the pertrubed node, (e.g. hsa100) : " << endl;
			cin >> perturbed_node;
			string perturb_rid = graph->get_rep_id_from_id(perturbed_node);
			if(perturb_rid == ""){
				cerr << "Error: No rep id found in the graph for the perturbed node " + perturbed_node << endl;
				continue;
			}
			vector<string> source_nodes_set;
			source_nodes_set.push_back(perturb_rid);

			vector<string> empty_set;
			int max_bound = std::numeric_limits<int>::max()-1;
			int direction = FORWARD;

			char bound_choice;
			cout << "Enter 'y' or 'Y' to give a bound, any other key to not... ";
			cin >> bound_choice;

			if(bound_choice == 'y' || bound_choice == 'Y'){
				int bound;
				cout << "Enter the bound: ";
				cin >> bound;
				max_bound = bound;
			}

			//do forward reachability from perturbed nodes
			int fwd_bkwd_gid = -1;
			int fwd_gid = graph_man->bounded_reach(gid, direction, source_nodes_set, empty_set, max_bound);
			if(fwd_gid != -1){
				//do backward reachability from differen expressed nodes
				GraphNew* graph_fwd = graph_man->get_graph(fwd_gid);
				string node_name, file_diff_nodes;
				vector<string> diif_exp_nodes_set;
				/*cout << "Enter the id of the differentially expressed nodes list ending with -1, e.g. hsa100 hsa200 -1 : ";
				cin >> node_name;*/
				cout << "Enter the file path with the list differentially expressed nodes : ";
				cin >> file_diff_nodes;
				ifstream fin(file_diff_nodes.c_str());

				while(fin >> node_name){
					string diff_rid = graph_fwd->get_rep_id_from_id(node_name);
					if(diff_rid != ""){
						//from the diff_exp_nodes_set pick only those that are actually targets of GErel edges in the graph fwd_gid
						int diff_nid = graph_fwd->get_nid_from_rep_id(diff_rid);
						vector<int> inlist_diff_node = graph_fwd->get_inlist(diff_nid);
						for(vector<int>::iterator itr = inlist_diff_node.begin(); itr != inlist_diff_node.end(); ++itr){
							if(graph_fwd->get_edge_type(*itr) == "GErel" && (graph_fwd->edge_has_subtype(*itr, "expression") || graph_fwd->edge_has_subtype(*itr, "repression"))){
								diif_exp_nodes_set.push_back(node_name);
								break;
							}
						}
					}
					else{
						cerr << "No rep id found in the graph for the differentiallly expressed node " + node_name + ", ignoring..."<< endl;
					}
				}

				if(diif_exp_nodes_set.empty()){
					cerr << "Error: None of the differentially expressed are nodes common with the fwd reachable graph from the pertubed nodes" << endl;
					continue;
				}

				direction = BACKWARD;

				fwd_bkwd_gid = graph_man->bounded_reach(fwd_gid, direction, diif_exp_nodes_set, empty_set, max_bound);
				if(fwd_bkwd_gid != -1){
					cout << "Forward-Backward Reachability Graph: " << fwd_bkwd_gid << endl;

					cout << "Writing color file \"fwd_bkwd_color\" for coloring source and target nodes" << endl;
					ofstream fout("fwd_bkwd_color");

					if(fout.is_open()){
						//give bluish color to source node:
						fout <<  perturb_rid + " 128 128 255" << endl;

						//give reddish color to differen-expressed nodes
						//255 128 128

						for(vector<string>::iterator itr = diif_exp_nodes_set.begin(); itr != diif_exp_nodes_set.end(); itr++){
							fout << *itr << " 255 128 128" << endl;
						}
						fout.close();
					}
					else{
						cerr << "Error: couldn't openfile fwd_bkwd_color for color info"  << endl;
					}

					//Testing starts:
					/*GraphNew* graph_fwd_bkwd = graph_man->get_graph(fwd_bkwd_gid);
					graph_fwd_bkwd->print_nodes_rids_with_no_in_edges(cout);*/
					//Testing ends


				}
				else{
					cerr << "Error: bkwd reachability from perturbed didn't work" << endl;
				}
			}
			else{
				cerr << "Error: fwd reachability from perturbed didn't work" << endl;
			}
		}

		else if (command == "read_rules" || command == "rr"){
			string filename;
			cin >> filename;
			yyin = fopen(filename.c_str(), "r");
			yyparse();
			fclose(yyin);
			cout << "Number of rules in template file: " << list_of_rules.size() << endl;
			//rules_ptr->add(global_list_of_rules);
			//global_list_of_rules.clear();//make empty so t
			/*these encodings come from supportinghat it can store another rules file*/
		}

		else if(command == "help" || command == "h"){
			display_commands();
		}

		else if (command == "exit") {
			break;
		}

		else if (command == "get_assignments_z3" || command == "get_assg_z3"){
			string z3_result, output_assigns;
			cin >> z3_result >> output_assigns;
			cout << "\nNote: for this command to successfully work\n the mapping of internal variables in the hash maps should be the recent ones\n";
			read_z3_output(z3_result, output_assigns);
		}
                
                else if (command == "undir" || command == "u") {
                        int graphnum;
                        cin >> graphnum;
                        GraphNew * graph = graph_man->get_graph(graphnum); // keep track of first node id -- subtract to get vector index -- through a macro
                        
                        // create undirected graph from graph
                        int graphnum_undirected = graph_man->convert_to_undirected_graph(graphnum);
                        cout << "New graph: " << graphnum_undirected << endl;
                }
               
                else if  (command == "mincut" || command == "mc"){
                        int graphnum;
                        cin >> graphnum;
                        GraphNew * graph = graph_man->get_graph(graphnum); // keep track of first node id -- subtract to get vector index -- through a macro
                        
                        // create undirected graph from graph
//                        int graphnum_undirected = graph_man->convert_to_undirected_graph(graphnum);
//                        GraphNew * ugraph = graph_man->get_graph(graphnum_undirected);
//                        
                        vector<int> node_ids = graph->get_node_ids();
                        graph->first_offset = node_ids.front();
                        
                        int num_of_nodes = node_ids.size();
                        vector<int> gomoryhu_parents(node_ids.size());
                        for (vector<int>::iterator vec_itr = node_ids.begin(); vec_itr != node_ids.end(); vec_itr++) {
                            gomoryhu_parents[GET_NODE_SEQ_NUM(graph, *vec_itr)] = 1;
                        }
#ifdef DEBUG_FLAG                            
                        debug_log << "Set parents of nodes to 0" << endl;
#endif                        
                        vector<set<int> > cut_edges(node_ids.size());
                        time(&time1);
                        
                        //int graphnum_undirected = graph_man->convert_to_undirected_graph(graphnum);
                        
#ifdef DEBUG_FLAG
                        debug_log << "Before calling compute_gh_tree" << endl;
#endif
                        graph_man->compute_gh_tree(graphnum, gomoryhu_parents, cut_edges);
                        
#ifdef DEBUG_FLAG
                        debug_log << "After returning from compute_gh_tree" << endl;
#endif                        
                        time(&time2);
                        cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;
                        
                        // populate the all-pair matrix with mincut edges
                        map<pair<int,int>, int> cut_edge_index; // cut_edge_index[i,j] = k means cut_edges[k] contains desired cut edges between i and j

                        //anjan's code to compute mincut matrix

                    /*    for(int i=0;i<node_ids.size();i++){
                            for(int j=i+1;j<node_ids.size();j++){
#ifdef DEBUG_FLAG                                
                                debug_log<<"between :"<<i<<"and "<<j<<":";
#endif                                
                                int node_i = i;
                                int node_j = j;
                                int prev_size_i = inf;
                                int prev_size_j = inf, min_size, node_index_i, node_index_j;
                                int count_i=0, count_j=0;
                                int n_size = node_ids.size();
                                vector<bool> i_true(n_size);
                                vector<bool> j_true(n_size);
                                map<int, int> map_i_to_count;
                                map<int, int> map_j_to_count;
                                vector<set<int> > cum_min_cut_edges_i(n_size);
                                vector<set<int> > cum_min_cut_edges_j(n_size);
                                set<int> curr_cut_edges;
                                while(true){
                                    if(node_i==0){
                                        i_true[0] = true;
                                        map_i_to_count[0] = count_i;
                                    }

                                    if(node_j==0){
                                        j_true[0] = true;
                                        map_j_to_count[0] = count_j;
                                    }

                                    if(node_i!=0){
                                        i_true[node_i] = true;
                                        map_i_to_count[node_i] = count_i;
                                        curr_cut_edges = cut_edges[node_i]; //cut_edges[i] tells the mincut edges between i and parent[i]
                                        min_size = curr_cut_edges.size();
                                        if(min_size<prev_size_i){
                                            cum_min_cut_edges_i[count_i] = curr_cut_edges;
                                            prev_size_i = min_size;
                                        }else{
                                            cum_min_cut_edges_i[count_i] = cum_min_cut_edges_i[count_i-1];
                                        }
                                        count_i++;
                                    }
                                    if(node_j!=0){
                                        j_true[node_j] = true;
                                        map_j_to_count[node_j]= count_j;
                                        curr_cut_edges = cut_edges[node_j] ;//cut_edges[i] tells the mincut edges between i and parent[i]
                                        min_size = curr_cut_edges.size();
                                        if(min_size<prev_size_j){
                                            cum_min_cut_edges_j[count_j] = curr_cut_edges;
                                            prev_size_j = min_size;
                                        }else{
                                            cum_min_cut_edges_j[count_j] = cum_min_cut_edges_j[count_j-1];
                                        }
                                        count_j++;
                                    }

                                    if(i_true[node_j]){
                                        std::set<int>::iterator it;
                                        node_index_j = map_j_to_count[node_j]-1;
                                        node_index_i = map_i_to_count[node_j]-1;
                                        if(count_i==0&&count_j>0){
                                                for(it =cum_min_cut_edges_j[node_index_j].begin(); it !=cum_min_cut_edges_j[node_index_j].end();++it)
                                                        cout<<*it<<"-";
                                                break;
                                        }
                                        if((cum_min_cut_edges_i[node_index_i].size()<cum_min_cut_edges_j[node_index_j].size())){
                                                for(it =cum_min_cut_edges_i[node_index_i].begin(); it !=cum_min_cut_edges_i[node_index_i].end();++it)
                                                        cout<<*it<<"-";
                                                break;
                                        }
                                        else{
                                                for(it =cum_min_cut_edges_j[node_index_j].begin(); it !=cum_min_cut_edges_j[node_index_j].end();++it)
                                                        cout<<*it<<"-";
                                                break;
                                        }
                                    }
                                    else if(j_true[node_i]){
                                        std::set<int>::iterator it;
                                        node_index_j = map_j_to_count[node_j]-1;
                                        node_index_i = map_i_to_count[node_j]-1;
                                        if(count_i>0&&count_j==0){
                                                for(it =cum_min_cut_edges_i[node_index_i].begin(); it !=cum_min_cut_edges_i[node_index_i].end();++it)
                                                        cout<<*it<<"-";
                                                break;
                                        }

                                        if((cum_min_cut_edges_i[node_index_i].size()<cum_min_cut_edges_j[node_index_j].size())){
                                                for(it =cum_min_cut_edges_i[node_index_i].begin(); it !=cum_min_cut_edges_i[node_index_i].end();++it)
                                                        cout<<*it<<"-";
                                                break;
                                        }
                                        else{
                                                for(it =cum_min_cut_edges_j[node_index_j].begin(); it !=cum_min_cut_edges_j[node_index_j].end();++it)
                                                        cout<<*it<<"-";
                                                break;
                                        }
                                    }

                                    node_i = gomoryhu_parents[node_i];
                                    node_j = gomoryhu_parents[node_j];

                                }
                                cout<<endl;
                            }
                        }

                        */
                        // get source and target nodes
                        // get the corresponding node nids
			list<int> source_nodes_nid_list;
                        string node_name, rep_id;
                        int node_id;
			cout << "Source node ids list (end with -1): ";
			cin >> node_name;
			while (node_name != "-1") {
                                rep_id = graph->get_rep_id_from_id(node_name);
                                if(rep_id == ""){
                                        cout << "Not found " << node_name << endl;
                                        continue;
                                }
                                else {
                                        node_id = graph->get_nid_from_rep_id(rep_id);
#ifdef ASSERT_FLAG
                                        assert(node_id != -1);
#endif
                                        source_nodes_nid_list.push_back(node_id);
                                }
                                
				cin >> node_name;
			}

			list<int> target_nodes_nid_list;
			cout << "Target node list (end with -1): ";
			cin >> node_name;
                        while (node_name != "-1") {
                                rep_id = graph->get_rep_id_from_id(node_name);
                                if(rep_id == ""){
                                        cout << "Not found " << node_name << endl;
                                        continue;
                                }
                                else {
                                        node_id = graph->get_nid_from_rep_id(rep_id);
#ifdef ASSERT_FLAG
                                        assert(node_id != -1);
#endif                                
                                        target_nodes_nid_list.push_back(node_id);
                                }
				cin >> node_name;
			}
                        
                        set<int> present_edges;
                        list<int>::iterator iter1, iter2;
                        
                        for(iter1 = source_nodes_nid_list.begin(); iter1 != source_nodes_nid_list.end(); iter1++){
                            for(iter2 = target_nodes_nid_list.begin(); iter2 != target_nodes_nid_list.end(); iter2++){
                                
                                graph_man->connect(graph, present_edges, cut_edges, cut_edge_index, *iter1, *iter2); // some more things may be need to be output
                                
                            }
                        }
                        
                        
                        int subgraph = graph_man->get_subgraph_with_edge_ids(graph, present_edges);
                        if(subgraph != -1){
				cout << "New subgraph id: " << subgraph << endl;
			}
			else{
				cerr << "Subgraph not created" << endl;
			}
                        
                        
                        
                }
//                else if  (command == "mincut" || command == "mc"){
//                        int graphnum;
//                        cin >> graphnum;
//                        
//                        GraphNew * graph = graph_man->get_graph(graphnum);
//                        V = graph->get_node_ids().size();
//                        E = graph->get_edge_ids().size();
//                        cout << "V = " << V << " E = " << E << endl;
//                        time(&time1);
//                        vector< vector <int> > minCutValues(V, vector<int>(V, inf));
//                        vector< vector <vector<int> > > minCutPointer(V, vector< vector <int> >(V, vector<int>(1,inf)));
//                        vector< vector <set<int> > > minCutEdgeIds(V, vector< set <int> >(V, set<int>()));
//                        vector< vector <vector<int> > > minCutEdgeIds2(V, vector< vector <int> >(V, vector<int>(0)));
//                        myGHTree(graph, minCutPointer, minCutEdgeIds);
//                        //GHTree(graph, minCutPointer);
//                        time(&time2);
//                        cout << "Computed min-cuts for Graph " << graphnum << endl;
//                        cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;
//                        
//                }
//                
//                else if (command == "connect") {
//                        int graphnum;
//                        cin >> graphnum;
//                        GraphNew * graph = graph_man->get_graph(graphnum);
//                        string node_name, rep_id;
//                        int node_id;
//                        
//                        // get source and target nodes
//                        // get the corresponding node nids
//			list<int> source_nodes_nid_list;
//			cout << "Source node ids list (end with -1): ";
//			cin >> node_name;
//			while (node_name != "-1") {
//                                rep_id = graph->get_rep_id_from_id(node_name);
//                                if(rep_id == ""){
//                                        cout << "Not found " << node_name << endl;
//                                        continue;
//                                }
//                                else {
//                                        node_id = graph->get_nid_from_rep_id(rep_id);
//#ifdef ASSERT_FLAG
//                                        assert(node_id != -1);
//#endif
//                                        source_nodes_nid_list.push_back(node_id);
//                                }
//                                
//				cin >> node_name;
//			}
//
//			list<int> target_nodes_nid_list;
//			cout << "Target node list (end with -1): ";
//			cin >> node_name;
//                        while (node_name != "-1") {
//                                rep_id = graph->get_rep_id_from_id(node_name);
//                                if(rep_id == ""){
//                                        cout << "Not found " << node_name << endl;
//                                        continue;
//                                }
//                                else {
//                                        node_id = graph->get_nid_from_rep_id(rep_id);
//#ifdef ASSERT_FLAG
//                                        assert(node_id != -1);
//#endif                                
//                                        target_nodes_nid_list.push_back(node_id);
//                                }
//				cin >> node_name;
//			}
//			
//                        // get min-cut matrix
//                        V = graph->get_node_ids().size();
//                        E = graph->get_edge_ids().size();
//                        cout << "V = " << V << " E = " << E << endl;
//                        //vector< vector <vector<int> > > minCutPointer(V, vector< vector<int> >(V, vector<int>(1)));
//                        //vector< vector <vector<int> > > minCutPointer(V, vector< vector <pair<int, int> > >(V, vector<pair<int, int> >(1, make_pair(-2,-2))));
//                        //vector< vector <vector<int> > > minCutEdgeIds(V, vector< vector <int> >(V, vector<int>(1)));
//                        //getMinCutMatrix_dummy(graph, minCutMatrix); // to be replaced with GHTree method
//                        time(&time1);
//                        vector< vector <int> > minCutValues(V, vector<int>(V, inf));
//                        vector< vector <vector<int> > > minCutPointer(V, vector< vector <int> >(V, vector<int>(1, inf)));
//                        vector< vector <set<int> > > minCutEdgeIds(V, vector< set <int> >(V, set<int>()));
//                        //myGHTree(graph, minCutPointer, minCutEdgeIds);
//                        GHTree(graph, minCutPointer);
//                        time(&time2);
//                        cout << "Computed min-cuts for Graph " << graphnum << endl;
//                        cout << "Total time taken (in sec): " << difftime(time2, time1) << endl;
//                         
//                        set<int> present_edges;
//                        list<int>::iterator iter1, iter2;
//                        
//                        for(iter1 = source_nodes_nid_list.begin(); iter1 != source_nodes_nid_list.end(); iter1++){
//                            for(iter2 = target_nodes_nid_list.begin(); iter2 != target_nodes_nid_list.end(); iter2++){
//                                graph_man->connect(graph, present_edges, minCutEdgeIds, *iter1, *iter2);
//                                //graph_man->connect(graph, present_edges, minCutPointer, *iter1, *iter2);
//                            }
//                        }
//                        
//                        
////                        int source_nid = source_nodes_nid_list.front();
////                        int target_nid = target_nodes_nid_list.front();
//                        
////                        graph_man->connect(graph, present_edges, minCutMatrix, source_nid, target_nid);
//                        
//                        int subgraph = graph_man->get_subgraph_with_edge_ids(graph, present_edges);
//                        if(subgraph != -1){
//				cout << "New subgraph id: " << subgraph << endl;
//			}
//			else{
//				cerr << "Subgraph not created" << endl;
//			}
//
//                }

		else {
			cout << "Did you enter one of the following valid commands?" << endl;
			display_commands();
			continue;
		}

	}
        debug_log.close();
}
