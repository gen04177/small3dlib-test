#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#define S3L_STRICT_NEAR_CULLING 0

#if TEXTURES
  #define S3L_PERSPECTIVE_CORRECTION 2
#else
  #define S3L_PERSPECTIVE_CORRECTION 0
#endif

#define S3L_NEAR (S3L_F / 5)
#define S3L_Z_BUFFER 1
#define S3L_PIXEL_FUNCTION drawPixel
#define S3L_RESOLUTION_X 800
#define S3L_RESOLUTION_Y 600

#define TEXTURE_W 256
#define TEXTURE_H 256

#include "../../small3dlib.h"
#include "alligatorModel.h"

void printHelp(void)
{
    printf("AlligatorModel: example program for small3dlib.\n\n");

    printf("\nby Miloslav Ciz, released under CC0 1.0\n");

    printf("\n@gen04177 v0.1\n");
}

S3L_Unit normals[ALLIGATOR_VERTEX_COUNT * 3];
S3L_Scene scene;
S3L_Vec4 teleportPoint;
S3L_Vec4 toLight;

uint32_t pixels[S3L_RESOLUTION_X * S3L_RESOLUTION_Y];

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *textureSDL;
SDL_Surface *screenSurface;
SDL_Event event;
SDL_GameController *controller = NULL;

void sdlInit(void)
{
  window = SDL_CreateWindow("model viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, S3L_RESOLUTION_X, S3L_RESOLUTION_Y, SDL_WINDOW_SHOWN); 
  renderer = SDL_CreateRenderer(window,-1,0);
  textureSDL = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STATIC, S3L_RESOLUTION_X, S3L_RESOLUTION_Y);
  screenSurface = SDL_GetWindowSurface(window);

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
  controller = SDL_GameControllerOpen(0);
  if (!controller)
  {
    printf("Failed to open game controller: %s\n", SDL_GetError());
  }
}

void sdlEnd(void)
{
  SDL_GameControllerClose(controller);
  SDL_DestroyTexture(textureSDL);
  SDL_DestroyRenderer(renderer); 
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void sdlUpdate(void)
{
  SDL_UpdateTexture(textureSDL, NULL, pixels, S3L_RESOLUTION_X * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, textureSDL, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void clearScreen(void)
{
  memset(pixels, 200, S3L_RESOLUTION_X * S3L_RESOLUTION_Y * sizeof(uint32_t));
}

static inline void setPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
{
  uint8_t *p = ((uint8_t *) pixels) + (y * S3L_RESOLUTION_X + x) * 4 + 1;

  *p = blue;
  ++p;
  *p = green;
  ++p;
  *p = red;
}

void sampleTexture(const uint8_t *tex, int32_t u, int32_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
  u = S3L_wrap(u, TEXTURE_W);
  v = S3L_wrap(v, TEXTURE_H);

  const uint8_t *t = tex + (v * TEXTURE_W + u) * 4;

  *r = *t;
  t++;
  *g = *t;
  t++;
  *b = *t;
}

void drawPixel(S3L_PixelInfo *p)
{
  static uint32_t previousTriangle = 1000;
  static S3L_Vec4 n0, n1, n2;

  if (p->triangleID != previousTriangle)
  {
    S3L_getIndexedTriangleValues(
      p->triangleIndex,
      scene.models[p->modelIndex].triangles,
      normals, 3, &n0, &n1, &n2);

    previousTriangle = p->triangleID;
  }

  S3L_Vec4 normal;
  normal.x = S3L_interpolateBarycentric(n0.x, n1.x, n2.x, p->barycentric);
  normal.y = S3L_interpolateBarycentric(n0.y, n1.y, n2.y, p->barycentric);
  normal.z = S3L_interpolateBarycentric(n0.z, n1.z, n2.z, p->barycentric);

  S3L_vec3Normalize(&normal);

  S3L_Unit shading = (S3L_vec3Dot(normal, toLight) + S3L_F) / 2;
  shading = S3L_interpolate(shading, 0, p->depth, 32 * S3L_F);

  int index = (p->y * S3L_RESOLUTION_X + p->x) * 4;
  ((uint8_t *)pixels)[index + 1] = S3L_clamp(S3L_interpolateByUnitFrom0(200, shading), 0, 255); // R
  ((uint8_t *)pixels)[index + 2] = S3L_clamp(S3L_interpolateByUnitFrom0(255, shading), 0, 255); // G
  ((uint8_t *)pixels)[index + 3] = S3L_clamp(S3L_interpolateByUnitFrom0(150, shading), 0, 255); // B
}

int main()
{

  printHelp();

  sdlInit();

  S3L_vec4Set(&toLight, 10, -10, -10, 0);  
  S3L_vec3Normalize(&toLight);

  alligatorModelInit();
  S3L_computeModelNormals(alligatorModel, normals, 0);
  S3L_sceneInit(&alligatorModel, 1, &scene);

  scene.camera.transform.translation.z = -8 * S3L_F;
  scene.camera.transform.translation.x = 9 * S3L_F;
  scene.camera.transform.translation.y = 6 * S3L_F;
  S3L_lookAt(scene.models[0].transform.translation, &(scene.camera.transform));

  int running = 1;

  while (running)
  {
    clearScreen();
    S3L_newFrame();
    S3L_drawScene(scene);
    sdlUpdate();

    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        running = 0;
    }

    int16_t xAxis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
    int16_t yAxis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

    float xNorm = xAxis / 32767.0f;
    float yNorm = yAxis / 32767.0f;

    scene.models[0].transform.rotation.y += xNorm * S3L_F / 16;
    scene.models[0].transform.rotation.x += yNorm * S3L_F / 16;
  }

  sdlEnd();
  return 0;
}