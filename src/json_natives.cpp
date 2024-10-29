#include "extension.h"

static cell_t json_doc_parse(IPluginContext* pContext, const cell_t* params)
{
	char* string;
	pContext->LocalToString(params[1], &string);

	bool is_file = params[2];

	yyjson_read_err readError;
	yyjson_doc* idoc;

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

	if (is_file)
	{
		char realpath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", string);
		idoc = yyjson_read_file(realpath, params[3], nullptr, &readError);

		if (readError.code) {
			pContext->ReportError("Failed to parse JSON file: %s (error code: %u, msg: %s, position: %d)",
				string, readError.code, readError.msg, readError.pos);
			return BAD_HANDLE;
		}

		pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, nullptr);
		pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut);
	}
	else
	{
		idoc = yyjson_read_opts(string, strlen(string), params[3], nullptr, &readError);

		if (readError.code) {
			pContext->ReportError("Failed to parse JSON string: %s (error code: %u, position: %d, content: %.32s...)",
			readError.msg, readError.code, readError.pos, string);
			return BAD_HANDLE;
		}

		pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, nullptr);
	}

	yyjson_doc_free(idoc);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_doc_equals(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	return yyjson_mut_equals(handle1->m_pVal_mut, handle2->m_pVal_mut);
}

static cell_t json_doc_copy_deep(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for deep copy of JSON value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_get_type_desc(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_type_desc(handle->m_pVal_mut), nullptr);

	return 1;
}

