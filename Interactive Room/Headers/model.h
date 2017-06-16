#ifndef MODEL_H
#define MODEL_H

#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"
#include "camera.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
using namespace glm;

enum Shift {
	SHIFT_UP,
	SHIFT_DOWN,
	SHIFT_LEFT,
	SHIFT_RIGHT,
	SHIFT_FORWARD,
	SHIFT_BACKWARD,
};

enum Rotate {
	ROTATE_UP,
	ROTATE_DOWN,
	ROTATE_LEFT,
	ROTATE_RIGHT,
	ROTATE_UP_LEFT,
	ROTATE_UP_RIGHT
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model
{
public:
	/*  Model Data */
	vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	float scale;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;
	//steps to translate
	const float step = 5.f;
	//angle of rotation
	const float angle = 1.5f;
	//stores all models to make shader switching easier
	static vector<Model*> models;
	int ID;

	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Model(string const &path, bool gamma = false, float scale = 0.02f) : gammaCorrection(gamma)
	{
		this->scale = scale;
		model_matrix = glm::scale(mat4(1), vec3(scale));
		loadModel(path);
		displacementFromOrigin = vec4(scale * 0.5f * vec3(xmax + xmin, ymax + ymin, zmax + zmin), 0);
		models.push_back(this);
		ID = models.size();
	}

	// draws the model, and thus all its meshes
	void Draw()
	{
		(*shade).use();
		(*shade).setInt("id", ID);
		(*shade).setMat4("model", model_matrix);
		for (unsigned int i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(*shade);
		}
	}

	// returns the original displacement of a model object in respect to the origin
	vec3 displacement() {
		return vec3(displacementFromOrigin);
	}

	// shifts an object in the direction specified
	void shift(Shift direction) {
		switch (direction) {
		case SHIFT_UP:
			displacementFromOrigin += scale * step * vec4(normalize(vec3(0, (*cam).Up.y, 0)), 0);
			model_matrix = translate(model_matrix, vec3(transpose(model_matrix) / scale * step * vec4(normalize(vec3(0, (*cam).Up.y, 0)), 0)));
			break;
		case SHIFT_DOWN:
			displacementFromOrigin += scale * -step * vec4(normalize(vec3(0, (*cam).Up.y, 0)), 0);
			model_matrix = translate(model_matrix, vec3(transpose(model_matrix) / scale * -step * vec4(normalize(vec3(0, (*cam).Up.y, 0)), 0)));
			break;
		case SHIFT_LEFT:
			displacementFromOrigin += scale * -step * vec4((*cam).Right, 0);
			model_matrix = translate(model_matrix, vec3(transpose(model_matrix) / scale * -step * vec4((*cam).Right, 0)));
			break;
		case SHIFT_RIGHT:
			displacementFromOrigin += scale * step * vec4((*cam).Right, 0);
			model_matrix = translate(model_matrix, vec3(transpose(model_matrix) / scale * step * vec4((*cam).Right, 0)));
			break;
		case SHIFT_FORWARD:
			displacementFromOrigin += scale * step * vec4(normalize(vec3((*cam).Front.x, 0, (*cam).Front.z)), 0);
			model_matrix = translate(model_matrix, vec3(transpose(model_matrix) / scale * step * vec4(normalize(vec3((*cam).Front.x, 0, (*cam).Front.z)), 0)));
			break;
		case SHIFT_BACKWARD:
			displacementFromOrigin += scale * -step * vec4(normalize(vec3((*cam).Front.x, 0, (*cam).Front.z)), 0);
			model_matrix = translate(model_matrix, vec3(transpose(model_matrix) / scale * -step * vec4(normalize(vec3((*cam).Front.x, 0, (*cam).Front.z)), 0)));
			break;
		}
	}

	//rotates an object in the direction specified
	void rotate(Rotate direction) {
		mat4 rotation;
		mat4 trans;
		mat4 transBack;

		trans = translate(trans, -vec3(displacementFromOrigin));
		transBack = translate(transBack, vec3(displacementFromOrigin));

		switch (direction) {
		case ROTATE_UP:
			rotation = glm::rotate(mat4(1), radians(angle), (*cam).Right);
			break;
		case ROTATE_DOWN:
			rotation = glm::rotate(mat4(1), radians(-angle), (*cam).Right);
			break;
		case ROTATE_LEFT:
			rotation = glm::rotate(mat4(1), radians(-angle), normalize(vec3(0, (*cam).Up.y, 0)));
			break;
		case ROTATE_RIGHT:
			rotation = glm::rotate(mat4(1), radians(angle), normalize(vec3(0, (*cam).Up.y, 0)));
			break;
		case ROTATE_UP_LEFT:
			rotation = glm::rotate(mat4(1), radians(angle), normalize(vec3((*cam).Front.x, 0, (*cam).Front.z)));
			break;
		case ROTATE_UP_RIGHT:
			rotation = glm::rotate(mat4(1), radians(-angle), normalize(vec3((*cam).Front.x, 0, (*cam).Front.z)));
			break;
		}
		model_matrix = transBack * rotation * trans * model_matrix;
	}

	//sets the shader that will be used to draw the object
	void setShader(Shader* shader) {
		shade = shader;
	}

	//sets the Camera that will be used for relative transformations
	void setCamera(Camera* camera) {
		cam = camera;
	}

private:
	//model matrix used to rotate and shift Model object
	mat4 model_matrix;
	//used to get the location of the center of an object in order to rotate it
	vec4 displacementFromOrigin;
	float xmin, ymin, zmin, xmax, ymax, zmax;
	//for first time setup of xmin ,ymin, zmin, xmax, ymax, and zmax
	bool first = true;
	//makes drawing objects and switching shaders more seamless
	Shader* shade;
	//Camera holder to shift and rotate according to camera
	Camera* cam;

	/*  Functions   */
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const &path)
	{
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene.
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	// processes mesh
	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		if (first) {
			//min
			xmin = mesh->mVertices[0].x;
			ymin = mesh->mVertices[0].y;
			zmin = mesh->mVertices[0].z;
			//max
			xmax = mesh->mVertices[0].x;
			ymax = mesh->mVertices[0].y;
			zmax = mesh->mVertices[0].z;
			first = false;
		}
		// data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder vec3 first.

			if (mesh->mVertices) {
				// positions
				vector.x = mesh->mVertices[i].x;
				if (vector.x < xmin)
					xmin = vector.x;
				if (vector.x > xmax)
					xmax = vector.x;

				vector.y = mesh->mVertices[i].y;
				if (vector.y < ymin)
					ymin = vector.y;
				if (vector.y > ymax)
					ymax = vector.y;

				vector.z = mesh->mVertices[i].z;
				if (vector.z < zmin)
					zmin = vector.z;
				if (vector.z > zmax)
					zmax = vector.z;

				vertex.Position = vector;
			}
			if (mesh->mNormals) {
				// normals
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			// texture coordinates
			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				vec2 vec;
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = vec2(0.0f, 0.0f);
			if (mesh->mTangents) {
				// tangent
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
			}
			if (mesh->mBitangents) {
				// bitangent
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			vertices.push_back(vertex);
		}
		// now walk through each of the mesh's faces (a face is a mesh's triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
			// Same applies to other texture as the following list summarizes:
			// diffuse: texture_diffuseN
			// specular: texture_specularN
			// normal: texture_normalN

			// 1. diffuse maps
			vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. specular maps
			vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			// 3. normal maps
			vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// 4. height maps
			vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}

		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // if texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = SOIL_load_image(filename.c_str(), &width, &height, &nrComponents, SOIL_LOAD_AUTO);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		SOIL_free_image_data(data);
	}
	else
	{
		cout << "Texture failed to load at path: " << path << endl;
		SOIL_free_image_data(data);
	}

	return textureID;
}
#endif
