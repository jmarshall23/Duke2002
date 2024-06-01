/*
===========================================================================

IcedTech
(c) 2024 by Justin Marshall - icecolddukde

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

#include "precompiled.h"
#include "tr_local.h"

#define NUM_SHADOW_GROUPS			6

static GLuint shadowFBO[NUM_SHADOW_GROUPS][6];
static GLuint shadowDepth[NUM_SHADOW_GROUPS][6];

const idDeclRenderProg* interactionProgram;
const idDeclRenderProg* baseDepthFillProgram;
const idDeclRenderProg* shadowDepthFillProgram;
const idDeclRenderProg* fogProgram;
const idDeclRenderProg* genericShaderPassProgram;

idCVar r_sb_maxDrawDistance("r_sb_maxDrawDistance", "-1", CVAR_RENDERER | CVAR_INTEGER, "don't draw anything beyond this point");
idCVar r_sb_shadowDrawDistance("r_sb_shadowDrawDistance", "1000", CVAR_RENDERER | CVAR_INTEGER, "don't draw shadows beyond this point");
idCVar r_sb_noShadows("r_sb_noShadows", "0", CVAR_RENDERER | CVAR_BOOL, "don't draw any occluders");
idCVar r_sb_shadowMapSize("r_sb_shadowMapSize", "2048", CVAR_RENDERER | CVAR_FLOAT, "Shadow Map Size");
idCVar r_sb_frustomFOV("r_sb_frustomFOV", "92", CVAR_RENDERER | CVAR_FLOAT, "oversize FOV for point light side matching");
idCVar r_sb_useCulling("r_sb_useCulling", "1", CVAR_RENDERER | CVAR_BOOL, "cull geometry to individual side frustums");

idCVar r_sb_polyOfsFactor("r_sb_polyOfsFactor", "0", CVAR_RENDERER | CVAR_FLOAT, "polygonOffset factor for drawing shadow buffer");
idCVar r_sb_polyOfsUnits("r_sb_polyOfsUnits", "3000", CVAR_RENDERER | CVAR_FLOAT, "polygonOffset units for drawing shadow buffer");
idCVar r_sb_occluderFacing("r_sb_occluderFacing", "1", CVAR_RENDERER | CVAR_INTEGER, "0 = front faces, 1 = back faces, 2 = midway between");

idCVar r_sb_singleSide("r_sb_singleSide", "-1", CVAR_RENDERER | CVAR_INTEGER, "only draw a single side (0-5) of point lights");

// from world space to light origin, looking down the X axis
static float	unflippedLightMatrix[16];

// from world space to OpenGL view space, looking down the negative Z axis
static float	lightMatrix[6][16];
static float	mvpLightMatrix[6][16];

// from OpenGL view space to OpenGL NDC ( -1 : 1 in XYZ )
static float	lightProjectionMatrix[16];

static int		lightBufferSize = -1;
static float	viewLightAxialSize;

static int		lightNumSides = -1;

static bool		depthFillActive = false;
static bool		genericRendererActive = false;

static int		shadowGroupNum = 0;

// Declare a global variable for uniform state
idInteractionUniformState interactionUniformState;
idDepthFillUniformState depthFillUniformState;
idDepthFillUniformState shadowFillUniformState;
idGenericRenderUniformState genericRenderUniformState;

/*
====================
RB_EXP_CreateFBO
====================
*/
static void RB_EXP_CreateFBO(GLuint* fbo, GLuint* color, GLuint* depth, int width, int height) {
	qglGenFramebuffersEXT(1, fbo);
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *fbo);

	if (color != nullptr) {
		// Create color texture
		qglGenTextures(1, color);
		qglBindTexture(GL_TEXTURE_2D, *color);
		qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, *color, 0);
	}

	// Create depth texture
	qglGenTextures(1, depth);
	qglBindTexture(GL_TEXTURE_2D, *depth);
	qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, *depth, 0);

	GLenum status = qglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		common->Error("Failed to create FBO");
	}

	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/*
====================
RB_EXP_Init
====================
*/
void RB_EXP_Init(void) {
	lightBufferSize = r_sb_shadowMapSize.GetInteger();

	for (int d = 0; d < NUM_SHADOW_GROUPS; d++)
	{
		for (int i = 0; i < 6; i++) {
			RB_EXP_CreateFBO(&shadowFBO[d][i], nullptr, &shadowDepth[d][i], lightBufferSize, lightBufferSize);
		}
	}

	interactionProgram = renderSystem->FindRenderProgram("interaction", false);
	baseDepthFillProgram = renderSystem->FindRenderProgram("depthfill", false);
	shadowDepthFillProgram = renderSystem->FindRenderProgram("shadowfill", false);
	fogProgram = renderSystem->FindRenderProgram("postprocess/fog", false);
	genericShaderPassProgram = renderSystem->FindRenderProgram("shaderpasses/generic", false);

	{
		GLuint program = genericShaderPassProgram->GetProgram();
		genericRenderUniformState.textureMatrix = qglGetUniformLocation(program, "textureMatrix");
		genericRenderUniformState.diffuseImage = qglGetUniformLocation(program, "diffuseImage");
	}
	
	{
		GLuint program = baseDepthFillProgram->GetProgram();
		depthFillUniformState.textureMatrix = qglGetUniformLocation(program, "textureMatrix");
		depthFillUniformState.ambientLightColor = qglGetUniformLocation(program, "ambientLightColor");
		depthFillUniformState.diffuseImage = qglGetUniformLocation(program, "diffuseImage");
	}

	{
		GLuint program = shadowDepthFillProgram->GetProgram();
		shadowFillUniformState.textureMatrix = qglGetUniformLocation(program, "textureMatrix");
		shadowFillUniformState.ambientLightColor = qglGetUniformLocation(program, "ambientLightColor");
		shadowFillUniformState.diffuseImage = qglGetUniformLocation(program, "diffuseImage");
	}

	// Get uniform locations and store them in interactionUniformState
	{
		GLuint program = interactionProgram->GetProgram();
		interactionUniformState.lightMatrices = qglGetUniformLocation(program, "lightMatrices");
		interactionUniformState.modelMatrix = qglGetUniformLocation(program, "modelMatrix");
		interactionUniformState.lightOrigin = qglGetUniformLocation(program, "lightOrigin");
		interactionUniformState.viewOrigin = qglGetUniformLocation(program, "viewOrigin");
		interactionUniformState.lightProjectS = qglGetUniformLocation(program, "lightProjectS");
		interactionUniformState.lightProjectT = qglGetUniformLocation(program, "lightProjectT");
		interactionUniformState.lightProjectQ = qglGetUniformLocation(program, "lightProjectQ");
		interactionUniformState.lightFalloffS = qglGetUniformLocation(program, "lightFalloffS");
		interactionUniformState.bumpMatrixS = qglGetUniformLocation(program, "bumpMatrixS");
		interactionUniformState.bumpMatrixT = qglGetUniformLocation(program, "bumpMatrixT");
		interactionUniformState.diffuseMatrixS = qglGetUniformLocation(program, "diffuseMatrixS");
		interactionUniformState.diffuseMatrixT = qglGetUniformLocation(program, "diffuseMatrixT");
		interactionUniformState.specularMatrixS = qglGetUniformLocation(program, "specularMatrixS");
		interactionUniformState.specularMatrixT = qglGetUniformLocation(program, "specularMatrixT");
		interactionUniformState.colorModulate = qglGetUniformLocation(program, "colorModulate");
		interactionUniformState.colorAdd = qglGetUniformLocation(program, "colorAdd");
		interactionUniformState.diffuse = qglGetUniformLocation(program, "diffuse");
		interactionUniformState.specular = qglGetUniformLocation(program, "specular");
		interactionUniformState.bumpImage = qglGetUniformLocation(program, "bumpImage");
		interactionUniformState.lightFalloffImage = qglGetUniformLocation(program, "lightFalloffImage");
		interactionUniformState.lightImage = qglGetUniformLocation(program, "lightImage");
		interactionUniformState.diffuseImage = qglGetUniformLocation(program, "diffuseImage");
		interactionUniformState.specularImage = qglGetUniformLocation(program, "specularImage");
		interactionUniformState.specularTableImage = qglGetUniformLocation(program, "specularTableImage");

		for (int i = 0; i < 6; i++) {
			interactionUniformState.shadowMaps[i] = qglGetUniformLocation(program, va("shadowMaps[%d]", i));
		}

		interactionUniformState.numSides = qglGetUniformLocation(program, "numSides");
	}	
}

