#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <set>
#include <map>
#include <mpi.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
using namespace std;

#define INF 100000000

int convert(int x, int y) {
    return x * 64 + y;
}

vector<int> valid_neigh(int pos) {
    int x = pos / 64;
    int y = pos % 64;

    vector<int> neigh;

    if (y - 1 >= 0 && y - 1 < 64) neigh.push_back(convert(x, y - 1));
    if (y + 1 >= 0 && y + 1 < 64) neigh.push_back(convert(x, y + 1));
    if (x - 1 >= 0 && x - 1 < 64) neigh.push_back(convert(x - 1, y));
    if (x + 1 >= 0 && x + 1 < 64) neigh.push_back(convert(x + 1, y));

    return neigh;
}

int local_node(int pos,int my_rank,int size){
    int x = pos / 64;
    int y = pos % 64;

    return convert(x-(my_rank)*(64/size), y);

}

int owner(int pos,int size){
    int x= pos/64;

    return x/16 ;
}

int if_next(int pos,int size){
    int owner_x = owner(pos,size);
    int owner_next_x = owner(pos+64,size);

    if(owner_x==owner_next_x)return -1;

    if(owner_next_x < 4) return owner_next_x;

    return -1;

}

int if_prev(int pos,int size){
    if(pos<64)return -1;

    int owner_x = owner(pos,size);
    int owner_prev_x = owner(pos-64,size);

    if(owner_x==owner_prev_x)return -1;

    if(owner_prev_x >= 0) return owner_prev_x;

    return -1;

}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int my_rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *matrix = nullptr;
    std::string filepath = "../output.txt";  // File containing the matrix

    if (my_rank == 0) {  // Only the root process handles file reading
        matrix = new int[64*64];

        std::ifstream infile(filepath);
        if (!infile.is_open()) {
            std::cerr << "Unable to open file: " << filepath << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1); // Exit all processes due to error
            return 1;
        }

        std::string line;
        int index = 0;
        while (std::getline(infile, line) && index < 64*64) {
            std::istringstream iss(line);
            int value;
            while (iss >> value) {
                matrix[index++] = value;
            }
        }

        infile.close();

        // Ensure we read exactly 64x64 values
        if (index != 64*64) {
            std::cerr << "File does not contain the correct number of elements." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1); // Exit all processes due to error
            return 1;
        }
    }

    // if(my_rank==0){
    //     for (int i = 0; i < 64; ++i) {
    //         for (int j = 0; j < 64; ++j) {
    //             cout << matrix[i * 64 + j] << " ";
    //         }
    //         cout << endl;
    //     }
    // }


    // Determine the portion of the matrix each process will handle
    int local_rows = 64 / size;
    int local_elements = local_rows * 64;
    int *local_matrix = new int[local_elements];

    // Scatter the matrix data to all processes
    MPI_Scatter(matrix, local_elements, MPI_INT, local_matrix, local_elements, MPI_INT, 0, MPI_COMM_WORLD);

    // if(my_rank==1){
    //     for (int i = 0; i < local_rows; ++i) {
    //         for (int j = 0; j < 64; ++j) {
    //             cout << local_matrix[i * 64 + j] << " ";
    //         }
    //         cout << endl;
    //     }
    // }
    

    // Compute shortest paths locally
    int src = 63;
    int dest = 63 * 64;
    int *dist = new int[local_elements];
    int *path = new int[local_elements];
    for (int i = 0; i < local_elements; i++) {
        path[i] = -1;
    }
    set<int> local_T;
    set<int> local_V_T;

    for (int i = local_elements*(my_rank) ; i < local_elements*(my_rank+1); ++i) {
        if (i == src){
            local_T.insert(src);
            dist[local_node(src,my_rank,size)] = 0;
            continue;
        }
        local_V_T.insert(i);
    }

    // if(my_rank==0){
    //     for (auto x : local_T) {
    //         cout <<x<< endl;
    //     }
    // }

    for (auto v : local_V_T) {
        int local_v = local_node(v,my_rank,size);
        dist[local_v] = INF;
    }
   
    if(my_rank==0){
        if(local_matrix[62]==1){
            path[62]=63;
            dist[62]=1;
        }
        if(local_matrix[127]==1){
            path[127]=63;
            dist[127]=1;
        }  
    }
    
    // if(my_rank==1){
    //     for(int i=0;i<8;i++){
    //         cout<<dist[i]<<" ";
    //     }cout<<endl;
    // }

    

    for (int i = 0 ; i < 64*64; ++i) {
        int u = -1;
        int dist_u = INF;

        

        // Finding the local minimum
        for (int v : local_V_T) {
            int local_v = local_node(v, my_rank, size);
            if (dist[local_v] < dist_u) {
                dist_u = dist[local_v];
                u = v;
            }
        }

        // if(my_rank==1){
        //     cout<<"vksn "<<u<<" "<<dist_u<<endl;  
        // }

        // Structure to hold the local minimum and its corresponding vertex
        struct {
            int value;
            int vertex;
        } local_min = {dist_u, u}, global_min;

        // Reduce to find the global minimum and the vertex achieving it
        MPI_Allreduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

        // Print the result on each process
        // if (global_min.vertex != -1 && my_rank==0) { // Check if a valid vertex was found
        //     // cout<<"III "<<i<<endl;
        //     printf("Process %d: The vertex with the global minimum distance is %d with distance %d.\n",
        //         my_rank, global_min.vertex, global_min.value);
        // }

        // if(my_rank == 0 && global_min.vertex==3106){
        //     cout<<"maa chuda "<<i<<endl;
        // }

        // cout<<"owner "<<owner(global_min.vertex,size)<<endl;
        if(owner(global_min.vertex,size)==my_rank){
            local_V_T.erase(u);
            local_T.insert(u);
            // cout<<"u "<<u<<endl;
            vector<int> neigh_u = valid_neigh( global_min.vertex);
            for (auto v : neigh_u) {
                
                if(owner(v,size)!=my_rank)continue;
                
                if (local_T.find(v) != local_T.end())continue;
                int local_v = local_node(v,my_rank,size);
                
                if(local_matrix[local_v]==0)continue;
                // if(global_min.vertex==3106){
                // cout<<"hehe"<<endl;
                // cout<<v<<endl;
                // cout<<"hehe"<<endl;
                // }

                if(global_min.value + 1 < dist[local_v]){
                    path[local_v] = global_min.vertex;
                    dist[local_v] = min(dist[local_v], global_min.value + 1);
                }
                
                // cout<<"hiii"<<endl;
            } 
        }

        // if(i==65 && my_rank==0){
        //     cout<<"hrhrhrhr "<<global_min.vertex<<" "<<dist[256]<<endl;
        // }

        // if(my_rank==0){
        //     for(auto x: local_V_T){
        //         cout<<x<<" ";
        //     }cout<<endl;
        // }

        int next_p = if_next(global_min.vertex,size);
        int prev_p = if_prev(global_min.vertex,size);

        // cout<<"next_p "<<next_p<<endl;
        if(my_rank==next_p){
            int local_v= global_min.vertex % 64;
            // if(global_min.vertex==3042){
            //     cout<<"hehe"<<endl;
            //     cout<<path[local_v]<<endl;
            //     cout<<"hehe"<<endl;
            //     }
            if(local_matrix[local_v]!=0){
                if(global_min.value + 1 < dist[local_v]){
                    path[local_v] = global_min.vertex;
                    dist[local_v] = min(dist[local_v], global_min.value + 1);
                }
            }
        }

        if(my_rank==prev_p){
            int local_v =  15* 64 + global_min.vertex % 64;
            if(global_min.vertex==3106){
                cout<<"hehe"<<endl;
                cout<<path[local_v]<<endl;
                cout<<"hehe"<<endl;
                }
            if(local_matrix[local_v]!=0){
               if(global_min.value + 1 < dist[local_v]){
                    path[local_v] = global_min.vertex;
                    dist[local_v] = min(dist[local_v], global_min.value + 1);
                }
            }
        }


        // if(my_rank==0 && i<100){
        //     // cout<<"iteration "<<i<<endl;
           
        //     //     cout<<dist[64*15]<<" ";


        //     cout<<"iteration "<<i<<" "<<global_min.vertex<<endl;

            
        // }


    }

    if (my_rank == 3) {
        if(dist[15*64]!=INF){
            cout<<"Have path"<<endl;
        }
        else{
            cout<<"Don't have path"<<endl;
        }
    }

    for(int i=0;i<local_elements;i++){
        if(dist[i]!=INF){
            local_matrix[i]=2;
        }
    }

    int *full_matrix = nullptr;
    int *full_path = nullptr;
    if (my_rank == 0) {
        full_matrix = new int[64 * 64]; // Only allocate on the root processor
        full_path = new int[64 * 64];
    }

    // Gather all local matrices to the root processor
    MPI_Gather(local_matrix, local_elements, MPI_INT, 
               full_matrix, local_elements, MPI_INT, 
               0, MPI_COMM_WORLD);
               // Gather all local paths to processor 1
    MPI_Gather(path, local_elements, MPI_INT, 
               full_path, local_elements, MPI_INT, 
               0, MPI_COMM_WORLD); // Note the root is now 1

    // On root processor, print the gathered matrix
    if (my_rank == 0) {

        cout<<"PATH"<<endl;
        int node = 63*64;
        if(matrix[node]==0)cout<<"FALSE"<<endl;
        while(node!=63){
            cout<<(node/64)<<" "<<node%64<<endl;

            node=full_path[node];
            if(matrix[node]==0)cout<<"FALSE"<<endl;
        }

        // for (int i = 0; i < 64 * 64; i++) {
        //     if (i % 64 == 0) std::cout << std::endl; // New line for each row
        //     std::cout << full_matrix[i] << " ";
        // }
        // std::cout << std::endl;
        delete[] full_matrix;
    }





    delete[] local_matrix;

    MPI_Finalize();
    return 0;
}
