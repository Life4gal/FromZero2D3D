//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
//
// 定义一些实用的特效类
// Define utility effect classes.
//***************************************************************************************

//
// 该头文件需要在包含特效类实现的源文件中使用，且必须晚于Effects.h和d3dUtil.h包含
// 

#ifndef EFFECTHELPER_H
#define EFFECTHELPER_H

// 若类需要内存对齐，从该类派生
template<class DerivedType>
struct AlignedType
{
	void* operator new(size_t size)
	{
		const size_t alignedSize = __alignof(DerivedType);

		static_assert(alignedSize > 8, "AlignedNew is only useful for types with > 8 byte alignment! Did you forget a __declspec(align) on DerivedType?");

		void* ptr = _aligned_malloc(size, alignedSize);

		if (!ptr)
			throw std::bad_alloc();

		return ptr;
	}

	void operator delete(void* ptr)
	{
		_aligned_free(ptr);
	}
};

struct CBufferBase
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	CBufferBase() : isDirty() {}
	virtual ~CBufferBase() = default;

	CBufferBase(const CBufferBase& other) = default;
	CBufferBase(CBufferBase && other) noexcept = default;
	CBufferBase& operator=(const CBufferBase & other) = default;
	CBufferBase& operator=(CBufferBase && other) noexcept = default;

	BOOL isDirty;
	ComPtr<ID3D11Buffer> cBuffer;

	virtual HRESULT CreateBuffer(ID3D11Device * device) = 0;
	virtual void UpdateBuffer(ID3D11DeviceContext * deviceContext) = 0;
	virtual void BindVS(ID3D11DeviceContext * deviceContext) = 0;
	virtual void BindHS(ID3D11DeviceContext * deviceContext) = 0;
	virtual void BindDS(ID3D11DeviceContext * deviceContext) = 0;
	virtual void BindGS(ID3D11DeviceContext * deviceContext) = 0;
	virtual void BindCS(ID3D11DeviceContext * deviceContext) = 0;
	virtual void BindPS(ID3D11DeviceContext * deviceContext) = 0;
};

template<UINT startSlot, class T>
struct CBufferObject : CBufferBase
{
	T data;

	CBufferObject() : CBufferBase(), data() {}

	HRESULT CreateBuffer(ID3D11Device * device) override
	{
		if (cBuffer != nullptr)
			return S_OK;
		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.ByteWidth = sizeof(T);
		return device->CreateBuffer(&cbd, nullptr, cBuffer.GetAddressOf());
	}

	void UpdateBuffer(ID3D11DeviceContext * deviceContext) override
	{
		if (isDirty)
		{
			isDirty = false;
			D3D11_MAPPED_SUBRESOURCE mappedData;
			/*
				获取指向缓冲区中数据的指针并拒绝GPU对该缓冲区的访问
				HRESULT ID3D11DeviceContext::Map(
					ID3D11Resource           *pResource,          // [In]包含ID3D11Resource接口的资源对象
					UINT                     Subresource,         // [In]缓冲区资源填0
					D3D11_MAP                MapType,             // [In]D3D11_MAP枚举值，指定读写相关操作
					UINT                     MapFlags,            // [In]填0，CPU需要等待GPU使用完毕当前缓冲区
					D3D11_MAPPED_SUBRESOURCE *pMappedResource     // [Out]获取到的已经映射到缓冲区的内存
				);

				D3D11_MAP成员					含义
				D3D11_MAP_READ					映射到内存的资源用于读取。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_READ标签
				D3D11_MAP_WRITE					映射到内存的资源用于写入。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_WRITE标签
				D3D11_MAP_READ_WRITE			映射到内存的资源用于读写。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_READ和D3D11_CPU_ACCESS_WRITE标签
				D3D11_MAP_WRITE_DISCARD			映射到内存的资源用于写入，之前的资源数据将会被抛弃。该资源在创建的时候必须绑定了D3D11_CPU_ACCESS_WRITE和D3D11_USAGE_DYNAMIC标签
				D3D11_MAP_WRITE_NO_OVERWRITE	映射到内存的资源用于写入，但不能复写已经存在的资源。该枚举值只能用于顶点/索引缓冲区。
												该资源在创建的时候需要有D3D11_CPU_ACCESS_WRITE标签，在Direct3D 11不能用于设置了D3D11_BIND_CONSTANT_BUFFER标签的资源，但在11.1后可以。具体可以查阅MSDN文档

				在创建资源的时候指定Usage为D3D11_USAGE_DYNAMIC、CPUAccessFlags为D3D11_CPU_ACCESS_WRITE，
				允许常量缓冲区从CPU写入，首先通过ID3D11DeviceContext::Map方法获取内存映射，
				然后再更新到映射好的内存区域，最后通过ID3D11DeviceContext::Unmap方法解除占用。
				适合于需要频繁更新，如每几帧更新一次，或每帧更新一次或多次的资源。

			*/
			deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
			memcpy_s(mappedData.pData, sizeof(T), &data, sizeof(T));
			/*
				让指向资源的指针无效并重新启用GPU对该资源的访问权限
				void ID3D11DeviceContext::Unmap(
					ID3D11Resource *pResource,      // [In]包含ID3D11Resource接口的资源对象
					UINT           Subresource      // [In]缓冲区资源填0
				);
			*/
			deviceContext->Unmap(cBuffer.Get(), 0);
		}
	}
	
	void BindVS(ID3D11DeviceContext * deviceContext) override
	{
		deviceContext->VSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindHS(ID3D11DeviceContext * deviceContext) override
	{
		deviceContext->HSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindDS(ID3D11DeviceContext * deviceContext) override
	{
		deviceContext->DSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindGS(ID3D11DeviceContext * deviceContext) override
	{
		deviceContext->GSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindCS(ID3D11DeviceContext * deviceContext) override
	{
		deviceContext->CSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}

	void BindPS(ID3D11DeviceContext * deviceContext) override
	{
		deviceContext->PSSetConstantBuffers(startSlot, 1, cBuffer.GetAddressOf());
	}
};

#endif

