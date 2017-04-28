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

void read_dfb_3d_matrix() {
    int i,j,k;
    std::string delimiter = " ";
    std::string line;
    std::ifstream dfb_file("./../DFB_files/dfb_bunny.txt");
    if (dfb_file.is_open())
    {
        i = 0;
        k = 0;
        while ( getline (dfb_file,line) )
        {
            if(line.size() < 2) {
                i = 0;
                k += 1;
                continue;
            }
            size_t pos = 0;
            std::string token;
            j = 0;
            while ((pos = line.find(delimiter)) != std::string::npos) {
                token = line.substr(0, pos);
                float d = std::atof(token.c_str());
                dfb[i][j][k] = d;
                //std::cout << i << " " << j << " " << k << std::endl;
                //if (d > 0.0) std::cout << d << std::endl;
                //std::cout << token << std::endl;
                line.erase(0, pos + delimiter.length());
                j+=1;
            }
            i+=1;

        }
        dfb_file.close();
    }
}

void init() {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			for (int k = 0; k < GRID_SIZE; k++) {
				//weight[i][j][k] = std::numeric_limits<float>::max();
				visited[i][j][k] = false;
				(pathLink[i][j][k]).isNonTerminatingNode = false;
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
				int y = voxel.y + j;
				int z = voxel.z + k;
				if (isValidIndex(x, y, z) && !visited[x][y][z] && dfb[x][y][z] > 0) {
					Voxel neighbour;
					neighbour.x = x;
					neighbour.y = y;
					neighbour.z = z;
					neighbour.w = dfb[x][y][z];
					pathLink[x][y][z] = voxel;
					(pathLink[x][y][z]).isNonTerminatingNode = true;
					visited[neighbour.x][neighbour.y][neighbour.z] = true;
					pq.push(neighbour);
				}
			}
		}
	}
	//std::cout << "pq size: " << pq.size() << std::endl;
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
		//std::cout << c.x << " " << c.y << " " << c.z << std::endl;
		//std::cout << "before size " << pq.size() << std::endl;
		pq.pop();
		//std::cout << "after size " << pq.size() << std::endl;
		//return;
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
	v.isNonTerminatingNode = true;
	while(v.isNonTerminatingNode) {
		centerLine.push_back(v);
		v = pathLink[v.x][v.y][v.z];
	}
	centerLine.push_back(v);

	return centerLine;
}

int main(int argc, const char** argv){

//	if( argc != 2 || strcmp(argv[1], "-h") == 0 ){
//		fprintf(stderr, "Usage: vdbtest <obj file>\n");
//		exit(0);
//	}
    read_dfb_3d_matrix();
    init();
    mst_extractor(0, 21, 47);
    std::vector<Voxel> centerLine = centerline_extractor(45, 105, 14);
    for (int i = 0; i < centerLine.size(); i++) {
        std::cout << centerLine[i].x << " " << centerLine[i].y << " " << centerLine[i].z << std::endl;
    }
}
