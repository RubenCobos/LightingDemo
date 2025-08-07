#include "Header.h"

// LIGHT SOURCES

LightConstants LightSources =
{
	{XMScalarCos(5 * XM_PIDIV4), XMScalarSin(5 * XM_PIDIV4), 0.0f, 0.0f},
	{-15.0f, 10.0f, 5.0f, 1.0f},
	{5.0f, 40.0f, 65.0f, 1.0f},
	{0.0f, 0.0f, -1.0f, 0.0f},
	{3.0f, 3.0f, 3.0f, 1.0f},
	{0.2f, 0.2f, 0.2f, 1.0f},
	{0.0f, 0.0f, 0.0f, 1.0f}
};

// SPOTLIGHT DIRECTION

XMVECTOR spotAimStart = { 0.0f, 0.0f, -1.0f, 0.0f };

// CALCULATE SURFACE NORMALS

void CalculateSurfaceNormals()
{
	size_t triangles = 0;
	UINT16 i0 = 0;
	UINT16 i1 = 0;
	UINT16 i2 = 0;
	XMVECTOR p0 = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR p1 = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR p2 = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR u = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR v = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR s0 = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR s1 = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR s2 = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR f = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (unsigned int i = 0; i < RenderItemCount; ++i)
	{
		triangles = RenderItems[i].indexCount / 3;

		if (RenderItems[i].instance > 1)
		{
			continue;
		}

		for (size_t t = 0; t < triangles; ++t)
		{
			i0 = IndexArray[RenderItems[i].baseIndex + (t * 3 + 0)];
			i1 = IndexArray[RenderItems[i].baseIndex + (t * 3 + 1)];
			i2 = IndexArray[RenderItems[i].baseIndex + (t * 3 + 2)];
			i0 = RenderItems[i].baseVertex + i0;
			i1 = RenderItems[i].baseVertex + i1;
			i2 = RenderItems[i].baseVertex + i2;
			p0 = XMLoadFloat4(&VertexArray[i0].position);
			p1 = XMLoadFloat4(&VertexArray[i1].position);
			p2 = XMLoadFloat4(&VertexArray[i2].position);
			u = p1 - p0;
			v = p2 - p0;
			f = XMVector3Cross(u, v);
			f = XMVector3Normalize(f);
			s0 = XMLoadFloat4(&VertexArray[i0].normal);
			s1 = XMLoadFloat4(&VertexArray[i1].normal);
			s2 = XMLoadFloat4(&VertexArray[i2].normal);
			s0 = s0 + f;
			s1 = s1 + f;
			s2 = s2 + f;
			XMStoreFloat4(&VertexArray[i0].normal, s0);
			XMStoreFloat4(&VertexArray[i1].normal, s1);
			XMStoreFloat4(&VertexArray[i2].normal, s2);
		}

		i0 = RenderItems[i].baseVertex + RenderItems[i].vertexCount;

		for (UINT16 k = RenderItems[i].baseVertex; k < i0; ++k)
		{
			s0 = XMLoadFloat4(&VertexArray[k].normal);
			s0 = XMVector3Normalize(s0);
			XMStoreFloat4(&VertexArray[k].normal, s0);
		}
	}
}

// DEBUGGER FUNCTION

/* Adds line segments at the of the vertex array and adds the
   line segments corresponding to a particular geometry as a
   seperate render item at the end of the std::vector. Offset
   by RenderItemCount when making a draw call. */

void ShowNormals()
{
	unsigned int baseVertex = (unsigned int)VertexArray.size();

	GeometryObject mesh;
	GeometryObject lines;
	Vertex v;
	Vertex e0;
	Vertex e1;
	XMVECTOR point = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMVECTOR displacement = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (unsigned int i = 0; i < RenderItemCount; ++i)
	{
		mesh = RenderItems[i];

		for (unsigned int c = 0; c < mesh.vertexCount; ++c)
		{
			v = VertexArray[(size_t)(mesh.baseVertex + c)];
			e0 = v;
			e1 = v;
			point = XMLoadFloat4(&v.position);
			displacement = XMLoadFloat4(&v.normal);
			XMStoreFloat4(&e1.position, point + displacement);
			VertexArray.push_back(e0);
			VertexArray.push_back(e1);
		}

		lines.WorldMatrix = mesh.WorldMatrix;
		lines.baseVertex = baseVertex;
		lines.vertexCount = 2 * mesh.vertexCount;
		baseVertex = baseVertex + (2 * mesh.vertexCount);
		RenderItems.push_back(lines);
	}
}

