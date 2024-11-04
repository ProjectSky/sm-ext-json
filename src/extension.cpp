#include "extension.h"

JsonExtension g_JsonExtension;
SMEXT_LINK(&g_JsonExtension);

HandleType_t g_htJSON;
JSONHandler g_JSONHandler;

bool JsonExtension::SDK_OnLoad(char* error, size_t maxlen, bool late)
{
	sharesys->AddNatives(myself, json_natives);
	sharesys->RegisterLibrary(myself, "yyjson");

	HandleAccess haJSON;
	handlesys->InitAccessDefaults(nullptr, &haJSON);
	haJSON.access[HandleAccess_Delete] = 0;

	HandleError err;
	g_htJSON = handlesys->CreateType("YYJSON", &g_JSONHandler, 0, nullptr, &haJSON, myself->GetIdentity(), &err);

	if (!g_htJSON) {
		snprintf(error, maxlen, "Failed to create YYJSON handle type (err: %d)", err);
		return false;
	}
	
	return true;
}

void JsonExtension::SDK_OnUnload()
{
	handlesys->RemoveType(g_htJSON, myself->GetIdentity());
}

void JSONHandler::OnHandleDestroy(HandleType_t type, void* object)
{
	delete (YYJsonWrapper*)object;
}

YYJsonWrapper* JsonExtension::GetJSONPointer(IPluginContext* pContext, Handle_t handle)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	YYJsonWrapper* pYYJsonWrapper;
	if ((err = handlesys->ReadHandle(handle, g_htJSON, &sec, (void**)&pYYJsonWrapper)) != HandleError_None)
	{
		pContext->ThrowNativeError("Invalid YYJSON handle %x (error %d)", handle, err);
		return nullptr;
	}

	return pYYJsonWrapper;
}