static cell_t json_obj_parse_str(IPluginContext* pContext, const cell_t* params)
{
	char* string;
	pContext->LocalToString(params[1], &string);

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_opts(string, strlen(string), params[2], nullptr, &readError);

	if (readError.code) {
		pContext->ReportError("Failed to parse JSON string: %s (error code: %u, position: %d, content: %.32s...)",
			readError.msg, readError.code, readError.pos, string);
		delete pYYJsonWrapper;
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, nullptr);
	yyjson_doc_free(idoc);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON object from string (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_obj_parse_file(IPluginContext* pContext, const cell_t* params)
{
	char* path;
	pContext->LocalToString(params[1], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_file(realpath, params[2], nullptr, &readError);

	if (readError.code) {
		pContext->ReportError("Failed to parse JSON file: %s (error code: %u, msg: %s, position: %d)",
			realpath, readError.code, readError.msg, readError.pos);
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut);
	yyjson_doc_free(idoc);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON object from file (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_arr_parse_str(IPluginContext* pContext, const cell_t* params)
{
	char* string;
	pContext->LocalToString(params[1], &string);

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_opts(string, strlen(string), params[2], nullptr, &readError);

	if (readError.code) {
		pContext->ReportError("Failed to parse JSON string: %s (error code: %u, position: %d, content: %.32s...)",
			readError.msg, readError.code, readError.pos, string);
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, nullptr);
	yyjson_doc_free(idoc);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON array from string (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_arr_parse_file(IPluginContext* pContext, const cell_t* params)
{
	char* path;
	pContext->LocalToString(params[1], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_file(realpath, params[2], nullptr, &readError);

	if (readError.code) {
		pContext->ReportError("Failed to parse JSON file: %s (error code: %u, msg: %s, position: %d)",
			realpath, readError.code, readError.msg, readError.pos);
		return BAD_HANDLE;
	}

	pYYJsonWrapper->m_pDocument_mut = yyjson_doc_mut_copy(idoc, nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut);
	yyjson_doc_free(idoc);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON array from file (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_get_type(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_get_type(handle->m_pVal_mut);
}

static cell_t json_val_get_subtype(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_get_subtype(handle->m_pVal_mut);
}

static cell_t json_val_is_array(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_is_arr(handle->m_pVal_mut);
}

static cell_t json_val_is_object(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_is_obj(handle->m_pVal_mut);
}

static cell_t json_obj_init(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj(pYYJsonWrapper->m_pDocument_mut);

	yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut, pYYJsonWrapper->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for initialized JSON object (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_create_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_bool(pYYJsonWrapper->m_pDocument_mut, params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON boolean value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_create_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_float(pYYJsonWrapper->m_pDocument_mut, sp_ctof(params[1]));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON float value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_create_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_int(pYYJsonWrapper->m_pDocument_mut, params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON integer value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_create_intger64(IPluginContext* pContext, const cell_t* params)
{
	char* val;
	pContext->LocalToString(params[1], &val);

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_uint(pYYJsonWrapper->m_pDocument_mut, strtoll(val, nullptr, 10));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON integer64 value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_create_str(IPluginContext* pContext, const cell_t* params)
{
	char* string;
	pContext->LocalToString(params[1], &string);

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_strcpy(pYYJsonWrapper->m_pDocument_mut, string);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON string value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_val_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_bool(handle->m_pVal_mut)) {
		pContext->ReportError("Type mismatch: expected boolean value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
		return 0;
	}

	return yyjson_mut_get_bool(handle->m_pVal_mut);
}

static cell_t json_val_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_real(handle->m_pVal_mut)) {
		pContext->ReportError("Type mismatch: expected float value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
		return 0;
	}

	return sp_ftoc(yyjson_mut_get_real(handle->m_pVal_mut));
}

static cell_t json_val_get_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_int(handle->m_pVal_mut)) {
		pContext->ReportError("Type mismatch: expected integer value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
		return 0;
	}

	return yyjson_mut_get_int(handle->m_pVal_mut);
}

static cell_t json_val_get_intger64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_int(handle->m_pVal_mut)) {
		pContext->ReportError("Type mismatch: expected integer64 value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
		return 0;
	}

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(handle->m_pVal_mut));
	pContext->StringToLocalUTF8(params[2], params[3], result, nullptr);

	return 1;
}

static cell_t json_val_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_str(handle->m_pVal_mut)) {
		pContext->ReportError("Type mismatch: expected string value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
		return 0;
	}

	pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_str(handle->m_pVal_mut), nullptr);

	return 1;
}

static cell_t json_val_get_serialized_size(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	size_t json_size;

	char* json_str = yyjson_mut_val_write(handle->m_pVal_mut, 0, &json_size);

	if (json_str) {
		free(json_str);
		return json_size + 1;
	}

	return 0;
}

static cell_t json_val_create_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_null(pYYJsonWrapper->m_pDocument_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON null value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_arr_init(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pDocument_mut = yyjson_mut_doc_new(nullptr);
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr(pYYJsonWrapper->m_pDocument_mut);
	yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut, pYYJsonWrapper->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for initialized JSON array (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_arr_get_size(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_size(handle->m_pVal_mut);
}

static cell_t json_arr_get_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON array value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_arr_get_first(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr_get_first(handle->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for first JSON array value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_arr_get_last(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr_get_last(handle->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for last JSON array value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_arr_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val* arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	if (!yyjson_mut_is_bool(arr)) {
		pContext->ReportError("Type mismatch at index %d: expected boolean value, got %s",
			params[2], yyjson_mut_get_type_desc(arr));
		return 0;
	}

	return yyjson_mut_get_bool(arr);
}

static cell_t json_arr_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val* arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	if (!yyjson_mut_is_real(arr)) {
		pContext->ReportError("Type mismatch at index %d: expected float value, got %s",
			params[2], yyjson_mut_get_type_desc(arr));
		return 0;
	}

	return sp_ftoc(yyjson_mut_get_real(arr));
}

static cell_t json_arr_get_integer(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val* arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	if (!yyjson_mut_is_int(arr)) {
		pContext->ReportError("Type mismatch at index %d: expected integer value, got %s",
			params[2], yyjson_mut_get_type_desc(arr));
		return 0;
	}

	return yyjson_mut_get_int(arr);
}

static cell_t json_arr_get_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val* arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	if (!yyjson_mut_is_int(arr)) {
		pContext->ReportError("Type mismatch at index %d: expected integer64 value, got %s",
			params[2], yyjson_mut_get_type_desc(arr));
		return 0;
	}

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(arr));
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t json_arr_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val* arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	if (!yyjson_mut_is_str(arr)) {
		pContext->ReportError("Type mismatch at index %d: expected string value, got %s",
			params[2], yyjson_mut_get_type_desc(arr));
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(arr), nullptr);

	return 1;
}

static cell_t json_arr_is_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_val* arr = yyjson_mut_arr_get(handle->m_pVal_mut, params[2]);

	if (!arr) return BAD_HANDLE;

	return yyjson_mut_is_null(arr);
}

static cell_t json_arr_replace_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], handle->m_pVal_mut) != nullptr;
}

static cell_t json_arr_replace_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_bool(handle->m_pDocument_mut, params[3])) != nullptr;
}

static cell_t json_arr_replace_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[3]))) != nullptr;
}

static cell_t json_arr_replace_integer(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_int(handle->m_pDocument_mut, params[3])) != nullptr;
}

static cell_t json_arr_replace_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* val;
	pContext->LocalToString(params[3], &val);

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_uint(handle->m_pDocument_mut, strtoll(val, nullptr, 10))) != nullptr;
}

