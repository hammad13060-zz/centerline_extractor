#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/util/Util.h>
#include <openvdb/math/Transform.h>

void split(const std::string& in, std::vector<std::string>& result){
	
	size_t pos = 0;
	size_t ndx = in.find(' ');
	
	while( true ){
	
		if( in[pos] != ' ' )
			result.push_back( in.substr(pos, ndx - pos) );

		if( ndx == std::string::npos ) break;
		pos = ndx+1;
		ndx = in.find(' ', pos);
	}
}

void ReadMesh(const char* filename, 
		std::vector<openvdb::Vec3s>& verts,
		std::vector<openvdb::Vec3I>& trys,
		std::vector<openvdb::Vec4I>& polys){
	
	std::ifstream file;
	file.open(filename, std::ifstream::in);
	
	std::string line;
	std::vector<std::string> vals;
	while(getline(file, line)){

		vals.clear();
		split(line, vals);
		// vertex
		if( line[0] == 'v' ){
			if( vals.size() != 4 ){
				fprintf(stderr, "problem with string: %s\n", line.c_str());
				exit(1);
			}
			verts.push_back( openvdb::Vec3s(
				atof(vals[1].c_str()), atof(vals[2].c_str()),
				atof(vals[3].c_str())) );
		}
		
		// face, using verticies in ->INDEX SPACE ?<-
		else if( line[0] == 'f' ){
			if( vals.size() != 4 && vals.size() != 5 ){
				fprintf(stderr, "problem with string: %s\n", line.c_str());
				exit(1);
			}
			
			else if( vals.size() == 4 ){
				trys.push_back( openvdb::Vec3I(
					atoi(vals[1].c_str()), atoi(vals[2].c_str()),
					atoi(vals[3].c_str())) );
			}

			else{
				polys.push_back( openvdb::Vec4I(
					atoi(vals[1].c_str()), atoi(vals[2].c_str()),
					atoi(vals[3].c_str()), atoi(vals[4].c_str())) );
			}
		}
	}
	openvdb::math::Transform transform = openvdb::math::Transform();
	// transform all the verts to index space
	for( size_t i = 0; i < verts.size(); i++ ){
    	verts[i] = transform.worldToIndex(verts[i]);
	}
	
	file.close();
}

void WriteMesh(const char* filename,
		openvdb::tools::VolumeToMesh &mesh ){
	
	std::ofstream file;
	file.open(filename);
	
	openvdb::tools::PointList *verts = &mesh.pointList();
	openvdb::tools::PolygonPoolList *polys = &mesh.polygonPoolList();
	
	for( size_t i = 0; i < mesh.pointListSize(); i++ ){
		openvdb::Vec3s &v = (*verts)[i];
		file << "v " << v[0] << " " << v[1] << " " << v[2] << std::endl;
	}

	for( size_t i = 0; i < mesh.polygonPoolListSize(); i++ ){
	
		for( size_t ndx = 0; ndx < (*polys)[i].numTriangles(); ndx++ ){
			openvdb::Vec3I *p = &((*polys)[i].triangle(ndx));
			file << "f " << p->x() << " " << p->y() << " " << p->z() << std::endl;
		}

		for( size_t ndx = 0; ndx < (*polys)[i].numQuads(); ndx++ ){
			openvdb::Vec4I *p = &((*polys)[i].quad(ndx));
			file << "f " << p->x() << " " << p->y() << " " 
									 << p->z() << " " << p->w() << std::endl;
		}
	}

	file.close();
}

int main(int argc, const char** argv){

	if( argc != 2 || strcmp(argv[1], "-h") == 0 ){
		fprintf(stderr, "Usage: vdbtest <obj file>\n");
		exit(0);
	}

	openvdb::initialize();

	// extract the verts and faces from object file
	std::vector<openvdb::Vec3s> verts;
	std::vector<openvdb::Vec4I> polys;
	std::vector<openvdb::Vec3I> triangles;
	ReadMesh(argv[1], verts, triangles, polys);

	std::cout << "points: " << verts.size() 
						<< " polys: " << polys.size() << std::endl;
	
	// first create a transform for the grid
	//openvdb::math::Transform::Ptr transform = openvdb::math::Transform::createLinearTransform();
	openvdb::math::Transform transform = openvdb::math::Transform();
	
	// now create the levelset
	/*openvdb::tools::MeshToVolume<openvdb::FloatGrid> levelset(transform);
	levelset.convertToLevelSet(verts, polys);
	
	// get the grid from the levelset
	openvdb::FloatGrid::Ptr grid = levelset.distGridPtr();*/

	//openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(transform, verts, triangles, polys, openvdb::LEVEL_SET_HALF_WIDTH));
	//openvdb::FloatGrid::Ptr grid = openvdb::gridPtrCast<openvdb::FloatGrid>(openvdb::tools::meshToLevelSet(transform, verts, triangles, polys, 3.0f));
	openvdb::FloatGrid::Ptr grid = openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(transform, verts, triangles, polys, 3.0f);

	// convert the level set into a sparse fog volume
	openvdb::tools::sdfToFogVolume<openvdb::FloatGrid>(grid.operator*());
	
	std::cout << "active voxel count: " << grid->activeVoxelCount() << std::endl;

/*	
	// prep some output to write the grid
	std::string output(argv[1]);
	output += ".vdb";
	openvdb::io::File file(output.c_str());
	openvdb::GridPtrVec gridsToWrite;
	gridsToWrite.push_back(grid);
	file.write(gridsToWrite);
	file.close();
*/

	// convert volume back to mesh and output it
	openvdb::tools::VolumeToMesh mesher;
	mesher.operator()<openvdb::FloatGrid>( grid.operator*() );
	std::cout << "points: " << mesher.pointListSize() 
						<< " polys: " << mesher.polygonPoolListSize() << std::endl;

	WriteMesh("testing.obj", mesher);
	return 0;
}
