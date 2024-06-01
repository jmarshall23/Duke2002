// Model_obj.h
//

#pragma once

bool OBJ_LoadOBJ(const char* filename, idList<idDrawVert>& vertices, idList<int>& indices);
void OBJ_ParseVertex(const idStr& line, idList<idVec3>& positions);
void OBJ_ParseTexCoord(const idStr& line, idList<idVec2>& texCoords);
void OBJ_ParseNormal(const idStr& line, idList<idVec3>& normals);
void OBJ_ParseFace(const idStr& line, idList<int>& indices, const idList<idVec3>& positions, const idList<idVec2>& texCoords, const idList<idVec3>& normals, idList<idDrawVert>& vertices);