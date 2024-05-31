#include "MeshManager.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <json.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <mio.hpp>
#include "../Direct3DResources.hpp"
#include <DirectXMath.h>
#include "../PhysXManager/PhysXManager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cooking/PxCooking.h>

using JSON = nlohmann::json;
using namespace SBEngine;
using namespace physx;

struct VertexData {
	float pos[3];
	float uv[2];
	float norm[3];
};

SBEngine::MeshManager_t SBEngine::meshManager = {};

ID3D11ShaderResourceView* LoadTexture(const std::string& FilePath) {
	int TextureWidth, TextureHeight, TextureChannels = 0;

	unsigned char* TextureBytes = stbi_load(FilePath.c_str(), &TextureWidth, &TextureHeight, &TextureChannels, 4);
	int TextureBytesPerRow = 4 * TextureWidth;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = TextureWidth;
	textureDesc.Height = TextureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA SubresourceData = {};
	SubresourceData.pSysMem = TextureBytes;
	SubresourceData.SysMemPitch = TextureBytesPerRow;

	ID3D11Texture2D* Texture;
	SBEngine::Direct3DResources.d3d11Device->CreateTexture2D(&textureDesc, &SubresourceData, &Texture);

	ID3D11ShaderResourceView* TextureView;
	SBEngine::Direct3DResources.d3d11Device->CreateShaderResourceView(Texture, nullptr, &TextureView);
	Texture->Release();

	free(TextureBytes);

	return TextureView;
}

void MeshManager_t::StartLoading() {
	std::filesystem::directory_iterator DirectoryIterator("meshes");
	for (const std::filesystem::directory_entry& Directory : DirectoryIterator) {

		if (!Directory.is_directory())
			continue;

		const std::string& PathString = Directory.path().generic_string();

		if (!std::filesystem::exists(PathString + "\\description.json")) {
			std::cout << "'description.json' is missing in directory '" << PathString + "\\description.json" << "'" << std::endl;
			continue;
		}

		std::error_code Error;
		mio::mmap_source MappedFile = mio::make_mmap_source(PathString + "\\description.json", 0, mio::map_entire_file, Error);

		if (MappedFile.begin() == nullptr) {
			std::cout << "Failed to load file '" << PathString + "\\description.json" << "' " << Error.message() << std::endl;
			continue;
		}

		JSON JsonData;
		JsonData.parse(std::string(MappedFile.begin(), MappedFile.end()));

		MappedFile.unmap();

		if (!JsonData.contains("modelMesh") || !JsonData.contains("material")) {
			std::cout << "Json file '" << PathString + "\\description.json" << "' doesnt contain 'modelMesh' or 'material'" << std::endl;
			continue;
		}

		// Because the Standard Filesystem gives / and not \ why tho?
		const std::string& MeshName = PathString.substr(PathString.find_last_of('/') + 1, PathString.size());

		Mesh* LoadedMesg = new Mesh;
		LoadedMesg->modelName = MeshName;
		std::vector<DirectX::XMFLOAT3> AllVerticies;
		Assimp::Importer Importer;
		Importer.SetPropertyFloat("scale", 2.0F);
		
		const aiScene* Scene = Importer.ReadFile(PathString + "\\" + JsonData["modelMesh"].get<std::string>(), aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenUVCoords);

		for (int SceneIterator = 0; SceneIterator < Scene->mNumMeshes; ++SceneIterator) {
			Model NewModel;

			const aiMesh* Mesh = Scene->mMeshes[SceneIterator];

			std::vector<VertexData> VertexVec;
			for (int VertexIterator = 0; VertexIterator < Mesh->mNumVertices; ++VertexIterator) {
				aiVector3D VertexP = Mesh->mVertices[VertexIterator];
				aiVector3D VertexN = Mesh->mNormals[VertexIterator];

				float VertexU, VertexV;

				if (Mesh->mNumUVComponents[SceneIterator] > VertexIterator) {
					VertexU = Mesh->mTextureCoords[SceneIterator][VertexIterator].x;
					VertexV = Mesh->mTextureCoords[SceneIterator][VertexIterator].y;
				} else {
					if (Mesh->mTangents != nullptr && Mesh->mBitangents != nullptr) {
						VertexU = Mesh->mTangents[VertexIterator].x;
						VertexV = Mesh->mBitangents[VertexIterator].y;
					} else {
						VertexU = 0.0F;
						VertexV = 0.0F;
					}
				}

				VertexVec.push_back({ VertexP.x, VertexP.y, VertexP.z, VertexU, VertexV, VertexN.x, VertexN.y, VertexN.z });

				AllVerticies.push_back({ VertexP.x, VertexP.y, VertexP.z });
			}

			std::vector<unsigned int> IndiceVec;
			for (int FaceIterator = 0; FaceIterator < Mesh->mNumFaces; ++FaceIterator) {
				aiFace* Face = &Mesh->mFaces[FaceIterator];
				for (unsigned int IndiceIterator = 0; IndiceIterator < Face->mNumIndices; ++IndiceIterator) {
					IndiceVec.push_back(Face->mIndices[IndiceIterator]);
				}
			}

			D3D11_BUFFER_DESC vertexBufferDesc = {};
			vertexBufferDesc.ByteWidth = sizeof(VertexData) * VertexVec.size();
			vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			D3D11_SUBRESOURCE_DATA vertexResource = { VertexVec.data() };
			
			HRESULT hResult = SBEngine::Direct3DResources.d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexResource, &NewModel.vertexBuffer);
			if (FAILED(hResult))
				assert("Vertex Buffer Creation Failed");

			D3D11_BUFFER_DESC indiceBufferDesc = {};
			indiceBufferDesc.ByteWidth = sizeof(unsigned int) * IndiceVec.size();
			indiceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indiceBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

			D3D11_SUBRESOURCE_DATA indiceResource = { IndiceVec.data() };

			hResult = SBEngine::Direct3DResources.d3d11Device->CreateBuffer(&indiceBufferDesc, &indiceResource, &NewModel.indiceBuffer);
			if (FAILED(hResult))
				assert("Indice Buffer Creation Failed");

			std::vector<DirectX::XMFLOAT3> collisionVertex;
			for (unsigned int I = 0; I < VertexVec.size(); ++I) {
				auto& currElem = VertexVec[I];
				collisionVertex.push_back({ currElem.pos[0], currElem.pos[1], currElem.pos[2] });
			}

			NewModel.modelTexture.diffuseMap = LoadTexture(PathString + "\\" + JsonData["material"]["volume"].get<std::string>());
			NewModel.numIndices = IndiceVec.size();

			LoadedMesg->AddModel(NewModel);
		}

		PxConvexMeshDesc ConvexMeshDesc;
		ConvexMeshDesc.points.count = AllVerticies.size();
		ConvexMeshDesc.points.data = AllVerticies.data();
		ConvexMeshDesc.points.stride = sizeof(DirectX::XMFLOAT3);
		ConvexMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
		
		


		PxCookingParams ConvexMeshParams(PhysXManager.mPhysics->getTolerancesScale());

		PxConvexMesh* ConvexMesh = PxCreateConvexMesh(ConvexMeshParams, ConvexMeshDesc, PhysXManager.mPhysics->getPhysicsInsertionCallback());

		// Erstelle eine Form mit dem konvexen Mesh
		LoadedMesg->physicsModel = PhysXManager.mPhysics->createShape(PxConvexMeshGeometry(ConvexMesh), *PhysXManager.mMaterial, false);

		ConvexMesh->release();

		this->meshList[MeshName] = LoadedMesg;
	}
}