/*
====================
RB_EXP_Shutdown
====================
*/
void RB_EXP_Shutdown(void) {
	for (int d = 0; d < NUM_SHADOW_GROUPS; d++)
	{
		for (int i = 0; i < 6; i++) {
			qglDeleteFramebuffersEXT(1, &shadowFBO[d][i]);
		}
	}
}

/*
====================
RB_EXP_BindDepthFill
====================
*/
void RB_EXP_BindDepthFill(void) {
	GLuint program = baseDepthFillProgram->GetProgram(); 

	depthFillActive = true;

	// Use the shader program
	qglUseProgram(program);
	qglUniform1i(depthFillUniformState.diffuseImage, 0);

	idVec4 ambientLightColor(backEnd.viewDef->atmosphere.ambientLightColor.x, backEnd.viewDef->atmosphere.ambientLightColor.y, backEnd.viewDef->atmosphere.ambientLightColor.z, 1.0);
	qglUniform4fv(depthFillUniformState.ambientLightColor, 1, ambientLightColor.ToFloatPtr());
}

/*
====================
RB_EXP_BindGenericRender
====================
*/
void RB_EXP_BindGenericRender(void) {
	GLuint program = genericShaderPassProgram->GetProgram();

	genericRendererActive = true;

	// Use the shader program
	qglUseProgram(program);
	qglUniform1i(genericRenderUniformState.diffuseImage, 0);
}

/*
====================
RB_EXP_UploadTextureMatrix
====================
*/
void RB_EXP_UploadTextureMatrix(float matrix[16]) {
	if (depthFillActive)
	{
		qglUniformMatrix4fv(depthFillUniformState.textureMatrix, 1, GL_FALSE, matrix);
	}

	if (genericRendererActive)
	{
		//qglUniformMatrix4fv(genericRenderUniformState.textureMatrix, 1, GL_FALSE, matrix);
	}
}

/*
====================
RB_EXP_UnbindDepthFill
====================
*/
void RB_EXP_UnbindDepthFill(void) {
	depthFillActive = false;

	qglUseProgram(0);
}

/*
====================
RB_EXP_UnBind_GenericShader
====================
*/
void RB_EXP_UnBind_GenericShader(void) {
	genericRendererActive = false;

	qglUseProgram(0);
}


/*
====================
RB_EXP_BindFBO
====================
*/
void RB_EXP_BindFBO(GLuint fbo) {
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
}

