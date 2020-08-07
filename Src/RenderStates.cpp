#include "RenderStates.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace Microsoft::WRL;

ComPtr<ID3D11RasterizerState> RenderStates::RSNoCull = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSWireframe = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSCullClockWise = nullptr;
ComPtr<ID3D11RasterizerState> RenderStates::RSDepth = nullptr;

ComPtr<ID3D11SamplerState> RenderStates::SSPointClamp = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSAnistropicWrap = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSLinearWrap = nullptr;
ComPtr<ID3D11SamplerState> RenderStates::SSShadow = nullptr;

ComPtr<ID3D11BlendState> RenderStates::BSAlphaToCoverage = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSNoColorWrite = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSTransparent = nullptr;
ComPtr<ID3D11BlendState> RenderStates::BSAdditive = nullptr;

ComPtr<ID3D11DepthStencilState> RenderStates::DSSLessEqual = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSWriteStencil = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSDrawWithStencil = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDoubleBlend = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthTest = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthWrite = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthTestWithStencil = nullptr;
ComPtr<ID3D11DepthStencilState> RenderStates::DSSNoDepthWriteWithStencil = nullptr;

bool RenderStates::IsInit()
{
	// 一般来说初始化操作会把所有的状态都创建出来
	return RSWireframe != nullptr;
}

