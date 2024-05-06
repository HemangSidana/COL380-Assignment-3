#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <set>
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
    vector<int> cur;
    std::random_device rd;
    std::mt19937 gen(rd());
    if(rank==0){
        parent[63]=63;
        local_graph[63]=1;
        for(int i=0; i<vertices; i++){
            if(i==63) continue;
            cur.push_back(i);
            if(i%64==63){
                shuffle(cur.begin(), cur.end(), gen);
                for(auto val: cur) v.push_back(val);
                cur.clear();
            }
        }
    }
    else{
        if(rank==3){parent[15*64]=15*64; local_graph[15*64]=1;}
        for(int i=64; i<vertices; i++){
            if(i==15*63 && rank==3) continue;
            cur.push_back(i);
            if(i%64==63){
                shuffle(cur.begin(), cur.end(), gen);
                for(auto val: cur) v.push_back(val);
                cur.clear();
            }
        }
    }
    
    
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
    if(rank>0){
        int* recvArray = new int[64];
        MPI_Recv(recvArray, 64, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int found=0;
        for(int i=0; i<64; i++){
            if(recvArray[i] && local_graph[i+64]){
                found=i+1; break;
            }
        }
        if(found){
            for(int i=0; i<64; i++){
                if(recvArray[i] && (i==0 || local_graph[i-1]==0)){
                    local_graph[i]=1;
                }
            }
        }
        else{
            local_graph[found-1]=1;
            for(int i=found; i<64; i++){
                if(recvArray[i]==1) break;
                local_graph[i]=1;
            }
        }
        delete[] recvArray;
    }
    if(rank<3){
        int* sendArray = new int[64];
        for(int i=0; i<64; i++) sendArray[i]=local_graph[i+15*64];
        MPI_Send(sendArray, 64, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        delete[] sendArray;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){
        graph= (int*) malloc(num_vertices*sizeof(int));
    }
    MPI_Gather(local_graph, vertices, MPI_INT, graph, vertices, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank==0){
        for(int i=0; i<num_vertices; i++){
            cout<<graph[i]<<" ";
            if(i%64==63) cout<<endl;
        }
    }
    MPI_Finalize();
}