/*
====================
RB_EXP_UnbindFBO
====================
*/
void RB_EXP_UnbindFBO(void) {
	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/*
==================
R_EXP_CalcLightAxialSize

all light side projections must currently match, so non-centered
and non-cubic lights must take the largest length
==================
*/
float	R_EXP_CalcLightAxialSize(viewLight_t* vLight) {
	float	max = 0;

	if (!vLight->lightDef->parms.pointLight) {
		idVec3	dir = vLight->lightDef->parms.target - vLight->lightDef->parms.origin;
		max = dir.Length();
		return max;
	}

	for (int i = 0; i < 3; i++) {
		float	dist = fabs(vLight->lightDef->parms.lightCenter[i]);
		dist += vLight->lightDef->parms.lightRadius[i];
		if (dist > max) {
			max = dist;
		}
	}
	return max;
}


static const int CULL_RECEIVER = 1;	// still draw occluder, but it is out of the view
static const int CULL_OCCLUDER_AND_RECEIVER = 2;	// the surface doesn't effect the view at all

/*
==================
RB_EXP_CullInteractions

Sets surfaceInteraction_t->cullBits
==================
*/
void RB_EXP_CullInteractions(viewLight_t* vLight, idPlane frustumPlanes[6]) {
	for (idInteraction* inter = vLight->lightDef->firstInteraction; inter; inter = inter->lightNext) {
		const idRenderEntityLocal* entityDef = inter->entityDef;
		if (!entityDef) {
			continue;
		}
		if (inter->numSurfaces < 1) {
			continue;
		}

		int	culled = 0;

		if (r_sb_useCulling.GetBool()) {
			// transform light frustum into object space, positive side points outside the light
			idPlane	localPlanes[6];
			int		plane;
			for (plane = 0; plane < 6; plane++) {
				R_GlobalPlaneToLocal(entityDef->modelMatrix, frustumPlanes[plane], localPlanes[plane]);
			}

			// cull the entire entity bounding box
			// has referenceBounds been tightened to the actual model bounds?
			idVec3	corners[8];
			for (int i = 0; i < 8; i++) {
				corners[i][0] = entityDef->referenceBounds[i & 1][0];
				corners[i][1] = entityDef->referenceBounds[(i >> 1) & 1][1];
				corners[i][2] = entityDef->referenceBounds[(i >> 2) & 1][2];
			}

			for (plane = 0; plane < 6; plane++) {
				int		j;
				for (j = 0; j < 8; j++) {
					// if a corner is on the negative side (inside) of the frustum, the surface is not culled
					// by this plane
					if (corners[j] * localPlanes[plane].ToVec4().ToVec3() + localPlanes[plane][3] < 0) {
						break;
					}
				}
				if (j == 8) {
					break;			// all points outside the light
				}
			}
			if (plane < 6) {
				culled = CULL_OCCLUDER_AND_RECEIVER;
			}
		}

		for (int i = 0; i < inter->numSurfaces; i++) {
			surfaceInteraction_t* surfInt = &inter->surfaces[i];

			if (!surfInt->ambientTris) {
				continue;
			}
			surfInt->expCulled = culled;
		}

	}
}


/*
==================
RB_EXP_RenderOccluders
==================
*/
void RB_EXP_RenderOccluders(viewLight_t* vLight, int side) {
	shadowDepthFillProgram->Bind();
	GL_SelectTexture(0);
	qglUniform1i(shadowFillUniformState.diffuseImage, 0);
	idVec4 ambientLightColor(1.0, 1.0, 1.0, 1.0);
	qglUniform4fv(shadowFillUniformState.ambientLightColor, 1, ambientLightColor.ToFloatPtr());

	for (idInteraction* inter = vLight->lightDef->firstInteraction; inter; inter = inter->lightNext) {
		const idRenderEntityLocal* entityDef = inter->entityDef;
		if (!entityDef) {
			continue;
		}
		if (inter->numSurfaces < 1) {
			continue;
		}

		// no need to check for current on this, because each interaction is always
		// a different space
		float matrix[16];
		qglMatrixMode(GL_MODELVIEW);
		myGlMultMatrix(inter->entityDef->modelMatrix, lightMatrix[side], matrix);
		qglLoadMatrixf(matrix);

		// draw each surface
		for (int i = 0; i < inter->numSurfaces; i++) {
			surfaceInteraction_t* surfInt = &inter->surfaces[i];

			if (!surfInt->ambientTris) {
				continue;
			}
			if (surfInt->shader && !surfInt->shader->SurfaceCastsShadow()) {
				continue;
			}

			// cull it
			if (surfInt->expCulled == CULL_OCCLUDER_AND_RECEIVER) {
				continue;
			}

			float textureIdentityMatrix[16] = {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};			

			// render it
			const srfTriangles_t* tri = surfInt->ambientTris;

			if (tri->numIndexes <= 0) {
				continue;
			}

			if (!tri->ambientCache) {
				R_CreateAmbientCache(const_cast<srfTriangles_t*>(tri), false);
			}
			idDrawVert* ac = (idDrawVert*)vertexCache.Position(tri->ambientCache);

			// Vertex positions
			qglEnableVertexAttribArrayARB(ATTR_POSITION);
			qglVertexAttribPointerARB(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->xyz.ToFloatPtr());

			// Texture coordinates
			qglEnableVertexAttribArrayARB(ATTR_TEXCOORD);
			qglVertexAttribPointerARB(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->st.ToFloatPtr());

			// Normals
			qglEnableVertexAttribArrayARB(ATTR_NORMAL);
			qglVertexAttribPointerARB(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->normal.ToFloatPtr());

			// Tangent 0
			qglEnableVertexAttribArrayARB(ATTR_TANGENT0);
			qglVertexAttribPointerARB(ATTR_TANGENT0, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->tangents[0].ToFloatPtr());

			// Tangent 1
			qglEnableVertexAttribArrayARB(ATTR_TANGENT1);
			qglVertexAttribPointerARB(ATTR_TANGENT1, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->tangents[1].ToFloatPtr());

			RB_EXP_UploadTextureMatrix(textureIdentityMatrix);
			if (surfInt->shader) {
				const shaderStage_t* pStage = surfInt->shader->GetDiffuseStage();

				if (pStage)
				{
					pStage->texture.image->Bind();

					// set the texture matrix if needed
#if 0
					if (pStage->texture.hasMatrix) {
						drawSurf_t tempSurface = {};
						tempSurface.geo = surfInt->ambientTris;
						tempSurface.material = surfInt->shader;
						tempSurface.shaderRegisters = surfInt->shader->ConstantRegisters();

						if (tempSurface.shaderRegisters == nullptr)
						{
							float* regs = (float*)R_FrameAlloc(surfInt->shader->GetNumRegisters() * sizeof(float));
							tempSurface.shaderRegisters = regs;
							surfInt->shader->EvaluateRegisters(regs, entityDef->parms.shaderParms, backEnd.viewDef, entityDef->parms.referenceSound);
						}

						RB_LoadShaderTextureMatrix(tempSurface.shaderRegisters, &pStage->texture);
					}
#endif
				}
				else
				{
					globalImages->whiteImage->Bind();				
				}
			}
			else {
				globalImages->whiteImage->Bind();
			}

			RB_DrawElementsWithCounters(tri);
		}
	}

	qglUseProgram(0);
	qglDisableVertexAttribArrayARB(ATTR_POSITION);
	qglDisableVertexAttribArrayARB(ATTR_TEXCOORD);
	qglDisableVertexAttribArrayARB(ATTR_NORMAL);
	qglDisableVertexAttribArrayARB(ATTR_TANGENT0);
	qglDisableVertexAttribArrayARB(ATTR_TANGENT1);
	qglDisableVertexAttribArrayARB(ATTR_COLOR);

	globalImages->whiteImage->Bind();
}


/*
==================
RB_RenderShadowBuffer
==================
*/
void RB_RenderShadowBuffer(viewLight_t * vLight, int side) {
	float	xmin, xmax, ymin, ymax;
	float	width, height;
	float	zNear;

	float	fov = r_sb_frustomFOV.GetFloat();

	depthFillActive = true;

	//
	// set up 90 degree projection matrix
	//
	zNear = 4;

	ymax = zNear * tan(fov * idMath::PI / 360.0f);
	ymin = -ymax;

	xmax = zNear * tan(fov * idMath::PI / 360.0f);
	xmin = -xmax;

	width = xmax - xmin;
	height = ymax - ymin;

	lightProjectionMatrix[0] = 2 * zNear / width;
	lightProjectionMatrix[4] = 0;
	lightProjectionMatrix[8] = 0;
	lightProjectionMatrix[12] = 0;

	lightProjectionMatrix[1] = 0;
	lightProjectionMatrix[5] = 2 * zNear / height;
	lightProjectionMatrix[9] = 0;
	lightProjectionMatrix[13] = 0;

	// this is the far-plane-at-infinity formulation, and
	// crunches the Z range slightly so w=0 vertexes do not
	// rasterize right at the wraparound point
	lightProjectionMatrix[2] = 0;
	lightProjectionMatrix[6] = 0;
	lightProjectionMatrix[10] = -0.999f;
	lightProjectionMatrix[14] = -2.0f * zNear;

	lightProjectionMatrix[3] = 0;
	lightProjectionMatrix[7] = 0;
	lightProjectionMatrix[11] = -1;
	lightProjectionMatrix[15] = 0;

	RB_EXP_BindFBO(shadowFBO[shadowGroupNum][side]);

	qglMatrixMode(GL_PROJECTION);
	qglLoadMatrixf(lightProjectionMatrix);
	qglMatrixMode(GL_MODELVIEW);

	qglViewport(0, 0, lightBufferSize, lightBufferSize);
	qglScissor(0, 0, lightBufferSize, lightBufferSize);
	qglStencilFunc(GL_ALWAYS, 0, 255);

	qglClearColor(0, 0, 0, 0);
	qglClearDepth(1.0);
	GL_State(GLS_DEPTHFUNC_LESS | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);	// make sure depth mask is off before clear
	qglClear(GL_DEPTH_BUFFER_BIT);

	// draw all the occluders
	qglColor3f(1, 1, 1);
	GL_SelectTexture(0);
	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);

	backEnd.currentSpace = NULL;

	static float	s_flipMatrix[16] = {
		// convert from our coordinate system (looking down X)
		// to OpenGL's coordinate system (looking down -Z)
		0, 0, -1, 0,
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	};

	float	viewMatrix[16];

	idVec3	vec;
	idVec3	origin = vLight->lightDef->globalLightOrigin;

	if (!vLight->lightDef->parms.pointLight) {
		// projected light
		vec = vLight->lightDef->parms.target;
		vec.Normalize();
		viewMatrix[0] = vec[0];
		viewMatrix[4] = vec[1];
		viewMatrix[8] = vec[2];

		vec = vLight->lightDef->parms.right;
		vec.Normalize();
		viewMatrix[1] = -vec[0];
		viewMatrix[5] = -vec[1];
		viewMatrix[9] = -vec[2];

		vec = vLight->lightDef->parms.up;
		vec.Normalize();
		viewMatrix[2] = vec[0];
		viewMatrix[6] = vec[1];
		viewMatrix[10] = vec[2];
	}
	else {
		// side of a point light
		memset(viewMatrix, 0, sizeof(viewMatrix));
		switch (side) {
		case 0:
			viewMatrix[0] = 1;
			viewMatrix[9] = 1;
			viewMatrix[6] = -1;
			break;
		case 1:
			viewMatrix[0] = -1;
			viewMatrix[9] = -1;
			viewMatrix[6] = -1;
			break;
		case 2:
			viewMatrix[4] = 1;
			viewMatrix[1] = -1;
			viewMatrix[10] = 1;
			break;
		case 3:
			viewMatrix[4] = -1;
			viewMatrix[1] = -1;
			viewMatrix[10] = -1;
			break;
		case 4:
			viewMatrix[8] = 1;
			viewMatrix[1] = -1;
			viewMatrix[6] = -1;
			break;
		case 5:
			viewMatrix[8] = -1;
			viewMatrix[1] = 1;
			viewMatrix[6] = -1;
			break;
		}
	}

	viewMatrix[12] = -origin[0] * viewMatrix[0] + -origin[1] * viewMatrix[4] + -origin[2] * viewMatrix[8];
	viewMatrix[13] = -origin[0] * viewMatrix[1] + -origin[1] * viewMatrix[5] + -origin[2] * viewMatrix[9];
	viewMatrix[14] = -origin[0] * viewMatrix[2] + -origin[1] * viewMatrix[6] + -origin[2] * viewMatrix[10];

	viewMatrix[3] = 0;
	viewMatrix[7] = 0;
	viewMatrix[11] = 0;
	viewMatrix[15] = 1;

	memcpy(unflippedLightMatrix, viewMatrix, sizeof(unflippedLightMatrix));
	myGlMultMatrix(viewMatrix, s_flipMatrix, lightMatrix[side]);

	myGlMultMatrix(lightMatrix[side], lightProjectionMatrix, mvpLightMatrix[side]);

	// create frustum planes
	idPlane	globalFrustum[6];

	// near clip
	globalFrustum[0][0] = -viewMatrix[0];
	globalFrustum[0][1] = -viewMatrix[4];
	globalFrustum[0][2] = -viewMatrix[8];
	globalFrustum[0][3] = -(origin[0] * globalFrustum[0][0] + origin[1] * globalFrustum[0][1] + origin[2] * globalFrustum[0][2]);

	// far clip
	globalFrustum[1][0] = viewMatrix[0];
	globalFrustum[1][1] = viewMatrix[4];
	globalFrustum[1][2] = viewMatrix[8];
	globalFrustum[1][3] = -globalFrustum[0][3] - viewLightAxialSize;

	// side clips
	globalFrustum[2][0] = -viewMatrix[0] + viewMatrix[1];
	globalFrustum[2][1] = -viewMatrix[4] + viewMatrix[5];
	globalFrustum[2][2] = -viewMatrix[8] + viewMatrix[9];

	globalFrustum[3][0] = -viewMatrix[0] - viewMatrix[1];
	globalFrustum[3][1] = -viewMatrix[4] - viewMatrix[5];
	globalFrustum[3][2] = -viewMatrix[8] - viewMatrix[9];

	globalFrustum[4][0] = -viewMatrix[0] + viewMatrix[2];
	globalFrustum[4][1] = -viewMatrix[4] + viewMatrix[6];
	globalFrustum[4][2] = -viewMatrix[8] + viewMatrix[10];

	globalFrustum[5][0] = -viewMatrix[0] - viewMatrix[2];
	globalFrustum[5][1] = -viewMatrix[4] - viewMatrix[6];
	globalFrustum[5][2] = -viewMatrix[8] - viewMatrix[10];

	// is this nromalization necessary?
	for (int i = 0; i < 6; i++) {
		globalFrustum[i].ToVec4().ToVec3().Normalize();
	}

	for (int i = 2; i < 6; i++) {
		globalFrustum[i][3] = -(origin * globalFrustum[i].ToVec4().ToVec3());
	}

	RB_EXP_CullInteractions(vLight, globalFrustum);


	// FIXME: we want to skip the sampling as well as the generation when not casting shadows
	if (!r_sb_noShadows.GetBool() && vLight->lightShader->LightCastsShadows()) {
		//
		// set polygon offset for the rendering
		//
		switch (r_sb_occluderFacing.GetInteger()) {
		case 0:		// front sides
			qglPolygonOffset(r_sb_polyOfsFactor.GetFloat(), r_sb_polyOfsUnits.GetFloat());
			qglEnable(GL_POLYGON_OFFSET_FILL);
			RB_EXP_RenderOccluders(vLight, side);
			qglDisable(GL_POLYGON_OFFSET_FILL);
			break;
		case 1:		// back sides
			qglPolygonOffset(-r_sb_polyOfsFactor.GetFloat(), -r_sb_polyOfsUnits.GetFloat());
			qglEnable(GL_POLYGON_OFFSET_FILL);
			GL_Cull(CT_BACK_SIDED);
			RB_EXP_RenderOccluders(vLight, side);
			GL_Cull(CT_FRONT_SIDED);
			qglDisable(GL_POLYGON_OFFSET_FILL);
			break;
		case 2:		// both sides
#if 0
			GL_Cull(CT_BACK_SIDED);
			RB_EXP_RenderOccluders(vLight);
			GL_Cull(CT_FRONT_SIDED);
			shadowImage[2]->Bind();
			qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize);

			RB_EXP_RenderOccluders(vLight);
			shadowImage[1]->Bind();
			qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize);

			// fragment program to combine the two depth images
			qglBindProgramARB(GL_VERTEX_PROGRAM_ARB, depthMidpointVertexProgram);
			qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, depthMidpointFragmentProgram);
			qglEnable(GL_VERTEX_PROGRAM_ARB);
			qglEnable(GL_FRAGMENT_PROGRAM_ARB);

			GL_SelectTextureNoClient(1);
			shadowImage[1]->Bind();
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			GL_SelectTextureNoClient(0);
			shadowImage[2]->Bind();
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			// draw a full screen quad
			qglMatrixMode(GL_PROJECTION);
			qglLoadIdentity();
			qglOrtho(0, 1, 0, 1, -1, 1);
			qglMatrixMode(GL_MODELVIEW);
			qglLoadIdentity();

			GL_State(GLS_DEPTHFUNC_ALWAYS);

			qglBegin(GL_TRIANGLE_FAN);
			qglTexCoord2f(0, 0);
			qglVertex2f(0, 0);
			qglTexCoord2f(0, lightBufferSizeFraction);
			qglVertex2f(0, 1);
			qglTexCoord2f(lightBufferSizeFraction, lightBufferSizeFraction);
			qglVertex2f(1, 1);
			qglTexCoord2f(lightBufferSizeFraction, 0);
			qglVertex2f(1, 0);
			qglEnd();

			qglDisable(GL_VERTEX_PROGRAM_ARB);
			qglDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif
			break;
		}
	}

	qglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);


	// reset the normal view matrix

	qglMatrixMode(GL_PROJECTION);
	qglLoadMatrixf(backEnd.viewDef->projectionMatrix);
	qglMatrixMode(GL_MODELVIEW);

	// the current modelView matrix is not valid
	backEnd.currentSpace = NULL;

	depthFillActive = false;
}

