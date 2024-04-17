//DH2323 skeleton code, Lab2 (SDL2 version)
#include <iostream>
#include <glm/glm.hpp>
#include "SDL2auxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL2Aux *sdlAux;
int t;
vector<Triangle> triangles;
float focalLength = SCREEN_HEIGHT;
vec3 cameraPos(0, 0, -3);

// ----------------------------------------------------------------------------
// STRUCTURE

struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update(void);
void Draw(void);
bool ClosestIntersection(vec3 start, vec3 dir, const vector <Triangle >& triangles, Intersection& closeIntersection);

int main( int argc, char* argv[] )
{
	sdlAux = new SDL2Aux(SCREEN_WIDTH, SCREEN_HEIGHT);
	t = SDL_GetTicks();	// Set start value for timer.
	LoadTestModel(triangles);

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
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_UP])
	{

	}
	if (keystate[SDL_SCANCODE_DOWN])
	{

	}
	if (keystate[SDL_SCANCODE_LEFT])
	{

	}
	if (keystate[SDL_SCANCODE_RIGHT])
	{

	}
}

void Draw()
{
	sdlAux->clearPixels();

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			vec3 dir(x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2, focalLength);
			dir = glm::normalize(dir);

			Intersection closeIntersection;
			vec3 color(0, 0, 0);

			if (ClosestIntersection(cameraPos, dir, triangles, closeIntersection))
			{
				const Triangle& hit = triangles[closeIntersection.triangleIndex];
				color = hit.color;
			}

			sdlAux->putPixel(x, y, color);
		}
	}
	sdlAux->render();
}

bool ClosestIntersection(vec3 start, vec3 dir, const vector <Triangle >& triangles, Intersection& closeIntersection)
{
	float m = std::numeric_limits<float>::max();
	bool intersect = false;

	for (size_t i = 0; i < triangles.size(); i++)
	{
		const Triangle triangle = triangles[i];
		vec3 v0 = triangle.v0;
		vec3 v1 = triangle.v1;
		vec3 v2 = triangle.v2;
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;
		vec3 b = start - v0;
		mat3 A(-dir, e1, e2);
		vec3 x = glm::inverse(A) * b;

		float t = x.x, u = x.y, v = x.z;

		if (u >= 0 && v >= 0 && (u + v) <= 1 && t >= 0 && t < m)
		{
			intersect = true;
			m = t;
			closeIntersection.position = start + t * dir;
			closeIntersection.distance = t;
			closeIntersection.triangleIndex = i;
		}

	}
	return intersect;
}