static cell_t json_arr_replace_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_null(handle->m_pDocument_mut)) != nullptr;
}

static cell_t json_arr_replace_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* val;
	pContext->LocalToString(params[3], &val);

	return yyjson_mut_arr_replace(handle->m_pVal_mut, params[2], yyjson_mut_strcpy(handle->m_pDocument_mut, val)) != nullptr;
}

static cell_t json_arr_append_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle1->m_pVal_mut, handle2->m_pVal_mut);
}

static cell_t json_arr_append_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_bool(handle->m_pDocument_mut, params[2]));
}

static cell_t json_arr_append_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[2])));
}

static cell_t json_arr_append_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_int(handle->m_pDocument_mut, params[2]));
}

static cell_t json_arr_append_intger64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* val;
	pContext->LocalToString(params[2], &val);

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_uint(handle->m_pDocument_mut, strtoll(val, nullptr, 10)));
}

static cell_t json_arr_append_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_null(handle->m_pDocument_mut));
}

static cell_t json_arr_append_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* str;
	pContext->LocalToString(params[2], &str);

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut, str));
}

static cell_t json_arr_remove(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove(handle->m_pVal_mut, params[2]) != nullptr;
}

static cell_t json_arr_remove_first(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove_first(handle->m_pVal_mut) != nullptr;
}

static cell_t json_arr_remove_last(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove_last(handle->m_pVal_mut) != nullptr;
}

static cell_t json_arr_remove_range(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_remove_range(handle->m_pVal_mut, params[2], params[3]);
}

static cell_t json_arr_clear(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_arr_clear(handle->m_pVal_mut);
}

static cell_t json_doc_write_to_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* json_str = yyjson_mut_val_write(handle->m_pVal_mut, params[4], nullptr);

	if (json_str) {
		pContext->StringToLocalUTF8(params[2], params[3], json_str, nullptr);

		free(json_str);
		return 1;
	}

	return 0;
}

static cell_t json_doc_write_to_file(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);

	yyjson_write_err writeError;

	bool is_success = yyjson_mut_write_file(realpath, handle->m_pDocument_mut, params[3], nullptr, &writeError);

	if (writeError.code) {
		pContext->ReportError("Failed to write JSON to file: %s (error code: %u)", writeError.msg, writeError.code);
		return BAD_HANDLE;
	}

	return is_success;
}

static cell_t json_obj_get_size(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	return yyjson_mut_obj_size(handle->m_pVal_mut);
}

static cell_t json_obj_get_key(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_obj_iter iter;
	yyjson_mut_obj_iter_init(handle->m_pVal_mut, &iter);

	for (size_t i = 0; i < params[2]; i++) {
		yyjson_mut_obj_iter_next(&iter);
	}

	yyjson_mut_val* key = yyjson_mut_obj_iter_next(&iter);
	if (!key) {
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(key), nullptr);

	return 1;
}

static cell_t json_obj_get_val_at(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	yyjson_mut_obj_iter iter;
	yyjson_mut_obj_iter_init(handle->m_pVal_mut, &iter);

	for (size_t i = 0; i < params[2]; i++) {
		yyjson_mut_obj_iter_next(&iter);
	}

	yyjson_mut_val* key = yyjson_mut_obj_iter_next(&iter);
	if (!key) {
		return 0;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj_iter_get_val(key);
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON object value at index %d (error code: %d)", params[2], err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_obj_get_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON object value with key '%s' (error code: %d)", key, err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_obj_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	if (!yyjson_mut_is_bool(obj)) {
		pContext->ReportError("Type mismatch for key '%s': expected boolean value, got %s", key, yyjson_mut_get_type_desc(obj));
		return 0;
	}

	return yyjson_mut_get_bool(obj);
}

static cell_t json_obj_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	if (!yyjson_mut_is_real(obj)) {
		pContext->ReportError("Type mismatch for key '%s': expected float value, got %s", key, yyjson_mut_get_type_desc(obj));
		return 0;
	}

	return sp_ftoc(yyjson_mut_get_real(obj));
}