void GL_SelectTextureNoClient(int unit);

void RB_EXP_DrawInteraction(const drawInteraction_t* din) {
	GLuint program = interactionProgram->GetProgram(); // Assuming the shader program ID is stored in drawInteraction_t

	// Use the shader program
	qglUseProgram(program);

	// Set uniforms for light matrices
	qglUniformMatrix4fv(interactionUniformState.lightMatrices, 6, GL_FALSE, &mvpLightMatrix[0][0]);
	qglUniformMatrix4fv(interactionUniformState.modelMatrix, 1, GL_FALSE, din->surfModelMatrix);

	qglUniform4fv(interactionUniformState.lightOrigin, 1, din->localLightOrigin.ToFloatPtr());
	qglUniform4fv(interactionUniformState.viewOrigin, 1, din->localViewOrigin.ToFloatPtr());
	qglUniform4fv(interactionUniformState.lightProjectS, 1, din->lightProjection[0].ToFloatPtr());
	qglUniform4fv(interactionUniformState.lightProjectT, 1, din->lightProjection[1].ToFloatPtr());
	qglUniform4fv(interactionUniformState.lightProjectQ, 1, din->lightProjection[2].ToFloatPtr());
	qglUniform4fv(interactionUniformState.lightFalloffS, 1, din->lightProjection[3].ToFloatPtr());
	qglUniform4fv(interactionUniformState.bumpMatrixS, 1, din->bumpMatrix[0].ToFloatPtr());
	qglUniform4fv(interactionUniformState.bumpMatrixT, 1, din->bumpMatrix[1].ToFloatPtr());
	qglUniform4fv(interactionUniformState.diffuseMatrixS, 1, din->diffuseMatrix[0].ToFloatPtr());
	qglUniform4fv(interactionUniformState.diffuseMatrixT, 1, din->diffuseMatrix[1].ToFloatPtr());
	qglUniform4fv(interactionUniformState.specularMatrixS, 1, din->specularMatrix[0].ToFloatPtr());
	qglUniform4fv(interactionUniformState.specularMatrixT, 1, din->specularMatrix[1].ToFloatPtr());

	static const float zero[4] = { 0, 0, 0, 0 };
	static const float one[4] = { 1, 1, 1, 1 };
	static const float negOne[4] = { -1, -1, -1, -1 };

	switch (din->vertexColor) {
	case SVC_IGNORE:
		qglUniform4fv(interactionUniformState.colorModulate, 1, zero);
		qglUniform4fv(interactionUniformState.colorAdd, 1, one);
		break;
	case SVC_MODULATE:
		qglUniform4fv(interactionUniformState.colorModulate, 1, one);
		qglUniform4fv(interactionUniformState.colorAdd, 1, zero);
		break;
	case SVC_INVERSE_MODULATE:
		qglUniform4fv(interactionUniformState.colorModulate, 1, negOne);
		qglUniform4fv(interactionUniformState.colorAdd, 1, one);
		break;
	}

	qglUniform4fv(interactionUniformState.diffuse, 1, din->diffuseColor.ToFloatPtr());
	qglUniform4fv(interactionUniformState.specular, 1, din->specularColor.ToFloatPtr());

	GL_SelectTextureNoClient(1);
	din->bumpImage->Bind();
	qglUniform1i(interactionUniformState.bumpImage, 1);

	GL_SelectTextureNoClient(2);
	din->lightFalloffImage->Bind();
	qglUniform1i(interactionUniformState.lightFalloffImage, 2);

	GL_SelectTextureNoClient(3);
	din->lightImage->Bind();
	qglUniform1i(interactionUniformState.lightImage, 3);

	GL_SelectTextureNoClient(4);
	din->diffuseImage->Bind();
	qglUniform1i(interactionUniformState.diffuseImage, 4);

	GL_SelectTextureNoClient(5);
	din->specularImage->Bind();
	qglUniform1i(interactionUniformState.specularImage, 5);

	qglUniform1i(interactionUniformState.specularTableImage, 6);

	for (int i = 0; i < 6; i++) {
		GL_SelectTextureNoClient(7 + i);
		qglEnable(GL_TEXTURE_2D);
		qglBindTexture(GL_TEXTURE_2D, shadowDepth[shadowGroupNum][i]);
		qglUniform1i(interactionUniformState.shadowMaps[i], 7 + i);
	}

	qglUniform1i(interactionUniformState.numSides, lightNumSides);

	// Draw the elements
	RB_DrawElementsWithCounters(din->surf->geo);

	// Unbind the shader program
	qglUseProgram(0);
}