void RenderStates::InitAll(ID3D11Device * device)
{
	// 先前初始化过的话就没必要重来了
	if (IsInit())
		return;
	// ******************
	// 初始化光栅化器状态
	//
	/*
		typedef struct D3D11_RASTERIZER_DESC
		{
			D3D11_FILL_MODE FillMode;          // 填充模式
			D3D11_CULL_MODE CullMode;          // 裁剪模式
			BOOL FrontCounterClockwise;        // 是否三角形顶点按逆时针排布时为正面
			
			下面三个参数名为斜率缩放偏移的光栅化状态属性
			INT DepthBias;                     // 一个固定的应用偏移量
			FLOAT DepthBiasClamp;              // 所允许的最大深度偏移量。以此来设置深度偏移量的上限。不难想象，极其陡峭的倾斜度会导致斜率缩放偏移量过大，从而造成peter-panning失真
			FLOAT SlopeScaledDepthBias;        // 根据多边形的斜率来控制偏移程度的缩放因子
			
			BOOL DepthClipEnable;              // 是否允许深度测试将范围外的像素进行裁剪，默认TRUE
			BOOL ScissorEnable;                // 是否允许指定矩形范围的裁剪，若TRUE，则需要在RSSetScissor设置像素保留的矩形区域
			BOOL MultisampleEnable;            // 是否允许多重采样
			BOOL AntialiasedLineEnable;        // 是否允许反走样线，仅当多重采样为FALSE时才有效
		} 	D3D11_RASTERIZER_DESC;

		枚举值								含义
		D3D11_FILL_WIREFRAME = 2			线框填充方式
		D3D11_FILL_SOLID = 3				面填充方式

		D3D11_CULL_NONE = 1					无背面裁剪，即三角形无论处在视野的正面还是背面都能看到
		D3D11_CULL_FRONT = 2				对处在视野正面的三角形进行裁剪
		D3D11_CULL_BACK = 3					对处在视野背面的三角形进行裁剪

		默认光栅化状态如下：
			FillMode = D3D11_FILL_SOLID;
			CullMode = D3D11_CULL_BACK;
			FrontCounterClockwise = FALSE;
			DepthBias = 0;
			SlopeScaledDepthBias = 0.0f;
			DepthBiasClamp = 0.0f;
			DepthClipEnable	= TRUE;
			ScissorEnable = FALSE;
			MultisampleEnable = FALSE;
			AntialiasedLineEnable = FALSE;
	 */
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));

	// 线框模式
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	/*
		HRESULT ID3D11Device::CreateRasterizerState(
			const D3D11_RASTERIZER_DESC *pRasterizerDesc,    // [In]光栅化状态描述
			ID3D11RasterizerState **ppRasterizerState) = 0;  // [Out]输出光栅化状态
	 */
	HR(device->CreateRasterizerState(&rasterizerDesc, RSWireframe.GetAddressOf()));

	// 无背面剔除模式
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	HR(device->CreateRasterizerState(&rasterizerDesc, RSNoCull.GetAddressOf()));

	// 顺时针剔除模式
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	HR(device->CreateRasterizerState(&rasterizerDesc, RSCullClockWise.GetAddressOf()));

	// 深度偏移模式
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	/*
		[出自MSDN]
		如果当前的深度缓冲区采用UNORM格式并且绑定在输出合并阶段，或深度缓冲区还没有被绑定
		则偏移量的计算过程如下：
		
		Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
		
		这里的r是在深度缓冲区格式转换为float32类型后，其深度值可取到大于0的最小可表示的值
		MaxDepthSlope则是像素在水平方向和竖直方向上的深度斜率的最大值
		[结束MSDN引用]
		
		对于一个24位的深度缓冲区来说， r = 1 / 2^24
		
		例如：DepthBias = 100000 ==> 实际的DepthBias = 100000/2^24 = .006
		
		本Demo中的方向光始终与地面法线呈45度夹角，故取斜率为1.0f
		以下数据极其依赖于实际场景，因此我们需要对特定场景反复尝试才能找到最合适

		注意：深度偏移发生在光栅化期间（裁剪之后），因此不会对几何体裁剪造成影响
	 */
	rasterizerDesc.DepthBias = 100000;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 1.0f;
	HR(device->CreateRasterizerState(&rasterizerDesc, RSDepth.GetAddressOf()));

	// ******************
	// 初始化采样器状态
	/*
		typedef struct D3D11_SAMPLER_DESC
		{
			D3D11_FILTER Filter;                    // 所选过滤器
			D3D11_TEXTURE_ADDRESS_MODE AddressU;    // U方向寻址模式
			D3D11_TEXTURE_ADDRESS_MODE AddressV;    // V方向寻址模式
			D3D11_TEXTURE_ADDRESS_MODE AddressW;    // W方向寻址模式
			FLOAT MipLODBias;   // mipmap等级偏移值，最终算出的mipmap等级会加上该偏移值
			UINT MaxAnisotropy;                     // 最大各向异性等级(1-16)
			D3D11_COMPARISON_FUNC ComparisonFunc;   // 这节不讨论
			FLOAT BorderColor[ 4 ];     // 边界外的颜色，使用D3D11_TEXTURE_BORDER_COLOR时需要指定
			FLOAT MinLOD;   // 若mipmap等级低于MinLOD，则使用等级MinLOD。最小允许设为0
			FLOAT MaxLOD;   // 若mipmap等级高于MaxLOD，则使用等级MaxLOD。必须比MinLOD大
		} 	D3D11_SAMPLER_DESC;

		D3D11_FILTER部分枚举含义如下：
		枚举值											缩小		放大		mipmap
		D3D11_FILTER_MIN_MAG_MIP_POINT					点采样		点采样		点采样
		D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR			点采样		点采样		线性采样
		D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT		点采样		线性采样	点采样
		D3D11_FILTER_MIN_MAG_MIP_LINEAR					线性采样	线性采样	线性采样
		D3D11_FILTER_ANISOTROPIC						各向异性	各向异性	各向异性

		D3D11_TEXTURE_ADDRESS_MODE是单个方向的寻址模式，
		有时候纹理坐标会超过1.0或者小于0.0，这时候寻址模式可以解释边界外的情况，
		含义如下：
			D3D11_TEXTURE_ADDRESS_WRAP是将指定纹理坐标分量的值[t, t + 1], t ∈ Z映射到[0.0, 1.0]，
			因此作用到u和v分量时看起来就像是把用一张贴图紧密平铺到其他位置上
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_WRAP.png

			D3D11_TEXTURE_ADDRESS_MIRROR在每个整数点处翻转纹理坐标值。例如u在[0.0, 1.0]按正常纹理坐标寻址，
			在[1.0, 2.0]内则翻转，在[2.0, 3.0]内又回到正常的寻址，以此类推
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_MIRROR.png

			D3D11_TEXTURE_ADDRESS_CLAMP对指定纹理坐标分量，小于0.0的值都取作0.0，
			大于1.0的值都取作1.0，在[0.0, 1.0]的纹理坐标不变
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_CLAMP.png

			D3D11_TEXTURE_ADDRESS_BORDER对于指定纹理坐标分量的值在[0.0, 1.0]外的区域都使用BorderColor进行填充
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_BORDER.png

			D3D11_TEXTURE_ADDRESS_MIRROR_ONCE相当于MIRROR和CLAMP的结合，仅[-1.0,1.0]的范围内镜像有效，
			若小于-1.0则取-1.0，大于1.0则取1.0，在[-1.0, 0.0]进行翻转
			https://github.com/Life4gal/FromZero2D3D/blob/master/PIC/D3D11_TEXTURE_ADDRESS_MIRROR_ONCE.png
	*/
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));

	// 点过滤与Clamp模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(device->CreateSamplerState(&sampDesc, SSPointClamp.GetAddressOf()));
	
	// 线性过滤与Wrap模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	/*
		HRESULT ID3D11Device::CreateSamplerState(
			const D3D11_SAMPLER_DESC *pSamplerDesc, // [In]采样器状态描述
			ID3D11SamplerState **ppSamplerState);   // [Out]输出的采样器
	 */
	HR(device->CreateSamplerState(&sampDesc, SSLinearWrap.GetAddressOf()));

	// 各向异性过滤与Wrap模式
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MaxAnisotropy = 4;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(device->CreateSamplerState(&sampDesc, SSAnistropicWrap.GetAddressOf()));

	// 深度比较与Border模式
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	// 函数中传入的depth将会出现在比较运算符的左边，即 depth <= sampleDepth
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampDesc.BorderColor[0] = { 1.0f };
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(device->CreateSamplerState(&sampDesc, SSShadow.GetAddressOf()));

	// ******************
	// 初始化混合状态
	//
	/*
		typedef struct D3D11_BLEND_DESC
		{
			BOOL AlphaToCoverageEnable;    // 默认关闭，这里
			BOOL IndependentBlendEnable;   // 是否每个渲染目标都有独立的混合混合描述，关闭的话都使用索引为0的描述信息
			D3D11_RENDER_TARGET_BLEND_DESC RenderTarget[ 8 ];
		} 	D3D11_BLEND_DESC;

		typedef struct D3D11_RENDER_TARGET_BLEND_DESC
		{
			BOOL BlendEnable;             // 是否开启混合
			D3D11_BLEND SrcBlend;         // 源颜色混合因子
			D3D11_BLEND DestBlend;        // 目标颜色混合因子
			D3D11_BLEND_OP BlendOp;       // 颜色混合运算符
			D3D11_BLEND SrcBlendAlpha;    // 源Alpha混合因子
			D3D11_BLEND DestBlendAlpha;   // 目标Alpha混合因子
			D3D11_BLEND_OP BlendOpAlpha;  // Alpha混合运算符
			UINT8 RenderTargetWriteMask;  // D3D11_COLOR_WRITE_ENABLE枚举类型来指定可以写入的颜色
		} 	D3D11_RENDER_TARGET_BLEND_DESC;

		枚举值								含义
		D3D11_COLOR_WRITE_ENABLE_RED = 1	可以写入红色
		D3D11_COLOR_WRITE_ENABLE_GREEN = 2	可以写入绿色
		D3D11_COLOR_WRITE_ENABLE_BLUE = 4	可以写入蓝色
		D3D11_COLOR_WRITE_ENABLE_ALPHA = 8	可以写入ALPHA通道
		D3D11_COLOR_WRITE_ENABLE_ALL = 15	可以写入所有颜色

		HRESULT ID3D11Device::CreateBlendState(
			const D3D11_BLEND_DESC *pBlendStateDesc,    // [In]混合状态描述
			ID3D11BlendState **ppBlendState);           // [Out]输出混合状态
	 */
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	auto& rtDesc = blendDesc.RenderTarget[0];
	
	// Alpha-To-Coverage模式
	blendDesc.AlphaToCoverageEnable = true;
	blendDesc.IndependentBlendEnable = false;
	rtDesc.BlendEnable = false;
	rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HR(device->CreateBlendState(&blendDesc, BSAlphaToCoverage.GetAddressOf()));

	// 透明混合模式
	// Color = SrcAlpha * SrcColor + (1 - SrcAlpha) * DestColor 
	// Alpha = SrcAlpha
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	rtDesc.BlendEnable = true;
	rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	HR(device->CreateBlendState(&blendDesc, BSTransparent.GetAddressOf()));

	// 加法混合模式
	// Color = SrcColor + DestColor
	// Alpha = SrcAlpha
	rtDesc.SrcBlend = D3D11_BLEND_ONE;
	rtDesc.DestBlend = D3D11_BLEND_ONE;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	HR(device->CreateBlendState(&blendDesc, BSAdditive.GetAddressOf()));

	
	// 无颜色写入混合模式
	// Color = DestColor
	// Alpha = DestAlpha
	rtDesc.BlendEnable = false;
	rtDesc.SrcBlend = D3D11_BLEND_ZERO;
	rtDesc.DestBlend = D3D11_BLEND_ONE;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtDesc.RenderTargetWriteMask = 0;
	HR(device->CreateBlendState(&blendDesc, BSNoColorWrite.GetAddressOf()));
	
	// ******************
	// 初始化深度/模板状态
	//
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	// 允许使用深度值一致的像素进行替换的深度/模板状态
	// 该状态用于绘制天空盒，因为深度值为1.0时默认无法通过深度测试
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = false;
	HR(device->CreateDepthStencilState(&dsDesc, DSSLessEqual.GetAddressOf()));

	// 镜面标记深度/模板状态
	// 这里不写入深度信息
	// 无论是正面还是背面，原来指定的区域的模板值都会被写入StencilRef
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// 对于背面的几何体我们是不进行渲染的，所以这里的设置无关紧要
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	HR(device->CreateDepthStencilState(&dsDesc, DSSWriteStencil.GetAddressOf()));

	// 反射绘制深度/模板状态
	// 由于要绘制反射镜面，需要更新深度
	// 仅当镜面标记模板值和当前设置模板值相等时才会进行绘制
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	// 对于背面的几何体我们是不进行渲染的，所以这里的设置无关紧要
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	HR(device->CreateDepthStencilState(&dsDesc, DSSDrawWithStencil.GetAddressOf()));

	// 无二次混合深度/模板状态
	// 允许默认深度测试
	// 通过自递增使得原来StencilRef的值只能使用一次，实现仅一次混合
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	// 对于背面的几何体我们是不进行渲染的，所以这里的设置无关紧要
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	HR(device->CreateDepthStencilState(&dsDesc, DSSNoDoubleBlend.GetAddressOf()));

	// 关闭深度测试的深度/模板状态
	// 若绘制非透明物体，务必严格按照绘制顺序
	// 绘制透明物体则不需要担心绘制顺序
	// 而默认情况下模板测试就是关闭的
	dsDesc.DepthEnable = false;
	dsDesc.StencilEnable = false;
	HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthTest.GetAddressOf()));

	// 关闭深度测试
	// 若绘制非透明物体，务必严格按照绘制顺序
	// 绘制透明物体则不需要担心绘制顺序
	// 对满足模板值条件的区域才进行绘制
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	// 对于背面的几何体我们是不进行渲染的，所以这里的设置无关紧要
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthTestWithStencil.GetAddressOf()));

	// 进行深度测试，但不写入深度值的状态
	// 若绘制非透明物体时，应使用默认状态
	// 绘制透明物体时，使用该状态可以有效确保混合状态的进行
	// 并且确保较前的非透明物体可以阻挡较后的一切物体
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = false;
	HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthWrite.GetAddressOf()));

	// 进行深度测试，但不写入深度值的状态
	// 若绘制非透明物体时，应使用默认状态
	// 绘制透明物体时，使用该状态可以有效确保混合状态的进行
	// 并且确保较前的非透明物体可以阻挡较后的一切物体
	// 对满足模板值条件的区域才进行绘制
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	// 对于背面的几何体我们是不进行渲染的，所以这里的设置无关紧要
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthWriteWithStencil.GetAddressOf()));

	// ******************
	// 设置调试对象名
	//
	D3D11SetDebugObjectName(RSCullClockWise.Get(), "RSCullClockWise");
	D3D11SetDebugObjectName(RSNoCull.Get(), "RSNoCull");
	D3D11SetDebugObjectName(RSWireframe.Get(), "RSWireframe");
	D3D11SetDebugObjectName(RSDepth.Get(), "RSDepth");

	D3D11SetDebugObjectName(SSAnistropicWrap.Get(), "SSAnistropicWrap");
	D3D11SetDebugObjectName(SSLinearWrap.Get(), "SSLinearWrap");
	D3D11SetDebugObjectName(SSShadow.Get(), "SSShadow");

	D3D11SetDebugObjectName(BSAlphaToCoverage.Get(), "BSAlphaToCoverage");
	D3D11SetDebugObjectName(BSNoColorWrite.Get(), "BSNoColorWrite");
	D3D11SetDebugObjectName(BSTransparent.Get(), "BSTransparent");
	D3D11SetDebugObjectName(BSAdditive.Get(), "BSAdditive");

	D3D11SetDebugObjectName(DSSLessEqual.Get(), "DSSLessEqual");
	D3D11SetDebugObjectName(DSSWriteStencil.Get(), "DSSWriteStencil");
	D3D11SetDebugObjectName(DSSDrawWithStencil.Get(), "DSSDrawWithStencil");
	D3D11SetDebugObjectName(DSSNoDoubleBlend.Get(), "DSSNoDoubleBlend");
	D3D11SetDebugObjectName(DSSNoDepthTest.Get(), "DSSNoDepthTest");
	D3D11SetDebugObjectName(DSSNoDepthWrite.Get(), "DSSNoDepthWrite");
	D3D11SetDebugObjectName(DSSNoDepthTestWithStencil.Get(), "DSSNoDepthTestWithStencil");
	D3D11SetDebugObjectName(DSSNoDepthWriteWithStencil.Get(), "DSSNoDepthWriteWithStencil");
}