static cell_t json_obj_get_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	if (!yyjson_mut_is_int(obj)) {
		pContext->ReportError("Type mismatch for key '%s': expected integer value, got %s", key, yyjson_mut_get_type_desc(obj));
		return 0;
	}

	return yyjson_mut_get_int(obj);
}

static cell_t json_obj_get_intger64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	if (!yyjson_mut_is_int(obj)) {
		pContext->ReportError("Type mismatch for key '%s': expected integer64 value, got %s", key, yyjson_mut_get_type_desc(obj));
		return 0;
	}

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(obj));
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t json_obj_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	if (!yyjson_mut_is_str(obj)) {
		pContext->ReportError("Type mismatch for key '%s': expected string value, got %s", key, yyjson_mut_get_type_desc(obj));
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(obj), nullptr);

	return 1;
}

static cell_t json_obj_clear(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	return yyjson_mut_obj_clear(obj);
}

static cell_t json_obj_is_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* obj = yyjson_mut_obj_get(handle->m_pVal_mut, key);

	if (!obj) return BAD_HANDLE;

	return yyjson_mut_is_null(obj);
}

static cell_t json_obj_has_key(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_obj_iter iter = yyjson_mut_obj_iter_with(handle->m_pVal_mut);

	bool ptr_use = params[3];

	yyjson_mut_val* val = ptr_use ? yyjson_mut_doc_ptr_get(handle->m_pDocument_mut, key) : yyjson_mut_obj_iter_get(&iter, key);

	return val != nullptr;
}

static cell_t json_obj_rename_key(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key, * new_key;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &new_key);

	return yyjson_mut_obj_rename_key(handle->m_pDocument_mut, handle->m_pVal_mut, key, new_key);
}

static cell_t json_obj_set_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);

	return yyjson_mut_obj_put(handle1->m_pVal_mut, yyjson_mut_str(handle1->m_pDocument_mut, key), val_copy);
}

static cell_t json_obj_set_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_bool(handle->m_pDocument_mut, params[3]));
}

static cell_t json_obj_set_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[3])));
}

static cell_t json_obj_set_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_int(handle->m_pDocument_mut, params[3]));
}

static cell_t json_obj_set_intger64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key, * val;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &val);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_uint(handle->m_pDocument_mut, strtoll(val, nullptr, 10)));
}

static cell_t json_obj_set_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_null(handle->m_pDocument_mut));
}

static cell_t json_obj_set_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key, * value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_str(handle->m_pDocument_mut, key), yyjson_mut_strcpy(handle->m_pDocument_mut, value));
}

static cell_t json_obj_remove(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_remove_key(handle->m_pVal_mut, key) != nullptr;
}

static cell_t json_ptr_get_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		pContext->ReportError("Failed to create handle for JSON pointer value (error code: %d)", err);
		return BAD_HANDLE;
	}

	return pYYJsonWrapper->m_handle;
}

static cell_t json_ptr_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val* ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	if (!yyjson_mut_is_bool(ptr)) {
		pContext->ReportError("Type mismatch at path '%s': expected boolean value, got %s", path, yyjson_mut_get_type_desc(ptr));
		return 0;
	}

	return yyjson_mut_get_bool(ptr);
}

static cell_t json_ptr_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val* ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	if (!yyjson_mut_is_real(ptr)) {
		pContext->ReportError("Type mismatch at path '%s': expected float value, got %s", path, yyjson_mut_get_type_desc(ptr));
		return 0;
	}

	return sp_ftoc(yyjson_mut_get_real(ptr));
}

static cell_t json_ptr_get_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val* ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	if (!yyjson_mut_is_int(ptr)) {
		pContext->ReportError("Type mismatch at path '%s': expected integer value, got %s", path, yyjson_mut_get_type_desc(ptr));
		return 0;
	}

	return yyjson_mut_get_int(ptr);
}

