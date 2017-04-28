#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <limits>

#define GRID_SIZE 128

typedef struct voxel {
	int x, y, z, w;
	bool isNonTerminatingNode;
} Voxel;

typedef struct Comparator {
	bool operator()(const Voxel& lhs, const Voxel& rhs) const {
    	return lhs.w > rhs.w;
    }	
} Comparator;

float dfb[GRID_SIZE][GRID_SIZE][GRID_SIZE];
//float weight[GRID_SIZE][GRID_SIZE][GRID_SIZE];
bool visited[GRID_SIZE][GRID_SIZE][GRID_SIZE];
Voxel pathLink[GRID_SIZE][GRID_SIZE][GRID_SIZE];

void init() {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			for (int k = 0; k < GRID_SIZE; k++) {
				//weight[i][j][k] = std::numeric_limits<float>::max();
				visited[i][j][k] = false;
			}
		}
	}
}

bool isValidIndex(int i, int j, int k) {
	if (i < 0 || i >= GRID_SIZE || j < 0 || j >= GRID_SIZE || k < 0 || k >= GRID_SIZE) return false;
	return true;
}

void insert_unmarked_neighbours(Voxel voxel, std::priority_queue<Voxel, std::vector<Voxel>, Comparator> &pq) {
	for (int i = -1; i <=1; i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				int x = voxel.x + i;
				int y = voxel.x + j;
				int z = voxel.x + k;
				if (isValidIndex(x, y, z) && !visited[x][y][z] && dfb[x][y][z] > 0) {
					Voxel neighbour;
					neighbour.x = x;
					neighbour.y = y;
					neighbour.z = z;
					neighbour.w = dfb[x][y][z];
					pathLink[x][y][z] = voxel;
					(pathLink[x][y][z]).isNonTerminatingNode = true;
				}
			}
		}
	}
}

void mst_extractor(int x, int y, int z) {
	std::priority_queue<Voxel, std::vector<Voxel>, Comparator> pq;
	std::vector<Voxel> pathLink;

	Voxel s;
	s.x = x;
	s.y = y;
	s.z = z;
	s.w = dfb[x][y][z];
	s.isNonTerminatingNode = false;
	visited[x][y][z] = true;

	Voxel c = s;
	insert_unmarked_neighbours(c, pq);
	while (!pq.empty()) {
		c = pq.top();
		pq.pop();
		visited[c.x][c.y][c.z] = true;
		insert_unmarked_neighbours(c, pq);
	}
}

std::vector<Voxel> centerline_extractor(int x, int y, int z) {
	std::vector<Voxel> centerLine;
	Voxel v;
	v.x = x;
	v.y = y;
	v.z = z;
	while(v.isNonTerminatingNode) {
		centerLine.push_back(v);
		v = pathLink[v.x][v.y][v.z];
	}
	centerLine.push_back(v);

	return centerLine;
}

int main(int argc, const char** argv){

	if( argc != 2 || strcmp(argv[1], "-h") == 0 ){
		fprintf(stderr, "Usage: vdbtest <obj file>\n");
		exit(0);
	}
}