void RB_EXP_CreateDrawInteractions(const drawSurf_t* surf) {
	if (!surf) {
		return;
	}

	// Perform setup here that will be constant for all interactions
	GL_State(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc);

	// texture 6 is the specular lookup table
	GL_SelectTextureNoClient(6);
	if (r_testARBProgram.GetBool()) {
		globalImages->specular2DTableImage->Bind();	// variable specularity in alpha channel
	}
	else {
		globalImages->specularTableImage->Bind();
	}

	for (; surf; surf = surf->nextOnLight) {
		// set the vertex pointers
		idDrawVert* ac = (idDrawVert*)vertexCache.Position(surf->geo->ambientCache);

		// Vertex positions
		qglEnableVertexAttribArrayARB(ATTR_POSITION);
		qglVertexAttribPointerARB(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->xyz.ToFloatPtr());

		// Texture coordinates
		qglEnableVertexAttribArrayARB(ATTR_TEXCOORD);
		qglVertexAttribPointerARB(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->st.ToFloatPtr());

		// Normals
		qglEnableVertexAttribArrayARB(ATTR_NORMAL);
		qglVertexAttribPointerARB(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->normal.ToFloatPtr());

		// Tangent 0
		qglEnableVertexAttribArrayARB(ATTR_TANGENT0);
		qglVertexAttribPointerARB(ATTR_TANGENT0, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->tangents[0].ToFloatPtr());

		// Tangent 1
		qglEnableVertexAttribArrayARB(ATTR_TANGENT1);
		qglVertexAttribPointerARB(ATTR_TANGENT1, 3, GL_FLOAT, GL_FALSE, sizeof(idDrawVert), ac->tangents[1].ToFloatPtr());

		qglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(idDrawVert), ac->color);

		// Error checking
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			printf("OpenGL error: %d\n", error);
		}

		// Execute the draw interaction
		RB_CreateSingleDrawInteractions(surf, RB_EXP_DrawInteraction);
	}

	// Cleanup
	qglDisableVertexAttribArrayARB(ATTR_POSITION);
	qglDisableVertexAttribArrayARB(ATTR_TEXCOORD);
	qglDisableVertexAttribArrayARB(ATTR_NORMAL);
	qglDisableVertexAttribArrayARB(ATTR_TANGENT0);
	qglDisableVertexAttribArrayARB(ATTR_TANGENT1);
	qglDisableVertexAttribArrayARB(ATTR_COLOR);

	// disable features
	for (int i = 0; i < 7; i++)
	{
		GL_SelectTextureNoClient(7 + i);
		qglDisable(GL_TEXTURE_2D);
	}

	GL_SelectTextureNoClient(6);
	globalImages->BindNull();

	GL_SelectTextureNoClient(5);
	globalImages->BindNull();

	GL_SelectTextureNoClient(4);
	globalImages->BindNull();

	GL_SelectTextureNoClient(3);
	globalImages->BindNull();

	GL_SelectTextureNoClient(2);
	globalImages->BindNull();

	GL_SelectTextureNoClient(1);
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture(0);
}

