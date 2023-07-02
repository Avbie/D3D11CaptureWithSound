#pragma once
#include "framework.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using Microsoft::WRL::ComPtr;

namespace D3D
{
	/// <summary>
	/// Structure für ein Array das pro Element aus diesem Struct besteht
	/// </summary>
	struct Vertex
	{
		struct
		{
			float x;
			float y;
		} fPos;
		struct
		{
			float h;
			float v;
		} fTex;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} fcol;
	};
	/// <summary>
	/// Structure die für die init. von VertexBuffer benötigt wird
	/// Normalized Koordinaten/Vertices, Normalized UV-Mapping für Sampler, Farben: 8x4 Bit pro Farbe
	/// </summary>
	struct VertexBufferContent
	{
		UINT uStride = 0;
		UINT uOffset = 0;
		D3D11_BUFFER_DESC VertexBufferDesc = {};
		D3D11_SUBRESOURCE_DATA VertexSubResData = {};
		Vertex aVertices[4] = {
		-1.0f, 1.0f,	0.f, 0.f,	0,0,0,0,
		 1.0f, 1.0f,	1.f, 0.f,	0,0,0,0,
		 1.0f,-1.0f,	1.f, 1.f,	0,0,0,0,
		-1.0f,-1.0f,	0.f, 1.f,	0,0,0,0
		};
		ComPtr<ID3D11Buffer> pVertexBuffer;
	};
	/// <summary>
	/// Structure die für die init. von IndexBuffer benötigt wird
	/// Ermöglicht mit CreateIndexBuffer() indizierter Zugriff auf die VertexBuffer
	/// </summary>
	struct IndexBufferContent
	{
	public:
		IndexBufferContent()
		{
			pIndexBuffer.ReleaseAndGetAddressOf();
			myIndexBufferDesc = {};
			myIndexSubResData = {};
		}
		ComPtr<ID3D11Buffer> pIndexBuffer;
		D3D11_BUFFER_DESC myIndexBufferDesc;
		D3D11_SUBRESOURCE_DATA myIndexSubResData;

		// 2 Dreiecke jeweils im Uhrzeigersinn = Viereck
		// Immer nur eine Richtung, sonst Fehler
		const unsigned short iIndices[6] =
		{
			0,1,2,
			0,2,3
		};
		// overloaded operator. Used as init.: Object = {};
		IndexBufferContent operator = (const D3D::IndexBufferContent& other)
		{
			pIndexBuffer = other.pIndexBuffer;
			myIndexBufferDesc = other.myIndexBufferDesc;
			myIndexSubResData = other.myIndexSubResData;

			return *this;
		}
	};
	/// <summary>
	/// Struktur für eine TransformationsMatrix
	/// Enthält als Array Matrizen (hier nur ein Element)
	/// Kann im VertexShader benutzt werden um pro Frame Daten in GPU zu manipulieren
	/// </summary>
	struct ConstantBuffer
	{
		struct
		{
			float element[4][4];

		} transform;						// registered cbuffer in VertexShader (not about name, about structure and index)
	
	};
	/// <summary>
	/// Struct enthält ein Struct mit MyConstantBuffer der ein Array von Matrizen (hier nur ein Element) beinhaltet
	/// ConstantBuffer können in D3D11 benutzt werden um pro Frame Daten in der GPU zu manipulieren
	/// Manipulation der Vertexdaten mit Hilfe dieser Matrizen über Matrizenberechnungen
	/// Besser als alle Punkte neu zu senden: kostet weniger Speicher/Weniger Zeit
	/// </summary>
	struct ConstantBufferContent
	{
	public:
		ConstantBufferContent()
		{
			fAngle = 0.0f;
			myConstantBuffer =
			{
				{
					std::cos(fAngle), std::sin(fAngle), 0.0f, 0.0f,
					-std::sin(fAngle), std::cos(fAngle), 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				}
			};
			pConstantBuffer.ReleaseAndGetAddressOf();
		}
		float fAngle;
		ConstantBuffer myConstantBuffer;
		ComPtr<ID3D11Buffer> pConstantBuffer;
	};

}