#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include "smsdk_ext.h"
#include <parson.h>

class JsonExtension : public SDKExtension
{
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);

	virtual void SDK_OnUnload();
};

class JSONHandler : public IHandleTypeDispatch
{
public:
	void OnHandleDestroy(HandleType_t type, void *object);
};

class JSONArrayHandler : public IHandleTypeDispatch
{
public:
	void OnHandleDestroy(HandleType_t type, void *object);
};

class JSONObjectHandler : public IHandleTypeDispatch
{
public:
	void OnHandleDestroy(HandleType_t type, void *object);
};

extern JsonExtension g_JsonExtension;

extern HandleType_t htJSON;
extern HandleType_t htJSONObject;
extern HandleType_t htJSONArray;

extern JSONHandler g_JSONHandler;
extern JSONArrayHandler g_JSONArrayHandler;
extern JSONObjectHandler g_JSONObjectHandler;

extern const sp_nativeinfo_t JsonNatives[];

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
