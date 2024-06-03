// dnAnim_vertex.h
//

#pragma once

//
// dnDeclVertexAnim
//
class dnDeclVertexAnim : public idDecl {
public:
	idDict animData; // Store animation data here

	virtual const char* DefaultDefinition() const override;
	virtual bool Parse(const char* text, const int textLength) override;
	virtual void FreeData() override;

	const idDict* FindAnim(const char* name) const;

	// Add methods to access and manipulate the animation data
	const idStr& GetModel() const;
	const idList<idDict>& GetAnims() const;

private:
	idStr model; // MD3 model path
	idList<idDict> anims; // List of animations
};

//
// dnVertexAnimator
//
class dnVertexAnimator {
public:
	idRenderModel*					SetModel(const char* modelDecl);
	void							PlayAnim(const char* animName, bool loop);
	int								GetCurrentFrame() const { return currentFrame; }
	int								GetLastFrame() const { return lastFrame; }
	float							GetBacklerp() const { return backlerp; }
	void							Update(void);
	bool							IsAnimDone(void);
	void							ClearAllAnims(void);
	jointHandle_t					GetJointHandle(const char* name);
	bool							GetJointTransform(jointHandle_t jointHandle, idVec3& offset, idMat3& axis);
private:
	const idDict*					currentAnim = nullptr;
	bool							isLooping = false;

	int								lastTime = 0;
	int								start_frame = 0;
	int								end_frame = 0;

	float							currentFrame= 0;
	int								lastFrame = 0;
	float							backlerp = 0;

	idStr							currentAnimName;

	const dnDeclVertexAnim*			vertexAnim;
	idRenderModel*					renderModel = nullptr;
};