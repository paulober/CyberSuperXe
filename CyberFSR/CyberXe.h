// ReSharper disable CppInconsistentNaming
#pragma once
#include "pch.h"
#include "ViewMatrixHook.h"
#include "NvParameter.h"
#include "DebugOverlay.h"
#include <xess/xess.h>

class FeatureContext;

//Global Context
class CyberFsrContext
{
	CyberFsrContext();
public:
	std::shared_ptr<Config> MyConfig;

	VkDevice VulkanDevice;
	VkInstance VulkanInstance;
	VkPhysicalDevice VulkanPhysicalDevice;

	std::vector<std::unique_ptr<NvParameter>> Parameters;
	NvParameter* AllocateParameter();
	void DeleteParameter(NvParameter* parameter);

	ankerl::unordered_dense::map <unsigned int, std::unique_ptr<FeatureContext>> Contexts;
	FeatureContext* CreateContext();
	void DeleteContext(const NVSDK_NGX_Handle* handle);

	static std::shared_ptr<CyberFsrContext> instance()
	{
		static std::shared_ptr<CyberFsrContext> INSTANCE{new CyberFsrContext()};
		return INSTANCE;
	}
};

class FeatureContext
{
public:
	std::unique_ptr<ViewMatrixHook> ViewMatrix;
	NVSDK_NGX_Handle Handle;
	ID3D12Device* DxDevice;
	//FfxFsr2Context FsrContext;
	//FfxFsr2ContextDescription FsrContextDescription;
	xess_context_handle_t XeSSContext;
	xess_d3d12_init_params_t XeSSInitParams;
	std::unique_ptr<DebugOverlay> DebugLayer;
	std::vector<unsigned char> ScratchBuffer;

	unsigned int Width{}, Height{}, RenderWidth{}, RenderHeight{};
	NVSDK_NGX_PerfQuality_Value PerfQualityValue = NVSDK_NGX_PerfQuality_Value_Balanced;
	float Sharpness = 1.0f;
	float MVScaleX{}, MVScaleY{};
	float JitterOffsetX{}, JitterOffsetY{};
};
