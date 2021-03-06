#include "Vertex.h"

/*
	 typedef struct D3D11_INPUT_ELEMENT_DESC
	 {
		LPCSTR SemanticName;    // 语义名
		UINT SemanticIndex;     // 语义名对应的索引值
		DXGI_FORMAT Format;     // DXGI数据格式
		UINT InputSlot;         // 输入槽
		UINT AlignedByteOffset; // 对齐的字节偏移量
		D3D11_INPUT_CLASSIFICATION InputSlotClass;  // 输入槽类别(顶点/实例)
		UINT InstanceDataStepRate;  // 实例数据步进值
	 } 	D3D11_INPUT_ELEMENT_DESC;

	InputSlotClass：指定输入的元素是作为顶点元素还是实例元素。枚举值含义如下：
	枚举值							含义
	D3D11_INPUT_PER_VERTEX_DATA		作为顶点元素
	D3D11_INPUT_PER_INSTANCE_DATA	作为实例元素

	InstanceDataStepRate:指定每份实例数据绘制出多少个实例。
	例如，假如你想绘制6个实例，但提供了只够绘制3个实例的数据，
	1份实例数据绘制出1种颜色，分别为红、绿、蓝。那么我们可以设置该成员的值为2，
	使得前两个实例绘制成红色，中间两个实例绘制成绿色，后两个实例绘制成蓝色。
	通常在绘制实例的时候我们会将该成员的值设为1，保证1份数据绘制出1个实例。
	对于顶点成员来说，设置该成员的值为0.
 */

const D3D11_INPUT_ELEMENT_DESC VertexPos::InputLayout[1] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC VertexPosColor::InputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC VertexPosTex::InputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC VertexPosSize::InputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC VertexPosNormalColor::InputLayout[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC VertexPosNormalTex::InputLayout[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC VertexPosNormalTangentTex::InputLayout[4] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
