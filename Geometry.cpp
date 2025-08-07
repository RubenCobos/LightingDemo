#include "Header.h"
#include <unordered_map>
#include <functional>

// GEOMETRY STORAGE

vector<Vertex> VertexArray;

vector<UINT16> IndexArray;

vector<GeometryObject> RenderItems;

// GEOMETRY STORAGE POSITIONS

unsigned int baseVertex;
unsigned int VertexCount;
unsigned int baseIndex;
unsigned int IndexCount;
unsigned int RenderItemCount;

// UNIQUE VERTEX TRACKER FOR SUBDIVIDING TRIANGLES

struct VertexHasher
{
	hash<float> hasher;

	size_t operator()(const Vertex& v) const
	{
		float x = v.position.x;
		float y = v.position.y;
		float z = v.position.z;
		float w = v.position.w;

		return hasher(x) ^ hasher(y) ^ hasher(z) ^ hasher(w);
	}
};

struct VertexEquality
{
	bool operator()(const Vertex& v1, const Vertex& v2) const
	{
		return v1.position.x == v2.position.x && v1.position.y == v2.position.y && v1.position.z == v2.position.z && v1.position.w == v2.position.w;
	}
};

unordered_map<Vertex, UINT16, VertexHasher, VertexEquality> unique_vertices;

// SUBDIVIDE TRIANGLES

static void Subdivide(vector<Vertex>& vertices, vector<UINT16>& indeces)
{
	unsigned int triangleCount = (unsigned int)indeces.size() / 3;

	vector<Vertex> subVertices = vertices;

	vector<UINT16> subIndeces = indeces;

	UINT16 I_A = 0; Vertex A;
	UINT16 I_B = 0; Vertex B;
	UINT16 I_C = 0; Vertex C;

	UINT16 I_D = 0; Vertex D;
	UINT16 I_E = 0; Vertex E;
	UINT16 I_F = 0; Vertex F;

	XMVECTOR v0;
	XMVECTOR v1;
	XMVECTOR m;

	UINT16 index = 0;

	vertices.resize(0);

	indeces.resize(0);

	unique_vertices.clear();

	// GENERATE NEW TRIANGLES

	for (unsigned int i = 0; i < triangleCount; ++i)
	{
		// original vertices

		A = subVertices[subIndeces[(size_t)(3 * i + 0)]];
		B = subVertices[subIndeces[(size_t)(3 * i + 1)]];
		C = subVertices[subIndeces[(size_t)(3 * i + 2)]];

		// midpoint D

		v0 = XMLoadFloat4(&A.position);
		v1 = XMLoadFloat4(&B.position);

		m = 0.5f * (v0 + v1);

		m = XMVectorSetW(m, 1.0f);

		XMStoreFloat4(&D.position, m);

		// midpoint E

		v0 = XMLoadFloat4(&B.position);
		v1 = XMLoadFloat4(&C.position);

		m = 0.5f * (v0 + v1);

		m = XMVectorSetW(m, 1.0f);

		XMStoreFloat4(&E.position, m);

		// midpoint f

		v0 = XMLoadFloat4(&A.position);
		v1 = XMLoadFloat4(&C.position);

		m = 0.5f * (v0 + v1);

		m = XMVectorSetW(m, 1.0f);

		XMStoreFloat4(&F.position, m);

		// add vertex A

		if ((I_A = unique_vertices[A]) == 0)
		{
			I_A = index;

			unique_vertices[A] = index + 1;

			vertices.push_back(A); ++index;
		}
		else { I_A = I_A - 1; }

		// add vertex B

		if ((I_B = unique_vertices[B]) == 0)
		{
			I_B = index;

			unique_vertices[B] = index + 1;

			vertices.push_back(B); ++index;
		}
		else { I_B = I_B - 1; }

		// add vertex C

		if ((I_C = unique_vertices[C]) == 0)
		{
			I_C = index;

			unique_vertices[C] = index + 1;

			vertices.push_back(C); ++index;
		}
		else { I_C = I_C - 1; }

		// add vertex D

		if ((I_D = unique_vertices[D]) == 0)
		{
			I_D = index;

			unique_vertices[D] = index + 1;

			vertices.push_back(D); ++index;
		}
		else { I_D = I_D - 1; }

		// add vertex E

		if ((I_E = unique_vertices[E]) == 0)
		{
			I_E = index;

			unique_vertices[E] = index + 1;

			vertices.push_back(E); ++index;
		}
		else { I_E = I_E - 1; }

		// add vertex f

		if ((I_F = unique_vertices[F]) == 0)
		{
			I_F = index;

			unique_vertices[F] = index + 1;

			vertices.push_back(F); ++index;
		}
		else { I_F = I_F - 1; }

		// add triangles

		indeces.push_back(I_A);
		indeces.push_back(I_D);
		indeces.push_back(I_F);

		indeces.push_back(I_D);
		indeces.push_back(I_E);
		indeces.push_back(I_F);

		indeces.push_back(I_F);
		indeces.push_back(I_E);
		indeces.push_back(I_C);

		indeces.push_back(I_D);
		indeces.push_back(I_B);
		indeces.push_back(I_E);
	}
}

