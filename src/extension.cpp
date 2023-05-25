#include "extension.h"

JsonExtension g_JsonExtension;
SMEXT_LINK(&g_JsonExtension);

HandleType_t htJSON;
JSONHandler g_JSONHandler;

bool JsonExtension::SDK_OnLoad(char* error, size_t maxlen, bool late)
{
	sharesys->AddNatives(myself, JsonNatives);
	sharesys->RegisterLibrary(myself, "json");
	
	HandleAccess haJSON;
	handlesys->InitAccessDefaults(nullptr, &haJSON);
	
	// todo: compatible with rip-ext (rename it?)
	htJSON = handlesys->CreateType("JSON", &g_JSONHandler, 0, nullptr, &haJSON, myself->GetIdentity(), nullptr);
	return true;
}

void JsonExtension::SDK_OnUnload()
{
	handlesys->RemoveType(htJSON, myself->GetIdentity());
}

void JSONHandler::OnHandleDestroy(HandleType_t type, void *object)
{
}