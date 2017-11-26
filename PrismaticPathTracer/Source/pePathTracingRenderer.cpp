#include "pePathTracingRenderer.h"
#include "Components/peCameraComponent.h"
#include "Math/MathUtil.h"
#include "Util/Intersections.h"
#include "Util/Ray.h"
#include "peEngine.h"

#include "FileSystem\lodepng.h"
#include "Tracers/pePathTracer.h"
#include "Type/peColor.h"

#include <sstream>

#include "Components/peStaticRenderComponent.h"
#include "Components/peTransformComponent.h"
#include "FileSystem/peFileSystemUtil.h"
#include <GL/glew.h>
#include <GL\freeglut.h>

static void dumpPPM(const pe::peVector<pe::RGBA_8Bit> &pixels,
                    const uint32_t width, const uint32_t height,
                    const std::string &path) {
  FILE *fp; /* b - binary mode */
  auto err = fopen_s(&fp, path.c_str(), "wb");
  (void)fprintf(fp, "P6\n%d %d\n255\n", width, height);
  for (auto &px : pixels) {
    fwrite(px.data(), 3, 1, fp);
  }
  fclose(fp);
}

void pe::pePathTracingRenderer::Init() {
  _windowWidth = 800;
  _windowHeight = 600;

  _window = std::make_unique<peGlWindow>();
  _window->Create(_windowWidth, _windowHeight);
  _window->CreateRenderContext();
}

void pe::pePathTracingRenderer::Shutdown() {
  _window->Destroy();
  _window = nullptr;
}

void pe::pePathTracingRenderer::Update(double deltaTime) {

  GatherLights();

  peScene scene;
  scene.BuildScene(_primitiveEntites, _staticEntities, _pointLights,
                   _directionalLights);

  pePathTracer pathTracer{scene};
  pathTracer.BeginRenderProcess(_windowWidth, _windowHeight);

  _window->SetActive();

  pePathTracer::ImageData_t image;
  uint32_t version = 0;

  GLuint texID;
  glGenTextures(1, &texID);

  glBindTexture(GL_TEXTURE_2D, texID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowWidth, _windowHeight, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  while (true) {

    _window->SetActive();

    if (pathTracer.HasNewResult()) {
      pathTracer.GetResult(image);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowWidth, _windowHeight, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, image.data());

      // std::stringstream ss;
      // ss << "D:\\tmp\\tracer_" << version << ".png";
      // DumpPNG(image, _windowWidth, _windowHeight, ss.str());
    }

    glViewport(0, 0, _windowWidth, _windowHeight);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_TRIANGLES);

    glTexCoord2f(0, 0);
    glVertex2f(-1.f, -1.f);

    glTexCoord2f(1, 0);
    glVertex2f(1.f, -1.f);

    glTexCoord2f(1, 1);
    glVertex2f(1.f, 1.f);

    glTexCoord2f(0, 0);
    glVertex2f(-1.f, -1.f);

    glTexCoord2f(0, 1);
    glVertex2f(-1.f, 1.f);

    glTexCoord2f(1, 1);
    glVertex2f(1.f, 1.f);

    glEnd();

    _window->Present();
  }

  getchar();
}

void pe::pePathTracingRenderer::RegisterDrawableEntity(const peEntity &entity) {
  const auto primitiveComponent =
      entity.GetComponent<pePrimitiveRenderComponent>();
  if (primitiveComponent) {
    _primitiveEntites.push_back(primitiveComponent);
  }

  const auto staticRenderComponent =
      entity.GetComponent<peStaticRenderComponent>();
  if (staticRenderComponent) {
    _staticEntities.push_back(staticRenderComponent);
  }
}

void pe::pePathTracingRenderer::DeregisterDrawableEntity(
    const peEntity &entity) {
  auto eraseComponent = [&](auto &vec) {
    auto where =
        std::find_if(vec.begin(), vec.end(), [&](auto &componentHandle) {
          return componentHandle.GetEntity() == entity;
        });
    if (where == vec.end())
      return;
    vec.erase(where);
  };

  eraseComponent(_primitiveEntites);
}

void pe::pePathTracingRenderer::RegisterRenderResource(
    const peWeakPtr<peRenderResource> &res) {}

void pe::pePathTracingRenderer::DeregisterRenderResource(
    const peWeakPtr<peRenderResource> &res) {}

void pe::pePathTracingRenderer::GatherLights() {
  _pointLights.clear();
  _directionalLights.clear();

  auto &entitySys = PrismaticEngine.GetWorld()->EntityManager();

  for (auto pointLight : entitySys.AllComponents<pePointLightComponent>()) {
    _pointLights.push_back(pointLight);
  }

  for (auto dirLight : entitySys.AllComponents<peDirectionalLightComponent>()) {
    _directionalLights.push_back(dirLight);
  }
}
