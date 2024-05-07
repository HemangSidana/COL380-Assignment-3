#include <mpi.h>
#include <iostream>
#include <queue>
#include <vector>
#include <random>
#include <fstream>

using namespace std;

#define num_vertices 64*64
#define num_proc 4
#define vertices num_vertices/num_proc

int main(){
    std::random_device rd;
    std::mt19937 gen(rd());
    int *graph = NULL;
    MPI_Init(NULL,NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int *local_graph= (int*) malloc(vertices*sizeof(int));
    int *above= (int*) malloc(64*sizeof(int));
    for(int i=0; i<64; i++) above[i]=0;
    for(int i=0; i<vertices; i++) local_graph[i]=0;
    int receive=0;
    queue<int> q;
    if(rank==0){
        local_graph[63]=1;
        q.push(63);
    }
    while(true){
        if(rank>0){
            int recvSize;
            MPI_Recv(&recvSize, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int* recvArray = new int[recvSize];
            MPI_Recv(recvArray, recvSize, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(recvSize>0 && receive==0){
                // for (int i = 0; i < recvSize; i++) {
                int val= recvArray[0];
                q.push(val+64);
                local_graph[val+64]=1;
                local_graph[val]=1;
                above[val]=1;
                // }
                receive=1;
            }
            delete[] recvArray;
        }
        vector<int> send;
        int z= q.size();
        for(int i=0; i<z; i++){
            int x= q.front();
            q.pop();
            std::uniform_real_distribution<double> distribution(0.0, 1.0);
            double random_value = distribution(gen);
            if(random_value < 0.5){
                q.push(x);
                continue;
            }
            if(x>=64){
                int y= x-64;
                int count= (y>=64 ? local_graph[y-64] : above[y]) + ((y%64>0) ? local_graph[y-1] : 0) + ((y%64<63) ? local_graph[y+1] : 0) + ((y<64) ? above[y] : 0);
                if(count==0 && !local_graph[y] && y>=64){ q.push(y); local_graph[y]=1;}
            }
            if((x%64)>0){
                int y= x-1;
                int count= (y>=64 ? local_graph[y-64] : above[y]) + ((y%64>0) ? local_graph[y-1] : 0) + ((y+64<vertices) ? local_graph[y+64] : 0);
                if(count==0 && !local_graph[y]){ q.push(y); local_graph[y]=1;}
            }
            if((x%64)<63){
                int y= x+1;
                int count= (y>=64 ? local_graph[y-64] : above[y]) + ((y+64<vertices) ? local_graph[y+64] : 0) + ((y%64<63) ? local_graph[y+1] : 0);
                if(count==0 && !local_graph[y]){ q.push(y); local_graph[y]=1;}
            }
            if(x+64<vertices){
                int y= x+64;
                int count= ((y+64<vertices) ? local_graph[y+64] : 0) + ((y%64>0) ? local_graph[y-1] : 0) + ((y%64<63) ? local_graph[y+1] : 0);
                if(count==0 && !local_graph[y] && (rank!=3 || y+64<vertices)){ 
                    q.push(y); 
                    local_graph[y]=1;
                    if(y+64>=vertices){
                        send.push_back(y%64);
                    }
                }
            }
        }
        
        if(rank<3){
            // Send message to rank+1
            int sendSize = send.size();
            MPI_Send(&sendSize, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            int* sendArray = new int[sendSize];
            for(int i=0; i<sendSize; i++) sendArray[i]=send[i];
            MPI_Send(sendArray, sendSize, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            delete[] sendArray;
        }
        int sz= q.size();
        MPI_Allreduce(MPI_IN_PLACE, &sz, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if(sz==0) break;
    }
    if(rank==3){
        int i;
        for(i=0; i<64; i++){
            local_graph[i+15*64]=1;
            if(local_graph[i+14*64]==1){
                break;
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){
        graph= (int*) malloc(num_vertices*sizeof(int));
    }
    MPI_Gather(local_graph, vertices, MPI_INT, graph, vertices, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank==0){
        std::ofstream outfile("../output.txt");
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
        // for(int i=0; i<num_vertices; i++){
        //     cout<<graph[i]<<" ";
        //     if(i%64==63) cout<<endl;
        // }
    }
    MPI_Finalize();
}