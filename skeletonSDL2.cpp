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

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;
SDL2Aux* sdlAux;
int t;
vector<Triangle> triangles;
float focalLength = SCREEN_HEIGHT;
vec3 cameraPos(0, 0, -3);
mat3 R = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));
float yaw = 0;
vec3 lightPos(0, -0.5, -0.7);
vec3 lightPower = 14.f * vec3(1, 1, 1);
vec3 indirectLight = 0.5f * vec3(1, 1, 1);


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
vec3 DirectLight(const Intersection& i);

int main(int argc, char* argv[])
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
	float dt = float(t2 - t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	float movement = 0.1;
	vec3 forward(R[2][0], R[2][1], R[2][2]);

	if (keystate[SDL_SCANCODE_UP])
	{
		cameraPos += movement * forward;
	}
	if (keystate[SDL_SCANCODE_DOWN])
	{
		cameraPos -= movement * forward;
	}
	if (keystate[SDL_SCANCODE_LEFT])
	{
		yaw -= movement;
		R = mat3(cos(yaw), 0, sin(yaw), 0, 1, 0, -sin(yaw), 0, cos(yaw));
	}
	if (keystate[SDL_SCANCODE_RIGHT])
	{
		yaw += movement;
		R = mat3(cos(yaw), 0, sin(yaw), 0, 1, 0, -sin(yaw), 0, cos(yaw));
	}

	// Move Light Source using WASDQE
	if (keystate[SDL_SCANCODE_W])
	{
		lightPos += movement * forward;
	}

	if (keystate[SDL_SCANCODE_S])
	{
		lightPos -= movement * forward;
	}

	if (keystate[SDL_SCANCODE_A])
	{
		lightPos -= movement * vec3(R[0][0], R[0][1], R[0][2]);
	}

	if (keystate[SDL_SCANCODE_D])
	{
		lightPos += movement * vec3(R[0][0], R[0][1], R[0][2]);
	}

	if (keystate[SDL_SCANCODE_Q])
	{
		lightPos -= movement * vec3(R[2][0], R[2][1], R[2][2]);
	}

	if (keystate[SDL_SCANCODE_E])
	{
		lightPos += movement * vec3(R[2][0], R[2][1], R[2][2]);
	}
}

void Draw()
{
	sdlAux->clearPixels();

	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			vec3 dir(x - SCREEN_WIDTH / 2, y - SCREEN_HEIGHT / 2, focalLength);
			dir = glm::normalize(dir);

			Intersection closeIntersection;
			vec3 color(0, 0, 0);

			if (ClosestIntersection(cameraPos, dir, triangles, closeIntersection))
			{
				// Direct Lighting (Task 6.3)
				// color = DirectLight(closeIntersection);

				// Get the color of the closest intersection for pho (Task 6.4 and 6.6)
				color = triangles[closeIntersection.triangleIndex].color;

				// Diffuse Lighting (Task 6.4)
				// color *= DirectLight(closeIntersection);

				// Direct Lighting and Indirect Lighting (Task 6.6)
				color *= (DirectLight(closeIntersection) + indirectLight);
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
		vec3 v0 = triangle.v0 * R;
		vec3 v1 = triangle.v1 * R;
		vec3 v2 = triangle.v2 * R;
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

vec3 DirectLight(const Intersection& i)
{
	vec3 n = triangles[i.triangleIndex].normal;
	vec3 r = glm::normalize(lightPos - i.position);
	float r2 = glm::distance(lightPos, i.position);
	float max = std::max(0.f, glm::dot(n, r));
	vec3 B = lightPower / (4 * 3.14159265359f * r2);

	// Surface Light Ray (Task 6.5)
	Intersection shadowIntersection;
	if (ClosestIntersection(i.position + 0.001f * n, r, triangles, shadowIntersection) && shadowIntersection.distance < r2)
	{
		return vec3(0, 0, 0);
	}

	return B * max;
}