static cell_t json_ptr_get_intger64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val* ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	if (!yyjson_mut_is_int(ptr)) {
		pContext->ReportError("Type mismatch at path '%s': expected integer64 value, got %s", path, yyjson_mut_get_type_desc(ptr));
		return 0;
	}

	char result[20];
	snprintf(result, sizeof(result), "%" PRIu64, yyjson_mut_get_uint(ptr));
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t json_ptr_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val* ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	if (!yyjson_mut_is_str(ptr)) {
		pContext->ReportError("Type mismatch at path '%s': expected string value, got %s", path, yyjson_mut_get_type_desc(ptr));
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(ptr), nullptr);

	return 1;
}

static cell_t json_ptr_get_is_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val* ptr = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	return yyjson_mut_is_null(ptr);
}

static cell_t json_ptr_get_length(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut, path, strlen(path), nullptr, &ptrGetError);

	if (ptrGetError.code) {
		pContext->ReportError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		return BAD_HANDLE;
	}

	return yyjson_mut_get_len(val);
}

static cell_t json_ptr_set_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_mut_val* val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle1->m_pDocument_mut, path, strlen(path), val_copy, true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_set_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_bool(handle->m_pDocument_mut, params[3]), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_set_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[3])), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_set_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, params[3]), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_set_intger64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path, * val;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &val);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, strtoll(val, nullptr, 10)), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_set_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path, * str;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &str);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_strcpy(handle->m_pDocument_mut, str), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_set_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_null(handle->m_pDocument_mut), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		pContext->ReportError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_add_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_mut_val* val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut, handle2->m_pVal_mut);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle1->m_pDocument_mut, path, strlen(path), val_copy, true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_add_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_bool(handle->m_pDocument_mut, params[3]), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_add_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_float(handle->m_pDocument_mut, sp_ctof(params[3])), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_add_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, params[3]), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_add_intger64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path, * val;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &val);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut, strtoll(val, nullptr, 10)), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_add_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path, * str;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &str);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_strcpy(handle->m_pDocument_mut, str), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_add_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut, path, strlen(path), yyjson_mut_null(handle->m_pDocument_mut), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		pContext->ReportError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_ptr_remove_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return BAD_HANDLE;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrRemoveError;
	bool success = yyjson_mut_ptr_removex(handle->m_pVal_mut, path, strlen(path), nullptr, &ptrRemoveError) != nullptr;

	if (ptrRemoveError.code) {
		pContext->ReportError("Failed to remove JSON pointer: %s (error code: %u, position: %d)", ptrRemoveError.msg, ptrRemoveError.code, ptrRemoveError.pos);
		return BAD_HANDLE;
	}

	return success;
}

static cell_t json_obj_foreach(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return BAD_HANDLE;

	// check type
	if (!yyjson_mut_is_obj(handle->m_pVal_mut)) {
		pContext->ReportError("Type mismatch: expected object value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
		return BAD_HANDLE;
	}

	yyjson_mut_obj_iter iter = yyjson_mut_obj_iter_with(handle->m_pVal_mut);
	yyjson_mut_val* key, * val;

	// skip iterated elements
	for (size_t i = 0; i < handle->m_iterIndex; i++) {
		if (!yyjson_mut_obj_iter_next(&iter)) {
			handle->ResetIterator();
			return false;
		}
	}

	if ((key = yyjson_mut_obj_iter_next(&iter)) != nullptr) {
		// Get key string
		pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_str(key), nullptr);

		// Create new wrapper for value
		val = yyjson_mut_obj_iter_get_val(key);
		YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
		pYYJsonWrapper->m_pVal_mut = val;
		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		// Mark as reference since this value shares the parent document. Prevents crashes on destruction
		pYYJsonWrapper->m_isReference = true;

		// Create handle
		HandleError err;
		HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
		Handle_t newHandle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

		if (!newHandle) {
			delete pYYJsonWrapper;
			pContext->ReportError("Failed to create handle for JSON object value (error code: %d)", err);
			return BAD_HANDLE;
		}

		pYYJsonWrapper->m_handle = newHandle;

		// Store value handle
		cell_t* valHandle;
		pContext->LocalToPhysAddr(params[4], &valHandle);
		*valHandle = newHandle;

		handle->m_iterIndex++;
		return true;
	}

	handle->ResetIterator();
	return false;
}

