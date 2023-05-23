#include "extension.h"

JsonExtension g_JsonExtension;
SMEXT_LINK(&g_JsonExtension);

HandleType_t htJSON;
HandleType_t htJSONObject;
HandleType_t htJSONArray;

JSONHandler g_JSONHandler;
JSONArrayHandler g_JSONArrayHandler;
JSONObjectHandler g_JSONObjectHandler;

bool JsonExtension::SDK_OnLoad(char* error, size_t maxlen, bool late)
{
	sharesys->AddNatives(myself, JsonNatives);
	sharesys->RegisterLibrary(myself, "json");
	
	HandleAccess haJSON;
	handlesys->InitAccessDefaults(nullptr, &haJSON);
	
	htJSON = handlesys->CreateType("JSON", &g_JSONHandler, 0, nullptr, &haJSON, myself->GetIdentity(), nullptr);
	htJSONObject = handlesys->CreateType("JSONObject", &g_JSONObjectHandler, 0, nullptr, nullptr, myself->GetIdentity(), nullptr);
	htJSONArray = handlesys->CreateType("JSONArray", &g_JSONArrayHandler, 0, nullptr, nullptr, myself->GetIdentity(), nullptr);
	return true;
}

void JsonExtension::SDK_OnUnload()
{
	handlesys->RemoveType(htJSON, myself->GetIdentity());
	handlesys->RemoveType(htJSONObject, myself->GetIdentity());
	handlesys->RemoveType(htJSONArray, myself->GetIdentity());
}

void JSONHandler::OnHandleDestroy(HandleType_t type, void *object)
{
}

void JSONArrayHandler::OnHandleDestroy(HandleType_t type, void *object)
{
}

void JSONObjectHandler::OnHandleDestroy(HandleType_t type, void *object)
{
}