/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

void		Eclass_InitForSourceDirectory( const char *path );
eclass_t *	Eclass_ForName( const char *name, bool has_brushes );
bool		Eclass_hasModel(eclass_t *e, idVec3 &vMin, idVec3 &vMax);

class idEditorEntity {
public:
	class idEditorEntity *prev, *next;
	idEditorBrush		brushes;					// head/tail of list
	int			undoId, redoId, entityId;	// used for undo/redo
	idVec3		origin;
	qhandle_t	lightDef;
	qhandle_t	modelDef;
	idSoundEmitter *soundEmitter;
	eclass_t *	eclass;
	idDict		epairs;
	eclass_t *	md3Class;
	idMat3		rotation;
	idVec3 		lightOrigin;		// for lights that have been combined with models
	idMat3		lightRotation;		// ''
	bool		trackLightOrigin;	
	idCurve<idVec3> *curve;

	renderEntity_t	refent;

public:
	void		BuildEntityRenderState(bool update);
};

void		ParseEpair(idDict *dict);
const char *ValueForKey(idEditorEntity *ent, const char *key);
int			GetNumKeys(idEditorEntity *ent);
const char *GetKeyString(idEditorEntity *ent, int iIndex);
void		SetKeyValue (idEditorEntity *ent, const char *key, const char *value, bool trackAngles = true);
void		DeleteKey (idEditorEntity *ent, const char *key);
float		FloatForKey (idEditorEntity *ent, const char *key);
int			IntForKey (idEditorEntity *ent, const char *key);
bool		GetVectorForKey (idEditorEntity *ent, const char *key, idVec3 &vec);
bool		GetVector4ForKey (idEditorEntity *ent, const char *key, idVec4 &vec);
bool		GetFloatForKey(idEditorEntity *end, const char *key, float *f);
void		SetKeyVec3(idEditorEntity *ent, const char *key, idVec3 v);
void		SetKeyMat3(idEditorEntity *ent, const char *key, idMat3 m);
bool		GetMatrixForKey(idEditorEntity *ent, const char *key, idMat3 &mat);

void		Entity_UpdateSoundEmitter( idEditorEntity *ent );
idCurve<idVec3> *Entity_MakeCurve( idEditorEntity *e );
void		Entity_UpdateCurveData( idEditorEntity *e );
void		Entity_SetCurveData( idEditorEntity *e );
void		Entity_Free (idEditorEntity *e);
void		Entity_FreeEpairs(idEditorEntity *e);
int			Entity_MemorySize(idEditorEntity *e);
idEditorEntity *	Entity_Parse (bool onlypairs, idEditorBrush* pList = NULL);
void		Entity_Write (idEditorEntity *e, FILE *f, bool use_region);
void		Entity_WriteSelected(idEditorEntity *e, FILE *f);
void		Entity_WriteSelected(idEditorEntity *e, CMemFile*);
idEditorEntity *	Entity_Create (eclass_t *c, bool forceFixed = false);
idEditorEntity *	Entity_Clone (idEditorEntity *e);
void		Entity_AddToList(idEditorEntity *e, idEditorEntity *list);
void		Entity_RemoveFromList(idEditorEntity *e);
bool		EntityHasModel(idEditorEntity *ent);

void		Entity_LinkBrush (idEditorEntity *e, idEditorBrush *b);
void		Entity_UnlinkBrush (idEditorBrush *b);
idEditorEntity *	FindEntity(const char *pszKey, const char *pszValue);
idEditorEntity *	FindEntityInt(const char *pszKey, int iValue);
idEditorEntity *	Entity_New();
void		Entity_SetName(idEditorEntity *e, const char *name);

int			GetUniqueTargetId(int iHint);
eclass_t *	GetCachedModel(idEditorEntity *pEntity, const char *pName, idVec3 &vMin, idVec3 &vMax);

//Timo : used for parsing epairs in brush primitive
void		Entity_Name(idEditorEntity *e, bool force);

bool		IsBrushSelected(idEditorBrush* bSel);
