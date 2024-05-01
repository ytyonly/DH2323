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
SDL2Aux* sdlAux;
int t;
vector<Triangle> triangles;
float focalLength = SCREEN_HEIGHT;
vec3 cameraPos = vec3(0, 0, -3.001);
float cameraSpeed = 0.01;
float yaw = 0;
mat3 R = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));
vec3 currentColor;
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

// ----------------------------------------------------------------------------
// STRUCT
struct Pixel
{
	int x;
	int y;
	float zinv;
};

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update(void);
void Draw(void);
void VertexShader(const vec3& v, ivec2& p);
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL(ivec2 a, ivec2 b, vec3 color);
void DrawPolygonEdges(const vector<vec3>& vertices);
void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void DrawRows(const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels);
void DrawPolygon(const vector<vec3>& vertices);
// overloading for Task 6
void VertexShader(const vec3& v, Pixel& p);
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
void DrawPolygonRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels);
void DrawPolygon_new(const vector<vec3>& vertices);
void DrawLineSDL(Pixel a, Pixel b, vec3 color);

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
		cameraPos += vec3(0, 0, 1) * cameraSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_DOWN]) {
		cameraPos += vec3(0, 0, -1) * cameraSpeed * dt;
	}
	if (keystate[SDL_SCANCODE_LEFT]) {
		yaw -= 0.1;
		R = mat3(cos(yaw), 0, sin(yaw), 0, 1, 0, -sin(yaw), 0, cos(yaw));
	}
	if (keystate[SDL_SCANCODE_RIGHT]) {
		yaw += 0.1;
		R = mat3(cos(yaw), 0, sin(yaw), 0, 1, 0, -sin(yaw), 0, cos(yaw));
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
	/*
	sdlAux->clearPixels();
	for (int i = 0; i < triangles.size(); ++i)
	{
		currentColor = triangles[i].color; //added for task 5
		vector<vec3> vertices(3);
		vertices[0] = triangles[i].v0 * R;
		vertices[1] = triangles[i].v1 * R;
		vertices[2] = triangles[i].v2 * R;
		//DrawPolygonEdges(vertices); //Just for task 4
		DrawPolygon(vertices); //For task 5 not task 4
	}
	sdlAux->render();
	*/

	// Task 6 code
	sdlAux->clearPixels();
	// init the depth buffer
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			depthBuffer[y][x] = 0;
		}
	}

	for (int i = 0; i < triangles.size(); ++i)
	{
		currentColor = triangles[i].color;
		vector<vec3> vertices(3);
		vertices[0] = triangles[i].v0 * R;
		vertices[1] = triangles[i].v1 * R;
		vertices[2] = triangles[i].v2 * R;
		DrawPolygon_new(vertices);
	}

	sdlAux->render();

}

