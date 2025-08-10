#include "models.h"

/*
Reference tutorial:
https://sibras.github.io/OpenGL4-Tutorials/docs/Tutorials/05-Tutorial5/
*/
/* WIP - bone animation...
std::vector<VAO> load_model(const char * objFile) {

    std::vector<VAO> all_vaos;

    const aiScene * p_Scene = aiImportFile( objFile,
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_ImproveCacheLocality |
        aiProcess_SortByPType);
 
    if (!p_Scene) {
        std::cout << "Error loading file " << aiGetErrorString() << std::endl;
        return all_vaos;
    }

    for (unsigned i = 0; i < p_Scene->mNumMeshes; i++) {

        const aiMesh * p_AIMesh = p_Scene->mMeshes[i];
            
        // Create the new mesh data
        const unsigned modelSizeVertices = p_AIMesh->mNumVertices * 8 * sizeof(GLfloat);
        const unsigned modelSizeIndices  = p_AIMesh->mNumFaces * 3 * sizeof(GLuint);

        GLfloat * vbo_head = (GLfloat *)malloc(modelSizeVertices);
        GLuint * ebo_head = (GLuint *)malloc(modelSizeIndices);
        
        // retain head pointer, by using alternative for assignment...
        GLfloat * vbo_ptr = vbo_head;
        GLuint * ebo_ptr = ebo_head;

        for (unsigned j = 0; j < p_AIMesh->mNumVertices; j++) {
            *vbo_ptr++ = (GLfloat) p_AIMesh->mVertices[j].x;
            *vbo_ptr++ = (GLfloat) p_AIMesh->mVertices[j].y;
            *vbo_ptr++ = (GLfloat) p_AIMesh->mVertices[j].z;
            *vbo_ptr++ = (GLfloat) p_AIMesh->mNormals[j].x;
            *vbo_ptr++ = (GLfloat) p_AIMesh->mNormals[j].y;
            *vbo_ptr++ = (GLfloat) p_AIMesh->mNormals[j].z;
            *vbo_ptr++ = (GLfloat) p_AIMesh->mTextureCoords[0][j].x;
            *vbo_ptr++ = (GLfloat) p_AIMesh->mTextureCoords[0][j].y;
        }
        for (unsigned j = 0; j < p_AIMesh->mNumFaces; j++) {
            *ebo_ptr++ = (GLuint) p_AIMesh->mFaces[j].mIndices[0];
            *ebo_ptr++ = (GLuint) p_AIMesh->mFaces[j].mIndices[1];
            *ebo_ptr++ = (GLuint) p_AIMesh->mFaces[j].mIndices[2];
        }
        
        VAO vao((GLfloat*) vbo_head, (GLsizeiptr) modelSizeVertices,
                (GLuint*)  ebo_head, (GLsizeiptr) modelSizeIndices);
        
        all_vaos.push_back(vao);
        vao.LinkAttrib(vao.vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
        vao.LinkAttrib(vao.vbo, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        vao.LinkAttrib(vao.vbo, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        free(vbo_head);
        free(ebo_head);
    }
    return all_vaos;
}


   // Retrieve bone transformations
    std::vector<aiMatrix4x4> boneTransforms;
    if (p_Scene->mRootNode) {
        aiMatrix4x4 rootTransform = p_Scene->mRootNode->mTransformation;
        boneTransforms.push_back(rootTransform);
        
        // Traverse the node hierarchy to get bone transformations
        std::function<void(aiNode *, const aiMatrix4x4 &)> traverseNodes = [&](aiNode * node, const aiMatrix4x4 & parentTransform) {
            aiMatrix4x4 nodeTransform = node->mTransformation;
            aiMatrix4x4 globalTransform = parentTransform * nodeTransform;
            
            for (unsigned i = 0; i < node->mNumMeshes; i++) {
                unsigned meshIndex = node->mMeshes[i];
                const aiMesh * mesh = p_Scene->mMeshes[meshIndex];
                
                for (unsigned j = 0; j < mesh->mNumBones; j++) {
                    aiBone * bone = mesh->mBones[j];
                    std::string boneName(bone->mName.data);
                    
                    // Store the bone transformation matrix
                    aiMatrix4x4 boneTransform = globalTransform * bone->mOffsetMatrix;
                    boneTransforms.push_back(boneTransform);
                }
            }
            
            for (unsigned i = 0; i < node->mNumChildren; i++) {
                traverseNodes(node->mChildren[i], globalTransform);
            }
        };
        
        traverseNodes(p_Scene->mRootNode, rootTransform);
    }
    
    // Retrieve animation data (if available)
    if (p_Scene->mNumAnimations > 0) {
        aiAnimation * animation = p_Scene->mAnimations[0];
        
        // Process animation channels and keyframes
        for (unsigned i = 0; i < animation->mNumChannels; i++) {
            aiNodeAnim * channel = animation->mChannels[i];
            std::string channelName(channel->mNodeName.data);
            
            // Process position, rotation, and scaling keyframes
            // Store the keyframe data as needed
        }
    }*/