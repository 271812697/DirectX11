//#include "../pch.h"
#include "../core/log.h"
#include "model.h"
#include "mesh.h"
#include "material.h"
#include "animator.h"


namespace component {

    constexpr unsigned int max_vtx_bones = 4;
   
    
    static inline DirectX::XMMATRIX AssimpMat2(const aiMatrix4x4& m) {
       return  DirectX::XMMATRIX(&m.a1);
    }

   

    Node::Node(int nid, int pid, const std::string& name) : nid(nid), pid(pid), bid(-1), name(name), alive(0) {
        CORE_ASERT(nid >= 0 && pid < nid, "Parent node is not processed before its children!");
    }

    bool Node::IsBone() const {
        return bid >= 0;
    }

    bool Node::Animated() const {
        return (bid >= 0) && alive;
    }

    Model::Model(const std::string& filepath, Quality quality, bool animate) :  animated(animate) {
        this->vtx_format.reset();
        this->meshes.clear();
        this->materials.clear();

        unsigned int import_options = static_cast<unsigned int>(quality)
            | aiProcess_FlipUVs
            | aiProcess_Triangulate
            | aiProcess_GenSmoothNormals
            | aiProcess_FindInvalidData
            | aiProcess_ValidateDataStructure
            | aiProcess_CalcTangentSpace
            | aiProcess_LimitBoneWeights
            ;

        // for static models, let Assimp pre-transform all vertices for us (will lose the hierarchy)
        if (!animated) {
            import_options |= aiProcess_PreTransformVertices;
        }

        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);  // stick to "4 bones per vertex" rule
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, false);

        CORE_TRACE("Start loading model: {0}...", filepath);
        auto start_time = std::chrono::high_resolution_clock::now();

        this->ai_root = importer.ReadFile(filepath, import_options);

        if (!ai_root || ai_root->mRootNode == nullptr || ai_root->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            CORE_ERROR("Failed to import model: {0}", filepath);
            CORE_ERROR("Assimp error: {0}", importer.GetErrorString());
            __debugbreak();
            return;
        }
        directory = filepath.substr(0, filepath.find_last_of('/')) + "/";
        ProcessTree(ai_root->mRootNode, -1);  // recursively process and store the hierarchy info
        ProcessNode(ai_root->mRootNode);      // recursively process every node before return

        if (animated) {
            unsigned int cnt = std::count_if(nodes.begin(), nodes.end(), [](const Node& node) { return node.bid >= 0; });
            CORE_ASERT(n_bones == cnt, "Corrupted data: duplicate or missing bones!");
            CORE_ASERT(n_bones <= 150, "Animation can only support up to 150 bones!");
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> loading_time = end_time - start_time;
        CORE_TRACE("Model import complete! Total loading time: {0:.2f} ms", loading_time.count());


        CORE_TRACE("Generating model loading report...... (for reference)");
        CORE_TRACE("-----------------------------------------------------");

        CORE_DEBUG("total # of meshes:     {0}", n_meshes);
        CORE_DEBUG("total # of vertices:   {0:.2f}k ({1})", n_verts * 0.001f, n_verts);
        CORE_DEBUG("total # of triangles:  {0:.2f}k ({1})", n_tris * 0.001f, n_tris);
        CORE_TRACE("-----------------------------------------------------");

        CORE_DEBUG("vertex has position ? [{0}]", vtx_format.test(0) ? "Y" : "N");
        CORE_DEBUG("vertex has normal   ? [{0}]", vtx_format.test(1) ? "Y" : "N");
        CORE_DEBUG("vertex has uv set 1 ? [{0}]", vtx_format.test(2) ? "Y" : "N");
        CORE_DEBUG("vertex has uv set 2 ? [{0}]", vtx_format.test(3) ? "Y" : "N");
        CORE_DEBUG("vertex has tan/btan ? [{0}]", vtx_format.test(4) ? "Y" : "N");
        CORE_TRACE("-----------------------------------------------------");

        std::string all_mtls = "not available";

        if (!materials_cache.empty()) {
            auto it = materials_cache.begin();
            all_mtls = it->first + " (id = " + std::to_string(it->second) + ")";
            std::advance(it, 1);
            while (it != materials_cache.end()) {
                all_mtls += (", " + it->first + " (id = " + std::to_string(it->second) + ")");
                std::advance(it, 1);
            }
        }

        CORE_DEBUG("internal materials: {0}", all_mtls);
        CORE_TRACE("-----------------------------------------------------");
        if (ai_root->mNumAnimations > 0) {
            /*
            CORE_TRACE("Start loading animaitions of model: {0}...", filepath);
            for (size_t i = 0; i < ai_root->mNumAnimations; i++) {
                aiAnimation* ai_animation = ai_root->mAnimations[i];

                auto ani = std::make_shared<Animation>(ai_animation, this);
                animations.push_back(ani);
            }
            animation = animations[0];*/
        }


        if constexpr (false) {
            delete ai_root;
            importer.FreeScene();
        }
    }

    void Model::ProcessTree(aiNode* ai_node, int parent) {
        aiString& ai_name = ai_node->mName;
        auto& node = nodes.emplace_back(n_nodes++, parent, ai_name.length == 0 ? "unnamed" : ai_name.C_Str());
        node.n2p = AssimpMat2(ai_node->mTransformation);
        
        int next_parent = n_nodes - 1;  // each recursive call has a separate local copy

        for (unsigned int i = 0; i < ai_node->mNumChildren; i++) {
            aiNode* child_node = ai_node->mChildren[i];
            ProcessTree(child_node, next_parent);
        }
    }

    void Model::ProcessNode(aiNode* ai_node) {
        // allocate storage for meshes upfront in every recursion
        meshes.reserve(meshes.size() + ai_node->mNumMeshes);

        // iteratively process every mesh in the current node
        for (unsigned int i = 0; i < ai_node->mNumMeshes; i++) {
            unsigned int mesh_id = ai_node->mMeshes[i];
            aiMesh* ai_mesh = ai_root->mMeshes[mesh_id];
            ProcessMesh(ai_mesh);
        }

        // recursively process all children of the current node
        for (unsigned int i = 0; i < ai_node->mNumChildren; i++) {
            aiNode* child_node = ai_node->mChildren[i];
            ProcessNode(child_node);
        }
    }

    void Model::ProcessMesh(aiMesh* ai_mesh) {
        std::vector<Mesh::Vertex> vertices;
        std::vector<UINT> indices;
        std::bitset<6> local_format;

        vertices.reserve(ai_mesh->mNumVertices);  // reserve storage upfront
        indices.reserve(ai_mesh->mNumFaces * 3);  // our polygons are always triangles

        // determine local vertex format for this mesh
        local_format.set(0, ai_mesh->HasPositions());
        local_format.set(1, ai_mesh->HasNormals());
        local_format.set(2, ai_mesh->HasTextureCoords(0));
        local_format.set(3, ai_mesh->HasTextureCoords(1) && ai_mesh->GetNumUVChannels() > 1);
        local_format.set(4, ai_mesh->HasTangentsAndBitangents());
        local_format.set(5, ai_mesh->HasTangentsAndBitangents());

        if (n_verts == 0) {
            vtx_format = local_format;
        }

        static bool warned = false;
        if (vtx_format != local_format && !warned) {
            CORE_WARN("Inconsistent vertex format! Some meshes have attributes missing...");
            warned = true;
        }

        vtx_format |= local_format;  // bitwise or on every pair of bits

        // construct mesh vertices, w/o bones data
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {
            Mesh::Vertex vertex{};
            vertex.bone_id = {-1,-1,-1,-1};  // initialize bone id to -1 instead of 0

            if (local_format.test(0)) {
                aiVector3D& ai_position = ai_mesh->mVertices[i];
                vertex.position = { ai_position.x, ai_position.y, ai_position.z };
            }

            if (local_format.test(1)) {
                aiVector3D& ai_normal = ai_mesh->mNormals[i];
                vertex.normal = { ai_normal.x, ai_normal.y, ai_normal.z };
            }

            if (local_format.test(2)) {
                aiVector3D& ai_uv = ai_mesh->mTextureCoords[0][i];  // 1st UV set
                vertex.uv = { ai_uv.x, ai_uv.y };
            }

            if (local_format.test(3)) {
                aiVector3D& ai_uv2 = ai_mesh->mTextureCoords[1][i];  // 2nd UV set
                vertex.uv2 = { ai_uv2.x, ai_uv2.y };
            }

            // tangents and bitangents always come in pairs, if one exists, so does the other
            if (local_format.test(4) && local_format.test(5)) {
                aiVector3D& ai_tangent = ai_mesh->mTangents[i];
                aiVector3D& ai_binormal = ai_mesh->mBitangents[i];
                vertex.tangent = { ai_tangent.x, ai_tangent.y, ai_tangent.z };
                vertex.binormal = { ai_binormal.x, ai_binormal.y, ai_binormal.z };
            }

            vertices.push_back(vertex);
            n_verts++;
        }

        // construct mesh indices
        for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) {
            aiFace& triangle = ai_mesh->mFaces[i];

            for (unsigned int j = 0; j < triangle.mNumIndices; j++) {
                indices.push_back(triangle.mIndices[j]);
            }

            n_tris++;
        }

        // fill out the missing bones data in vertices
        if (animated) {
            for (unsigned int i = 0; i < ai_mesh->mNumBones; i++) {
                aiBone* ai_bone = ai_mesh->mBones[i];
                std::string name = ai_bone->mName.C_Str();              
                auto it = std::find_if(nodes.begin(),nodes.end(), [&name](const Node& node) { return name == node.name; });
                CORE_ASERT(it != nodes.end(), "Invalid bone, cannot find a match in the nodes hierarchy!");
                Node& node = nodes[it->nid];  // vector `nodes` is indexed by node id
                if (node.bid < 0) {  // new bone
                    node.m2n = AssimpMat2(ai_bone->mOffsetMatrix);
                    node.bid = n_bones++;
                }

                for (int j = 0; j < ai_bone->mNumWeights; j++) {
                    unsigned int vtx_id = ai_bone->mWeights[j].mVertexId;
                    const float  weight = ai_bone->mWeights[j].mWeight;
                    CORE_ASERT(vtx_id < vertices.size(), "Vertex id out of bound!");

                    auto& vertex = vertices[vtx_id];
                    //bool full = glm::all(glm::greaterThanEqual(vertex.bone_id, ivec4(0)));
                    //CORE_ASERT(!full, "Found more than 4 bones per vertex, check the import settings!");
                    auto* b = &vertex.bone_id.x;
                    auto* w= &vertex.bone_wt.x;
                    for (int k = 0; k < max_vtx_bones; k++) {
                        if (b[k] < 0) {
                            b[k] = node.bid;
                            w[k] = weight;
                            break;  // one bone only sets one slot
                        }
                    }
                }
            }
        }

        auto& mesh = meshes.emplace_back(vertices, indices);  // move construct mesh in-place
        n_meshes++;

        // establish the association between mesh and material
        aiMaterial* ai_material = ai_root->mMaterials[ai_mesh->mMaterialIndex];
        ProcessMaterial(ai_material, mesh);
    }

    void Model::ProcessMaterial(aiMaterial* ai_material, const Mesh& mesh) {
        CORE_ASERT(ai_material != nullptr, "Corrupted assimp data: material is nullptr!");
        aiString name;
        if (ai_material->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
            CORE_ERROR("Unable to load mesh's material ...");
            return;
        }

        std::string matkey{ name.C_Str() };

        // check if the matkey already exists in local cache
        if (materials_cache.find(matkey) != materials_cache.end()) {
            UINT matid = materials_cache[matkey];
            mesh.SetMaterialID(matid);  // reuse the previous matid since the material is shared
            return;
        }

        // new material, store the matkey in local cache
        UINT matid = mat_count++;
        materials_cache[matkey] = matid;
        mesh.SetMaterialID(matid);  
        /*
        
        for (unsigned int i = 0; i < ai_material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
            aiString str;
            ai_material->GetTexture(aiTextureType_DIFFUSE, i, &str);
            std::string realpath = directory + str.C_Str();
           // Texture_Diffuse[matid] = MakeAsset<Texture2D>(realpath.c_str());
        }
        for (unsigned int i = 0; i < ai_material->GetTextureCount(aiTextureType_NORMALS); i++) {
            aiString str;
            ai_material->GetTexture(aiTextureType_NORMALS, i, &str);
            std::string realpath = directory + str.C_Str();
            //Texture_Diffuse[matid] = MakeAsset<Texture>(realpath.c_str());
        }
        
        
        */

    }

    Material& Model::SetMaterial(const std::string& matkey, asset_ref<Material>&& material) {
        CORE_ASERT(materials_cache.count(matkey) > 0, "Invalid material key: {0}", matkey);

        // notice that we expect the material param to be a temporary rvalue that is a copy of
        // the original asset_ref in the asset manager, hence we can directly move it in place

        UINT matid = materials_cache[matkey];
        materials.insert_or_assign(matid, std::move(material));
        auto& mat = materials.at(matid);
        //mat.SetTexture(pbr_t::albedo, Texture_Diffuse[matid]);
        return mat;
    }

    void Model::AttachMotion(const std::string& filepath) {
        /*
        if (!animated) {
            CORE_ERROR("Cannot attach animation to the model, model must be animated...");
        }

        const unsigned int import_options = 0
            | aiProcess_FlipUVs
            | aiProcess_Triangulate
            | aiProcess_GenSmoothNormals
            | aiProcess_FindInvalidData
            | aiProcess_ValidateDataStructure
            | aiProcess_CalcTangentSpace
            | aiProcess_LimitBoneWeights
            // | aiProcess_PreTransformVertices  // this flag must be disabled to load animation
            ;

        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
        importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS, true);

        CORE_TRACE("Start loading animation from: {0}...", filepath);
        const aiScene* scene = importer.ReadFile(filepath, import_options);

        // note that we don't need to check `scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE`
        // here because the file can contain animations ONLY (without vertices, meshes)
        // in which case the scene will be incomplete but the animation is still valid

        if (!scene || scene->mRootNode == nullptr) {
            CORE_ERROR("Failed to import animation: {0}", filepath);
            CORE_ERROR("Assimp error: {0}", importer.GetErrorString());
            __debugbreak();
            return;
        }
        auto ai_scene = scene;
        CORE_ASERT(ai_scene->mNumAnimations > 0, "The input file does not contain animations!");
        for (size_t i = 0; i < ai_scene->mNumAnimations; i++) {
            aiAnimation* ai_animation = ai_scene->mAnimations[i];

            auto ani = std::make_shared<Animation>(ai_animation, this);
            animations.push_back(ani);
        }
        animation = animations[0];
        */

    }

}