void VertexShader(const vec3& v, ivec2& p) {

	vec3 pos = (v - cameraPos);

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

void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels)
{
	//Find the max and min y-value of the polygon
	int maxY = glm::max(glm::max(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
	int minY = glm::min(glm::min(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);

	//compute the number of rows it occupies (like example 40-10+1=31)
	int rowAmount = maxY - minY + 1;

	//Resize leftPixels and rightPixels so that they have an element for each row
	leftPixels.resize(rowAmount);
	rightPixels.resize(rowAmount);

	//Initialize the x-coordinates in leftPixeks to some really large value and the x-coordiantes in the rightPixels to some really small value
	for (int i = 0; i < rowAmount; i++)
	{
		leftPixels[i].x = +numeric_limits<int>::max();
		rightPixels[i].x = -numeric_limits<int>::max();
		
		leftPixels[i].y = i + minY; //Starts from minimum and increases
		rightPixels[i].y = i + minY;
	}

	//Loop through all edges of the polygon and use linear interpolation to find the x-coordinate for
	//each row it occupies. Update the corresponding values in rightPixels and leftPixels.
	for (int i = 0; i < vertexPixels.size(); ++i)
	{
		// First vertex
		ivec2 p1 = vertexPixels[i];

		// Next vertex
		ivec2 p2 = vertexPixels[(i + 1) % vertexPixels.size()];

		ivec2 delta = glm::abs(p2 - p1);

		// Compute the number of pixels on this edge
		int pixels = max(delta.x, delta.y) + 1;

		// Interpolate to get the line pixels
		vector<ivec2> line(pixels);
		Interpolate(p1, p2, line);

		// Update leftPixels and rightPixels with the values from the interpolated line
		for (int j = 0; j < line.size(); j++) {
			for (int k = 0; k < rowAmount; k++) {
				if (leftPixels[k].y == line[j].y) {
					if (leftPixels[k].x > line[j].x)
						leftPixels[k].x = line[j].x;
					if (rightPixels[k].x < line[j].x)
						rightPixels[k].x = line[j].x;
					break;
				}
			}
		}
	}
}

//Calls putPixel for each pixel between the start and end for each row
void DrawPolygonRows(const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels)
{
	for (int i = 0; i < leftPixels.size(); i++)
	{
		DrawLineSDL(leftPixels[i], rightPixels[i], currentColor);
	}
}

void DrawPolygon(const vector<vec3>& vertices)
{
	int V = vertices.size();
	vector<ivec2> vertexPixels(V);
	for (int i = 0; i < V; ++i)
	{
		VertexShader(vertices[i], vertexPixels[i]);
	}
	vector<ivec2> leftPixels;
	vector<ivec2> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	DrawPolygonRows(leftPixels, rightPixels);
}


// ----------------------------------------------------------------------------
// Below is the function rewritten for Task 6

void VertexShader(const vec3& v, Pixel& p)
{
	vec3 pos = (v - cameraPos);

	p.x = focalLength * pos.x / pos.z + SCREEN_WIDTH / 2;
	p.y = focalLength * pos.y / pos.z + SCREEN_HEIGHT / 2;
	p.zinv = 1.0 / pos.z;
}

void ComputePolygonRows_(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
{
	int maxY = glm::max(glm::max(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
	int minY = glm::min(glm::min(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
	int rowAmount = maxY - minY + 1;
	leftPixels.resize(rowAmount);
	rightPixels.resize(rowAmount);
	for (int i = 0; i < rowAmount; i++)
	{
		leftPixels[i].x = +numeric_limits<int>::max();
		rightPixels[i].x = -numeric_limits<int>::max();

		leftPixels[i].y = i + minY; //Starts from minimum and increases
		rightPixels[i].y = i + minY;
	}

	for (int i = 0; i < vertexPixels.size(); ++i)
	{
		// convert pixel to ivec2
		ivec2 p1 = ivec2(vertexPixels[i].x, vertexPixels[i].y);
		ivec2 p2 = ivec2(vertexPixels[(i + 1) % vertexPixels.size()].x, vertexPixels[(i + 1) % vertexPixels.size()].y);
		ivec2 delta = glm::abs(p2 - p1);
		int pixels = max(delta.x, delta.y) + 1;
		vector<ivec2> line(pixels);
		Interpolate(p1, p2, line);
		for (int j = 0; j < line.size(); j++) {
			for (int k = 0; k < rowAmount; k++) {
				if (leftPixels[k].y == line[j].y) {
					if (leftPixels[k].x > line[j].x) {
						leftPixels[k].x = line[j].x;
					}
					if (rightPixels[k].x < line[j].x) {
						rightPixels[k].x = line[j].x;
					}
					break;
				}
			}
		}
	}
}

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result)
{
	int N = result.size();
	float stepX = float(b.x - a.x) / max(N - 1, 1);
	float stepY = float(b.y - a.y) / max(N - 1, 1);
	float stepZInv = float(b.zinv - a.zinv) / max(N - 1, 1);

	Pixel current = a;
	for (int i = 0; i < N; ++i)
	{
		// Update each variable of pixel and avoid cumulative errors 
		current.x = a.x + (float)i * stepX;
		current.y = a.y + (float)i * stepY;
		current.zinv = a.zinv + (float)i * stepZInv;
		result[i] = current;
	}
}

void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels)
{
	int maxY = glm::max(glm::max(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
	int minY = glm::min(glm::min(vertexPixels[0].y, vertexPixels[1].y), vertexPixels[2].y);
	int rowAmount = maxY - minY + 1;
	leftPixels.resize(rowAmount);
	rightPixels.resize(rowAmount);
	for (int i = 0; i < rowAmount; i++)
	{
		leftPixels[i].x = +numeric_limits<int>::max();
		rightPixels[i].x = -numeric_limits<int>::max();

		leftPixels[i].y = i + minY; //Starts from minimum and increases
		rightPixels[i].y = i + minY;
	}

	for (int i = 0; i < vertexPixels.size(); ++i)
	{
		// convert pixel to ivec2
		ivec2 p1 = ivec2(vertexPixels[i].x, vertexPixels[i].y);
		ivec2 p2 = ivec2(vertexPixels[(i + 1) % vertexPixels.size()].x, vertexPixels[(i + 1) % vertexPixels.size()].y);
		ivec2 delta = glm::abs(p2 - p1);
		int pixels = max(delta.x, delta.y) + 1;
		
		vector<Pixel> line(pixels);
		
		Interpolate(vertexPixels[i], vertexPixels[(i + 1) % vertexPixels.size()], line);
		for (int j = 0; j < line.size(); j++) {
			for (int k = 0; k < rowAmount; k++) {
				if (leftPixels[k].y == line[j].y) {
					if (leftPixels[k].x > line[j].x) {
						leftPixels[k].x = line[j].x;
						leftPixels[k].zinv = line[j].zinv;
					}
					if (rightPixels[k].x < line[j].x) {
						rightPixels[k].x = line[j].x;
						rightPixels[k].zinv = line[j].zinv;
					}
					break;
				}
			}
		}
	}
}

void DrawLineSDL(Pixel a, Pixel b, vec3 color)
{
	ivec2 delta;
	delta.x = glm::abs(b.x - a.x);
	delta.y = glm::abs(b.y - a.y);
	int pixels = glm::max(delta.x, delta.y) + 1;

	vector<Pixel> line(pixels);
	Interpolate(a, b, line);
	for (int i = 0; i < pixels; i++)
	{
		int x = line[i].x;
		int y = line[i].y;

		// Ensure the pixel is within the screen bounds
		if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
		{
			// Render color if closer to camera
			if (line[i].zinv > depthBuffer[y][x])
			{
				// Update depthBuffer with the current closest
				depthBuffer[y][x] = line[i].zinv;
				sdlAux->putPixel(x, y, currentColor);

			}
		}
	}
}

void DrawPolygonRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels)
{
	for (int i = 0; i < leftPixels.size(); ++i)
	{
		DrawLineSDL(leftPixels[i], rightPixels[i], currentColor);
		//DrawLineSDL(ivec2(leftPixels[i].x, leftPixels[i].y), ivec2(rightPixels[i].x, rightPixels[i].y), currentColor);

	}
};

void DrawPolygon_new(const vector<vec3>& vertices)
{
	int V = vertices.size();
	vector<Pixel> vertexPixels(V);
	for (int i = 0; i < V; ++i)
	{
		VertexShader(vertices[i], vertexPixels[i]);
	}
	vector<Pixel> leftPixels;
	vector<Pixel> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	DrawPolygonRows(leftPixels, rightPixels);
}