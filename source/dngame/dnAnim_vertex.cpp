// dnAnim_vertex.cpp
//

#include "precompiled.h"
#include "dnGame_local.h"

/*
========================================

dnDeclVertexAnim

========================================
*/

/*
=====================
dnDeclVertexAnim::DefaultDefinition
=====================
*/
const char* dnDeclVertexAnim::DefaultDefinition() const {
	return "{\n\t\"model\" : \"path/to/model.md3\"\n\t\"anim <name> <start_frame> <end_frame>\"\n}\n";
}

/*
=====================
dnDeclVertexAnim::Parse
=====================
*/
bool dnDeclVertexAnim::Parse(const char* text, const int textLength) {
	idLexer src;
	idToken token;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	while (src.ReadToken(&token)) {
		if (token == "model") {
			src.ReadToken(&token);
			model = token;
		}
		else if (token == "anim") {
			idDict anim;
			src.ReadToken(&token); // Read anim name
			anim.Set("name", token);

			src.ReadToken(&token); // Read start frame
			anim.Set("start_frame", token);

			src.ReadToken(&token); // Read end frame
			anim.Set("end_frame", token);

			anims.Append(anim);
		}
		else if (token == "}") {
			break;
		}
		else {
			src.Warning("Unknown token: %s", token.c_str());
			return false;
		}
	}
	return true;
}

/*
=====================
dnDeclVertexAnim::FindAnim
=====================
*/
const idDict* dnDeclVertexAnim::FindAnim(const char* name) const {
	for (int i = 0; i < anims.Num(); i++) {
		idStr animName = anims[i].GetString("name");
		if (animName == name) {
			return &anims[i];
		}
	}

	return nullptr;
}

/*
=====================
dnDeclVertexAnim::FreeData
=====================
*/
void dnDeclVertexAnim::FreeData() {
	model.Clear();
	anims.Clear();
}

/*
=====================
dnDeclVertexAnim::GetModel
=====================
*/
const idStr& dnDeclVertexAnim::GetModel() const {
	return model;
}

/*
=====================
dnDeclVertexAnim::GetAnims
=====================
*/
const idList<idDict>& dnDeclVertexAnim::GetAnims() const {
	return anims;
}

/*
========================================

dnDeclVertexAnim

========================================
*/

/*
=====================
dnVertexAnimator::SetModel
=====================
*/
idRenderModel* dnVertexAnimator::SetModel(const char* modelDecl) {
	vertexAnim = (const dnDeclVertexAnim *)declManager->FindType(DECL_VERTEXMODELDEF, modelDecl, false);
	if (!vertexAnim) {
		gameLocal.Warning("dnVertexAnimator::SetModel: Failed to load vertex model decl %s\n", modelDecl);
		return nullptr;
	}

	start_frame = 0;
	end_frame = 0;
	currentFrame = 0;
	isLooping = false;

	int currentTime = sys->GetMilliseconds();
	lastTime = currentTime;

	renderModel = renderModelManager->FindModel(vertexAnim->GetModel());
	return renderModel;
}

/*
=====================
dnVertexAnimator::PlayAnim
=====================
*/
void dnVertexAnimator::PlayAnim(const char* animName, bool loop) {
	currentAnim = vertexAnim->FindAnim(animName);
	if (currentAnim == nullptr) {
		common->Warning("Failed to find vertex animation %s\n", animName);
		return;
	}

	if (isLooping && loop && currentAnimName == animName)
		return;

	currentAnimName = animName;

	int currentTime = sys->GetMilliseconds();
	lastTime = currentTime;

	isLooping = loop;
	start_frame = currentFrame = currentAnim->GetInt("start_frame");
	end_frame = currentAnim->GetInt("end_frame");
}

/*
=====================
dnVertexAnimator::Update
=====================
*/
#include <cmath> // Include for fmod

void dnVertexAnimator::Update(void) {
	if (vertexAnim == nullptr || currentAnim == nullptr) {
		return;
	}

	// Get the current time and calculate the delta time in seconds
	int currentTime = sys->GetMilliseconds();
	int deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	float deltaTimeSeconds = deltaTime / 1000.0f;

	// Calculate the total animation duration in seconds
	float animSpeed = 24.0f;
	int frameCount = end_frame - start_frame;
	float animDuration = frameCount / animSpeed;

	// Update the current frame based on the elapsed time
	float framesAdvanced = deltaTimeSeconds * animSpeed;
	lastFrame = currentFrame;
	currentFrame += deltaTimeSeconds * animSpeed;

	// Handle looping or clamping to end_frame
	if (currentFrame >= end_frame) {
		if (isLooping) {
			currentFrame = start_frame;			
		}
		else {
			currentFrame = end_frame;
		}		
	}

	// Calculate the backlerp value
	float frameFraction = fmod(framesAdvanced, 1.0f);
	backlerp = 1.0f - frameFraction;
}

/*
=====================
dnVertexAnimator::Update
=====================
*/
bool dnVertexAnimator::IsAnimDone(void) {
	if (isLooping) {
		return false;
	}

	return currentFrame >= end_frame;
}

/*
=====================
dnVertexAnimator::ClearAllAnims
=====================
*/
void dnVertexAnimator::ClearAllAnims(void) {
	currentAnim = nullptr;
}

/*
=====================
dnVertexAnimator::GetJointHandle
=====================
*/
jointHandle_t dnVertexAnimator::GetJointHandle(const char* name) {
	if (renderModel == nullptr)
		return INVALID_JOINT;

	return renderModel->FindTag(name);
}

/*
=====================
dnVertexAnimator::GetJointTransform
=====================
*/
bool dnVertexAnimator::GetJointTransform(jointHandle_t jointHandle, idVec3& offset, idMat3& axis) {
	offset.Zero();
	axis.Identity();

	if (renderModel == nullptr)
		return false;

	if (jointHandle == INVALID_JOINT)
		return false;

	md3Tag_t* tag = renderModel->GetTag(jointHandle, currentFrame);
	if (tag == nullptr)
		return false;

	offset = tag->origin;
	axis = idMat3(tag->axis[0], tag->axis[1], tag->axis[2]);

	return true;
}