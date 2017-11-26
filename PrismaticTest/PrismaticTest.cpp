// PrismaticTest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "peEngine.h"
#include "stdafx.h"

#include "Components/peCameraComponent.h"
#include "Components/peLightComponent.h"
#include "Components/pePrimitiveRenderComponent.h"
#include "Components/peStaticRenderComponent.h"
#include "Components/peTransformComponent.h"
#include "Exceptions/peExceptions.h"
#include "OBJ_Loader.h"
#include "Rendering/peMesh.h"
#include "Rendering\Utility\peBxDF.h"
#include <exception>
#include <fbxsdk.h>
#include <stdio.h>
#include <unordered_set>

#pragma warning(disable : 4100)

using namespace pe;

std::vector<std::unique_ptr<pe::peBxDF>> _bsdfs;

struct Vertex {
  glm::vec3 pos;
  glm::vec3 nor;
};

static void LoadOBJ(const std::string &path) {
  objl::Loader loader;
  if (!loader.LoadFile(path)) {
    PrismaticEngine.GetLogging()->LogError("File %s could not be loaded!",
                                           path.c_str());
    return;
  }

  // Create material for objects
  // TODO Read this from obj as well
  auto lambertColor = pe::Spectrum_t{1.f, 0.f, 1.f};
  auto lambert = pe::peLambert{lambertColor};
  auto lambertBrdf = std::make_unique<pe::peLambert>(lambertColor);
  pe::BSDF bsdf;
  bsdf.Add(lambertBrdf.get());

  _bsdfs.push_back(std::move(lambertBrdf));

  auto material = pe::peMaterial::New();
  material->SetOfflineData(bsdf);

  VertexLayout vertexLayout;
  vertexLayout.components.push_back(
      VertexComponent{VertexAttribute::Position, VertexDataType::Float});
  vertexLayout.components.push_back(
      VertexComponent{VertexAttribute::Normal, VertexDataType::Float});

  auto &entityMgr = PrismaticEngine.GetWorld()->EntityManager();

  for (auto &objMesh : loader.LoadedMeshes) {
    auto mesh = pe::peMesh::New(vertexLayout);

    peVector<Vertex> verts;
    verts.resize(objMesh.Vertices.size());
    for (size_t idx = 0; idx < verts.size(); ++idx) {
      auto &objVert = objMesh.Vertices[idx];
      auto &vert = verts[idx];
      vert.pos = {objVert.Position.X, objVert.Position.Y, objVert.Position.Z};
      vert.nor = {objVert.Normal.X, objVert.Normal.Y, objVert.Normal.Z};
    }

    mesh->SetVertexData(
        {reinterpret_cast<char *>(verts.data()),
         reinterpret_cast<char *>(verts.data() + verts.size())});
    mesh->SetIndexData({objMesh.Indices.begin(), objMesh.Indices.end()});

    auto entity = entityMgr.CreateEntity();
    auto renderComponent = entity.AddComponent<peStaticRenderComponent>();
    auto &rc = *renderComponent;
    rc.material = material;
    rc.mesh = mesh;
  }
}

static glm::mat4 FbxToGlmMat(const FbxAMatrix &fbxMat) {
  return glm::mat4{
      {fbxMat.mData[0][0], fbxMat.mData[0][1], fbxMat.mData[0][2],
       fbxMat.mData[0][3]},
      {fbxMat.mData[1][0], fbxMat.mData[1][1], fbxMat.mData[1][2],
       fbxMat.mData[1][3]},
      {fbxMat.mData[2][0], fbxMat.mData[2][1], fbxMat.mData[2][2],
       fbxMat.mData[2][3]},
      {fbxMat.mData[3][0], fbxMat.mData[3][1], fbxMat.mData[3][2],
       fbxMat.mData[3][3]},
  };
}

static glm::mat4 FbxToGlmMat(const FbxMatrix &fbxMat) {
  return glm::mat4{
      {fbxMat.mData[0][0], fbxMat.mData[0][1], fbxMat.mData[0][2],
       fbxMat.mData[0][3]},
      {fbxMat.mData[1][0], fbxMat.mData[1][1], fbxMat.mData[1][2],
       fbxMat.mData[1][3]},
      {fbxMat.mData[2][0], fbxMat.mData[2][1], fbxMat.mData[2][2],
       fbxMat.mData[2][3]},
      {fbxMat.mData[3][0], fbxMat.mData[3][1], fbxMat.mData[3][2],
       fbxMat.mData[3][3]},
  };
}

