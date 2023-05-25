#include "extension.h"

static JSON_Value *GetJSONFromHandle(IPluginContext *pContext, Handle_t hndl)
{
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());

	JSON_Value *json;
	if ((err = handlesys->ReadHandle(hndl, htJSON, &sec, (void **)&json)) != HandleError_None)
	{
		pContext->ReportError("Invalid JSON handle %x (error %d)", hndl, err);
		return nullptr;
	}

	return json;
}

static cell_t pawn_json_parse(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	bool is_file = params[2];
	bool with_comments = params[3];
	
	JSON_Value *handle;

	if (is_file)
	{
		char realpath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", string);
		handle = with_comments ? json_parse_file(realpath) : json_parse_file_with_comments(realpath);
	}
	else
	{
		handle = with_comments ? json_parse_string(string) : json_parse_string_with_comments(string);
	}
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON parse handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_object_from_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	bool with_comments = params[2];
	
	JSON_Value *handle = with_comments ? json_parse_string_with_comments(string) : json_parse_string(string);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_from_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_object_from_file(IPluginContext *pContext, const cell_t *params)
{
	char *path;
	pContext->LocalToString(params[1], &path);

	bool with_comments = params[2];

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	JSON_Value *handle = with_comments ? json_parse_file_with_comments(realpath) : json_parse_file(realpath);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_from_file handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_array_from_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);

	bool with_comments = params[2];
	
	JSON_Value *handle = with_comments ? json_parse_string_with_comments(string) : json_parse_string(string);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON array_from_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_array_from_file(IPluginContext *pContext, const cell_t *params)
{
	char *path;
	pContext->LocalToString(params[1], &path);

	bool with_comments = params[2];

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	JSON_Value *handle = with_comments ? json_parse_file_with_comments(realpath) : json_parse_file(realpath);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON array_from_file handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_equals(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle1 = GetJSONFromHandle(pContext, params[1]);
	JSON_Value *handle2 = GetJSONFromHandle(pContext, params[2]);

	if (handle1 == nullptr || handle2 == nullptr) return BAD_HANDLE;

	return json_value_equals(handle1, handle2) == 1;
}

static cell_t pawn_json_validate(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle1 = GetJSONFromHandle(pContext, params[1]);
	JSON_Value *handle2 = GetJSONFromHandle(pContext, params[2]);

	if (handle1 == nullptr || handle2 == nullptr) return BAD_HANDLE;

	return json_validate(handle1, handle2) == JSONSuccess;
}

static cell_t pawn_json_get_parent(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Value *parent = json_value_get_parent(handle);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, parent, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON get_parent handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_get_type(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	return json_value_get_type(handle);
}

static cell_t pawn_json_init_object(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = json_value_init_object();

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_object handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_init_array(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = json_value_init_array();

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_array handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_init_string(IPluginContext *pContext, const cell_t *params)
{
	char *string;
	pContext->LocalToString(params[1], &string);
	JSON_Value *handle = json_value_init_string(string);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_string handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_init_number(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = json_value_init_number(params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_number handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_init_real(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = json_value_init_number(sp_ctof(params[1]));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_real handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_init_bool(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = json_value_init_boolean(params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_boolean handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_init_null(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = json_value_init_null();

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, handle, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON init_null handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_deep_copy(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Value *jsonDeep = json_value_deep_copy(handle);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, jsonDeep, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON deep_copy handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_get_string(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	const char *string = json_value_get_string(handle);

	if (string == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[2], params[3], string, nullptr);

	return 1;
}

static cell_t pawn_json_get_number(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	return json_value_get_number(handle);
}

static cell_t pawn_json_get_real(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	float result = static_cast<float>(json_value_get_number(handle));

	return sp_ftoc(result);
}

static cell_t pawn_json_get_bool(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	return json_value_get_boolean(handle);
}

static cell_t pawn_json_array_get_value(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);
	JSON_Value *result = json_array_get_value(array, params[2]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, result, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON array_get_value handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_array_get_string(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	const char *string = json_array_get_string(array, params[2]);

	if (string == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[3], params[4], string, nullptr);

	return 1;
}

static cell_t pawn_json_array_is_null(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	JSON_Value *value = json_array_get_value(array, params[2]);

	return json_value_get_type(value) == JSONNull;
}

static cell_t pawn_json_array_get_number(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_get_number(array, params[2]);
}

static cell_t pawn_json_array_get_real(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	float result = static_cast<float>(json_array_get_number(array, params[2]));

	return sp_ftoc(result);
}

static cell_t pawn_json_array_get_bool(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_get_boolean(array, params[2]);
}

static cell_t pawn_json_array_get_count(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_get_count(array);
}

static cell_t pawn_json_array_replace_value(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle1 = GetJSONFromHandle(pContext, params[1]);

	if (handle1 == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle1);

	JSON_Value *handle2 = GetJSONFromHandle(pContext, params[3]);

	if (handle2 == nullptr) return BAD_HANDLE;

	if (json_value_get_parent(handle2))
	{
		handle2 = json_value_deep_copy(handle2);
	}

	return json_array_replace_value(array, params[2], handle2) == JSONSuccess;
}

static cell_t pawn_json_array_replace_string(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);
	
	char *string;
	pContext->LocalToString(params[3], &string);

	return json_array_replace_string(array, params[2], string) == JSONSuccess;
}

static cell_t pawn_json_array_replace_number(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_replace_number(array, params[2], params[3]) == JSONSuccess;
}

static cell_t pawn_json_array_replace_real(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_replace_number(array, params[2], sp_ctof(params[3])) == JSONSuccess;
}

static cell_t pawn_json_array_replace_bool(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_replace_boolean(array, params[2], params[3] != 0) == JSONSuccess;
}

static cell_t pawn_json_array_replace_null(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_replace_null(array, params[2]) == JSONSuccess;
}

static cell_t pawn_json_array_append_value(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle1 = GetJSONFromHandle(pContext, params[1]);

	if (handle1 == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle1);

	JSON_Value *value = GetJSONFromHandle(pContext, params[2]);

	if (value == nullptr) return BAD_HANDLE;

	if (json_value_get_parent(value))
	{
		value = json_value_deep_copy(value);
	}

	return json_array_append_value(array, value) == JSONSuccess;
}

static cell_t pawn_json_array_append_string(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	char *string;
	pContext->LocalToString(params[2], &string);
	return json_array_append_string(array, string) == JSONSuccess;
}

static cell_t pawn_json_array_append_number(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_append_number(array, params[2]) == JSONSuccess;
}

static cell_t pawn_json_array_append_real(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_append_number(array, sp_ctof(params[2])) == JSONSuccess;
}

static cell_t pawn_json_array_append_bool(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_append_boolean(array, params[2] != 0) == JSONSuccess;
}

static cell_t pawn_json_array_append_null(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;
	
	JSON_Array *array = json_value_get_array(handle);

	return json_array_append_null(array) == JSONSuccess;
}

static cell_t pawn_json_array_remove(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_remove(array, params[2]) == JSONSuccess;
}

static cell_t pawn_json_array_clear(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Array *array = json_value_get_array(handle);

	return json_array_clear(array) == JSONSuccess;
}

static cell_t pawn_json_object_get_value(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);
	
	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Value *result = is_dot ? json_object_dotget_value(object, key) : json_object_get_value(object, key);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, result, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_get_value handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_object_get_string(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);
	
	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[5];

	const char *string = is_dot ? json_object_dotget_string(object, key) : json_object_get_string(object, key);

	if (string == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[3], params[4], string, nullptr);

	return 1;
}

static cell_t pawn_json_object_get_number(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	double result = is_dot ? json_object_dotget_number(object, key) : json_object_get_number(object, key);

	return result;
}

static cell_t pawn_json_object_get_real(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	double data = is_dot ? json_object_dotget_number(object, key) : json_object_get_number(object, key);
	float result = static_cast<float>(data);

	return sp_ftoc(result);
}

static cell_t pawn_json_object_get_bool(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	int result = is_dot ? json_object_dotget_boolean(object, key) : json_object_get_boolean(object, key);

	return result;
}

static cell_t pawn_json_object_get_count(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	return json_object_get_count(object);
}

static cell_t pawn_json_object_get_name(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	const char *name = json_object_get_name(object, params[2]);

	if (name == nullptr) return 0;
	
	pContext->StringToLocalUTF8(params[3], params[4], name, nullptr);

	return 1;
}

static cell_t pawn_json_object_get_value_at(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	JSON_Value *result = json_object_get_value_at(object, params[2]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	Handle_t hndl = handlesys->CreateHandleEx(htJSON, result, &sec, nullptr, &err);
	
	if (hndl == BAD_HANDLE)
	{
		pContext->ReportError("Could not create JSON object_get_value_at handle (error %d)", err);
		return BAD_HANDLE;
	}
	
	return hndl;
}

static cell_t pawn_json_object_has_value(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	int result;

	bool is_dot = params[4];

	if (params[3] == JSONError)
	{
		result = is_dot ? json_object_dothas_value(object, key) : json_object_has_value(object, key);
	}
	else
	{
		result = is_dot ? json_object_dothas_value_of_type(object, key, params[3]) : json_object_has_value_of_type(object, key, params[3]);
	}

	return result;
}

static cell_t pawn_json_object_has_key(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	return json_object_get_value(object, key) != nullptr;
}

static cell_t pawn_json_object_set_value(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle1 = GetJSONFromHandle(pContext, params[1]);

	if (handle1 == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle1);
	JSON_Value *handle2 = GetJSONFromHandle(pContext, params[3]);

	if (handle2 == nullptr) return BAD_HANDLE;

	char *key;
	pContext->LocalToString(params[2], &key);

	if (json_value_get_parent(handle2))
	{
		handle2 = json_value_deep_copy(handle2);
	}

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_value(object, key, handle2) : json_object_set_value(object, key, handle2);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_string(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key, *value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_string(object, key, value) : json_object_set_string(object, key, value);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_number(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_number(object, key, params[3]) : json_object_set_number(object, key, params[3]);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_real(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;
	
	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_number(object, key, sp_ctof(params[3])) : json_object_set_number(object, key, sp_ctof(params[3]));

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_bool(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[4];

	JSON_Status result = is_dot ? json_object_dotset_boolean(object, key, params[3]) : json_object_set_boolean(object, key, params[3]);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_set_null(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;
	
	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Status result = is_dot ? json_object_dotset_null(object, key) : json_object_set_null(object, key);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_remove(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Status result = is_dot ? json_object_dotremove(object, key) : json_object_remove(object, key);

	return result == JSONSuccess;
}

static cell_t pawn_json_object_clear(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	return json_object_clear(object) == JSONSuccess;
}

static cell_t pawn_json_object_is_null(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	JSON_Object *object = json_value_get_object(handle);

	char *key;
	pContext->LocalToString(params[2], &key);

	bool is_dot = params[3];

	JSON_Value *value = is_dot ? json_object_dotget_value(object, key) : json_object_get_value(object, key);

	return json_value_get_type(value) == JSONNull;
}

static cell_t pawn_json_serial_size(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	bool pretty = params[2];

	size_t result = pretty ? json_serialization_size_pretty(handle) : json_serialization_size(handle);

	return (params[3]) ? result : result - 1;
}

static cell_t pawn_json_serial_to_string(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;

	bool pretty = params[4];

	char *result = pretty ? json_serialize_to_string_pretty(handle) : json_serialize_to_string(handle);

	if (result == nullptr) return 0;

	pContext->StringToLocalUTF8(params[2], params[3], result, nullptr);

	json_free_serialized_string(result);

	return 1;
}

static cell_t pawn_json_serial_to_file(IPluginContext *pContext, const cell_t *params)
{
	JSON_Value *handle = GetJSONFromHandle(pContext, params[1]);

	if (handle == nullptr) return BAD_HANDLE;
	
	char *path;
	pContext->LocalToString(params[2], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);

	bool pretty = params[3];

	JSON_Status JSResult = pretty ? json_serialize_to_file_pretty(handle, realpath) : json_serialize_to_file(handle, realpath);

	return JSResult == JSONSuccess;
}

const sp_nativeinfo_t JsonNatives[] =
{
	{"json_parse", pawn_json_parse},
	{"json_equals", pawn_json_equals},
	{"json_validate", pawn_json_validate},
	{"json_get_parent", pawn_json_get_parent},
	{"json_get_type", pawn_json_get_type},
	{"json_init_object", pawn_json_init_object},
	{"json_init_array", pawn_json_init_array},
	{"json_init_string", pawn_json_init_string},
	{"json_init_number", pawn_json_init_number},
	{"json_init_real", pawn_json_init_real},
	{"json_init_bool", pawn_json_init_bool},
	{"json_init_null", pawn_json_init_null},
	{"json_deep_copy", pawn_json_deep_copy},
	{"json_get_string", pawn_json_get_string},
	{"json_get_number", pawn_json_get_number},
	{"json_get_real", pawn_json_get_real},
	{"json_get_bool", pawn_json_get_bool},
	{"json_array_get_value", pawn_json_array_get_value},
	{"json_array_get_string", pawn_json_array_get_string},
	{"json_array_get_count", pawn_json_array_get_count},
	{"json_array_get_number", pawn_json_array_get_number},
	{"json_array_get_real", pawn_json_array_get_real},
	{"json_array_get_bool", pawn_json_array_get_bool},
	{"json_array_is_null", pawn_json_array_is_null},
	{"json_array_replace_value", pawn_json_array_replace_value},
	{"json_array_replace_string", pawn_json_array_replace_string},
	{"json_array_replace_number", pawn_json_array_replace_number},
	{"json_array_replace_real", pawn_json_array_replace_real},
	{"json_array_replace_bool", pawn_json_array_replace_bool},
	{"json_array_replace_null", pawn_json_array_replace_null},
	{"json_array_append_value", pawn_json_array_append_value},
	{"json_array_append_string", pawn_json_array_append_string},
	{"json_array_append_number", pawn_json_array_append_number},
	{"json_array_append_real", pawn_json_array_append_real},
	{"json_array_append_bool", pawn_json_array_append_bool},
	{"json_array_append_null", pawn_json_array_append_null},
	{"json_array_remove", pawn_json_array_remove},
	{"json_array_clear", pawn_json_array_clear},
	{"json_object_get_value", pawn_json_object_get_value},
	{"json_object_get_string", pawn_json_object_get_string},
	{"json_object_get_number", pawn_json_object_get_number},
	{"json_object_get_real", pawn_json_object_get_real},
	{"json_object_get_bool", pawn_json_object_get_bool},
	{"json_object_get_count", pawn_json_object_get_count},
	{"json_object_get_name", pawn_json_object_get_name},
	{"json_object_get_value_at", pawn_json_object_get_value_at},
	{"json_object_has_value", pawn_json_object_has_value},
	{"json_object_has_key", pawn_json_object_has_key},
	{"json_object_is_null", pawn_json_object_is_null},
	{"json_object_set_value", pawn_json_object_set_value},
	{"json_object_set_string", pawn_json_object_set_string},
	{"json_object_set_number", pawn_json_object_set_number},
	{"json_object_set_real", pawn_json_object_set_real},
	{"json_object_set_bool", pawn_json_object_set_bool},
	{"json_object_set_null", pawn_json_object_set_null},
	{"json_object_remove", pawn_json_object_remove},
	{"json_object_clear", pawn_json_object_clear},
	{"json_serial_size", pawn_json_serial_size},
	{"json_serial_to_string", pawn_json_serial_to_string},
	{"json_serial_to_file", pawn_json_serial_to_file},

	// JSONObject
	{"JSONObject.JSONObject", pawn_json_init_object},
	{"JSONObject.Size.get", pawn_json_object_get_count},
	{"JSONObject.Get", pawn_json_object_get_value},
	{"JSONObject.GetBool", pawn_json_object_get_bool},
	{"JSONObject.GetFloat", pawn_json_object_get_real},
	{"JSONObject.GetInt", pawn_json_object_get_number},
	{"JSONObject.GetString", pawn_json_object_get_string},
	{"JSONObject.Clear", pawn_json_object_clear},
	{"JSONObject.IsNull", pawn_json_object_is_null},
	{"JSONObject.GetName", pawn_json_object_get_name},
	{"JSONObject.GetValueAt", pawn_json_object_get_value_at},
	{"JSONObject.HasValue", pawn_json_object_has_value},
	{"JSONObject.HasKey", pawn_json_object_has_key},
	{"JSONObject.Set", pawn_json_object_set_value},
	{"JSONObject.SetBool", pawn_json_object_set_bool},
	{"JSONObject.SetFloat", pawn_json_object_set_real},
	{"JSONObject.SetInt", pawn_json_object_set_number},
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
	{"JSONArray.GetFloat", pawn_json_array_get_real},
	{"JSONArray.GetInt", pawn_json_array_get_number},
	{"JSONArray.GetString", pawn_json_array_get_string},
	{"JSONArray.IsNull", pawn_json_array_is_null},
	{"JSONArray.Set", pawn_json_array_replace_value},
	{"JSONArray.SetBool", pawn_json_array_replace_bool},
	{"JSONArray.SetFloat", pawn_json_array_replace_real},
	{"JSONArray.SetInt", pawn_json_array_replace_number},
	{"JSONArray.SetNull", pawn_json_array_replace_null},
	{"JSONArray.SetString", pawn_json_array_replace_string},
	{"JSONArray.Push", pawn_json_array_append_value},
	{"JSONArray.PushBool", pawn_json_array_append_bool},
	{"JSONArray.PushFloat", pawn_json_array_append_real},
	{"JSONArray.PushInt", pawn_json_array_append_number},
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
	{"JSON.Int", pawn_json_init_number},
	{"JSON.Float", pawn_json_init_real},
	{"JSON.Bool", pawn_json_init_bool},
	{"JSON.Null", pawn_json_init_null},
	{"JSON.DeepCopy", pawn_json_deep_copy},
	{"JSON.GetString", pawn_json_get_string},
	{"JSON.GetInt", pawn_json_get_number},
	{"JSON.GetFloat", pawn_json_get_real},
	{"JSON.GetBool", pawn_json_get_bool},
	{"JSON.SerialSize", pawn_json_serial_size},
	{"JSON.Type.get", pawn_json_get_type},
	{nullptr,	nullptr}
};