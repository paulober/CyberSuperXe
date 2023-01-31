#include "pch.h"
#include "Config.h"
#include "CyberXe.h"
#include <xess/xess_d3d12.h>
#include <xess/xess_d3d12_debug.h>
#include "DirectXHooks.h"
#include "Util.h"

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_Ext(
	unsigned long long InApplicationId,
	const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo,
	NVSDK_NGX_Version InSDKVersion,
	unsigned long long unknown0)
{	
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D12_Init(
	unsigned long long InApplicationId,
	const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo,
	NVSDK_NGX_Version InSDKVersion)
{
	return NVSDK_NGX_D3D12_Init_Ext(
		InApplicationId, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_ProjectID(
	const char* InProjectId,
	NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion,
	const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	return NVSDK_NGX_D3D12_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_Result NVSDK_NGX_D3D12_Init_with_ProjectID(
	const char* InProjectId,
	NVSDK_NGX_EngineType InEngineType,
	const char* InEngineVersion,
	const wchar_t* InApplicationDataPath,
	ID3D12Device* InDevice,
	const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo,
	NVSDK_NGX_Version InSDKVersion)
{
	return NVSDK_NGX_D3D12_Init_Ext(
		0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_Shutdown(void)
{
	CyberFsrContext::instance()->Parameters.clear();
	CyberFsrContext::instance()->Contexts.clear();
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D12_Shutdown1(ID3D12Device* InDevice)
{
	CyberFsrContext::instance()->Parameters.clear();
	CyberFsrContext::instance()->Contexts.clear();
	return NVSDK_NGX_Result_Success;
}

//Deprecated Parameter Function - Internal Memory Tracking
NVSDK_NGX_Result NVSDK_NGX_D3D12_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	*OutParameters = CyberFsrContext::instance()->AllocateParameter();
	return NVSDK_NGX_Result_Success;
}

//TODO External Memory Tracking
NVSDK_NGX_Result NVSDK_NGX_D3D12_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	*OutParameters = new NvParameter();
	return NVSDK_NGX_Result_Success;
}

//TODO
NVSDK_NGX_Result NVSDK_NGX_D3D12_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	*OutParameters = new NvParameter();
	return NVSDK_NGX_Result_Success;
}

//TODO
NVSDK_NGX_Result NVSDK_NGX_D3D12_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	delete InParameters;
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D12_GetScratchBufferSize(
	NVSDK_NGX_Feature InFeatureId,
	const NVSDK_NGX_Parameter* InParameters,
	size_t* OutSizeInBytes)
{
	*OutSizeInBytes = ffxFsr2GetScratchMemorySizeDX12();
	return NVSDK_NGX_Result_Success;
}

// EvaluateRenderScale helper
inline _xess_quality_settings_t DLSS2XeSSQualityFeature(const NVSDK_NGX_PerfQuality_Value input)
{
	_xess_quality_settings_t output;

	switch (input)
	{
	case NVSDK_NGX_PerfQuality_Value_UltraQuality:
		output = XESS_QUALITY_SETTING_ULTRA_QUALITY;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxPerf:
		output = XESS_QUALITY_SETTING_PERFORMANCE;
		break;
	case NVSDK_NGX_PerfQuality_Value_Balanced:
		output = XESS_QUALITY_SETTING_BALANCED;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxQuality:
		output = XESS_QUALITY_SETTING_QUALITY;
		break;
	case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
	default:
		//Set out-of-range value for non-existing XeSS ultra performance mode
		output = static_cast<_xess_quality_settings_t>(XESS_QUALITY_SETTING_PERFORMANCE + 5);
		break;
	}

	return output;
}

NVSDK_NGX_Result NVSDK_NGX_D3D12_CreateFeature(
	ID3D12GraphicsCommandList* InCmdList,
	NVSDK_NGX_Feature InFeatureID,
	const NVSDK_NGX_Parameter* InParameters,
	NVSDK_NGX_Handle** OutHandle)
{
	const auto inParams = dynamic_cast<const NvParameter*>(InParameters);

	ID3D12Device* device;
	InCmdList->GetDevice(IID_PPV_ARGS(&device));

	auto instance = CyberFsrContext::instance();
	auto config = instance->MyConfig;
	auto deviceContext = CyberFsrContext::instance()->CreateContext();
	deviceContext->ViewMatrix = ViewMatrixHook::Create(*config);
#ifdef DEBUG_FEATURES
	deviceContext->DebugLayer = std::make_unique<DebugOverlay>(device, InCmdList);
#endif

	* OutHandle = &deviceContext->Handle;

	auto initParams = deviceContext->XeSSInitParams;

	const size_t scratchBufferSize = ffxFsr2GetScratchMemorySizeDX12();
	deviceContext->ScratchBuffer = std::vector<unsigned char>(scratchBufferSize);
	auto scratchBuffer = deviceContext->ScratchBuffer.data();
	
	/*FfxErrorCode errorCode = ffxFsr2GetInterfaceDX12(&initParams.callbacks, device, scratchBuffer, scratchBufferSize);
	ASSERT(errorCode == FFX_OK);*/

	/*initParams.device = ffxGetDeviceDX12(device);
	initParams.maxRenderSize.width = inParams->Width;
	initParams.maxRenderSize.height = inParams->Height;
	initParams.displaySize.width = inParams->OutWidth;
	initParams.displaySize.height = inParams->OutHeight;*/
	initParams.outputResolution.x = inParams->OutWidth;
	initParams.outputResolution.y = inParams->OutHeight;
	initParams.qualitySetting = DLSS2XeSSQualityFeature(inParams->PerfQualityValue);

	initParams.initFlags = XESS_INIT_FLAG_NONE;
	if (config->DepthInverted.value_or(inParams->DepthInverted))
	{
		initParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
	}
	if (config->AutoExposure.value_or(inParams->AutoExposure))
	{
		initParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
	}
	/*if (config->HDR.value_or(inParams->Hdr))
	{
		initParams.initFlags |= XESS_INIT_FLAG_NONE;
	}*/
	if (config->MotionVectors.value_or(inParams->MotionVectors))
	{
		initParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
	}
	if (config->JitterCancellation.value_or(inParams->JitterMotion))
	{
		initParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
	}
	// TODO: not sure about that
	if (config->DisplayResolution.value_or(!inParams->LowRes))
	{
		initParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
	}
	// TODO: maybe use XESS_INIT_FLAG_EXTERNAL_DESCRIPTOR_HEAP
	if (config->InfiniteFarPlane.value_or(false))
	{
		initParams.initFlags |= XESS_INIT_FLAG_NONE;
	}

	xess_result_t result = xessD3D12Init(deviceContext->XeSSContext, &initParams);
	assert(result == XESS_RESULT_SUCCESS);

	result = xessD3D12CreateContext(device, &deviceContext->XeSSContext);
	assert(result == XESS_RESULT_SUCCESS && deviceContext->XeSSContext);

	if (result != XESS_RESULT_SUCCESS)
	{
		printf("XeSS: XeSS is not supported on this device. Result - %s.", Util::ResultToString(result));
	}

	HookSetComputeRootSignature(InCmdList);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D12_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	const auto deviceContext = CyberFsrContext::instance()->Contexts[InHandle->Id].get();
	const xess_result_t result = xessDestroyContext(deviceContext->XeSSContext);
	deviceContext->XeSSContext = nullptr;
	assert(result == XESS_RESULT_SUCCESS);
	CyberFsrContext::instance()->DeleteContext(InHandle);
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D12_EvaluateFeature(
	ID3D12GraphicsCommandList* InCmdList,
	const NVSDK_NGX_Handle* InFeatureHandle,
	const NVSDK_NGX_Parameter* InParameters,
	PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	ID3D12RootSignature* orgRootSig = nullptr;

	rootSigMutex.lock();
	if (commandListVector.contains(InCmdList))
	{
		orgRootSig = commandListVector[InCmdList];
	}
	else
	{
		printf("Cant find the RootSig\n");
	}
	rootSigMutex.unlock();

	ID3D12Device* device;
	InCmdList->GetDevice(IID_PPV_ARGS(&device));
	auto instance = CyberFsrContext::instance();
	auto config = instance->MyConfig;
	auto deviceContext = CyberFsrContext::instance()->Contexts[InFeatureHandle->Id].get();

	if (orgRootSig)
	{
		const auto inParams = dynamic_cast<const NvParameter*>(InParameters);

		auto* xessContext = &deviceContext->XeSSContext;

		xess_d3d12_execute_params_t dispatchParameters = {};
		dispatchParameters.commandList = ffxGetCommandListDX12(InCmdList);
		dispatchParameters.color = ffxGetResourceDX12(xessContext, (ID3D12Resource*)inParams->Color, (wchar_t*)L"FSR2_InputColor");
		dispatchParameters.depth = ffxGetResourceDX12(xessContext, (ID3D12Resource*)inParams->Depth, (wchar_t*)L"FSR2_InputDepth");
		dispatchParameters.motionVectors = ffxGetResourceDX12(xessContext, (ID3D12Resource*)inParams->MotionVectors, (wchar_t*)L"FSR2_InputMotionVectors");
		dispatchParameters.exposure = ffxGetResourceDX12(xessContext, (ID3D12Resource*)inParams->ExposureTexture, (wchar_t*)L"FSR2_InputExposure");

		//Not sure if these two actually work
		if (!config->DisableReactiveMask.value_or(false))
		{
			dispatchParameters.reactive = ffxGetResourceDX12(xessContext, (ID3D12Resource*)inParams->InputBiasCurrentColorMask, (wchar_t*)L"FSR2_InputReactiveMap");
			dispatchParameters.transparencyAndComposition = ffxGetResourceDX12(xessContext, (ID3D12Resource*)inParams->TransparencyMask, (wchar_t*)L"FSR2_TransparencyAndCompositionMap");
		}

		dispatchParameters.output = ffxGetResourceDX12(xessContext, (ID3D12Resource*)inParams->Output, (wchar_t*)L"FSR2_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);

		dispatchParameters.jitterOffsetX = inParams->JitterOffsetX;
		dispatchParameters.jitterOffsetY = inParams->JitterOffsetY;

		dispatchParameters.motionVectorScaleX = static_cast<float>(inParams->MVScaleX);
		dispatchParameters.motionVectorScaleY = static_cast<float>(inParams->MVScaleY);

		dispatchParameters.resetHistory = inParams->ResetRender ? 1 : 0;

		float sharpness = Util::ConvertSharpness(inParams->Sharpness, config->SharpnessRange);
		dispatchParameters.enableSharpening = config->EnableSharpening.value_or(inParams->EnableSharpening);
		dispatchParameters.sharpness = config->Sharpness.value_or(sharpness);

		// deltatime hax
		static double lastFrameTime;
		const double currentTime = Util::MillisecondsNow();
		const double deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		dispatchParameters.frameTimeDelta = static_cast<float>(deltaTime);
		dispatchParameters.preExposure = 1.0f;
		dispatchParameters.renderSize.width = inParams->Width;
		dispatchParameters.renderSize.height = inParams->Height;

		//Hax Zone
		dispatchParameters.cameraFar = deviceContext->ViewMatrix->GetFarPlane();
		dispatchParameters.cameraNear = deviceContext->ViewMatrix->GetNearPlane();
		dispatchParameters.cameraFovAngleVertical = DirectX::XMConvertToRadians(deviceContext->ViewMatrix->GetFov());
		xess_result_t result = ffxFsr2ContextDispatch(xessContext, &dispatchParameters);
		assert(result == XESS_RESULT_SUCCESS);

		InCmdList->SetComputeRootSignature(orgRootSig);
	}
#ifdef DEBUG_FEATURES
	deviceContext->DebugLayer->AddText(L"DLSS2XeSS", DirectX::XMFLOAT2(1.0, 1.0));
	deviceContext->DebugLayer->Render(InCmdList);
#endif

	myCommandList = InCmdList;

	return NVSDK_NGX_Result_Success;
}