static void LoadFBX(const std::string &path) {
  auto &entityMgr = PrismaticEngine.GetWorld()->EntityManager();
  constexpr auto WindowWidth = 800, WindowHeight = 600;

  auto fbxSdkMgr = FbxManager::Create();
  auto ios = FbxIOSettings::Create(fbxSdkMgr, IOSROOT);
  fbxSdkMgr->SetIOSettings(ios);

  // Create an importer using the SDK manager.
  auto lImporter = FbxImporter::Create(fbxSdkMgr, "");

  // Use the first argument as the filename for the importer.
  if (!lImporter->Initialize(path.c_str(), -1, fbxSdkMgr->GetIOSettings())) {
    printf("Call to FbxImporter::Initialize() failed.\n");
    printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
    exit(-1);
  }

  auto lScene = FbxScene::Create(fbxSdkMgr, "myScene");

  // Import the contents of the file into the scene.
  lImporter->Import(lScene);
  lImporter->Destroy();

  VertexLayout vertexLayout;
  vertexLayout.components.push_back(
      VertexComponent{VertexAttribute::Position, VertexDataType::Float});
  vertexLayout.components.push_back(
      VertexComponent{VertexAttribute::Normal, VertexDataType::Float});

  // TODO Read this from obj as well
  auto lambertColor = pe::Spectrum_t{1.f, 1.f, 1.f};
  auto lambertBrdf = std::make_unique<pe::peLambert>(lambertColor);
  pe::BSDF defaultBsdf;
  defaultBsdf.Add(lambertBrdf.get());
  _bsdfs.push_back(std::move(lambertBrdf));

  auto defaultMaterial = pe::peMaterial::New();
  defaultMaterial->SetOfflineData(defaultBsdf);

  peUnorderedMap<FbxSurfaceMaterial *, peMaterial::Handle_t> materialMapping;

  // First extract all the geometry and convert it to meshes
  peUnorderedMap<FbxMesh *, peMesh::Handle_t> meshMapping;
  for (auto idx = 0; idx < lScene->GetGeometryCount(); ++idx) {
    auto geometry = lScene->GetGeometry(idx);
    if (!geometry->Is<FbxMesh>())
      continue;

    auto fbxMesh = static_cast<FbxMesh *>(geometry);

    peVector<Vertex> vertices;
    peVector<uint32_t> indices;

    // Generate vertices
    vertices.reserve(fbxMesh->GetPolygonCount() * 3);
    indices.resize(fbxMesh->GetPolygonCount() * 3);
    std::iota(indices.begin(), indices.end(), 0);

    for (auto polyIdx = 0; polyIdx < fbxMesh->GetPolygonCount(); ++polyIdx) {
      auto vertsInPolygon = fbxMesh->GetPolygonSize(polyIdx);
      if (vertsInPolygon != 3)
        throw std::runtime_error{
            "Only triangle meshes are supported currently!"};

      const auto polyIdxOffset = fbxMesh->GetPolygonVertexIndex(polyIdx);
      const auto polyIndices = fbxMesh->GetPolygonVertices() + polyIdxOffset;

      for (auto vertIdx = 0; vertIdx < vertsInPolygon; ++vertIdx) {
        auto &vert = vertices.emplace_back();
        auto ctrlPointIdx = fbxMesh->GetPolygonVertex(polyIdx, vertIdx);
        auto ctrlPoint = fbxMesh->GetControlPointAt(ctrlPointIdx);
        FbxVector4 normal;
        auto hasNormal =
            fbxMesh->GetPolygonVertexNormal(polyIdx, vertIdx, normal);

        vert.pos = {ctrlPoint.mData[0], ctrlPoint.mData[1], ctrlPoint.mData[2]};
        vert.nor = {normal.mData[0], normal.mData[1], normal.mData[2]};
      }
    }

    auto mesh = peMesh::New(vertexLayout);
    mesh->SetVertexData(
        {reinterpret_cast<char *>(vertices.data()),
         reinterpret_cast<char *>(vertices.data() + vertices.size())});
    mesh->SetIndexData(std::move(indices));

    meshMapping[fbxMesh] = mesh;
  }

  // Parse the scenegraph
  for (auto nodeIdx = 0; nodeIdx < lScene->GetNodeCount(); ++nodeIdx) {
    auto node = lScene->GetNode(nodeIdx);
    if (node->GetCamera()) {
      // auto fbxCam = node->GetCamera();
      //
      // auto camera = entityMgr.CreateEntity();
      // auto cameraComponent = camera.AddComponent<pe::peCameraComponent>();
      //
      // cameraComponent->view = FbxToGlmMat(node->EvaluateGlobalTransform());
      // auto &cameraSystem =
      //    entityMgr.GetComponentSystem<pe::peCameraComponent>();
      // cameraSystem.SetPerspectiveProjection(90_deg, 800 / 600.f, 0.1f, 40.f,
      //                                      cameraComponent);
      // TODO Projection is currently wrong...
      // cameraComponent.projection = FbxToGlmMat(
      //    fbxCam->ComputeProjectionMatrix(WindowWidth, WindowHeight));
      // cameraComponent->fov =
      //    Degrees{static_cast<float>(fbxCam->FieldOfViewX.Get())};
      // cameraComponent.zMin = static_cast<float>(fbxCam->GetNearPlane());
      // cameraComponent.zMax = static_cast<float>(fbxCam->GetFarPlane());
    }

    if (node->GetLight()) {
      auto fbxLight = node->GetLight();
      if (fbxLight->LightType.Get() == FbxLight::EType::ePoint) {
        auto light = entityMgr.CreateEntity();
        auto lightComponent = light.AddComponent<pe::pePointLightComponent>();

        auto fbxLightColor = fbxLight->Color.Get();
        auto lightIntensity =
            RGB_32BitFloat{static_cast<float>(fbxLightColor[0]),
                           static_cast<float>(fbxLightColor[1]),
                           static_cast<float>(fbxLightColor[2])};

        lightIntensity *= static_cast<float>(fbxLight->Intensity.Get());

        auto lightTransform = FbxToGlmMat(node->EvaluateGlobalTransform());

        lightComponent->intensity = lightIntensity;
        lightComponent->numSamples = 1;
        lightComponent->position = {lightTransform[3][0], lightTransform[3][1],
                                    lightTransform[3][2]};
      }
    }

    if (node->GetMaterialCount()) {
      auto fbxMaterial = node->GetMaterial(0);
      if (fbxMaterial->Is<FbxSurfaceLambert>()) {
        auto lambert = static_cast<FbxSurfaceLambert *>(fbxMaterial);

        auto diffuseColor = lambert->Diffuse.Get();
        auto brdf = std::make_unique<pe::peLambert>(
            pe::Spectrum_t{static_cast<float>(diffuseColor[0]),
                           static_cast<float>(diffuseColor[1]),
                           static_cast<float>(diffuseColor[2])});
        pe::BSDF bsdf;
        bsdf.Add(brdf.get());

        _bsdfs.push_back(std::move(brdf));

        auto material = pe::peMaterial::New();
        material->SetOfflineData(bsdf);

        materialMapping[fbxMaterial] = material;
      }
    }

    if (node->GetGeometry()) {
      auto fbxGeometry = node->GetGeometry();
      if (!fbxGeometry->Is<FbxMesh>())
        continue;
      auto fbxMesh = static_cast<FbxMesh *>(fbxGeometry);
      auto meshIter = meshMapping.find(fbxMesh);
      if (meshIter == meshMapping.end())
        continue;

      auto mesh = meshIter->second;

      auto entity = entityMgr.CreateEntity();
      auto renderComponent = entity.AddComponent<peStaticRenderComponent>();
      auto &rc = *renderComponent;

      peMaterial::Handle_t material = defaultMaterial;
      if (node->GetMaterial(0)) {
        auto matIter = materialMapping.find(node->GetMaterial(0));
        if (matIter != materialMapping.end())
          material = matIter->second;
      }

      rc.material = material;
      rc.mesh = mesh;

      auto transformComponent = entity.AddComponent<peTransformComponent>();
      transformComponent->transformation =
          FbxToGlmMat(node->EvaluateGlobalTransform());
    }
  }
}

