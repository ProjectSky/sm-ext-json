#include "extension.h"

static cell_t pawn_json_parse(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	bool is_file = params[2];
	bool with_comments = params[3];
	
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();

	if (is_file)
	{
		char realpath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", string);
		pParsonWrapper->m_pValue = with_comments ? json_parse_file_with_comments(realpath) : json_parse_file(realpath);
	}
	else
	{
		pParsonWrapper->m_pValue = with_comments ? json_parse_string_with_comments(string) : json_parse_string(string);
	}
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON parse handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_object_from_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	bool with_comments = params[2];

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	
	pParsonWrapper->m_pValue = with_comments ? json_parse_string_with_comments(string) : json_parse_string(string);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_from_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_object_from_file(IPluginContext *pContext, const cell_t *params)
{
	char *path;
	pContext->LocalToString(params[1], &path);

	bool with_comments = params[2];

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	
	pParsonWrapper->m_pValue = with_comments ? json_parse_file_with_comments(realpath) : json_parse_file(realpath);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_from_file handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_array_from_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	bool with_comments = params[2];
	
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	
	pParsonWrapper->m_pValue = with_comments ? json_parse_string_with_comments(string) : json_parse_string(string);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON array_from_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_array_from_file(IPluginContext *pContext, const cell_t *params)
{
	char *path;
	pContext->LocalToString(params[1], &path);

	bool with_comments = params[2];

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	
	pParsonWrapper->m_pValue = with_comments ? json_parse_file_with_comments(realpath) : json_parse_file(realpath);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON array_from_file handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_equals(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	ParsonWrapper *handle2 = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (handle1 == NULL || handle2 == NULL) return BAD_HANDLE;

	return json_value_equals(handle1->m_pValue, handle2->m_pValue) == 1;
}

static cell_t pawn_json_validate(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	ParsonWrapper *handle2 = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (handle1 == NULL || handle2 == NULL) return BAD_HANDLE;

	return json_validate(handle1->m_pValue, handle2->m_pValue) == JSONSuccess;
}

static cell_t pawn_json_get_parent(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_get_parent(handle->m_pValue);
	pParsonWrapper->m_bAllowFree = false;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	handle->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (handle->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON get_parent handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return handle->m_handle;
}

static cell_t pawn_json_get_type(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	return json_value_get_type(handle->m_pValue);
}

static cell_t pawn_json_init_object(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_object();

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		json_value_free(pParsonWrapper->m_pValue);
		pContext->ReportError("Could not create JSON init_object handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_init_array(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_array();

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		json_value_free(pParsonWrapper->m_pValue);
		pContext->ReportError("Could not create JSON init_array handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_init_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_string(string);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_init_integer(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_integer(params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_integer handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_init_integer64(IPluginContext *pContext, const cell_t *params)
{
	char *val;
	pContext->LocalToString(params[1], &val);

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_integer(strtoll(val, nullptr, 10));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_integer64 handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t json_init_float(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_number(sp_ctof(params[1]));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_float handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_init_bool(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_boolean(params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_boolean handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_init_null(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_init_null();

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_null handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_deep_copy(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_value_deep_copy(handle->m_pValue);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	handle->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (handle->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON deep_copy handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return handle->m_handle;
}

static cell_t pawn_json_get_string(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	const char *string = json_value_get_string(handle->m_pValue);

	if (string == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[2], params[3], string, nullptr);

	return 1;
}

static cell_t pawn_json_get_string_length(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	return json_value_get_string_len(handle->m_pValue);
}

static cell_t pawn_json_serialization_size(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	return json_serialization_size(handle->m_pValue);
}

static cell_t pawn_json_serialization_size_pretty(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	return json_serialization_size_pretty(handle->m_pValue);
}

static cell_t pawn_json_get_integer(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	return json_value_get_integer(handle->m_pValue);
}

static cell_t pawn_json_get_integer64(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	char result[20];
	snprintf(result, sizeof(result), "%lld", json_value_get_integer(handle->m_pValue));
	pContext->StringToLocalUTF8(params[2], params[3], result, nullptr);

	return 1;
}

static cell_t pawn_json_get_float(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	float result = json_value_get_number(handle->m_pValue);

	return sp_ftoc(result);
}

static cell_t pawn_json_get_bool(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	return json_value_get_boolean(handle->m_pValue);
}

static cell_t pawn_json_array_get_value(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = json_array_get_value(handle->m_pArray, params[2]);
	pParsonWrapper->m_bAllowFree = false;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON array_get_value handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_array_get_string(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	const char *string = json_array_get_string(handle->m_pArray, params[2]);

	if (string == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[3], params[4], string, nullptr);

	return 1;
}

static cell_t pawn_json_array_get_string_length(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_get_string_len(handle->m_pArray, params[2]);
}

static cell_t pawn_json_array_is_null(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	JSON_Value *value = json_array_get_value(handle->m_pArray, params[2]);

	return json_value_get_type(value) == JSONNull;
}

static cell_t pawn_json_array_get_integer(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_get_integer(handle->m_pArray, params[2]);
}

static cell_t pawn_json_array_get_integer64(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	int index = params[2];

	char result[20];
	snprintf(result, sizeof(result), "%lld", json_array_get_integer(handle->m_pArray, index));
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t pawn_json_array_get_float(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	float result = json_array_get_number(handle->m_pArray, params[2]);

	return sp_ftoc(result);
}

static cell_t pawn_json_array_get_bool(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_get_boolean(handle->m_pArray, params[2]);
}

static cell_t pawn_json_array_get_count(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_get_count(handle->m_pArray);
}

static cell_t pawn_json_array_replace_value(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle1 == NULL) return BAD_HANDLE;

	handle1->m_pArray = json_value_get_array(handle1->m_pValue);

	ParsonWrapper *handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (handle2 == NULL) return BAD_HANDLE;

	if (json_value_get_parent(handle2->m_pValue))
	{
		handle2->m_pValue = json_value_deep_copy(handle2->m_pValue);
	}
	else
	{
		handle2->m_bAllowFree = false;
	}

	return json_array_replace_value(handle1->m_pArray, params[2], handle2->m_pValue) == JSONSuccess;
}

static cell_t pawn_json_array_replace_string(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);
	
	char *string;
	pContext->LocalToString(params[3], &string);

	return json_array_replace_string(handle->m_pArray, params[2], string) == JSONSuccess;
}

static cell_t pawn_json_array_replace_integer(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_replace_integer(handle->m_pArray, params[2], params[3]) == JSONSuccess;
}

static cell_t pawn_json_array_replace_integer64(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	int index = params[2];

	char *val;
	pContext->LocalToString(params[3], &val);

	return json_array_replace_integer(handle->m_pArray, index, strtoll(val, nullptr, 10)) == JSONSuccess;
}

static cell_t pawn_json_array_replace_float(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_replace_number(handle->m_pArray, params[2], sp_ctof(params[3])) == JSONSuccess;
}

static cell_t pawn_json_array_replace_bool(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_replace_boolean(handle->m_pArray, params[2], params[3] != 0) == JSONSuccess;
}

static cell_t pawn_json_array_replace_null(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_replace_null(handle->m_pArray, params[2]) == JSONSuccess;
}

static cell_t pawn_json_array_append_value(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle1 == NULL) return BAD_HANDLE;

	handle1->m_pArray = json_value_get_array(handle1->m_pValue);

	ParsonWrapper *value = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (value == NULL) return BAD_HANDLE;

	if (json_value_get_parent(value->m_pValue))
	{
		value->m_pValue = json_value_deep_copy(value->m_pValue);
	}
	else
	{
		value->m_bAllowFree = false;
	}

	return json_array_append_value(handle1->m_pArray, value->m_pValue) == JSONSuccess;
}

static cell_t pawn_json_array_append_string(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	char *string;
	pContext->LocalToString(params[2], &string);
	return json_array_append_string(handle->m_pArray, string) == JSONSuccess;
}

static cell_t pawn_json_array_append_integer(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_append_integer(handle->m_pArray, params[2]) == JSONSuccess;
}

static cell_t pawn_json_array_append_integer64(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	char *val;
	pContext->LocalToString(params[2], &val);

	return json_array_append_integer(handle->m_pArray, strtoll(val, nullptr, 10)) == JSONSuccess;
}

static cell_t pawn_json_array_append_float(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_append_number(handle->m_pArray, sp_ctof(params[2])) == JSONSuccess;
}

static cell_t pawn_json_array_append_bool(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_append_boolean(handle->m_pArray, params[2] != 0) == JSONSuccess;
}

static cell_t pawn_json_array_append_null(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;
	
	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_append_null(handle->m_pArray) == JSONSuccess;
}

static cell_t pawn_json_array_remove(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_remove(handle->m_pArray, params[2]) == JSONSuccess;
}

static cell_t pawn_json_array_clear(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pArray = json_value_get_array(handle->m_pValue);

	return json_array_clear(handle->m_pArray) == JSONSuccess;
}

static cell_t pawn_json_object_get_value(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);
	
	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();
	pParsonWrapper->m_pValue = is_dot ? json_object_dotget_value(handle->m_pObject, key) : json_object_get_value(handle->m_pObject, key);
	pParsonWrapper->m_bAllowFree = false;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_get_value handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_object_get_string(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);
	
	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[5];

	const char *string = is_dot ? json_object_dotget_string(handle->m_pObject, key) : json_object_get_string(handle->m_pObject, key);

	if (string == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[3], params[4], string, nullptr);

	return 1;
}

static cell_t pawn_json_object_get_string_length(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);
	
	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	return is_dot ? json_object_dotget_string_len(handle->m_pObject, key) : json_object_get_string_len(handle->m_pObject, key);;
}

static cell_t pawn_json_object_get_integer(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	double result = is_dot ? json_object_dotget_integer(handle->m_pObject, key) : json_object_get_integer(handle->m_pObject, key);

	return result;
}

static cell_t pawn_json_object_get_integer64(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[5];

	char result[20];
	snprintf(result, sizeof(result), "%lld", is_dot ? json_object_dotget_integer(handle->m_pObject, key) : json_object_get_integer(handle->m_pObject, key));
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t pawn_json_object_get_float(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	float result = is_dot ? json_object_dotget_number(handle->m_pObject, key) : json_object_get_number(handle->m_pObject, key);

	return sp_ftoc(result);
}

static cell_t pawn_json_object_get_bool(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	int result = is_dot ? json_object_dotget_boolean(handle->m_pObject, key) : json_object_get_boolean(handle->m_pObject, key);

	return result;
}

static cell_t pawn_json_object_get_count(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	return json_object_get_count(handle->m_pObject);
}

static cell_t pawn_json_object_get_name(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	const char *name = json_object_get_name(handle->m_pObject, params[2]);

	if (name == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[3], params[4], name, nullptr);

	return 1;
}

static cell_t pawn_json_object_get_value_at(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	ParsonWrapper* pParsonWrapper = new ParsonWrapper();

	pParsonWrapper->m_pValue = json_object_get_value_at(handle->m_pObject, params[2]);
	pParsonWrapper->m_bAllowFree = false;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pParsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pParsonWrapper, &sec, NULL, &err);
	
	if (pParsonWrapper->m_handle == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_get_value_at handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return pParsonWrapper->m_handle;
}

static cell_t pawn_json_object_has_value(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	int result;

	bool is_dot = params[4];

	if (params[3] == JSONError)
	{
		result = is_dot ? json_object_dothas_value(handle->m_pObject, key) : json_object_has_value(handle->m_pObject, key);
	}
	else
	{
		result = is_dot ? json_object_dothas_value_of_type(handle->m_pObject, key, params[3]) : json_object_has_value_of_type(handle->m_pObject, key, params[3]);
	}

	return result;
}

static cell_t pawn_json_object_has_key(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Value *value = is_dot ? json_object_dotget_value(handle->m_pObject, key) : json_object_get_value(handle->m_pObject, key);

	return value != NULL;
}

static cell_t pawn_json_object_set_value(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle1 == NULL) return BAD_HANDLE;

	handle1->m_pObject = json_value_get_object(handle1->m_pValue);
	ParsonWrapper *handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (handle2 == NULL) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	if (json_value_get_parent(handle2->m_pValue))
	{
		handle2->m_pValue = json_value_deep_copy(handle2->m_pValue);
	}
	else
	{
		handle2->m_bAllowFree = false;
	}

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_value(handle1->m_pObject, key, handle2->m_pValue) : json_object_set_value(handle1->m_pObject, key, handle2->m_pValue);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_string(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key, *value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_string(handle->m_pObject, key, value) : json_object_set_string(handle->m_pObject, key, value);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_integer(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_integer(handle->m_pObject, key, params[3]) : json_object_set_integer(handle->m_pObject, key, params[3]);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_integer64(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key, *val;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &val);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_integer(handle->m_pObject, key, strtoll(val, nullptr, 10)) : json_object_set_integer(handle->m_pObject, key, strtoll(val, nullptr, 10));

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_float(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;
	
	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_number(handle->m_pObject, key, sp_ctof(params[3])) : json_object_set_number(handle->m_pObject, key, sp_ctof(params[3]));

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_bool(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_boolean(handle->m_pObject, key, params[3]) : json_object_set_boolean(handle->m_pObject, key, params[3]);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_null(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;
	
	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Status result = is_dot ? json_object_dotset_null(handle->m_pObject, key) : json_object_set_null(handle->m_pObject, key);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_remove(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Status result = is_dot ? json_object_dotremove(handle->m_pObject, key) : json_object_remove(handle->m_pObject, key);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_clear(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	return json_object_clear(handle->m_pObject) == JSONSuccess;
}

static cell_t pawn_json_object_is_null(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	handle->m_pObject = json_value_get_object(handle->m_pValue);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Value *value = is_dot ? json_object_dotget_value(handle->m_pObject, key) : json_object_get_value(handle->m_pObject, key);

	return json_value_get_type(value) == JSONNull;
}

static cell_t pawn_json_serial_size(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	bool pretty = params[2];

	size_t result = pretty ? json_serialization_size_pretty(handle->m_pValue) : json_serialization_size(handle->m_pValue);

	return (params[3]) ? result : result - 1;
}

static cell_t pawn_json_serial_to_string(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;

	bool pretty = params[4];

	char *result = pretty ? json_serialize_to_string_pretty(handle->m_pValue) : json_serialize_to_string(handle->m_pValue);

	if (result == nullptr) return 0;

	pContext->StringToLocalUTF8(params[2], params[3], result, nullptr);

	json_free_serialized_string(result);

	return 1;
}

static cell_t pawn_json_serial_to_file(IPluginContext *pContext, const cell_t *params)
{
	ParsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (handle == NULL) return BAD_HANDLE;
	
	char *path;
	pContext->LocalToString(params[2], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);

	bool pretty = params[3];

	JSON_Status JSResult = pretty ? json_serialize_to_file_pretty(handle->m_pValue, realpath) : json_serialize_to_file(handle->m_pValue, realpath);

	return JSResult == JSONSuccess;
}

static cell_t pawn_json_set_escape_slashes(IPluginContext *pContext, const cell_t *params)
{
	bool escape_slashes = params[1];

	json_set_escape_slashes(escape_slashes);

	return 1;
}

static cell_t pawn_json_set_float_serialize(IPluginContext *pContext, const cell_t *params)
{
	char *format;
	pContext->LocalToString(params[1], &format);

	json_set_float_serialization_format(format);

	return 1;
}

const sp_nativeinfo_t json_natives[] =
{
	{"json_parse", pawn_json_parse},
	{"json_equals", pawn_json_equals},
	{"json_validate", pawn_json_validate},
	{"json_get_parent", pawn_json_get_parent},
	{"json_get_type", pawn_json_get_type},
	{"json_init_object", pawn_json_init_object},
	{"json_init_array", pawn_json_init_array},
	{"json_init_string", pawn_json_init_string},
	{"json_init_integer", pawn_json_init_integer},
	{"json_init_integer64", pawn_json_init_integer64},
	{"json_init_float", json_init_float},
	{"json_init_bool", pawn_json_init_bool},
	{"json_init_null", pawn_json_init_null},
	{"json_deep_copy", pawn_json_deep_copy},
	{"json_get_string", pawn_json_get_string},
	{"json_get_integer", pawn_json_get_integer},
	{"json_get_integer64", pawn_json_get_integer64},
	{"json_get_float", pawn_json_get_float},
	{"json_get_bool", pawn_json_get_bool},
	{"json_array_get_value", pawn_json_array_get_value},
	{"json_array_get_string", pawn_json_array_get_string},
	{"json_array_get_string_length", pawn_json_array_get_string_length},
	{"json_array_get_count", pawn_json_array_get_count},
	{"json_array_get_integer", pawn_json_array_get_integer},
	{"json_array_get_integer64", pawn_json_array_get_integer64},
	{"json_array_get_float", pawn_json_array_get_float},
	{"json_array_get_bool", pawn_json_array_get_bool},
	{"json_array_is_null", pawn_json_array_is_null},
	{"json_array_replace_value", pawn_json_array_replace_value},
	{"json_array_replace_string", pawn_json_array_replace_string},
	{"json_array_replace_integer", pawn_json_array_replace_integer},
	{"json_array_replace_integer64", pawn_json_array_replace_integer64},
	{"json_array_replace_float", pawn_json_array_replace_float},
	{"json_array_replace_bool", pawn_json_array_replace_bool},
	{"json_array_replace_null", pawn_json_array_replace_null},
	{"json_array_append_value", pawn_json_array_append_value},
	{"json_array_append_string", pawn_json_array_append_string},
	{"json_array_append_integer", pawn_json_array_append_integer},
	{"json_array_append_integer64", pawn_json_array_append_integer64},
	{"json_array_append_float", pawn_json_array_append_float},
	{"json_array_append_bool", pawn_json_array_append_bool},
	{"json_array_append_null", pawn_json_array_append_null},
	{"json_array_remove", pawn_json_array_remove},
	{"json_array_clear", pawn_json_array_clear},
	{"json_object_get_value", pawn_json_object_get_value},
	{"json_object_get_string", pawn_json_object_get_string},
	{"json_object_get_string_length", pawn_json_object_get_string_length},
	{"json_object_get_integer", pawn_json_object_get_integer},
	{"json_object_get_integer64", pawn_json_object_get_integer64},
	{"json_object_get_float", pawn_json_object_get_float},
	{"json_object_get_bool", pawn_json_object_get_bool},
	{"json_object_get_count", pawn_json_object_get_count},
	{"json_object_get_name", pawn_json_object_get_name},
	{"json_object_get_value_at", pawn_json_object_get_value_at},
	{"json_object_has_key", pawn_json_object_has_key},
	{"json_object_has_value", pawn_json_object_has_value},
	{"json_object_is_null", pawn_json_object_is_null},
	{"json_object_set_value", pawn_json_object_set_value},
	{"json_object_set_string", pawn_json_object_set_string},
	{"json_object_set_integer", pawn_json_object_set_integer},
	{"json_object_set_integer64", pawn_json_object_set_integer64},
	{"json_object_set_float", pawn_json_object_set_float},
	{"json_object_set_bool", pawn_json_object_set_bool},
	{"json_object_set_null", pawn_json_object_set_null},
	{"json_object_remove", pawn_json_object_remove},
	{"json_object_clear", pawn_json_object_clear},
	{"json_serial_size", pawn_json_serial_size},
	{"json_serial_to_string", pawn_json_serial_to_string},
	{"json_serial_to_file", pawn_json_serial_to_file},
	{"json_set_escape_slashes", pawn_json_set_escape_slashes},
	{"json_set_float_serialize", pawn_json_set_float_serialize},

	// JSONObject
	{"JSONObject.JSONObject", pawn_json_init_object},
	{"JSONObject.Size.get", pawn_json_object_get_count},
	{"JSONObject.Get", pawn_json_object_get_value},
	{"JSONObject.GetBool", pawn_json_object_get_bool},
	{"JSONObject.GetFloat", pawn_json_object_get_float},
	{"JSONObject.GetInt", pawn_json_object_get_integer},
	{"JSONObject.GetInt64", pawn_json_object_get_integer64},
	{"JSONObject.GetString", pawn_json_object_get_string},
	{"JSONObject.GetStringLength", pawn_json_object_get_string_length},
	{"JSONObject.Clear", pawn_json_object_clear},
	{"JSONObject.IsNull", pawn_json_object_is_null},
	{"JSONObject.GetName", pawn_json_object_get_name},
	{"JSONObject.GetValueAt", pawn_json_object_get_value_at},
	{"JSONObject.HasKey", pawn_json_object_has_key},
	{"JSONObject.HasValue", pawn_json_object_has_value},
	{"JSONObject.Set", pawn_json_object_set_value},
	{"JSONObject.SetBool", pawn_json_object_set_bool},
	{"JSONObject.SetFloat", pawn_json_object_set_float},
	{"JSONObject.SetInt", pawn_json_object_set_integer},
	{"JSONObject.SetInt64", pawn_json_object_set_integer64},
	{"JSONObject.SetNull", pawn_json_object_set_null},
	{"JSONObject.SetString", pawn_json_object_set_string},
	{"JSONObject.Remove", pawn_json_object_remove},
	{"JSONObject.Clear", pawn_json_object_clear},
	{"JSONObject.FromString", pawn_json_object_from_string},
	{"JSONObject.FromFile", pawn_json_object_from_file},

	// JSONArray
	{"JSONArray.JSONArray", pawn_json_init_array},
	{"JSONArray.Length.get", pawn_json_array_get_count},
	{"JSONArray.Get", pawn_json_array_get_value},
	{"JSONArray.GetBool", pawn_json_array_get_bool},
	{"JSONArray.GetFloat", pawn_json_array_get_float},
	{"JSONArray.GetInt", pawn_json_array_get_integer},
	{"JSONArray.GetInt64", pawn_json_array_get_integer64},
	{"JSONArray.GetString", pawn_json_array_get_string},
	{"JSONArray.GetStringLength", pawn_json_array_get_string_length},
	{"JSONArray.IsNull", pawn_json_array_is_null},
	{"JSONArray.Set", pawn_json_array_replace_value},
	{"JSONArray.SetBool", pawn_json_array_replace_bool},
	{"JSONArray.SetFloat", pawn_json_array_replace_float},
	{"JSONArray.SetInt", pawn_json_array_replace_integer},
	{"JSONArray.SetInt64", pawn_json_array_replace_integer64},
	{"JSONArray.SetNull", pawn_json_array_replace_null},
	{"JSONArray.SetString", pawn_json_array_replace_string},
	{"JSONArray.Push", pawn_json_array_append_value},
	{"JSONArray.PushBool", pawn_json_array_append_bool},
	{"JSONArray.PushFloat", pawn_json_array_append_float},
	{"JSONArray.PushInt", pawn_json_array_append_integer},
	{"JSONArray.PushInt64", pawn_json_array_append_integer64},
	{"JSONArray.PushNull", pawn_json_array_append_null},
	{"JSONArray.PushString", pawn_json_array_append_string},
	{"JSONArray.Remove", pawn_json_array_remove},
	{"JSONArray.Clear", pawn_json_array_clear},
	{"JSONArray.FromString", pawn_json_array_from_string},
	{"JSONArray.FromFile", pawn_json_array_from_file},

	// JSON
	{"JSON.ToString", pawn_json_serial_to_string},
	{"JSON.ToFile", pawn_json_serial_to_file},
	{"JSON.Parse", pawn_json_parse},
	{"JSON.Equals", pawn_json_equals},
	{"JSON.Validate", pawn_json_validate},
	{"JSON.GetParent", pawn_json_get_parent},
	{"JSON.String", pawn_json_init_string},
	{"JSON.Int", pawn_json_init_integer},
	{"JSON.Int64", pawn_json_init_integer64},
	{"JSON.Float", json_init_float},
	{"JSON.Bool", pawn_json_init_bool},
	{"JSON.Null", pawn_json_init_null},
	{"JSON.DeepCopy", pawn_json_deep_copy},
	{"JSON.GetString", pawn_json_get_string},
	{"JSON.GetStringLength", pawn_json_get_string_length},
	{"JSON.GetInt", pawn_json_get_integer},
	{"JSON.GetFloat", pawn_json_get_float},
	{"JSON.GetBool", pawn_json_get_bool},
	{"JSON.SerialSize", pawn_json_serial_size},
	{"JSON.EscapeSlashes", pawn_json_set_escape_slashes},
	{"JSON.FloatSerialize", pawn_json_set_float_serialize},
	{"JSON.Type.get", pawn_json_get_type},
	{"JSON.SerialSize.get", pawn_json_serialization_size},
	{"JSON.SerialSizePretty.get", pawn_json_serialization_size_pretty},
	{nullptr,	nullptr}
};