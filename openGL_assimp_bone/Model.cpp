#include "Model.h"
#include <glm/gtc/type_ptr.hpp> // value_ptr()

Model::Model(char* path)
{
    AssimpLoader* assimpLoader = new AssimpLoader(&modelData, path);
}
void Model::Draw(ShaderProgram* shader)
{
    for (unsigned int i = 0; i < modelData.meshes.size(); i++) {
        modelData.meshes[i].Draw(shader);
    }
}
glm::mat4 Model::CalcBoneTransform(aiAnimation* anim) {
    glm::mat4 matrix;
    
    //matrix = glm::translate(glm::mat4(1.0f), glm::vec3(rand() % 100, 0.0f, 0.0f));
    matrix = glm::rotate(glm::mat4(1.0f), glm::radians(rand() % 100 * 0.5f), glm::vec3(1.0f, 0, 0));
    return matrix;
}

int Model::BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
{
    glm::mat4 Identity = glm::mat4(1.0f);
    //initialization
    //if (modelData.scene->mNumAnimations == 0) {
        Transforms.resize(modelData.m_NumBones);
        for (unsigned int i = 0; i < modelData.m_NumBones; ++i) {
            Transforms[i] = glm::mat4(1.0f);
        }
        return 0;
    //}
    unsigned int numPosKeys = modelData.scene->mAnimations[0]->mChannels[0]->mNumPositionKeys;

    float TicksPerSecond = modelData.scene->mAnimations[0]->mTicksPerSecond != 0 ?
        modelData.scene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float duration = modelData.scene->mAnimations[0]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime;
    float AnimationTime = fmod(TimeInTicks, duration);

    ReadNodeHeirarchy(modelData.scene, AnimationTime, modelData.scene->mRootNode, Identity, glm::vec3(0.0f, 0.0f, 0.0f));

    //	debugSkeletonPose(skeleton_pose);

    Transforms.resize(modelData.m_NumBones);

    for (unsigned int i = 0; i < modelData.m_NumBones; ++i) {
        Transforms[i] = glm::mat4(1.0f);
        Transforms[i] = modelData.m_BoneInfo[i].FinalTransformation;
    }
    return 0;
}
void Model::ReadNodeHeirarchy(const aiScene* scene, float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform, glm::vec3 startpos)
{
    std::string NodeName(pNode->mName.data);
    const aiAnimation* pAnimation = scene->mAnimations[0];

    glm::mat4 NodeTransformation = glm::mat4(1.0f);


    aiMatrix4x4 tp1 = pNode->mTransformation;
    NodeTransformation = glm::transpose(glm::make_mat4(&tp1.a1));


    const aiNodeAnim* pNodeAnim = nullptr;
    pNodeAnim = modelData.Animations[pAnimation->mName.data][NodeName];


    if (pNodeAnim) {

        //Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        CalcInterpolatedRotaion(RotationQ, AnimationTime, pNodeAnim);

        aiMatrix3x3 tp = RotationQ.GetMatrix();
        glm::mat4 RotationM = glm::transpose(glm::make_mat3(&tp.a1));

        //Interpolate translation and generate translation transformation matrix
        aiVector3D Translation;
        CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);

        glm::mat4 TranslationM = glm::mat4(1.0f);
        TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));

        NodeTransformation = TranslationM * RotationM;
    }
    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (modelData.Bone_Mapping.find(NodeName) != modelData.Bone_Mapping.end()) {
        unsigned int NodeIndex = modelData.Bone_Mapping[NodeName];
        modelData.m_BoneInfo[NodeIndex].FinalTransformation = GlobalTransformation * modelData.m_BoneInfo[NodeIndex].offset;
        glm::fdualquat offsetDQ = glm::normalize(glm::fdualquat(glm::normalize(glm::quat_cast(modelData.m_BoneInfo[NodeIndex].offset)), glm::vec3(modelData.m_BoneInfo[NodeIndex].offset[3][0], modelData.m_BoneInfo[NodeIndex].offset[3][1], modelData.m_BoneInfo[NodeIndex].offset[3][2])));
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHeirarchy(scene, AnimationTime, pNode->mChildren[i], GlobalTransformation, startpos);
    }
}

void Model::CalcInterpolatedRotaion(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
    unsigned int NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();//normalized
}

void Model::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
    unsigned int NextPositionIndex = (PositionIndex + 1);
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

unsigned int Model::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);
    for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }
    assert(0);
    return 0;
}

unsigned int Model::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);
    for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);
    return 0;
}

unsigned int Model::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }
    assert(0);
    return 0;
}
