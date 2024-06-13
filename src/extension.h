#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include "smsdk_ext.h"
#include <parson.h>
#include <parsonwrapper.h>

class JsonExtension : public SDKExtension
{
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();
	ParsonWrapper *GetJSONPointer(IPluginContext *pContext, Handle_t handle);
};

class JSONHandler : public IHandleTypeDispatch
{
public:
	void OnHandleDestroy(HandleType_t type, void *object);
};

extern JsonExtension g_JsonExtension;
extern HandleType_t g_htJSON;
extern JSONHandler g_JSONHandler;
extern const sp_nativeinfo_t json_natives[];

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
