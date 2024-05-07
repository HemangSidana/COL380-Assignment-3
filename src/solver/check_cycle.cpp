#include <iostream>
#include <vector>
#include <queue>
#include <fstream>

using namespace std;

const int N = 64; // Assuming the matrix is 64x64
int matrix[N][N]; // The matrix representing the graph

struct Node {
    int x, y;
    int parent_x, parent_y; // Track the parent node to check for actual cycles
};

// Directions arrays for moving in the grid: right, down, left, up
int dx[] = {0, 1, 0, -1};
int dy[] = {1, 0, -1, 0};

bool isValid(int x, int y, vector<vector<bool>>& visited) {
    return x >= 0 && x < N && y >= 0 && y < N && matrix[x][y] == 0 && !visited[x][y];
}

bool bfs(int startX, int startY) {
    vector<vector<bool>> visited(N, vector<bool>(N, false));
    queue<Node> q;

    // Start BFS from the starting node
    q.push({startX, startY, -1, -1});
    visited[startX][startY] = true;

    while (!q.empty()) {
        Node current = q.front();
        q.pop();

        for (int i = 0; i < 4; ++i) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            // Check if the next node is within bounds and not visited
            if (isValid(nx, ny, visited)) {
                visited[nx][ny] = true;
                q.push({nx, ny, current.x, current.y});
            } else if (nx >= 0 && nx < N && ny >= 0 && ny < N && matrix[nx][ny] == 0) {
                // If the node is revisited and not the parent, it's a cycle
                if (!(nx == current.parent_x && ny == current.parent_y)) {
                    return true; // Cycle detected
                }
            }
        }
    }

    return false; // No cycle found
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
            // Ignore commas in CSV file
            if (j != N - 1) file.ignore();
        }
    }

    file.close();

    // Check for cycle starting from each cell that is passable
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (matrix[i][j] == 0) {
                if (bfs(i, j)) {
                    cout << "Cycle detected in the matrix." << endl;
                    return 0;
                }
            }
        }
    }

    cout << "No cycle detected." << endl;
    return 0;
}