void RB_EXP_DrawInteractions(void) {
	viewLight_t* vLight;
	const idMaterial* lightShader;

	GL_SelectTexture(0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glStencilFunc(GL_ALWAYS, 128, 255);

	// For each light, perform adding and shadowing
	for (vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next) {
		backEnd.vLight = vLight;

		// Skip fog lights and blend lights
		if (vLight->lightShader->IsFogLight() || vLight->lightShader->IsBlendLight()) {
			continue;
		}

		// Skip lights without interactions
		if (!vLight->localInteractions && !vLight->globalInteractions && !vLight->translucentInteractions) {
			continue;
		}

		// all light side projections must currently match, so non-centered
		// and non-cubic lights must take the largest length
		viewLightAxialSize = R_EXP_CalcLightAxialSize(vLight);

		float dist = idMath::Distance(vLight->lightDef->parms.origin, backEnd.viewDef->renderView.vieworg);

		if (r_sb_maxDrawDistance.GetInteger() >= 0)
		{
			if (dist >= r_sb_maxDrawDistance.GetInteger())
			{
				continue;
			}
		}

		int	side, sideStop;

		idScreenRect currentScissor = backEnd.currentScissor;

		if (vLight->lightShader->LightCastsShadows() && !r_sb_noShadows.GetBool() && dist < r_sb_shadowDrawDistance.GetInteger())
		{
			if (vLight->lightDef->parms.pointLight) {
				if (r_sb_singleSide.GetInteger() != -1) {
					side = r_sb_singleSide.GetInteger();
					sideStop = side + 1;
					lightNumSides = 1;
				}
				else {
					side = 0;
					sideStop = 6;
					lightNumSides = 6;
				}
			}
			else {
				side = -1;
				sideStop = 0;
				lightNumSides = 1;
			}

			for (; side < sideStop; side++) {
				// render a shadow buffer
				if (side == -1)
				{
					RB_RenderShadowBuffer(vLight, 0);
				}
				else
				{
					RB_RenderShadowBuffer(vLight, side);
				}
			}
		}
		else
		{
			lightNumSides = 0;
		}

		// set the window clipping
		qglViewport(tr.viewportOffset[0] + backEnd.viewDef->viewport.x1,
			tr.viewportOffset[1] + backEnd.viewDef->viewport.y1,
			backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
			backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1);

		// the scissor may be smaller than the viewport for subviews
		qglScissor(tr.viewportOffset[0] + backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
			tr.viewportOffset[1] + backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
			backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
			backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1);
		backEnd.currentScissor = backEnd.viewDef->scissor;

		lightShader = vLight->lightShader;

		// Draw interactions with shadows		
		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
		RB_EXP_CreateDrawInteractions(vLight->localInteractions);
		RB_EXP_CreateDrawInteractions(vLight->globalInteractions);

		// Draw translucent interactions
		if (!r_skipTranslucent.GetBool()) {
			backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
			RB_EXP_CreateDrawInteractions(vLight->translucentInteractions);
			backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
		}

		shadowGroupNum = (shadowGroupNum + 1) % NUM_SHADOW_GROUPS;

		qglUseProgram(0);
	}

	GL_SelectTexture(0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void RB_EXP_BindFog() {
	GLuint program = fogProgram->GetProgram();
	qglUseProgram(program);

	idVec4 fogColor(backEnd.viewDef->atmosphere.fogColor.x, backEnd.viewDef->atmosphere.fogColor.y, backEnd.viewDef->atmosphere.fogColor.z, 1.0);
	qglUniform4fv(qglGetUniformLocation(program, "fogColor"), 1, fogColor.ToFloatPtr());
	qglUniform1f(qglGetUniformLocation(program, "fogDensity"), backEnd.viewDef->atmosphere.fogDensity);
	qglUniform1f(qglGetUniformLocation(program, "fogStart"), backEnd.viewDef->atmosphere.fogStart);
	qglUniform1f(qglGetUniformLocation(program, "fogEnd"), backEnd.viewDef->atmosphere.fogEnd);
	qglUniform1f(qglGetUniformLocation(program, "near"), backEnd.viewDef->projectionMatrix[14] / (backEnd.viewDef->projectionMatrix[10] - 1.0f));

	// Pass the inverse projection matrix to the shader
	// Compute the inverse view-projection matrix
	idMat4 viewMatrix(backEnd.viewDef->worldSpace.modelViewMatrix);
	idMat4 projectionMatrix(backEnd.viewDef->projectionMatrix);
	idMat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
	idMat4 inverseViewProjectionMatrix = viewProjectionMatrix.Inverse();
	qglUniformMatrix4fv(qglGetUniformLocation(fogProgram->GetProgram(), "invProjectionMatrix"), 1, GL_FALSE, inverseViewProjectionMatrix.ToFloatPtr());

	GL_SelectTextureNoClient(1);
	globalImages->currentRenderImage->Bind();
	qglUniform1i(qglGetUniformLocation(program, "colorTexture"), 1);

	GL_SelectTextureNoClient(0);
	globalImages->currentDepthRenderImage->Bind();
	qglUniform1i(qglGetUniformLocation(program, "depthTexture"), 0);
}

void RB_Exp_RenderPostProcess(void) {
	if (r_skipPostProcess.GetBool()) {
		return;
	}

	GL_CheckErrors();
	if (!backEnd.currentRenderCopied)
	{
		globalImages->currentRenderImage->CopyFramebuffer(backEnd.viewDef->viewport.x1,
			backEnd.viewDef->viewport.y1, backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1,
			backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1, true);

		globalImages->currentDepthRenderImage->CopyDepthbuffer(backEnd.viewDef->viewport.x1,
			backEnd.viewDef->viewport.y1, backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1,
			backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1);

		GL_SelectTextureNoClient(0);
		globalImages->BindNull();
	}
	backEnd.currentRenderCopied = true;

	// set the window clipping
	qglViewport(tr.viewportOffset[0] + backEnd.viewDef->viewport.x1,
		tr.viewportOffset[1] + backEnd.viewDef->viewport.y1,
		backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
		backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1);

	// the scissor may be smaller than the viewport for subviews
	qglScissor(tr.viewportOffset[0] + backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
		tr.viewportOffset[1] + backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
		backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
		backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1);

	float projectionMatrix[16] = {};
	float modelViewMatrix[16] = {};

	projectionMatrix[0] = 2.0f / SCREEN_WIDTH;
	projectionMatrix[5] = -2.0f / SCREEN_HEIGHT;
	projectionMatrix[10] = -2.0f / 1.0f;
	projectionMatrix[12] = -1.0f;
	projectionMatrix[13] = 1.0f;
	projectionMatrix[14] = -1.0f;
	projectionMatrix[15] = 1.0f;

	modelViewMatrix[0] = 1.0f;
	modelViewMatrix[5] = 1.0f;
	modelViewMatrix[10] = 1.0f;
	modelViewMatrix[15] = 1.0f;

	// Set up matrices for 2D rendering
	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadMatrixf(&projectionMatrix[0]);

	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	qglLoadMatrixf(&modelViewMatrix[0]);

	// Render linear fog
	RB_EXP_BindFog();

	// Draw a full-screen quad
	qglDepthMask(GL_FALSE);
	qglBegin(GL_QUADS);
	qglTexCoord2f(0, 0); qglVertex2f(0, 0);
	qglTexCoord2f(1, 0); qglVertex2f(SCREEN_WIDTH, 0);
	qglTexCoord2f(1, 1); qglVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
	qglTexCoord2f(0, 1); qglVertex2f(0, SCREEN_HEIGHT);
	qglEnd();
	qglDepthMask(GL_TRUE);
	qglUseProgram(0);

	// Reset texture state
	GL_SelectTextureNoClient(1);
	globalImages->BindNull();

	GL_SelectTextureNoClient(0);
	globalImages->BindNull();

	// Restore matrices
	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();

	GL_CheckErrors();
}