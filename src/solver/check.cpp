#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

const int N = 64; // Matrix size
int matrix[N][N]; // The grid

// Directions arrays for moving in the grid (left, up, right, down)
int dx[] = {0, -1, 0, 1};
int dy[] = {-1, 0, 1, 0};

// Check if the cell is within the grid and is passable
bool isValid(int x, int y, vector<vector<bool>>& visited) {
    return x >= 0 && x < N && y >= 0 && y < N && matrix[x][y] == 1 && !visited[x][y];
}

// Recursive DFS to find path from src to dst and print the path
bool dfs(int x, int y, int dstX, int dstY, vector<vector<bool>>& visited, vector<pair<int, int>>& path) {
    // Mark the current cell as visited and add to path
    visited[x][y] = true;
    path.push_back({x, y});
    //cout<<x<<" "<<y<<endl;
    // If destination is reached, print the path and return true
    if (x == dstX && y == dstY) {
        for (const auto& p : path) {
            cout << "(" << p.first << ", " << p.second << ") "<<endl;
        }
        //cout << endl;
        return true;
    }

    // Explore the neighbors
    for (int i = 0; i < 4; ++i) {
        int newX = x + dx[i];
        int newY = y + dy[i];

        if (isValid(newX, newY, visited)) {
            if (dfs(newX, newY, dstX, dstY, visited, path)) {
                return true;  // Path found
            }
        }
    }

    // Backtrack: unmark the current cell and remove from path
    //
visited[x][y] = false;
    path.pop_back();
    return false;
}

int main() {
    ifstream file("../output.txt");
    if (!file) {
        cerr << "File could not be opened." << endl;
        return 1;
    }

    // Read matrix from file
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            file >> matrix[i][j];
            if (j != N - 1) file.ignore();
        }
    }

    file.close();

    // Visited matrix to keep track of visited nodes
    vector<vector<bool>> visited(N, vector<bool>(N, false));
    vector<pair<int, int>> path;

    // Starting and ending points
    int srcX = 0, srcY = 63;
    int dstX = 63, dstY = 0;

    if (!dfs(srcX, srcY, dstX, dstY, visited, path)) {
        cout << "No unique path exists." << endl;
    }

    return 0;
}
