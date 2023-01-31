#include "pch.h"
#include "Config.h"
#include "CyberXe.h"
#include "DirectXHooks.h"
#include "Util.h"

NvParameter* CyberFsrContext::AllocateParameter()
{
	Parameters.push_back(std::make_unique<NvParameter>());
	return Parameters.back().get();
}

void CyberFsrContext::DeleteParameter(NvParameter* parameter)
{
	const auto it = std::ranges::find_if(Parameters, [parameter](const auto& p) { return p.get() == parameter; });
	Parameters.erase(it);
}

FeatureContext* CyberFsrContext::CreateContext()
{
	const auto handleId = rand();
	Contexts[handleId] = std::make_unique<FeatureContext>();
	Contexts[handleId]->Handle.Id = handleId;
	return Contexts[handleId].get();
}

void CyberFsrContext::DeleteContext(const NVSDK_NGX_Handle* handle)
{
	auto handleId = handle->Id;

	const auto it = std::ranges::find_if(Contexts, [&handleId](const auto& p) { return p.first == handleId; });
	Contexts.erase(it);
}

CyberFsrContext::CyberFsrContext()
{
	MyConfig = std::make_unique<Config>(L"nvngx.ini");
}