static cell_t json_arr_foreach(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return BAD_HANDLE;

	if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
		pContext->ReportError("Type mismatch: expected array value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
		return BAD_HANDLE;
	}

	yyjson_mut_arr_iter iter = yyjson_mut_arr_iter_with(handle->m_pVal_mut);
	yyjson_mut_val* val;

	for (size_t i = 0; i < handle->m_iterIndex; i++) {
		if (!yyjson_mut_arr_iter_next(&iter)) {
			handle->ResetIterator();
			return false;
		}
	}

	if ((val = yyjson_mut_arr_iter_next(&iter)) != nullptr) {
		// Store current index
		cell_t* index;
		pContext->LocalToPhysAddr(params[2], &index);
		*index = handle->m_iterIndex;

		YYJsonWrapper* pYYJsonWrapper = new YYJsonWrapper();
		pYYJsonWrapper->m_pVal_mut = val;
		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		// Arrays do not need to mark as reference. but it's harmless
		pYYJsonWrapper->m_isReference = true;

		HandleError err;
		HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
		pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper, &sec, nullptr, &err);

		if (!pYYJsonWrapper->m_handle) {
			delete pYYJsonWrapper;
			pContext->ReportError("Failed to create handle for JSON array value (error code: %d)", err);
			return BAD_HANDLE;
		}

		cell_t* valHandle;
		pContext->LocalToPhysAddr(params[3], &valHandle);
		*valHandle = pYYJsonWrapper->m_handle;

		handle->m_iterIndex++;
		return true;
	}

	handle->ResetIterator();
	return false;
}

