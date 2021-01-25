#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices/*, std::vector<Texture> textures*/)
{
    this->vertices = vertices;
    this->indices = indices;
    //this->textures = textures;

    setupMesh();
}
void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //glGenBuffers(1, &BONE_VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    //glBindBuffer(GL_ARRAY_BUFFER, BONE_VBO);
    //glBufferData(GL_ARRAY_BUFFER, bones.size() * sizeof(Bone), &bones[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    /*
    // BoneIDs;
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    // Weights;
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)offsetof(VertexBoneData, Weights));
    */
    // vertex texture coords
    //glEnableVertexAttribArray(2);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // bone
    //glEnableVertexAttribArray(3);
    //glVertexAttribIPointer(3, 4, GL_INT, sizeof(Bone), (void*)0);

    glBindVertexArray(0);
}
void Mesh::Draw(ShaderProgram* shader)
{
    //unsigned int diffuseNr = 1;
    //unsigned int specularNr = 1;
    /*for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // 바인딩하기 전에 적절한 텍스처 유닛 활성화
        // 텍스처 넘버(diffuse_textureN 에서 N) 구하기
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        shader->setFloat(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);
    */


// mesh 그리기
    glBindVertexArray(VAO);
    
    // 보간, 계산
    // bone transform matrix 유니폼

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


BoneInfo::BoneInfo()
{
    offset = glm::mat4(1.0f);
    FinalTransformation = glm::mat4(1.0f);
    FinalTransDQ.real = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    FinalTransDQ.dual = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
}
VertexBoneData::VertexBoneData()
{
    Reset();
};
void VertexBoneData::Reset()
{
    for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
    {
        BoneIDs[i] = 0;
        Weights[i] = 0;
    }
}
void VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
{
    for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
        if (Weights[i] == 0.0) {
            BoneIDs[i] = BoneID;
            Weights[i] = Weight;
            return;
        }
    }
}