void OnSceneUpdate(float deltaTime) {}

void Init() {
  auto &entityMgr = PrismaticEngine.GetWorld()->EntityManager();

  LoadFBX("D:\\Development\\Prismatic\\Assets\\cornell_simple.fbx");

  auto camera = entityMgr.CreateEntity();
  auto cameraComponent = camera.AddComponent<pe::peCameraComponent>();

  auto &cameraSystem = entityMgr.GetComponentSystem<pe::peCameraComponent>();
  cameraSystem.LookAt({0, 2, 4}, {0, 0, 0}, {0, 1, 0}, cameraComponent);
  cameraSystem.SetPerspectiveProjection(90_deg, 800 / 600.f, 0.1f, 40.f,
                                        cameraComponent);

  // pe::peSpherePrimitive spherePrimitive;
  // spherePrimitive.center = {1, 0, 1};
  // spherePrimitive.radius = 0.3f;
  //
  // pe::BSDF bsdf;
  // bsdf.Add(_bsdfs[0].get());
  //
  // auto material = pe::peMaterial::New();
  // material->SetOfflineData(bsdf);
  //
  // auto sphere = CreatePrimitive();
  // sphere->primitive = spherePrimitive;
  // sphere->material = material;

  // auto light = entityMgr.CreateEntity();
  // auto lightComponent = light.AddComponent<pe::pePointLightComponent>();
  //
  // lightComponent->intensity = {60.f, 60.f, 60.f};
  // lightComponent->numSamples = 1;
  // lightComponent->position = {0.f, 3.f, 6.f};
}

int _tmain(int argc, _TCHAR *argv[]) {

  try {
    PrismaticEngine.Init();

    Init();

    PrismaticEngine.GetRenderer()->Update(0.16f);

    // PrismaticEngine.GetUpdateSystem()->RegisterUpdateCallback(OnSceneUpdate);
    // PrismaticEngine.GetUpdateSystem()->Run();
  } catch (pe::peBaseException &ex) {
    printf(ex.what());
  }

  _bsdfs.clear();

  PrismaticEngine.Shutdown();

  return 0;
}
