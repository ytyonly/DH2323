//DH2323 skeleton code, Lab3 (SDL2 version)
#include <iostream>
#include <glm/glm.hpp>
#include "SDL2auxiliary.h"
#include "TestModel.h"
#include <algorithm> //for max()

using namespace std;
using glm::vec3;
using glm::vec2;
using glm::mat3;
using glm::ivec2;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL2Aux *sdlAux;
int t;
vector<Triangle> triangles;
float focalLength = SCREEN_HEIGHT / 2;
vec3 cameraPos = vec3(0, 0, -3.001);
mat3 R = mat3(1.0);
float cameraSpeed = 0.01;

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update(void);
void Draw(void);
void VertexShader(const vec3& v, ivec2& p);
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL(ivec2 a, ivec2 b, vec3 color);
void DrawPolygonEdges(const vector<vec3>& vertices);

int main(int argc, char* argv[])
{
	LoadTestModel(triangles);  // Load model
	sdlAux = new SDL2Aux(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.

	while (!sdlAux->quitEvent())
	{
		Update();
		Draw();
	}
	sdlAux->saveBMP("screenshot.bmp");
	return 0;
}

void Update(void)
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_UP]) {
		// Move camera forward
	}
	if (keystate[SDL_SCANCODE_DOWN]) {
		// Move camera backward
	}
	if (keystate[SDL_SCANCODE_LEFT]) {
		// Move camera to the left
	}
	if (keystate[SDL_SCANCODE_RIGHT]) {
		// Move camera to the right
	}
	if (keystate[SDL_SCANCODE_W]) {
		cameraPos += vec3(0, 0, 1) * cameraSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_S]) {
		cameraPos += vec3(0, 0, -1) * cameraSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_A]) {
		cameraPos += vec3(-1, 0, 0) * cameraSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_D]) {
		cameraPos += vec3(1, 0, 0) * cameraSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_Q]) {
		float angle = -cameraSpeed * dt;
		R = R * mat3(cos(angle), 0, -sin(angle), 0, 1, 0, sin(angle), 0, cos(angle));
	}
	if (keystate[SDL_SCANCODE_E]) {
		float angle = -cameraSpeed * dt;
		R = R * mat3(cos(angle), 0, sin(angle), 0, 1, 0, -sin(angle), 0, cos(angle));
	}
}

void Draw()
{
	/* task 3 code
	sdlAux->clearPixels();

	for (int i = 0; i<triangles.size(); ++i)
	{
		vector<vec3> vertices(3);

		vertices[0] = triangles[i].v0;
		vertices[1] = triangles[i].v1;
		vertices[2] = triangles[i].v2;

		for (int v = 0; v < 3; ++v) {
			ivec2 projPos;
			VertexShader(vertices[v], projPos);
			vec3 color(1, 1, 1);
			sdlAux->putPixel(projPos.x, projPos.y, color);
		}
	}

	sdlAux->render();
	*/

	//Task 4 code
	sdlAux->clearPixels();
	for (int i = 0; i < triangles.size(); ++i)
	{
		vector<vec3> vertices(3);
		vertices[0] = triangles[i].v0;
		vertices[1] = triangles[i].v1;
		vertices[2] = triangles[i].v2;
		DrawPolygonEdges(vertices);
	}
	sdlAux->render();
}

void VertexShader(const vec3& v, ivec2& p) {

	vec3 pos = (v - cameraPos) * R;

	p.x = focalLength * pos.x / pos.z + SCREEN_WIDTH / 2;
	p.y = focalLength * pos.y / pos.z + SCREEN_HEIGHT / 2;
}

//goes from a to b an fills the result vector with the linear interpolation of doing so
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result)
{
	int N = result.size();
	vec2 step = vec2(b - a) / float(max(N - 1, 1));
	vec2 current(a);
	for (int i = 0; i < N; ++i)
	{
		result[i] = current;
		current += step;
	}
}

//Draws lines between the dots
void DrawLineSDL(ivec2 a, ivec2 b, vec3 color)
{
	//compute number of pixels to draw
	ivec2 delta = glm::abs(a - b);
	int pixels = glm::max(delta.x, delta.y) + 1;

	//get the pixel positions of the line using the interpolation function
	vector<ivec2> line(pixels);
	Interpolate(a, b, line);

	//loop through each pixel position and put a pixel there
	for (int i = 0; i < pixels; i++)
	{
		sdlAux->putPixel(line[i].x, line[i].y, color);
	}
}

void DrawPolygonEdges(const vector<vec3>& vertices)
{
	int V = vertices.size();
	//Transform each vertex from 3D world position to 2D image position:
	vector<ivec2> projectedVertices(V);
	for (int i = 0; i < V; ++i)
	{
		VertexShader(vertices[i], projectedVertices[i]);
	}
	//Loop over all vertices and draw the edge from it to the next vertex:
	for (int i = 0; i < V; ++i)
	{
		int j = (i + 1) % V; //The next vertex
		vec3 color(1, 1, 1);
		DrawLineSDL(projectedVertices[i], projectedVertices[j], color);
	}
}