// GEOMETRY GENERATING FUNCTIONS

static void GeneratePyramid(float base, float height, unsigned int subdivisions)
{
	baseVertex = (unsigned int)VertexArray.size();

	baseIndex = (unsigned int)IndexArray.size();

	Vertex v;

	XMVECTOR point = { 0.0f, 0.0f, 0.0f, 1.0f };

	vector<Vertex> localVertices;

	vector<UINT16> localIndeces;

	UINT16 A = 0;
	UINT16 B = 0;
	UINT16 C = 0;

	// BASE VERTICES

	// front left

	point = { -(base / 2), -(height / 2), -(base / 2), 1.0f };

	XMStoreFloat4(&v.position, point);

	localVertices.push_back(v);

	// front right

	point = { (base / 2), -(height / 2), -(base / 2), 1.0f };

	XMStoreFloat4(&v.position, point);

	localVertices.push_back(v);

	// back left

	point = { -(base / 2), -(height / 2), (base / 2), 1.0f };

	XMStoreFloat4(&v.position, point);

	localVertices.push_back(v);

	// back right

	point = { (base / 2), -(height / 2), (base / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	localVertices.push_back(v);

	// TOP VERTEX

	// cap

	point = { 0.0f, (height / 2), 0.0f, 1.0f };

	XMStoreFloat4(&v.position, point);

	localVertices.push_back(v);

	// FRONT FACE

	A = 0;
	B = 4;
	C = 1;

	localIndeces.push_back(A);
	localIndeces.push_back(B);
	localIndeces.push_back(C);

	// RIGHT FACE

	A = 1;
	B = 4;
	C = 3;

	localIndeces.push_back(A);
	localIndeces.push_back(B);
	localIndeces.push_back(C);

	// BACK FACE

	A = 3;
	B = 4;
	C = 2;

	localIndeces.push_back(A);
	localIndeces.push_back(B);
	localIndeces.push_back(C);

	// LEFT FACE

	A = 2;
	B = 4;
	C = 0;

	localIndeces.push_back(A);
	localIndeces.push_back(B);
	localIndeces.push_back(C);

	// BOTTOM FACE

	A = 0;
	B = 2;
	C = 3;

	localIndeces.push_back(C);
	localIndeces.push_back(B);
	localIndeces.push_back(A);

	A = 0;
	B = 3;
	C = 1;

	localIndeces.push_back(C);
	localIndeces.push_back(B);
	localIndeces.push_back(A);

	// SUBDIVIDE TRIANGLES

	for (unsigned int i = 0; i < subdivisions; ++i)
	{
		Subdivide(localVertices, localIndeces);
	}

	// ADD TO GLOBAL VERTICES

	for (Vertex& vertex : localVertices)
	{
		VertexArray.push_back(vertex);
	}

	// ADD TO GLOBAL INDECES

	for (UINT16& index : localIndeces)
	{
		IndexArray.push_back(index);
	}

	// VERTEX AND INDEX COUNT

	VertexCount = (unsigned int)localVertices.size();

	IndexCount = (unsigned int) localIndeces.size();
}

static void GenerateCube(float side, unsigned int subdivisions)
{
	baseVertex = (unsigned int)VertexArray.size();

	baseIndex = (unsigned int)IndexArray.size();

	Vertex v;

	XMVECTOR point = { 0.0f, 0.0f, 0.0f, 1.0f };

	vector<Vertex> cubeVertices;

	vector<UINT16> cubeIndeces;

	// BASE VERTICES

	// front left

	point = { -(side / 2), -(side / 2), -(side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// front right

	point = { (side / 2), -(side / 2), -(side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// back left

	point = { -(side / 2), -(side / 2), (side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// back right

	point = { (side / 2), -(side / 2), (side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// TOP VERTICES

	// front left

	point = { -(side / 2), (side / 2), -(side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// front right

	point = { (side / 2), (side / 2), -(side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// back left

	point = { -(side / 2), (side / 2), (side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// back right

	point = { (side / 2), (side / 2), (side / 2), 1.0f }; 

	XMStoreFloat4(&v.position, point);

	cubeVertices.push_back(v);

	// FRONT FACE

	UINT16 A = 0;
	UINT16 B = 0;
	UINT16 C = 0;
	UINT16 D = 0;

	A = 0;
	B = 4;
	C = 5;
	D = 1;

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(B);
	cubeIndeces.push_back(C);

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(C);
	cubeIndeces.push_back(D);

	// LEFT FACE

	A = 2;
	B = 6;
	C = 4;
	D = 0;

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(B);
	cubeIndeces.push_back(C);

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(C);
	cubeIndeces.push_back(D);

	// RIGHT FACE

	A = 1;
	B = 5;
	C = 7;
	D = 3;

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(B);
	cubeIndeces.push_back(C);

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(C);
	cubeIndeces.push_back(D);

	// BACK FACE

	A = 3;
	B = 7;
	C = 6;
	D = 2;

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(B);
	cubeIndeces.push_back(C);

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(C);
	cubeIndeces.push_back(D);

	// BOTTOM FACE

	A = 2;
	B = 0;
	C = 1;
	D = 3;

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(B);
	cubeIndeces.push_back(C);

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(C);
	cubeIndeces.push_back(D);

	// TOP FACE

	A = 4;
	B = 6;
	C = 7;
	D = 5;

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(B);
	cubeIndeces.push_back(C);

	cubeIndeces.push_back(A);
	cubeIndeces.push_back(C);
	cubeIndeces.push_back(D);

	// SUBDIVIDE TRIANGLES

	for (unsigned int i = 0; i < subdivisions; ++i)
	{
		Subdivide(cubeVertices, cubeIndeces);
	}

	// ADD TO GLOBAL VERTICES

	for (Vertex& vertex : cubeVertices)
	{
		VertexArray.push_back(vertex);
	}

	// ADD TO GLOBAL INDECES

	for (UINT16& index : cubeIndeces)
	{
		IndexArray.push_back(index);
	}

	// VERTEX AND INDEX COUNT

	VertexCount = (unsigned int) cubeVertices.size();

	IndexCount = (unsigned int) cubeIndeces.size();
}

static void GenerateGrid(float width, float height, unsigned int m, unsigned int n)
{
	baseVertex = (unsigned int)VertexArray.size();

	baseIndex = (unsigned int)IndexArray.size();

	const float cellWidth = width / (n - 1);

	const float cellHeight = height / (m - 1);

	const float startX = -0.5f * width;

	const float startY = 0.5f * height;

	float x = 0.0f;

	float y = 0.0f;

	XMVECTOR point = { 0.0f, 0.0f, 0.0f, 1.0f };

	Vertex v;

	// GENERATE VERTICES

	for (unsigned int r = 0; r < m; ++r)
	{
		y = startY - r * cellHeight;

		for (unsigned int c = 0; c < n; ++c)
		{
			x = startX + c * cellWidth;

			point = { x, y, 0.0f, 1.0f };

			XMStoreFloat4(&v.position, point);

			VertexArray.push_back(v);
		}
	}

	// GENERATE INDECES

	UINT A = 0;
	UINT B = 0;
	UINT C = 0;
	UINT D = 0;

	for (unsigned int r = 0; r < m - 1; ++r)
	{
		for (unsigned int c = 0; c < n - 1; ++c)
		{
			A = r * n + c;
			B = A + 1;
			C = A + n;
			D = C + 1;

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);

			IndexArray.push_back(C);
			IndexArray.push_back(B);
			IndexArray.push_back(D);
		}
	}

	// VERTEX AND INDEX COUNT

	VertexCount = ((unsigned int)VertexArray.size()) - baseVertex;

	IndexCount = ((unsigned int)IndexArray.size()) - baseIndex;
}

static XMVECTOR GenerateCuboid(unsigned int heightCells, unsigned int widthCells, unsigned int lengthCells, float cellWidth)
{
	baseVertex = (unsigned int) VertexArray.size();

	baseIndex = (unsigned int )IndexArray.size();

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	UINT16 A = 0;
	UINT16 B = 0;
	UINT16 C = 0;
	UINT16 D = 0;

	UINT16 faceEdge = 0;

	UINT16 newVertices = 0;

	// FRONT FACE

	// vertices

	for (unsigned int i = 0; i <= heightCells; ++i)
	{
		y = i * cellWidth;

		for (unsigned int j = 0; j <= widthCells; ++j)
		{
			x = j * cellWidth;

			VertexArray.push_back({{x, y, z, 1.0f}});
		}
	}

	// indeces

	for (UINT16 i = 0; i < heightCells; ++i)
	{
		for (UINT16 j = 0; j < widthCells; ++j)
		{
			A = i * (widthCells + 1) + j;
			B = A + (widthCells + 1);
			C = B + 1;
			D = A + 1;

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);

			IndexArray.push_back(A);
			IndexArray.push_back(C);
			IndexArray.push_back(D);
		}
	}

	// RIGHT FACE

	// vertices

	x = cellWidth * widthCells;

	for (unsigned int i = 0; i <= heightCells; ++i)
	{
		y = i * cellWidth;

		for (unsigned int j = 1; j <= lengthCells; ++j)
		{
			z = j * cellWidth;

			VertexArray.push_back({{x, y, z, 1.0f}});
		}
	}

	// indeces

	faceEdge = widthCells;

	newVertices = (widthCells + 1) * (heightCells + 1);

	for (UINT16 i = 0; i < heightCells; ++i)
	{
		for (UINT16 j = 0; j < lengthCells; ++j)
		{
			if (j == 0) // edge vertices
			{
				A = faceEdge + i * (widthCells + 1);
				B = A + (widthCells + 1);
				D = newVertices + i * lengthCells;
				C = D + lengthCells;
			}
			else // new vertices
			{
				A = newVertices + (i * lengthCells) + (j - 1);
				B = A + lengthCells;
				C = B + 1;
				D = A + 1;
			}

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);

			IndexArray.push_back(A);
			IndexArray.push_back(C);
			IndexArray.push_back(D);
		}
	}

	// BACK FACE

	// vertices

	z = cellWidth * lengthCells;

	for (unsigned int i = 0; i <= heightCells; ++i)
	{
		y = i * cellWidth;

		for (int j = (int)(widthCells - 1); j >= 0; --j)
		{
			x = j * cellWidth;

			VertexArray.push_back({{x, y, z, 1.0f}});
		}
	}

	// indeces
	
	faceEdge = newVertices + (lengthCells - 1);

	newVertices = newVertices + (lengthCells) * (heightCells + 1);

	for (UINT16 i = 0; i < heightCells; ++i)
	{
		for (UINT16 j = 0; j < widthCells; ++j)
		{
			if (j == 0) // edge vertices
			{
				A = faceEdge + i * lengthCells;
				B = A + lengthCells;
				D = newVertices + i * widthCells;
				C = D + widthCells;
			}
			else // new vertices
			{
				A = newVertices + (i * widthCells) + (j - 1);
				B = A + widthCells;
				C = B + 1;
				D = A + 1;
			}

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);

			IndexArray.push_back(A);
			IndexArray.push_back(C);
			IndexArray.push_back(D);
		}
	}

	// LEFT FACE

	// vertices

	x = 0.0f;

	for (unsigned int i = 0; i <= heightCells; ++i)
	{
		y = i * cellWidth;

		for (unsigned int j = lengthCells - 1; j > 0; --j) // edges meet
		{
			z = j * cellWidth;

			VertexArray.push_back({{x, y, z, 1.0f}});
		}
	}

	// indeces

	faceEdge = newVertices + (widthCells - 1);

	newVertices = newVertices + (widthCells) * (heightCells + 1);

	for (UINT16 i = 0; i < heightCells; ++i)
	{
		for (UINT16 j = 0; j < lengthCells; ++j)
		{
			if (j == 0) // start edge
			{
				A = faceEdge + i * widthCells;
				B = A + widthCells;
				D = newVertices + i * (lengthCells - 1);
				C = D + (lengthCells - 1);
			}
			else if (j == lengthCells - 1) // end edge
			{
				A = newVertices + (i * (lengthCells - 1)) + (j - 1);
				B = A + (lengthCells - 1);
				D = i * (widthCells + 1);
				C = D + (widthCells + 1);
			}
			else // new vertices
			{
				A = newVertices + (i * (lengthCells - 1)) + (j - 1);
				B = A + (lengthCells - 1);
				C = B + 1;
				D = A + 1;
			}

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);

			IndexArray.push_back(A);
			IndexArray.push_back(C);
			IndexArray.push_back(D);
		}
	}

	// TOP FACE

	// vertices

	y = cellWidth * heightCells;

	for (unsigned int i = 1; i < lengthCells; ++i) // edges on both sides
	{
		z = i * cellWidth;

		for (unsigned int j = 1; j < widthCells; ++j) // edges on both sides
		{
			x = j * cellWidth;

			VertexArray.push_back({{x, y, z, 1.0f}});
		}
	}

	// indeces

	newVertices = newVertices + (lengthCells - 1) * (heightCells + 1);

	for (UINT16 i = 0; i < lengthCells; ++i)
	{
		for (UINT16 j = 0; j < widthCells; ++j)
		{
			if (j == 0 && i == 0) // top left corner (wrap around)
			{
				A = (widthCells + 1) * heightCells;
				B = newVertices - 1;
				C = newVertices;
				D = A + 1;
			}
			else if (j == 0 && i > 0) // left edge
			{
				A = newVertices - i;

				if (i == (lengthCells - 1)) // back edge
				{
					B = 2 * widthCells * heightCells + 2 * widthCells + heightCells + lengthCells * heightCells + lengthCells;
					C = B - 1;
					D = newVertices + (lengthCells - 2) * (widthCells - 1);
				}
				else
				{
					B = A - 1;
					D = newVertices + (i - 1) * (widthCells - 1);
					C = D + (widthCells - 1);
				}
			}
			else if (j > 0 && i == 0) // front edge
			{
				A = (widthCells + 1) * heightCells + j;
				B = newVertices + (j - 1);

				if (j == widthCells - 1) // right edge
				{
					C = ((widthCells + 1) * (heightCells + 1)) + (lengthCells * heightCells);
					D = A + 1;
				}
				else
				{
					C = B + 1;
					D = A + 1;
				}
			}
			else if (j > 0 && i > 0) // middle
			{
				A = newVertices + ((i - 1) * (widthCells - 1)) + (j - 1);
				B = A + (widthCells - 1);

				if (j == widthCells - 1 && i != (lengthCells - 1)) // right edge (except back corner)
				{
					D = ((widthCells + 1) * (heightCells + 1)) + (lengthCells * heightCells) + (i - 1);
					C = D + 1;
				}
				else if (i == (lengthCells - 1) && j != widthCells - 1) // back edge (except back corner)
				{
					B = 2 * widthCells * heightCells + 2 * widthCells + heightCells + lengthCells * heightCells + lengthCells - j;
					C = B - 1;
					D = A + 1;
				}
				else if (i == (lengthCells - 1) && j == widthCells - 1) // back right corner
				{
					B = 2 * widthCells * heightCells + 2 * widthCells + heightCells + lengthCells * heightCells + lengthCells - j;
					D = ((widthCells + 1) * (heightCells + 1)) + (lengthCells * heightCells) + (i - 1);
					C = D + 1;
				}
				else
				{
					C = B + 1;
					D = A + 1;
				}
			}

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);

			IndexArray.push_back(A);
			IndexArray.push_back(C);
			IndexArray.push_back(D);
		}
	}

	// BOTTOM FACE

	// vertices

	y = 0;

	for (unsigned int i = 1; i < lengthCells; ++i) // edges on both sides
	{
		z = i * cellWidth;

		for (unsigned int j = 1; j < widthCells; ++j) // edges on both sides
		{
			x = j * cellWidth;

			VertexArray.push_back({{x, y, z, 1.0f}});
		}
	}

	// indeces

	newVertices = newVertices + (widthCells - 1) * (lengthCells - 1);

	for (UINT16 i = 0; i < lengthCells; ++i)
	{
		for (UINT16 j = 0; j < widthCells; ++j)
		{
			if (j == 0 && i == 0) // bottom left corner (wrap around)
			{
				A = 0;
				B = 2 * widthCells * heightCells + 2 * widthCells + 2 * lengthCells + lengthCells * heightCells + heightCells - 1;
				C = newVertices;
				D = A + 1;
			}
			else if (j == 0 && i > 0) // left edge
			{
				A = 2 * widthCells * heightCells + 2 * widthCells + 2 * lengthCells + lengthCells * heightCells + heightCells - i;

				if (i == (lengthCells - 1)) // back edge
				{
					B = (widthCells + 1) * (heightCells + 1) + (lengthCells) * (heightCells + 1) + (widthCells - 1);
					C = B - 1;
					D = newVertices + (i - 1) * (widthCells - 1);
				}
				else
				{
					B = A - 1;
					D = newVertices + (i - 1) * (widthCells - 1);
					C = D + (widthCells - 1);
				}
			}
			else if (j > 0 && i == 0) // front edge
			{
				A = j;
				B = newVertices + (j - 1);

				if (j == widthCells - 1) // right edge
				{
					C = (widthCells + 1) * (heightCells + 1);
					D = A + 1;
				}
				else
				{
					C = B + 1;
					D = A + 1;
				}
			}
			else if (j > 0 && i > 0) // middle
			{
				A = newVertices + ((i - 1) * (widthCells - 1)) + (j - 1);
				B = A + (widthCells - 1);

				if (j == widthCells - 1 && i != (lengthCells - 1)) // right edge (except back corner)
				{
					D = (widthCells + 1) * (heightCells + 1) + (i - 1);
					C = D + 1;
				}
				else if (i == (lengthCells - 1) && j != widthCells - 1) // back edge (except back corner)
				{
					B = (widthCells + 1) * (heightCells + 1) + (lengthCells) * (heightCells + 1) + (widthCells - 1) - j;
					C = B - 1;
					D = A + 1;
				}
				else if (i == (lengthCells - 1) && j == widthCells - 1) // back right corner
				{
					B = (widthCells + 1) * (heightCells + 1) + (lengthCells) * (heightCells + 1) + (widthCells - 1) - j;
					D = (widthCells + 1) * (heightCells + 1) + (lengthCells - 2);
					C = D + 1;
				}
				else
				{
					C = B + 1;
					D = A + 1;
				}
			}

			IndexArray.push_back(C);
			IndexArray.push_back(B);
			IndexArray.push_back(A);

			IndexArray.push_back(D);
			IndexArray.push_back(C);
			IndexArray.push_back(A);
		}
	}

	// VERTEX AND INDEX COUNT

	VertexCount = ((unsigned int)VertexArray.size()) - baseVertex;

	IndexCount = ((unsigned int)IndexArray.size()) - baseIndex;

	// RETURN CENTROID

	x = (widthCells * cellWidth) / 2;
	y = (heightCells * cellWidth) / 2;
	z = (lengthCells * cellWidth) / 2;

	return { x, y, z, 1.0f };
}

static void GenerateCylinder(float baseRadius, float capRadius, float height, unsigned int stacks, unsigned int slices)
{
	baseVertex = (unsigned int)VertexArray.size();

	baseIndex = (unsigned int)IndexArray.size();

	unsigned int oldVertexSize = (unsigned int)VertexArray.size();

	unsigned int oldIndexSize = (unsigned int)IndexArray.size();

	const float baseHeight = -(height / 2);
	const float deltaHeight = height / stacks;
	const float deltaRadius = (capRadius - baseRadius) / stacks;
	const float sliceAngle = XM_2PI / slices;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	XMVECTOR point = XMVectorZero();

	XMFLOAT4 position = { 0.0f, 0.0f, 0.0f, 1.0f };

	Vertex vertex = { position };

	// GENERATE RING VERTICES

	for (unsigned int ring = 0; ring <= stacks; ++ring)
	{
		for (unsigned int slice = 0; slice < slices; ++slice)
		{
			x = (baseRadius + ring * deltaRadius) * cosf(slice * sliceAngle);
			y = baseHeight + ring * deltaHeight;
			z = (baseRadius + ring * deltaRadius) * sinf(slice * sliceAngle);

			point = { x, y, z, 1.0f };

			XMStoreFloat4(&vertex.position, point);

			VertexArray.push_back(vertex);
		}
	}

	// GENERATE BASE CENTER VERTEX

	point = { 0.0f, baseHeight, 0.0f, 1.0f };

	XMStoreFloat4(&vertex.position, point);

	VertexArray.push_back(vertex);

	// GENERATE CAP CENTER VERTEX

	point = { 0.0f, baseHeight + stacks * deltaHeight, 0.0f, 1.0f };

	XMStoreFloat4(&vertex.position, point);

	VertexArray.push_back(vertex);

	// GENERATE SIDE TRIANGLES

	UINT16 A = 0;
	UINT16 B = 0;
	UINT16 C = 0;
	UINT16 D = 0;

	UINT16 A0 = 0;
	UINT16 B0 = 0;
	UINT16 C0 = 0;
	UINT16 D0 = 0;

	for (unsigned int ring = 0; ring < stacks; ++ring)
	{
		for (unsigned int slice = 0; slice < slices; ++slice)
		{
			A = slices * ring + slice;
			B = slices * (ring + 1) + slice;
			C = slices * (ring + 1) + slice + 1;
			D = slices * ring + slice + 1;

			if (slice == 0)
			{
				C0 = B;
				D0 = A;
			}

			if (slice == slices - 1)
			{
				C = C0;
				D = D0;
			}

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);
			IndexArray.push_back(A);
			IndexArray.push_back(C);
			IndexArray.push_back(D);
		}
	}

	// GENERATE BASE TRIANGLES

	A = ((UINT16)VertexArray.size()) - oldVertexSize - 2;

	for (int baseSlice = (slices - 1); baseSlice >= 0; --baseSlice)
	{
		B = baseSlice;
		C = baseSlice - 1;

		if (baseSlice == (slices - 1))
		{
			C0 = B;
		}

		if (baseSlice == 0)
		{
			C = C0;
		}

		IndexArray.push_back(C);
		IndexArray.push_back(B);
		IndexArray.push_back(A);
	}

	// GENERATE CAP TRIANGLES

	A = ((UINT16)VertexArray.size()) - oldVertexSize - 1;

	for (unsigned int capSlice = stacks * slices + (slices - 1); capSlice >= stacks * slices; --capSlice)
	{
		B = capSlice;
		C = capSlice - 1;

		if (capSlice == stacks * slices + (slices - 1))
		{
			C0 = B;
		}

		if (capSlice == stacks * slices)
		{
			C = C0;
		}

		IndexArray.push_back(A);
		IndexArray.push_back(B);
		IndexArray.push_back(C);
	}

	// VERTEX AND INDEX COUNT

	VertexCount = ((unsigned int)VertexArray.size()) - baseVertex;

	IndexCount = (unsigned int)IndexArray.size() - oldIndexSize;
}

static void GenerateSphere(float radius, unsigned int stacks, unsigned int slices)
{
	baseVertex = (unsigned int) VertexArray.size();
	baseIndex = (unsigned int) IndexArray.size();

	const float deltaHeight = (2 * radius) / stacks;
	const float sliceAngle = XM_2PI / slices;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	XMVECTOR point = XMVectorZero();
	XMFLOAT4 position = { 0.0f, 0.0f, 0.0f, 0.0f };

	Vertex vertex = { position };

	// GENERATE RING VERTICES

	for (unsigned int ring = 1; ring < stacks; ++ring)
	{
		y = -radius + ring * deltaHeight;

		float ringRadius = (float)sqrt(pow(radius, 2) - pow(y, 2));

		for (unsigned int slice = 0; slice < slices; ++slice)
		{
			x = ringRadius * cosf(slice * sliceAngle);
			z = ringRadius * sinf(slice * sliceAngle);

			point = { x, y, z, 1.0f };

			XMStoreFloat4(&vertex.position, point);

			VertexArray.push_back(vertex);
		}
	}

	// GENERATE TOP CAP VERTICES

	for (unsigned int ring = 0; ring < 2; ++ring)
	{
		y = y + deltaHeight / 2.25f;

		float ringRadius = (float)sqrt(pow(radius, 2) - pow(y, 2));

		for (unsigned int slice = 0; slice < slices; ++slice)
		{
			x = ringRadius * cosf(slice * sliceAngle);
			z = ringRadius * sinf(slice * sliceAngle);

			point = { x, y, z, 1.0f };

			XMStoreFloat4(&vertex.position, point);

			VertexArray.push_back(vertex);
		}
	}

	// NORTH POLE VERTEX

	point = { 0.0f, radius, 0.0f, 1.0f };

	XMStoreFloat4(&vertex.position, point);

	VertexArray.push_back(vertex);

	// SOUTH POLE VERTEX AND INDEX

	point = { 0.0f, -radius, 0.0f, 1.0f };

	XMStoreFloat4(&vertex.position, point);

	VertexArray.push_back(vertex);

	UINT16 BottomPoleIndex = (UINT16) (VertexArray.size() - 1) - baseVertex;

	// GENERATE BOTTOM CAP VERTICES

	for (unsigned int ring = 2; ring > 0; --ring)
	{
		y = (-radius + deltaHeight) - ring * (deltaHeight / 2.25f);

		float ringRadius = (float)sqrt(pow(radius, 2) - pow(y, 2));

		for (unsigned int slice = 0; slice < slices; ++slice)
		{
			x = ringRadius * cosf(slice * sliceAngle);
			z = ringRadius * sinf(slice * sliceAngle);

			point = { x, y, z, 1.0f };

			XMStoreFloat4(&vertex.position, point);

			VertexArray.push_back(vertex);
		}
	}

	// MIDDLE TRIANGLES

	UINT16 A = 0;
	UINT16 B = 0;
	UINT16 C = 0;
	UINT16 D = 0;

	for (unsigned int ring = 0; ring < stacks; ++ring)
	{
		for (unsigned int slice = 0; slice < slices; ++slice)
		{

			if (slice == slices - 1)
			{
				A = (ring * slices) + (slices - 1);
				B = A + slices;
				C = B - (slices - 1);
				D = A - (slices - 1);
			}
			else
			{
				A = (ring * slices) + slice;
				B = A + slices;
				C = B + 1;
				D = A + 1;
			}

			IndexArray.push_back(A);
			IndexArray.push_back(B);
			IndexArray.push_back(C);
			IndexArray.push_back(A);
			IndexArray.push_back(C);
			IndexArray.push_back(D);
		}
	}

	// TOP CAP TRIANGLES

	for (unsigned int slice = 0; slice < slices; ++slice)
	{
		if (slice == slices - 1)
		{
			A = stacks * slices + (slices - 1);
			B = stacks * slices + slices;
			C = stacks * slices;
		}
		else
		{
			A = stacks * slices + slice;
			B = stacks * slices + slices;
			C = A + 1;
		}

		IndexArray.push_back(A);
		IndexArray.push_back(B);
		IndexArray.push_back(C);
	}

	// SOUTH POLE CAP TRIANGLES

	for (unsigned int slice = 0; slice < slices; ++slice)
	{
		if (slice == slices - 1)
		{
			A = BottomPoleIndex;
			B = (BottomPoleIndex + 1) + slice;
			C = BottomPoleIndex + 1;
		}
		else
		{
			A = BottomPoleIndex;
			B = (BottomPoleIndex + 1) + slice;
			C = B + 1;
		}

		IndexArray.push_back(A);
		IndexArray.push_back(B);
		IndexArray.push_back(C);
	}

	// OUTER BOTTOM CAP STACK 1 TRIANGLES

	for (unsigned int slice = 0; slice < slices; ++slice)
	{
		if (slice == slices - 1)
		{
			A = (BottomPoleIndex + 1) + slice;
			B = A + slices;
			C = B - (slices - 1);
			D = A - (slices - 1);
		}
		else
		{
			A = (BottomPoleIndex + 1) + slice;
			B = A + slices;
			C = B + 1;
			D = A + 1;
		}

		IndexArray.push_back(A);
		IndexArray.push_back(B);
		IndexArray.push_back(C);
		IndexArray.push_back(A);
		IndexArray.push_back(C);
		IndexArray.push_back(D);
	}

	// OUTER BOTTOM CAP STACK 2 TRIANGLES

	for (unsigned int slice = 0; slice < slices; ++slice)
	{
		if (slice == slices - 1)
		{
			A = (BottomPoleIndex + 1) + slices + slice;
			B = slice;
			C = B - (slices - 1);
			D = A - (slices - 1);
		}
		else
		{
			A = (BottomPoleIndex + 1) + slices + slice;
			B = slice;
			C = B + 1;
			D = A + 1;
		}

		IndexArray.push_back(A);
		IndexArray.push_back(B);
		IndexArray.push_back(C);
		IndexArray.push_back(A);
		IndexArray.push_back(C);
		IndexArray.push_back(D);
	}

	// VERTEX AND INDEX COUNT

	VertexCount = ((unsigned int)VertexArray.size()) - baseVertex;

	IndexCount = ((unsigned int)IndexArray.size()) - baseIndex;
}

void BuildWorldGeometry()
{
	GeometryObject mesh;

	XMVECTOR center = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR displacement = { 0.0f, 0.0f, 0.0f, 0.0f };

	// CYLINDER 1

	GenerateCylinder(1.0f, 1.0f, 5.0f, 15, 20);

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(-3.0f, 0.0f, 10.0f));

	mesh.baseVertex = baseVertex;
	mesh.vertexCount = VertexCount;
	mesh.baseIndex = baseIndex;
	mesh.indexCount = IndexCount;
	mesh.instance = 1;

	// garnet

	mesh.material.DiffuseAlbedo = { 0.225f, 0.075f, 0.075f, 1.0f };
	mesh.material.SpecProperty = { 0.0536f, 0.0536f, 0.0536f, 1.0f };
	mesh.material.roughness = 10;

	RenderItems.push_back(mesh);

	// CYLINDER 2
	
	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(3.0f, 0.0f, 10.0f));

	mesh.instance = 2; // garnet

	RenderItems.push_back(mesh);

	// PYRAMID 1

	GeneratePyramid(2.0f, 2.0f, 3);

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(0.0f, 7.5f, 20.0f));

	mesh.baseVertex = baseVertex;
	mesh.vertexCount = VertexCount;
	mesh.baseIndex = baseIndex;
	mesh.indexCount = IndexCount;
	mesh.instance = 1;

	// jade

	mesh.material.DiffuseAlbedo = { 0.075f, 0.225f, 0.075f, 1.0f };
	mesh.material.SpecProperty = { 0.043f, 0.043f, 0.043f, 1.0f };
	mesh.material.roughness = 6;

	RenderItems.push_back(mesh);

	// PYRAMID 2 

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixRotationX(XM_PI) * XMMatrixTranslation(0.0f, 2.5f, 20.0f));

	mesh.instance = 2; // jade

	RenderItems.push_back(mesh);

	// GRID 1

	GenerateGrid(10.0f, 5.0f, 10, 17);

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixRotationY(-XM_PIDIV2) * XMMatrixTranslation(-15.0f, 10.0f, 30.0f));

	mesh.baseVertex = baseVertex;
	mesh.vertexCount = VertexCount;
	mesh.baseIndex = baseIndex;
	mesh.indexCount = IndexCount;
	mesh.instance = 1;

	// obsidian

	mesh.material.DiffuseAlbedo = { 0.045f, 0.045f, 0.045f, 1.0f };
	mesh.material.SpecProperty = { 0.04f, 0.04f, 0.04f, 1.0f };
	mesh.material.roughness = 16;

	RenderItems.push_back(mesh);

	// GRID 2

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixRotationY(XM_PIDIV2) * XMMatrixTranslation(15.0f, 10.0f, 30.0f));

	mesh.instance = 2; // obsidian

	RenderItems.push_back(mesh);

	// SPHERE 1

	GenerateSphere(2.0f, 15, 25);

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(-15.0f, 0.0f, 5.0f));

	mesh.baseVertex = baseVertex;
	mesh.vertexCount = VertexCount;
	mesh.baseIndex = baseIndex;
	mesh.indexCount = IndexCount;
	mesh.instance = 1;

	// lapis lazuli

	mesh.material.DiffuseAlbedo = { 0.03f, 0.045f, 0.15f, 1.0f };
	mesh.material.SpecProperty = { 0.028f, 0.028f, 0.028f, 1.0f };
	mesh.material.roughness = 10;

	RenderItems.push_back(mesh);

	// SPHERE 2

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(15.0f, 0.0f, 5.0f));

	mesh.instance = 2; // lapis lazuli

	RenderItems.push_back(mesh);

	// SPHERE 3

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixTranslation(-15.0f, 0.0f, -10.0f));

	mesh.instance = 3; // lapis lazuli

	RenderItems.push_back(mesh);

	// SPHERE 4

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity()* XMMatrixTranslation(15.0f, 0.0f, -10.0f));

	mesh.instance = 4; // lapis lazuli

	RenderItems.push_back(mesh);

	// SPHERE 5

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity()* XMMatrixTranslation(0.0f, 0.0f, -20.0f));

	mesh.instance = 5; // lapis lazuli

	RenderItems.push_back(mesh);

	// CUBE 1

	GenerateCube(10.0f, 5);

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity() * XMMatrixRotationY(XM_PIDIV4) * XMMatrixRotationX(XM_PIDIV4) * XMMatrixTranslation(0.0f, -15.0f, 20.0f));

	mesh.baseVertex = baseVertex;
	mesh.vertexCount = VertexCount;
	mesh.baseIndex = baseIndex;
	mesh.indexCount = IndexCount;
	mesh.instance = 1;

	// sugilite

	mesh.material.DiffuseAlbedo = { 0.075f, 0.03f, 0.15f, 1.0f };
	mesh.material.SpecProperty = { 0.04f, 0.04f, 0.04f, 1.0f };
	mesh.material.roughness = 10;

	RenderItems.push_back(mesh);

	// CUBOID 1

	GenerateCuboid(50, 10, 10, 1.0f);

	XMStoreFloat4x4(&mesh.WorldMatrix, XMMatrixIdentity()* XMMatrixTranslation(0.0f, 0.0f, 50.0f));

	mesh.baseVertex = baseVertex;
	mesh.vertexCount = VertexCount;
	mesh.baseIndex = baseIndex;
	mesh.indexCount = IndexCount;
	mesh.instance = 1;

	// turquoise

	mesh.material.DiffuseAlbedo = { 0.075f, 0.15f, 0.15f, 1.0f };
	mesh.material.SpecProperty = { 0.036, 0.036, 0.036, 1.0f };
	mesh.material.roughness = 10;

	RenderItems.push_back(mesh);

	// TOTAL GEOMETRIC OBJECTS

	RenderItemCount = (unsigned int) RenderItems.size();
}
