#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <set>
#include <fstream>
using namespace std;

#define num_vertices 64*64
#define num_proc 4
#define vertices num_vertices/num_proc

int find_parent(int x, int *parent){
    if(parent[x]==x) return x;
    int y=find_parent(parent[x],parent);
    parent[x]=y;
    return y;
}

int main(){
    int *graph= NULL;
    MPI_Init(NULL,NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int *local_graph= (int*) malloc(vertices*sizeof(int));
    for(int i=0; i<vertices; i++) local_graph[i]=0;
    int *parent= (int*) malloc(vertices*sizeof(int));
    for(int i=0; i<vertices; i++) parent[i]=-1;
    vector<int> v;
    std::random_device rd;
    std::mt19937 gen(rd());
    for(int i=0; i<vertices; i++){
        if(i%64==63){
            parent[i]=63;
            local_graph[i]=1;
        }
        if(i+64>=vertices) continue;
        v.push_back(i);
    }
    if(rank==3){
        for(int i=0; i<64; i++){
            local_graph[15*64+i]=1;
            parent[15*64+i]=63;
        }
    }
    shuffle(v.begin(),v.end(),gen);
    
    for(int j=0; j<v.size(); j++){
        int x= v[j];
        vector<int> near;
        if(x>=64 && local_graph[x-64]) near.push_back(x-64);
        if(x%64>0 && local_graph[x-1]) near.push_back(x-1);
        if(x%64<63 && local_graph[x+1]) near.push_back(x+1);
        if(x+64<vertices && local_graph[x+64]) near.push_back(x+64);
        if(near.size()==0){
            local_graph[x]=1;
            parent[x]=x;
            continue;
        }
        set<int> s;
        for(int z=0; z<near.size(); z++){
            s.insert(find_parent(near[z],parent));
        }
        if(s.size()==near.size()){
            local_graph[x]=1;
            parent[x]=x;
            for(auto it= s.begin(); it!=s.end(); it++){
                parent[(*it)]=x;
            }
        }
    }
    // MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){
        graph= (int*) malloc(num_vertices*sizeof(int));
    }
    MPI_Gather(local_graph, vertices, MPI_INT, graph, vertices, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank==0){
        // for(int i=0; i<num_vertices; i++){
        //     cout<<graph[i]<<" ";
        //     if(i%64==63) cout<<endl;
        // }
        std::ofstream outfile("../output2.txt");
        // Check if the file is open
        if (outfile.is_open()) {
            for (int i = 0; i < num_vertices; i++) {
                outfile << graph[i] << " ";
                if (i % 64 == 63) outfile << std::endl;
            }
            outfile.close();  // Close the file after writing
        } else {
            std::cout << "Unable to open file";
        }
    }
    MPI_Finalize();
}