const sp_nativeinfo_t json_natives[] =
{
	// JSONObject
	{"YYJSONObject.YYJSONObject", json_obj_init},
	{"YYJSONObject.Size.get", json_obj_get_size},
	{"YYJSONObject.Get", json_obj_get_val},
	{"YYJSONObject.GetBool", json_obj_get_bool},
	{"YYJSONObject.GetFloat", json_obj_get_float},
	{"YYJSONObject.GetInt", json_obj_get_int},
	{"YYJSONObject.GetInt64", json_obj_get_intger64},
	{"YYJSONObject.GetString", json_obj_get_str},
	{"YYJSONObject.Clear", json_obj_clear},
	{"YYJSONObject.IsNull", json_obj_is_null},
	{"YYJSONObject.GetKey", json_obj_get_key},
	{"YYJSONObject.GetValueAt", json_obj_get_val_at},
	{"YYJSONObject.HasKey", json_obj_has_key},
	{"YYJSONObject.RenameKey", json_obj_rename_key},
	{"YYJSONObject.Set", json_obj_set_val},
	{"YYJSONObject.SetBool", json_obj_set_bool},
	{"YYJSONObject.SetFloat", json_obj_set_float},
	{"YYJSONObject.SetInt", json_obj_set_int},
	{"YYJSONObject.SetInt64", json_obj_set_intger64},
	{"YYJSONObject.SetNull", json_obj_set_null},
	{"YYJSONObject.SetString", json_obj_set_str},
	{"YYJSONObject.Remove", json_obj_remove},
	{"YYJSONObject.Clear", json_obj_clear},
	{"YYJSONObject.FromString", json_obj_parse_str},
	{"YYJSONObject.FromFile", json_obj_parse_file},

	// JSONArray
	{"YYJSONArray.YYJSONArray", json_arr_init},
	{"YYJSONArray.Length.get", json_arr_get_size},
	{"YYJSONArray.Get", json_arr_get_val},
	{"YYJSONArray.First.get", json_arr_get_first},
	{"YYJSONArray.Last.get", json_arr_get_last},
	{"YYJSONArray.GetBool", json_arr_get_bool},
	{"YYJSONArray.GetFloat", json_arr_get_float},
	{"YYJSONArray.GetInt", json_arr_get_integer},
	{"YYJSONArray.GetInt64", json_arr_get_integer64},
	{"YYJSONArray.GetString", json_arr_get_str},
	{"YYJSONArray.IsNull", json_arr_is_null},
	{"YYJSONArray.Set", json_arr_replace_val},
	{"YYJSONArray.SetBool", json_arr_replace_bool},
	{"YYJSONArray.SetFloat", json_arr_replace_float},
	{"YYJSONArray.SetInt", json_arr_replace_integer},
	{"YYJSONArray.SetInt64", json_arr_replace_integer64},
	{"YYJSONArray.SetNull", json_arr_replace_null},
	{"YYJSONArray.SetString", json_arr_replace_str},
	{"YYJSONArray.Push", json_arr_append_val},
	{"YYJSONArray.PushBool", json_arr_append_bool},
	{"YYJSONArray.PushFloat", json_arr_append_float},
	{"YYJSONArray.PushInt", json_arr_append_int},
	{"YYJSONArray.PushInt64", json_arr_append_intger64},
	{"YYJSONArray.PushNull", json_arr_append_null},
	{"YYJSONArray.PushString", json_arr_append_str},
	{"YYJSONArray.Remove", json_arr_remove},
	{"YYJSONArray.RemoveFirst", json_arr_remove_first},
	{"YYJSONArray.RemoveLast", json_arr_remove_last},
	{"YYJSONArray.RemoveRange", json_arr_remove_range},
	{"YYJSONArray.Clear", json_arr_clear},
	{"YYJSONArray.FromString", json_arr_parse_str},
	{"YYJSONArray.FromFile", json_arr_parse_file},

	// JSON
	{"YYJSON.ToString", json_doc_write_to_str},
	{"YYJSON.ToFile", json_doc_write_to_file},
	{"YYJSON.Parse", json_doc_parse},
	{"YYJSON.Equals", json_doc_equals},
	{"YYJSON.DeepCopy", json_doc_copy_deep},
	{"YYJSON.GetTypeDesc", json_val_get_type_desc},
	{"YYJSON.GetSize", json_val_get_serialized_size},
	{"YYJSON.Size.get", json_val_get_serialized_size},
	{"YYJSON.Type.get", json_val_get_type},
	{"YYJSON.SubType.get", json_val_get_subtype},
	{"YYJSON.IsArray.get", json_val_is_array},
	{"YYJSON.IsObject.get", json_val_is_object},
	{"YYJSON.ForeachObject", json_obj_foreach},
	{"YYJSON.ForeachArray", json_arr_foreach},

	// JSON CREATE & GET
	{"YYJSON.CreateBool", json_val_create_bool},
	{"YYJSON.CreateFloat", json_val_create_float},
	{"YYJSON.CreateInt", json_val_create_int},
	{"YYJSON.CreateInt64", json_val_create_intger64},
	{"YYJSON.CreateNull", json_val_create_null},
	{"YYJSON.CreateString", json_val_create_str},
	{"YYJSON.GetBool", json_val_get_bool},
	{"YYJSON.GetFloat", json_val_get_float},
	{"YYJSON.GetInt", json_val_get_int},
	{"YYJSON.GetInt64", json_val_get_intger64},
	{"YYJSON.GetString", json_val_get_str},

	// JSON POINTER
	{"YYJSON.PtrGet", json_ptr_get_val},
	{"YYJSON.PtrGetBool", json_ptr_get_bool},
	{"YYJSON.PtrGetFloat", json_ptr_get_float},
	{"YYJSON.PtrGetInt", json_ptr_get_int},
	{"YYJSON.PtrGetInt64", json_ptr_get_intger64},
	{"YYJSON.PtrGetString", json_ptr_get_str},
	{"YYJSON.PtrGetIsNull", json_ptr_get_is_null},
	{"YYJSON.PtrGetLength", json_ptr_get_length},
	{"YYJSON.PtrSet", json_ptr_set_val},
	{"YYJSON.PtrSetBool", json_ptr_set_bool},
	{"YYJSON.PtrSetFloat", json_ptr_set_float},
	{"YYJSON.PtrSetInt", json_ptr_set_int},
	{"YYJSON.PtrSetInt64", json_ptr_set_intger64},
	{"YYJSON.PtrSetString", json_ptr_set_str},
	{"YYJSON.PtrSetNull", json_ptr_set_null},
	{"YYJSON.PtrAdd", json_ptr_add_val},
	{"YYJSON.PtrAddBool", json_ptr_add_bool},
	{"YYJSON.PtrAddFloat", json_ptr_add_float},
	{"YYJSON.PtrAddInt", json_ptr_add_int},
	{"YYJSON.PtrAddInt64", json_ptr_add_intger64},
	{"YYJSON.PtrAddString", json_ptr_add_str},
	{"YYJSON.PtrAddNull", json_ptr_add_null},
	{"YYJSON.PtrRemove", json_ptr_remove_val},
	{nullptr, nullptr}
};