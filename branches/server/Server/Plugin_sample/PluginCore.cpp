#include "PluginCore.h"
#include "PluginConfig.h"

#ifdef WIN32
#include "windows.h"
#endif

extern IPluginInterface *pluginptr;

extern "C" __declspec(dllexport) IPluginInterface *GetPlugin(void)
{
	return pluginptr;
}

PluginCore::PluginCore(void)
{
	server = NULL;
}

PluginCore::~PluginCore(void)
{
}

bool PluginCore::AttachToServer(void)
{
	typedef IPluginHandlerInterface *(*GetPluginHandlerFunctionType)();
	GetPluginHandlerFunctionType GetPluginHandlerFunction = NULL;
	GetPluginHandlerFunction = (GetPluginHandlerFunctionType)GetProcAddress(GetModuleHandle(NULL), "GetPluginHandler");
	if (GetPluginHandlerFunction == NULL)
	{
		return false;
	}
	server = GetPluginHandlerFunction();
	if (server == NULL)
	{
		return false;
	}
	return true;
}

void PluginCore::OnPluginLoad(void)
{
}

void PluginCore::OnPluginUnload(void)
{
}

char *PluginCore::GetName(void)
{
	return PLUGIN_NAME;
}

char *PluginCore::GetVersion(void)
{
	return PLUGIN_VERSION;
}

char *PluginCore::GetAuthor(void)
{
	return PLUGIN_AUTHOR;
}