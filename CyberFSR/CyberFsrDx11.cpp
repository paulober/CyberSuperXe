#include "pch.h"
#include "CyberXe.h"

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(void)
{
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown(void)
{
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	*OutParameters = CyberFsrContext::instance()->AllocateParameter();
	return NVSDK_NGX_Result_Success;
}