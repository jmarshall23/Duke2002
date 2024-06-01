// Model_obj.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "tr_local.h"
#include "Model_local.h"
#include "Model_obj.h"

bool OBJ_LoadOBJ(const char* filename, idList<idDrawVert>& vertices, idList<int>& indices) {
	idFile* file = fileSystem->OpenFileRead(filename);
	if (!file) {
		common->Warning("Failed to open OBJ file: %s", filename);
		return false;
	}

	idList<idVec3> positions;
	idList<idVec2> texCoords;
	idList<idVec3> normals;

	idStr line;
	while (file->ReadLine(line)) {
		if (line.Length() == 0 || line[0] == '#') continue;  // Skip empty lines and comments

		if (line.StartsWith("v ")) {
			OBJ_ParseVertex(line, positions);
		}
		else if (line.StartsWith("vt ")) {
			OBJ_ParseTexCoord(line, texCoords);
		}
		else if (line.StartsWith("vn ")) {
			OBJ_ParseNormal(line, normals);
		}
		else if (line.StartsWith("f ")) {
			OBJ_ParseFace(line, indices, positions, texCoords, normals, vertices);
		}
	}

	fileSystem->CloseFile(file);
	return true;
}

void OBJ_ParseVertex(const idStr& line, idList<idVec3>& positions) {
	idVec3 position;
	sscanf(line.c_str(), "v %f %f %f", &position.x, &position.y, &position.z);
	positions.Append(position);
}

void OBJ_ParseTexCoord(const idStr& line, idList<idVec2>& texCoords) {
	idVec2 texCoord;
	sscanf(line.c_str(), "vt %f %f", &texCoord.x, &texCoord.y);
	texCoords.Append(texCoord);
}

void OBJ_ParseNormal(const idStr& line, idList<idVec3>& normals) {
	idVec3 normal;
	sscanf(line.c_str(), "vn %f %f %f", &normal.x, &normal.y, &normal.z);
	normals.Append(normal);
}

void OBJ_ParseFace(const idStr& line, idList<int>& indices, const idList<idVec3>& positions, const idList<idVec2>& texCoords, const idList<idVec3>& normals, idList<idDrawVert>& vertices) {
	int vertexIndex[3], texCoordIndex[3], normalIndex[3];
	sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
		&vertexIndex[0], &texCoordIndex[0], &normalIndex[0],
		&vertexIndex[1], &texCoordIndex[1], &normalIndex[1],
		&vertexIndex[2], &texCoordIndex[2], &normalIndex[2]);

	for (int i = 0; i < 3; i++) {
		idDrawVert vert;
		vert.xyz = positions[vertexIndex[2 - i] - 1];
		vert.xyz = idVec3(vert.xyz.x, -vert.xyz.z, vert.xyz.y);
		vert.st = texCoords[texCoordIndex[2 - i] - 1];
		vert.st.y = 1.0f - vert.st.y;
		vert.normal = normals[normalIndex[2 - i] - 1];
		vertices.Append(vert);
		indices.Append(vertices.Num() - 1);
	}
}
