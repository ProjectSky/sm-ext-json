#include "extension.h"

static std::random_device g_randomDevice;
static std::mt19937 g_randomGenerator(g_randomDevice());

enum YYJSON_SORT_ORDER
{
	YYJSON_SORT_ASC = 0,
	YYJSON_SORT_DESC = 1,
	YYJSON_SORT_RANDOM = 2
};

#define JSON_PACK_ERROR_MSG_SIZE 256
typedef struct {
	char msg[JSON_PACK_ERROR_MSG_SIZE];
} yyjson_error_t;

static void json_pack_set_error(yyjson_error_t *error, const char *fmt, ...) {
	if (error) {
		va_list args;
		va_start(args, fmt);
		vsnprintf(error->msg, JSON_PACK_ERROR_MSG_SIZE, fmt, args);
		va_end(args);

		error->msg[JSON_PACK_ERROR_MSG_SIZE - 1] = '\0';
	}
}

static const char* skip_separators(const char* ptr) {
	while (*ptr && (isspace(*ptr) || *ptr == ':' || *ptr == ',')) {
		ptr++;
	}
	return ptr;
}

yyjson_mut_val* json_pack(yyjson_mut_doc* doc, const char* fmt, yyjson_error_t* error, IPluginContext* pContext, const cell_t* params, unsigned int& param_index) {
	if (!doc || !fmt || !*fmt) {
		json_pack_set_error(error, "Invalid argument(s)");
		return nullptr;
	}

	yyjson_mut_val* root = nullptr;
	const char* ptr = fmt;

	bool is_obj = false;
	if (*ptr == '{') {
		root = yyjson_mut_obj(doc);
		is_obj = true;
		ptr = skip_separators(ptr + 1);
	} else if (*ptr == '[') {
		root = yyjson_mut_arr(doc);
		ptr = skip_separators(ptr + 1);
	} else {
		json_pack_set_error(error, "Invalid format string: expected '{' or '['");
		return nullptr;
	}

	if (!root) {
		json_pack_set_error(error, "Failed to create root object/array");
		return nullptr;
	}

	yyjson_mut_val* key_val = nullptr;
	yyjson_mut_val* val = nullptr;

	while (*ptr && *ptr != '}' && *ptr != ']') {
		if (is_obj) {
			if (*ptr != 's') {
				json_pack_set_error(error, "Object key must be string, got '%c'", *ptr);
				return nullptr;
			}
		}
		switch (*ptr) {
			case 's': {
				if (is_obj) {
					char* key;
					pContext->LocalToString(params[param_index++], &key);
					if (!key) {
						json_pack_set_error(error, "Invalid string key");
						return nullptr;
					}
					key_val = yyjson_mut_strcpy(doc, key);
					if (!key_val) {
						json_pack_set_error(error, "Failed to create key");
						return nullptr;
					}
					
					ptr = skip_separators(ptr + 1);
					if (*ptr != 's' && *ptr != 'i' && *ptr != 'f' && *ptr != 'b' && 
						*ptr != 'n' && *ptr != '{' && *ptr != '[') {
						json_pack_set_error(error, "Invalid value type after key");
						return nullptr;
					}
					
					if (*ptr == '{' || *ptr == '[') {
						val = json_pack(doc, ptr, error, pContext, params, param_index);
						if (!val) {
							return nullptr;
						}
						int nested_level = 1;
						ptr++;
						while (nested_level > 0 && *ptr) {
							if (*ptr == '{' || *ptr == '[') nested_level++;
							if (*ptr == '}' || *ptr == ']') nested_level--;
							ptr++;
						}
					} else {
						switch (*ptr) {
							case 's': {
								char* val_str;
								pContext->LocalToString(params[param_index++], &val_str);
								if (!val_str) {
									json_pack_set_error(error, "Invalid string value");
									return nullptr;
								}
								val = yyjson_mut_strcpy(doc, val_str);
								ptr++;
								break;
							}
							case 'i': {
								cell_t* val_int;
								pContext->LocalToPhysAddr(params[param_index++], &val_int);
								val = yyjson_mut_int(doc, *val_int);
								ptr++;
								break;
							}
							case 'f': {
								cell_t* val_float;
								pContext->LocalToPhysAddr(params[param_index++], &val_float);
								val = yyjson_mut_float(doc, sp_ctof(*val_float));
								ptr++;
								break;
							}
							case 'b': {
								cell_t* val_bool;
								pContext->LocalToPhysAddr(params[param_index++], &val_bool);
								val = yyjson_mut_bool(doc, *val_bool != 0);
								ptr++;
								break;
							}
							case 'n': {
								val = yyjson_mut_null(doc);
								ptr++;
								break;
							}
						}
					}
					
					if (!val) {
						json_pack_set_error(error, "Failed to create value");
						return nullptr;
					}
					
					if (!yyjson_mut_obj_add(root, key_val, val)) {
						json_pack_set_error(error, "Failed to add value to object");
						return nullptr;
					}
				} else {
					char* val_str;
					pContext->LocalToString(params[param_index++], &val_str);
					if (!val_str) {
						json_pack_set_error(error, "Invalid string value");
						return nullptr;
					}
					if (!yyjson_mut_arr_add_strcpy(doc, root, val_str)) {
						json_pack_set_error(error, "Failed to add string to array");
						return nullptr;
					}
					ptr++;
				}
				break;
			}
			case 'i': {
				cell_t* val_int;
				pContext->LocalToPhysAddr(params[param_index++], &val_int);
				if (!yyjson_mut_arr_add_int(doc, root, *val_int)) {
					json_pack_set_error(error, "Failed to add integer to array");
					return nullptr;
				}
				ptr++;
				break;
			}
			case 'b': {
				cell_t* val_bool;
				pContext->LocalToPhysAddr(params[param_index++], &val_bool);
				if (!yyjson_mut_arr_add_bool(doc, root, *val_bool != 0)) {
					json_pack_set_error(error, "Failed to add boolean to array");
					return nullptr;
				}
				ptr++;
				break;
			}
			case 'n': {
				if (!yyjson_mut_arr_add_null(doc, root)) {
					json_pack_set_error(error, "Failed to add null to array");
					return nullptr;
				}
				ptr++;
				break;
			}
			case 'f': {
				cell_t* val_float;
				pContext->LocalToPhysAddr(params[param_index++], &val_float);
				if (!yyjson_mut_arr_add_float(doc, root, sp_ctof(*val_float))) {
					json_pack_set_error(error, "Failed to add float to array");
					return nullptr;
				}
				ptr++;
				break;
			}
			case '{':
			case '[': {
				val = json_pack(doc, ptr, error, pContext, params, param_index);
				if (!val) {
					return nullptr;
				}
				if (!yyjson_mut_arr_append(root, val)) {
					json_pack_set_error(error, "Failed to add nested value to array");
					return nullptr;
				}
				// Skip the nested format string
				int nested_level = 1;
				ptr++;
				while (nested_level > 0 && *ptr) {
					if (*ptr == '{' || *ptr == '[') nested_level++;
					if (*ptr == '}' || *ptr == ']') nested_level--;
					ptr++;
				}
				break;
			}
			default: {
				json_pack_set_error(error, "Invalid format character: %c", *ptr);
				return nullptr;
			}
		}
		ptr = skip_separators(ptr);
	}

	if (*ptr != (is_obj ? '}' : ']')) {
		json_pack_set_error(error, "Unexpected end of format string");
		return nullptr;
	}

	return root;
}

static cell_t json_val_pack(IPluginContext* pContext, const cell_t* params) {
	char* fmt;
	pContext->LocalToString(params[1], &fmt);
	
	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	
	yyjson_error_t error;
	unsigned int param_index = 2;
	
	pYYJsonWrapper->m_pVal_mut = json_pack(pYYJsonWrapper->m_pDocument_mut.get(), fmt, &error, pContext, params, param_index);
	
	if (!pYYJsonWrapper->m_pVal_mut) {
		return pContext->ThrowNativeError("Failed to pack JSON: %s", error.msg);
	}
  
	yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut.get(), pYYJsonWrapper->m_pVal_mut);
	
	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);
	
	if (!pYYJsonWrapper->m_handle) {
		return pContext->ThrowNativeError("Failed to create handle for packed JSON (error code: %d)", err);
	}
	
	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_doc_parse(IPluginContext* pContext, const cell_t* params)
{
	char* str;
	pContext->LocalToString(params[1], &str);

	bool is_file = params[2];
	bool is_mutable_doc = params[3];
	yyjson_read_flag read_flg = static_cast<yyjson_read_flag>(params[4]);

	yyjson_read_err readError;
	yyjson_doc* idoc;
	auto pYYJsonWrapper = CreateWrapper();

	if (is_file) {
		char realpath[PLATFORM_MAX_PATH];
		smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", str);
		idoc = yyjson_read_file(realpath, read_flg, nullptr, &readError);
	} else {
		idoc = yyjson_read_opts(str, strlen(str), read_flg, nullptr, &readError);
	}

	if (!idoc) {
		return pContext->ThrowNativeError("Failed to parse JSON data");
	}

	if (readError.code) {
		yyjson_doc_free(idoc);
		if (is_file) {
			return pContext->ThrowNativeError("Failed to parse JSON file: %s (error code: %u, msg: %s, position: %d)",
				str, readError.code, readError.msg, readError.pos);
		} else {
			return pContext->ThrowNativeError("Failed to parse JSON str: %s (error code: %u, position: %d)",
				readError.msg, readError.code, readError.pos);
		}
	}

	pYYJsonWrapper->m_readSize = yyjson_doc_get_read_size(idoc);

	if (is_mutable_doc) {
		pYYJsonWrapper->m_pDocument_mut = CopyDocument(idoc);
		pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut.get());
		yyjson_doc_free(idoc);
	} else {
		pYYJsonWrapper->m_pDocument = WrapImmutableDocument(idoc);
		pYYJsonWrapper->m_pVal = yyjson_doc_get_root(idoc);
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle) {
		return pContext->ThrowNativeError("Failed to create handle for JSON value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_doc_equals(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return 0;

	// if both are mutable, compare them directly
	if (handle1->IsMutable() && handle2->IsMutable()) {
		return yyjson_mut_equals(handle1->m_pVal_mut, handle2->m_pVal_mut);
	}

	// if both are immutable, compare them directly
	if (!handle1->IsMutable() && !handle2->IsMutable()) {
		auto doc1_mut = CopyDocument(handle1->m_pDocument.get());
		auto doc2_mut = CopyDocument(handle2->m_pDocument.get());

		if (!doc1_mut || !doc2_mut) {
			return pContext->ThrowNativeError("Failed to create mutable documents for comparison");
		}

		yyjson_mut_val* val1_mut = yyjson_mut_doc_get_root(doc1_mut.get());
		yyjson_mut_val* val2_mut = yyjson_mut_doc_get_root(doc2_mut.get());

		if (!val1_mut || !val2_mut) {
			return pContext->ThrowNativeError("Failed to get root values from mutable documents");
		}

		return yyjson_mut_equals(val1_mut, val2_mut);
	}

	// if one is mutable, convert the other to immutable
	YYJsonWrapper* immutable = handle1->IsMutable() ? handle2 : handle1;
	YYJsonWrapper* mutable_doc = handle1->IsMutable() ? handle1 : handle2;

	// copy immutable document to mutable
	auto doc_mut = CopyDocument(immutable->m_pDocument.get());
	if (!doc_mut) {
		return pContext->ThrowNativeError("Failed to create mutable document for comparison");
	}

	yyjson_mut_val* val_mut = yyjson_mut_doc_get_root(doc_mut.get());
	if (!val_mut) {
		return pContext->ThrowNativeError("Failed to get root value from mutable document");
	}

	return yyjson_mut_equals(mutable_doc->m_pVal_mut, val_mut);
}

static cell_t json_doc_copy_deep(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* targetDoc = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* sourceValue = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (!targetDoc || !sourceValue) return 0;

	auto pYYJsonWrapper = CreateWrapper();

	if (targetDoc->IsMutable()) {
		pYYJsonWrapper->m_pDocument_mut = CreateDocument();
		
		yyjson_mut_val* val_copy = nullptr;
		if (sourceValue->IsMutable()) {
			val_copy = yyjson_mut_val_mut_copy(pYYJsonWrapper->m_pDocument_mut.get(), sourceValue->m_pVal_mut);
		} else {
			val_copy = yyjson_val_mut_copy(pYYJsonWrapper->m_pDocument_mut.get(), sourceValue->m_pVal);
		}

		if (!val_copy) {
			return pContext->ThrowNativeError("Failed to copy JSON value");
		}

		yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut.get(), val_copy);
		pYYJsonWrapper->m_pVal_mut = val_copy;
	} else {
		yyjson_mut_doc* temp_doc = yyjson_mut_doc_new(nullptr);
		if (!temp_doc) {
			return pContext->ThrowNativeError("Failed to create temporary mutable document");
		}

		yyjson_mut_val* temp_val = nullptr;
		if (sourceValue->IsMutable()) {
			temp_val = yyjson_mut_val_mut_copy(temp_doc, sourceValue->m_pVal_mut);
		} else {
			temp_val = yyjson_val_mut_copy(temp_doc, sourceValue->m_pVal);
		}

		if (!temp_val) {
			yyjson_mut_doc_free(temp_doc);
			return pContext->ThrowNativeError("Failed to copy JSON value");
		}

		yyjson_mut_doc_set_root(temp_doc, temp_val);
		
		yyjson_doc* doc = yyjson_mut_doc_imut_copy(temp_doc, nullptr);
		yyjson_mut_doc_free(temp_doc);
		
		if (!doc) {
			return pContext->ThrowNativeError("Failed to create immutable document");
		}

		pYYJsonWrapper->m_pDocument = WrapImmutableDocument(doc);
		pYYJsonWrapper->m_pVal = yyjson_doc_get_root(doc);
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle) {
		return pContext->ThrowNativeError("Failed to create handle for JSON value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_val_get_type_desc(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_type_desc(handle->m_pVal_mut), nullptr);
	} else {
		pContext->StringToLocalUTF8(params[2], params[3], yyjson_get_type_desc(handle->m_pVal), nullptr);
	}

	return 1;
}

static cell_t json_obj_parse_str(IPluginContext* pContext, const cell_t* params)
{
	char* str;
	pContext->LocalToString(params[1], &str);

	auto pYYJsonWrapper = CreateWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_opts(str, strlen(str), params[2], nullptr, &readError);

	if (!idoc) {
		return pContext->ThrowNativeError("Failed to parse JSON data");
	}

	if (readError.code) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Failed to parse JSON str: %s (error code: %u, position: %d)",
			readError.msg, readError.code, readError.pos);
	}

	yyjson_val* root = yyjson_doc_get_root(idoc);
	
	if (!yyjson_is_obj(root)) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Root value is not an object (got %s)", yyjson_get_type_desc(root));
	}

	pYYJsonWrapper->m_readSize = yyjson_doc_get_read_size(idoc);
	pYYJsonWrapper->m_pDocument = WrapImmutableDocument(idoc);
	pYYJsonWrapper->m_pVal = root;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON object from string (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_obj_parse_file(IPluginContext* pContext, const cell_t* params)
{
	char* path;
	pContext->LocalToString(params[1], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	auto pYYJsonWrapper = CreateWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_file(realpath, params[2], nullptr, &readError);

	if (!idoc) {
		return pContext->ThrowNativeError("Failed to parse JSON data");
	}

	if (readError.code) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Failed to parse JSON file: %s (error code: %u, msg: %s, position: %d)",
			realpath, readError.code, readError.msg, readError.pos);
	}

	yyjson_val* root = yyjson_doc_get_root(idoc);

	if (!yyjson_is_obj(root)) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Root value in file is not an object (got %s)", yyjson_get_type_desc(root));
	}

	pYYJsonWrapper->m_readSize = yyjson_doc_get_read_size(idoc);
	pYYJsonWrapper->m_pDocument = WrapImmutableDocument(idoc);
	pYYJsonWrapper->m_pVal = root;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON object from file (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_parse_str(IPluginContext* pContext, const cell_t* params)
{
	char* str;
	pContext->LocalToString(params[1], &str);

	auto pYYJsonWrapper = CreateWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_opts(str, strlen(str), params[2], nullptr, &readError);

	if (!idoc) {
		return pContext->ThrowNativeError("Failed to parse JSON data");
	}

	if (readError.code) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Failed to parse JSON string: %s (error code: %u, position: %d)",
			readError.msg, readError.code, readError.pos);
	}

	yyjson_val* root = yyjson_doc_get_root(idoc);

	if (!yyjson_is_arr(root)) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Root value is not an array (got %s)", yyjson_get_type_desc(root));
	}

	pYYJsonWrapper->m_readSize = yyjson_doc_get_read_size(idoc);
	pYYJsonWrapper->m_pDocument = WrapImmutableDocument(idoc);
	pYYJsonWrapper->m_pVal = root;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON array from string (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_parse_file(IPluginContext* pContext, const cell_t* params)
{
	char* path;
	pContext->LocalToString(params[1], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);
	auto pYYJsonWrapper = CreateWrapper();

	yyjson_read_err readError;
	yyjson_doc* idoc = yyjson_read_file(realpath, params[2], nullptr, &readError);

	if (!idoc) {
		return pContext->ThrowNativeError("Failed to parse JSON data");
	}

	if (readError.code) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Failed to parse JSON file: %s (error code: %u, msg: %s, position: %d)",
			realpath, readError.code, readError.msg, readError.pos);
	}

	yyjson_val* root = yyjson_doc_get_root(idoc);

	if (!yyjson_is_arr(root)) {
		yyjson_doc_free(idoc);
		return pContext->ThrowNativeError("Root value in file is not an array (got %s)", yyjson_get_type_desc(root));
	}

	pYYJsonWrapper->m_readSize = yyjson_doc_get_read_size(idoc);
	pYYJsonWrapper->m_pDocument = WrapImmutableDocument(idoc);
	pYYJsonWrapper->m_pVal = root;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON array from file (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_index_of_bool(IPluginContext *pContext, const cell_t *params)
{
	YYJsonWrapper *handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	
	if (!handle) return 0;

	bool searchValue = params[2];

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		size_t idx, max;
		yyjson_mut_val *val;
		yyjson_mut_arr_foreach(handle->m_pVal_mut, idx, max, val) {
			if (yyjson_mut_is_bool(val) && yyjson_mut_get_bool(val) == searchValue) {
				return static_cast<cell_t>(idx);
			}
		}
	} else {
		if (!yyjson_is_arr(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		size_t idx, max;
		yyjson_val *val;
		yyjson_arr_foreach(handle->m_pVal, idx, max, val) {
			if (yyjson_is_bool(val) && yyjson_get_bool(val) == searchValue) {
				return static_cast<cell_t>(idx);
			}
		}
	}

	return -1;
}

static cell_t json_arr_index_of_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* searchStr;
	pContext->LocalToString(params[2], &searchStr);

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		size_t idx, max;
		yyjson_mut_val *val;
		yyjson_mut_arr_foreach(handle->m_pVal_mut, idx, max, val) {
			if (yyjson_mut_is_str(val) && strcmp(yyjson_mut_get_str(val), searchStr) == 0) {
				return static_cast<cell_t>(idx);
			}
		}
	} else {
		if (!yyjson_is_arr(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		size_t idx, max;
		yyjson_val *val;
		yyjson_arr_foreach(handle->m_pVal, idx, max, val) {
			if (yyjson_is_str(val) && strcmp(yyjson_get_str(val), searchStr) == 0) {
				return static_cast<cell_t>(idx);
			}
		}
	}

	return -1;
}

static cell_t json_arr_index_of_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	int searchValue = params[2];

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		size_t idx, max;
		yyjson_mut_val *val;
		yyjson_mut_arr_foreach(handle->m_pVal_mut, idx, max, val) {
			if (yyjson_mut_is_int(val) && yyjson_mut_get_int(val) == searchValue) {
				return static_cast<cell_t>(idx);
			}
		}
	}
	else {
		if (!yyjson_is_arr(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		size_t idx, max;
		yyjson_val *val;
		yyjson_arr_foreach(handle->m_pVal, idx, max, val) {
			if (yyjson_is_int(val) && yyjson_get_int(val) == searchValue) {
				return static_cast<cell_t>(idx);
			}
		}
	}

	return -1;
}

static cell_t json_arr_index_of_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* searchStr;
	pContext->LocalToString(params[2], &searchStr);

	char* endptr;
	errno = 0;
	long long searchValue = strtoll(searchStr, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0') {
		return pContext->ThrowNativeError("Invalid integer64 value: %s", searchStr);
	}

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		size_t idx, max;
		yyjson_mut_val *val;
		yyjson_mut_arr_foreach(handle->m_pVal_mut, idx, max, val) {
			if (yyjson_mut_is_int(val) && yyjson_mut_get_sint(val) == searchValue) {
				return static_cast<cell_t>(idx);
			}
		}
	}
	else {
		if (!yyjson_is_arr(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		size_t idx, max;
		yyjson_val *val;
		yyjson_arr_foreach(handle->m_pVal, idx, max, val) {
			if (yyjson_is_int(val) && yyjson_get_sint(val) == searchValue) {
				return static_cast<cell_t>(idx);
			}
		}
	}

	return -1;
}

static cell_t json_arr_index_of_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	double searchValue = static_cast<double>(sp_ctof(params[2]));

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		size_t idx, max;
		yyjson_mut_val *val;
		yyjson_mut_arr_foreach(handle->m_pVal_mut, idx, max, val) {
			if (yyjson_mut_is_real(val)) {
				double val_num = yyjson_mut_get_real(val);
				if (yyjson_equals_fp(val_num, searchValue)) {
					return static_cast<cell_t>(idx);
				}
			}
		}
	}
	else {
		if (!yyjson_is_arr(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		size_t idx, max;
		yyjson_val *val;
		yyjson_arr_foreach(handle->m_pVal, idx, max, val) {
			if (yyjson_is_real(val)) {
				double val_num = yyjson_get_real(val);
				if (yyjson_equals_fp(val_num, searchValue)) {
					return static_cast<cell_t>(idx);
				}
			}
		}
	}

	return -1;
}

static cell_t json_val_get_type(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_get_type(handle->m_pVal_mut);
	} else {
		return yyjson_get_type(handle->m_pVal);
	}
}

static cell_t json_val_get_subtype(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_get_subtype(handle->m_pVal_mut);
	} else {
		return yyjson_get_subtype(handle->m_pVal);
	}
}

static cell_t json_val_is_array(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_arr(handle->m_pVal_mut);
	} else {
		return yyjson_is_arr(handle->m_pVal);
	}
}

static cell_t json_val_is_object(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_obj(handle->m_pVal_mut);
	} else {
		return yyjson_is_obj(handle->m_pVal);
	}
}

static cell_t json_val_is_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_int(handle->m_pVal_mut);
	} else {
		return yyjson_is_int(handle->m_pVal);
	}
}

static cell_t json_val_is_uint(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_uint(handle->m_pVal_mut);
	} else {
		return yyjson_is_uint(handle->m_pVal);
	}
}

static cell_t json_val_is_sint(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_sint(handle->m_pVal_mut);
	} else {
		return yyjson_is_sint(handle->m_pVal);
	}
}

static cell_t json_val_is_num(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_num(handle->m_pVal_mut);
	} else {
		return yyjson_is_num(handle->m_pVal);
	}
}

static cell_t json_val_is_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_bool(handle->m_pVal_mut);
	} else {
		return yyjson_is_bool(handle->m_pVal);
	}
}

static cell_t json_val_is_true(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_true(handle->m_pVal_mut);
	} else {
		return yyjson_is_true(handle->m_pVal);
	}
}

static cell_t json_val_is_false(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_false(handle->m_pVal_mut);
	} else {
		return yyjson_is_false(handle->m_pVal);
	}
}

static cell_t json_val_is_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_real(handle->m_pVal_mut);
	} else {
		return yyjson_is_real(handle->m_pVal);
	}
}

static cell_t json_val_is_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_str(handle->m_pVal_mut);
	} else {
		return yyjson_is_str(handle->m_pVal);
	}
}

static cell_t json_val_is_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_null(handle->m_pVal_mut);
	} else {
		return yyjson_is_null(handle->m_pVal);
	}
}

static cell_t json_val_is_ctn(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return yyjson_mut_is_ctn(handle->m_pVal_mut);
	} else {
		return yyjson_is_ctn(handle->m_pVal);
	}
}

static cell_t json_val_is_mutable(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	return handle->IsMutable();
}

static cell_t json_val_is_immutable(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	return handle->IsImmutable();
}

static cell_t json_obj_init(IPluginContext* pContext, const cell_t* params)
{
	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj(pYYJsonWrapper->m_pDocument_mut.get());

	yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut.get(), pYYJsonWrapper->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for initialized JSON object (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_obj_init_with_str(IPluginContext* pContext, const cell_t* params)
{
	cell_t* addr;
	pContext->LocalToPhysAddr(params[1], &addr);
	cell_t array_size = params[2];
	
	if (array_size < 2) {
		return pContext->ThrowNativeError("Array must contain at least one key-value pair");
	}
	if (array_size % 2 != 0) {
		return pContext->ThrowNativeError("Array must contain an even number of strings (got %d)", array_size);
	}
	
	cell_t pair_count = array_size / 2;
	
	std::vector<const char*> kv_pairs;
	kv_pairs.reserve(array_size);
	
	// Convert and validate each string from the SourcePawn array
	for (cell_t i = 0; i < array_size; i += 2) {
		char* key;
		char* value;
		
		if (pContext->LocalToString(addr[i], &key) != SP_ERROR_NONE) {
			return pContext->ThrowNativeError("Failed to read key at index %d", i);
		}
		if (!key || !key[0]) {
			return pContext->ThrowNativeError("Empty key at index %d", i);
		}
		
		if (pContext->LocalToString(addr[i + 1], &value) != SP_ERROR_NONE) {
			return pContext->ThrowNativeError("Failed to read value at index %d", i + 1);
		}
		if (!value) {
			return pContext->ThrowNativeError("Invalid value at index %d", i + 1);
		}
		
		kv_pairs.push_back(key);
		kv_pairs.push_back(value);
	}

	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj_with_kv(
		pYYJsonWrapper->m_pDocument_mut.get(),
		kv_pairs.data(),
		pair_count
	);

	if (!pYYJsonWrapper->m_pVal_mut) {
		return pContext->ThrowNativeError("Failed to create JSON object from key-value pairs");
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle) {
		return pContext->ThrowNativeError("Failed to create handle (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_val_create_bool(IPluginContext* pContext, const cell_t* params)
{
	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_bool(pYYJsonWrapper->m_pDocument_mut.get(), params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON boolean value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_val_create_float(IPluginContext* pContext, const cell_t* params)
{
	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_float(pYYJsonWrapper->m_pDocument_mut.get(), sp_ctof(params[1]));

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON float value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_val_create_int(IPluginContext* pContext, const cell_t* params)
{
	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_int(pYYJsonWrapper->m_pDocument_mut.get(), params[1]);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON integer value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_val_create_integer64(IPluginContext* pContext, const cell_t* params)
{
	char* value;
	pContext->LocalToString(params[1], &value);

	char* endptr;
	errno = 0;
	long long num = strtoll(value, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0') {
		return pContext->ThrowNativeError("Invalid integer64 value: %s", value);
	}

	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_int(pYYJsonWrapper->m_pDocument_mut.get(), num);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON integer64 value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_val_create_str(IPluginContext* pContext, const cell_t* params)
{
	char* str;
	pContext->LocalToString(params[1], &str);

	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_strcpy(pYYJsonWrapper->m_pDocument_mut.get(), str);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON str value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_val_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_bool(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected boolean value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}
		return yyjson_mut_get_bool(handle->m_pVal_mut);
	}
	else {
		if (!yyjson_is_bool(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected boolean value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}
		return yyjson_get_bool(handle->m_pVal);
	}
}

static cell_t json_val_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_real(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected float value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		return sp_ftoc(static_cast<float>(yyjson_mut_get_real(handle->m_pVal_mut)));
	}
	else {
		if (!yyjson_is_real(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected float value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}
		return sp_ftoc(static_cast<float>(yyjson_get_real(handle->m_pVal)));
	}
}

static cell_t json_val_get_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_int(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected integer value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}
		return yyjson_mut_get_int(handle->m_pVal_mut);
	}
	else {
		if (!yyjson_is_int(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected integer value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}
		return yyjson_get_int(handle->m_pVal);
	}
}

static cell_t json_val_get_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	int64_t value;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_int(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected integer64 value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}
		value = yyjson_mut_get_sint(handle->m_pVal_mut);
	} else {
		if (!yyjson_is_int(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected integer64 value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}
		value = yyjson_get_sint(handle->m_pVal);
	}

	char result[21];
	snprintf(result, sizeof(result), "%" PRId64, value);
	pContext->StringToLocalUTF8(params[2], params[3], result, nullptr);

	return 1;
}

static cell_t json_val_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	const char* str;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_str(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected string value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}
		str = yyjson_mut_get_str(handle->m_pVal_mut);
	} else {
		if (!yyjson_is_str(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected string value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}
		str = yyjson_get_str(handle->m_pVal);
	}

	size_t len = strlen(str) + 1;
	size_t maxlen = static_cast<size_t>(params[3]);

	if (len > maxlen) {
		return pContext->ThrowNativeError("Buffer is too small (need %d, have %d)", len, maxlen);
	}

	pContext->StringToLocalUTF8(params[2], maxlen, str, nullptr);

	return 1;
}

static cell_t json_val_get_serialized_size(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t json_size;
	char* json_str;

	if (handle->IsMutable()) {
		json_str = yyjson_mut_val_write(handle->m_pVal_mut, params[2], &json_size);
	} else {
		json_str = yyjson_val_write(handle->m_pVal, params[2], &json_size);
	}

	if (json_str) {
		free(json_str);
		return static_cast<cell_t>(json_size + 1);
	}

	return 0;
}

static cell_t json_val_get_read_size(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle || !handle->m_readSize) return 0;

	return static_cast<cell_t>(handle->m_readSize + 1);
}

static cell_t json_val_create_null(IPluginContext* pContext, const cell_t* params)
{
	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_null(pYYJsonWrapper->m_pDocument_mut.get());

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON null value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_init(IPluginContext* pContext, const cell_t* params)
{
	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr(pYYJsonWrapper->m_pDocument_mut.get());
	yyjson_mut_doc_set_root(pYYJsonWrapper->m_pDocument_mut.get(), pYYJsonWrapper->m_pVal_mut);

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for initialized JSON array (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_init_with_str(IPluginContext* pContext, const cell_t* params)
{
	cell_t* addr;
	pContext->LocalToPhysAddr(params[1], &addr);
	cell_t array_size = params[2];
	
	std::vector<const char*> strs;
	strs.reserve(array_size);
	
	for (cell_t i = 0; i < array_size; i++) {
		char* str;
		pContext->LocalToString(addr[i], &str);
		strs.push_back(str);
	}

	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CreateDocument();
	
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_arr_with_strcpy(
		pYYJsonWrapper->m_pDocument_mut.get(),
		strs.data(),
		strs.size()
	);

	if (!pYYJsonWrapper->m_pVal_mut) {
		return pContext->ThrowNativeError("Failed to create JSON array from strings");
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle) {
		return pContext->ThrowNativeError("Failed to create handle (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_get_size(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return static_cast<cell_t>(yyjson_mut_arr_size(handle->m_pVal_mut));
	} else {
		return static_cast<cell_t>(yyjson_arr_size(handle->m_pVal));
	}
}

static cell_t json_arr_get_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	auto pYYJsonWrapper = CreateWrapper();

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_mut_val* val = yyjson_mut_arr_get(handle->m_pVal_mut, index);
		if (!val) {
			return pContext->ThrowNativeError("Failed to get value at index %d", index);
		}

		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		pYYJsonWrapper->m_pVal_mut = val;
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_val* val = yyjson_arr_get(handle->m_pVal, index);
		if (!val) {
			return pContext->ThrowNativeError("Failed to get value at index %d", index);
		}

		pYYJsonWrapper->m_pDocument = handle->m_pDocument;
		pYYJsonWrapper->m_pVal = val;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON array value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_get_first(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	auto pYYJsonWrapper = CreateWrapper();

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (arr_size == 0) {
			return pContext->ThrowNativeError("Cannot get first element: array is empty");
		}

		yyjson_mut_val* val = yyjson_mut_arr_get_first(handle->m_pVal_mut);
		if (!val) {
			return pContext->ThrowNativeError("Failed to get first element");
		}

		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		pYYJsonWrapper->m_pVal_mut = val;
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (arr_size == 0) {
			return pContext->ThrowNativeError("Cannot get first element: array is empty");
		}

		yyjson_val* val = yyjson_arr_get_first(handle->m_pVal);
		if (!val) {
			return pContext->ThrowNativeError("Failed to get first element");
		}

		pYYJsonWrapper->m_pDocument = handle->m_pDocument;
		pYYJsonWrapper->m_pVal = val;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for first JSON array value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_get_last(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	auto pYYJsonWrapper = CreateWrapper();

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (arr_size == 0) {
			return pContext->ThrowNativeError("Cannot get last element: array is empty");
		}

		yyjson_mut_val* val = yyjson_mut_arr_get_last(handle->m_pVal_mut);
		if (!val) {
			return pContext->ThrowNativeError("Failed to get last element");
		}

		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		pYYJsonWrapper->m_pVal_mut = val;
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (arr_size == 0) {
			return pContext->ThrowNativeError("Cannot get last element: array is empty");
		}

		yyjson_val* val = yyjson_arr_get_last(handle->m_pVal);
		if (!val) {
			return pContext->ThrowNativeError("Failed to get last element");
		}

		pYYJsonWrapper->m_pDocument = handle->m_pDocument;
		pYYJsonWrapper->m_pVal = val;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for last JSON array value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_arr_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_mut_val* val = yyjson_mut_arr_get(handle->m_pVal_mut, index);
		if (!yyjson_mut_is_bool(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected boolean value, got %s",
				index, yyjson_mut_get_type_desc(val));
		}

		return yyjson_mut_get_bool(val);
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_val* val = yyjson_arr_get(handle->m_pVal, index);
		if (!yyjson_is_bool(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected boolean value, got %s",
				index, yyjson_get_type_desc(val));
		}

		return yyjson_get_bool(val);
	}
}

static cell_t json_arr_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_mut_val* val = yyjson_mut_arr_get(handle->m_pVal_mut, index);
		if (!yyjson_mut_is_real(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected float value, got %s",
				index, yyjson_mut_get_type_desc(val));
		}

		return sp_ftoc(static_cast<float>(yyjson_mut_get_real(val)));
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_val* val = yyjson_arr_get(handle->m_pVal, index);
		if (!yyjson_is_real(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected float value, got %s",
				index, yyjson_get_type_desc(val));
		}

		return sp_ftoc(static_cast<float>(yyjson_get_real(val)));
	}
}

static cell_t json_arr_get_integer(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_mut_val* val = yyjson_mut_arr_get(handle->m_pVal_mut, index);
		if (!yyjson_mut_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected integer value, got %s",
				index, yyjson_mut_get_type_desc(val));
		}

		return yyjson_mut_get_int(val);
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_val* val = yyjson_arr_get(handle->m_pVal, index);
		if (!yyjson_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected integer value, got %s",
				index, yyjson_get_type_desc(val));
		}

		return yyjson_get_int(val);
	}
}

static cell_t json_arr_get_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t index = static_cast<size_t>(params[2]);
	int64_t value;

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_mut_val* val = yyjson_mut_arr_get(handle->m_pVal_mut, index);
		if (!yyjson_mut_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected integer64 value, got %s",
				index, yyjson_mut_get_type_desc(val));
		}
		value = yyjson_mut_get_sint(val);
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_val* val = yyjson_arr_get(handle->m_pVal, index);
		if (!yyjson_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected integer64 value, got %s",
				index, yyjson_get_type_desc(val));
		}
		value = yyjson_get_sint(val);
	}

	char result[21];
	snprintf(result, sizeof(result), "%" PRId64, value);
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t json_arr_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_mut_val* val = yyjson_mut_arr_get(handle->m_pVal_mut, index);
		if (!yyjson_mut_is_str(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected string value, got %s",
				index, yyjson_mut_get_type_desc(val));
		}

		const char* str = yyjson_mut_get_str(val);
		size_t len = yyjson_mut_get_len(val) + 1;
		size_t maxlen = static_cast<size_t>(params[4]);

		if (len > maxlen) {
			return pContext->ThrowNativeError("Buffer is too small (need %d, have %d)", len, maxlen);
		}

		pContext->StringToLocalUTF8(params[3], maxlen, str, nullptr);

		return 1;
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_val* val = yyjson_arr_get(handle->m_pVal, index);
		if (!yyjson_is_str(val)) {
			return pContext->ThrowNativeError("Type mismatch at index %d: expected string value, got %s",
				index, yyjson_get_type_desc(val));
		}

		const char* str = yyjson_get_str(val);
		size_t len = yyjson_get_len(val) + 1;
		size_t maxlen = static_cast<size_t>(params[4]);

		if (len > maxlen) {
			return pContext->ThrowNativeError("Buffer is too small (need %d, have %d)", len, maxlen);
		}

		pContext->StringToLocalUTF8(params[3], maxlen, str, nullptr);

		return 1;
	}
}

static cell_t json_arr_is_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_mut_val* val = yyjson_mut_arr_get(handle->m_pVal_mut, index);
		return yyjson_mut_is_null(val);
	} else {
		size_t arr_size = yyjson_arr_size(handle->m_pVal);
		if (index >= arr_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
		}

		yyjson_val* val = yyjson_arr_get(handle->m_pVal, index);
		return yyjson_is_null(val);
	}
}

static cell_t json_arr_replace_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return 0;

	if (!handle1->IsMutable()) {
		return pContext->ThrowNativeError("Cannot replace value in an immutable JSON array");
	}

	size_t arr_size = yyjson_mut_arr_size(handle1->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);
	
	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	yyjson_mut_val* val_copy = nullptr;
	if (handle2->IsMutable()) {
		val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal_mut);
	} else {
		val_copy = yyjson_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal);
	}

	if (!val_copy) {
		return pContext->ThrowNativeError("Failed to copy JSON value");
	}

	return yyjson_mut_arr_replace(handle1->m_pVal_mut, index, val_copy) != nullptr;
}

static cell_t json_arr_replace_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot replace value in an immutable JSON array");
	}

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);

	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	return yyjson_mut_arr_replace(handle->m_pVal_mut, index, yyjson_mut_bool(handle->m_pDocument_mut.get(), params[3])) != nullptr;
}

static cell_t json_arr_replace_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);

	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	return yyjson_mut_arr_replace(handle->m_pVal_mut, index, yyjson_mut_float(handle->m_pDocument_mut.get(), sp_ctof(params[3]))) != nullptr;
}

static cell_t json_arr_replace_integer(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot replace value in an immutable JSON array");
	}

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);

	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	return yyjson_mut_arr_replace(handle->m_pVal_mut, index, yyjson_mut_int(handle->m_pDocument_mut.get(), params[3])) != nullptr;
}

static cell_t json_arr_replace_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot replace value in an immutable JSON array");
	}

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);

	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	char* value;
	pContext->LocalToString(params[3], &value);

	char* endptr;
	errno = 0;
	long long num = strtoll(value, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0') {
		return pContext->ThrowNativeError("Invalid integer64 value: %s", value);
	}

	return yyjson_mut_arr_replace(handle->m_pVal_mut, index, yyjson_mut_int(handle->m_pDocument_mut.get(), num)) != nullptr;
}

static cell_t json_arr_replace_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot replace value in an immutable JSON array");
	}

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);

	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	return yyjson_mut_arr_replace(handle->m_pVal_mut, index, yyjson_mut_null(handle->m_pDocument_mut.get())) != nullptr;
}

static cell_t json_arr_replace_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot replace value in an immutable JSON array");
	}

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);

	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	char* val;
	pContext->LocalToString(params[3], &val);

	return yyjson_mut_arr_replace(handle->m_pVal_mut, index, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), val)) != nullptr;
}

static cell_t json_arr_append_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[2]);

	if (!handle1 || !handle2) return 0;

	if (!handle1->IsMutable()) {
		return pContext->ThrowNativeError("Cannot append value to an immutable JSON array");
	}

	yyjson_mut_val* val_copy = nullptr;
	if (handle2->IsMutable()) {
		val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal_mut);
	} else {
		val_copy = yyjson_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal);
	}

	if (!val_copy) {
		return pContext->ThrowNativeError("Failed to copy JSON value");
	}

	return yyjson_mut_arr_append(handle1->m_pVal_mut, val_copy);
}

static cell_t json_arr_append_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot append value to an immutable JSON array");
	}

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_bool(handle->m_pDocument_mut.get(), params[2]));
}

static cell_t json_arr_append_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot append value to an immutable JSON array");
	}

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_float(handle->m_pDocument_mut.get(), sp_ctof(params[2])));
}

static cell_t json_arr_append_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot append value to an immutable JSON array");
	}

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_int(handle->m_pDocument_mut.get(), params[2]));
}

static cell_t json_arr_append_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot append value to an immutable JSON array");
	}

	char* value;
	pContext->LocalToString(params[2], &value);

	char* endptr;
	errno = 0;
	long long num = strtoll(value, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0') {
		return pContext->ThrowNativeError("Invalid integer64 value: %s", value);
	}

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_int(handle->m_pDocument_mut.get(), num));
}

static cell_t json_arr_append_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot append value to an immutable JSON array");
	}

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_null(handle->m_pDocument_mut.get()));
}

static cell_t json_arr_append_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot append value to an immutable JSON array");
	}

	char* str;
	pContext->LocalToString(params[2], &str);

	return yyjson_mut_arr_append(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), str));
}

static cell_t json_arr_remove(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot remove value from an immutable JSON array");
	}

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	size_t index = static_cast<size_t>(params[2]);

	if (index >= arr_size) {
		return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, arr_size);
	}

	return yyjson_mut_arr_remove(handle->m_pVal_mut, index) != nullptr;
}

static cell_t json_arr_remove_first(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot remove value from an immutable JSON array");
	}

	if (yyjson_mut_arr_size(handle->m_pVal_mut) == 0) {
		return pContext->ThrowNativeError("Cannot remove first element from empty array");
	}

	return yyjson_mut_arr_remove_first(handle->m_pVal_mut) != nullptr;
}

static cell_t json_arr_remove_last(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot remove value from an immutable JSON array");
	}

	if (yyjson_mut_arr_size(handle->m_pVal_mut) == 0) {
		return pContext->ThrowNativeError("Cannot remove last element from empty array");
	}

	return yyjson_mut_arr_remove_last(handle->m_pVal_mut) != nullptr;
}

static cell_t json_arr_remove_range(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot remove value from an immutable JSON array");
	}

	size_t start_index = static_cast<size_t>(params[2]);
	size_t end_index = static_cast<size_t>(params[3]);
	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);

	if (start_index >= arr_size || end_index > arr_size || start_index > end_index) {
		return pContext->ThrowNativeError("Invalid range [%d, %d) for array of size %d",
			start_index, end_index, arr_size);
	}

	return yyjson_mut_arr_remove_range(handle->m_pVal_mut, start_index, end_index);
}

static cell_t json_arr_clear(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot clear an immutable JSON array");
	}

	return yyjson_mut_arr_clear(handle->m_pVal_mut);
}

static cell_t json_doc_write_to_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t json_size;
	char* json_str;

	if (handle->IsMutable()) {
		json_str = yyjson_mut_val_write(handle->m_pVal_mut, params[4], &json_size);
	} else {
		json_str = yyjson_val_write(handle->m_pVal, params[4], &json_size);
	}

	if (json_str) {
		pContext->StringToLocalUTF8(params[2], params[3], json_str, nullptr);
		free(json_str);
		return static_cast<cell_t>(json_size + 1);
	}

	return 0;
}

static cell_t json_doc_write_to_file(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);

	char realpath[PLATFORM_MAX_PATH];
	smutils->BuildPath(Path_Game, realpath, sizeof(realpath), "%s", path);

	yyjson_write_err writeError;
	bool is_success;

	if (handle->IsMutable()) {
		is_success = yyjson_mut_write_file(realpath, handle->m_pDocument_mut.get(), params[3], nullptr, &writeError);
	} else {
		is_success = yyjson_write_file(realpath, handle->m_pDocument.get(), params[3], nullptr, &writeError);
	}

	if (writeError.code) {
		return pContext->ThrowNativeError("Failed to write JSON to file: %s (error code: %u)", writeError.msg, writeError.code);
	}

	return is_success;
}

static cell_t json_obj_get_size(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return static_cast<cell_t>(yyjson_mut_obj_size(handle->m_pVal_mut));
	} else {
		return static_cast<cell_t>(yyjson_obj_size(handle->m_pVal));
	}
}

static cell_t json_obj_get_key(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t obj_size = yyjson_mut_obj_size(handle->m_pVal_mut);
		if (index >= obj_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, obj_size);
		}

		yyjson_mut_obj_iter iter;
		yyjson_mut_obj_iter_init(handle->m_pVal_mut, &iter);

		for (size_t i = 0; i < index; i++) {
			yyjson_mut_obj_iter_next(&iter);
		}

		yyjson_mut_val* key = yyjson_mut_obj_iter_next(&iter);
		if (!key) {
			return 0;
		}

		pContext->StringToLocalUTF8(params[3], params[4], yyjson_mut_get_str(key), nullptr);
	} else {
		size_t obj_size = yyjson_obj_size(handle->m_pVal);
		if (index >= obj_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, obj_size);
		}

		yyjson_obj_iter iter;
		yyjson_obj_iter_init(handle->m_pVal, &iter);

		for (size_t i = 0; i < index; i++) {
			yyjson_obj_iter_next(&iter);
		}

		yyjson_val* key = yyjson_obj_iter_next(&iter);
		if (!key) {
			return 0;
		}

		pContext->StringToLocalUTF8(params[3], params[4], yyjson_get_str(key), nullptr);
	}

	return 1;
}

static cell_t json_obj_get_val_at(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	auto pYYJsonWrapper = CreateWrapper();

	size_t index = static_cast<size_t>(params[2]);

	if (handle->IsMutable()) {
		size_t obj_size = yyjson_mut_obj_size(handle->m_pVal_mut);
		if (index >= obj_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, obj_size);
		}

		yyjson_mut_obj_iter iter;
		yyjson_mut_obj_iter_init(handle->m_pVal_mut, &iter);

		for (size_t i = 0; i < index; i++) {
			yyjson_mut_obj_iter_next(&iter);
		}

		yyjson_mut_val* key = yyjson_mut_obj_iter_next(&iter);
		if (!key) {
			return pContext->ThrowNativeError("Failed to get key at index %d", index);
		}

		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		pYYJsonWrapper->m_pVal_mut = yyjson_mut_obj_iter_get_val(key);
	} else {
		size_t obj_size = yyjson_obj_size(handle->m_pVal);
		if (index >= obj_size) {
			return pContext->ThrowNativeError("Index %d is out of bounds (size: %d)", index, obj_size);
		}

		yyjson_obj_iter iter;
		yyjson_obj_iter_init(handle->m_pVal, &iter);

		for (size_t i = 0; i < index; i++) {
			yyjson_obj_iter_next(&iter);
		}

		yyjson_val* key = yyjson_obj_iter_next(&iter);
		if (!key) {
			return pContext->ThrowNativeError("Failed to get key at index %d", index);
		}

		pYYJsonWrapper->m_pDocument = handle->m_pDocument;
		pYYJsonWrapper->m_pVal = yyjson_obj_iter_get_val(key);
	}

	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON object value at index %d (error code: %d)", params[2], err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_obj_get_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);

	auto pYYJsonWrapper = CreateWrapper();

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_obj_get(handle->m_pVal_mut, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		pYYJsonWrapper->m_pVal_mut = val;
	} else {
		yyjson_val* val = yyjson_obj_get(handle->m_pVal, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		pYYJsonWrapper->m_pDocument = handle->m_pDocument;
		pYYJsonWrapper->m_pVal = val;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON object value with key '%s' (error code: %d)", key, err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_obj_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_obj_get(handle->m_pVal_mut, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_mut_is_bool(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected boolean value, got %s", key, yyjson_mut_get_type_desc(val));
		}

		return yyjson_mut_get_bool(val);
	} else {
		yyjson_val* val = yyjson_obj_get(handle->m_pVal, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_is_bool(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected boolean value, got %s", key, yyjson_get_type_desc(val));
		}

		return yyjson_get_bool(val);
	}
}

static cell_t json_obj_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_obj_get(handle->m_pVal_mut, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_mut_is_real(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected float value, got %s", key, yyjson_mut_get_type_desc(val));
		}

		return sp_ftoc(static_cast<float>(yyjson_mut_get_real(val)));
	} else {
		yyjson_val* val = yyjson_obj_get(handle->m_pVal, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_is_real(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected float value, got %s", key, yyjson_get_type_desc(val));
		}

		return sp_ftoc(static_cast<float>(yyjson_get_real(val)));
	}
}

static cell_t json_obj_get_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_obj_get(handle->m_pVal_mut, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_mut_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected integer value, got %s", key, yyjson_mut_get_type_desc(val));
		}

		return yyjson_mut_get_int(val);
	} else {
		yyjson_val* val = yyjson_obj_get(handle->m_pVal, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected integer value, got %s", key, yyjson_get_type_desc(val));
		}

		return yyjson_get_int(val);
	}
}

static cell_t json_obj_get_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);
	int64_t value;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_obj_get(handle->m_pVal_mut, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_mut_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected integer64 value, got %s", key, yyjson_mut_get_type_desc(val));
		}
		value = yyjson_mut_get_sint(val);
	} else {
		yyjson_val* val = yyjson_obj_get(handle->m_pVal, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected integer64 value, got %s", key, yyjson_get_type_desc(val));
		}
		value = yyjson_get_sint(val);
	}

	char result[21];
	snprintf(result, sizeof(result), "%" PRId64, value);
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t json_obj_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_obj_get(handle->m_pVal_mut, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_mut_is_str(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected string value, got %s", key, yyjson_mut_get_type_desc(val));
		}

		const char* str = yyjson_mut_get_str(val);
		size_t len = yyjson_mut_get_len(val) + 1;
		size_t maxlen = static_cast<size_t>(params[4]);

		if (len > maxlen) {
			return pContext->ThrowNativeError("Buffer is too small (need %d, have %d)", len, maxlen);
		}

		pContext->StringToLocalUTF8(params[3], maxlen, str, nullptr);

		return 1;
	} else {
		yyjson_val* val = yyjson_obj_get(handle->m_pVal, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		if (!yyjson_is_str(val)) {
			return pContext->ThrowNativeError("Type mismatch for key '%s': expected string value, got %s", key, yyjson_get_type_desc(val));
		}

		const char* str = yyjson_get_str(val);
		size_t len = yyjson_get_len(val) + 1;
		size_t maxlen = static_cast<size_t>(params[4]);

		if (len > maxlen) {
			return pContext->ThrowNativeError("Buffer is too small (need %d, have %d)", len, maxlen);
		}

		pContext->StringToLocalUTF8(params[3], maxlen, str, nullptr);

		return 1;
	}
}

static cell_t json_obj_clear(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot clear an immutable JSON object");
	}

	return yyjson_mut_obj_clear(handle->m_pVal_mut);
}

static cell_t json_obj_is_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_obj_get(handle->m_pVal_mut, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		return yyjson_mut_is_null(val);
	} else {
		yyjson_val* val = yyjson_obj_get(handle->m_pVal, key);
		if (!val) {
			return pContext->ThrowNativeError("Key not found: %s", key);
		}

		return yyjson_is_null(val);
	}
}

static cell_t json_obj_has_key(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* key;
	pContext->LocalToString(params[2], &key);

	bool ptr_use = params[3];

	if (handle->IsMutable()) {
		if (ptr_use) {
			return yyjson_mut_doc_ptr_get(handle->m_pDocument_mut.get(), key) != nullptr;
		} else {
			yyjson_mut_obj_iter iter = yyjson_mut_obj_iter_with(handle->m_pVal_mut);
			return yyjson_mut_obj_iter_get(&iter, key) != nullptr;
		}
	} else {
		if (ptr_use) {
			return yyjson_doc_ptr_get(handle->m_pDocument.get(), key) != nullptr;
		} else {
			yyjson_obj_iter iter = yyjson_obj_iter_with(handle->m_pVal);
			return yyjson_obj_iter_get(&iter, key) != nullptr;
		}
	}
}

static cell_t json_obj_rename_key(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot rename key in an immutable JSON object");
	}

	char* old_key;
	pContext->LocalToString(params[2], &old_key);

	if (!yyjson_mut_obj_get(handle->m_pVal_mut, old_key)) {
		return pContext->ThrowNativeError("Key not found: %s", old_key);
	}

	char* new_key;
	pContext->LocalToString(params[3], &new_key);

	bool allow_duplicate = params[4];

	if (!allow_duplicate && yyjson_mut_obj_get(handle->m_pVal_mut, new_key)) {
		return pContext->ThrowNativeError("Key already exists: %s", new_key);
	}

	return yyjson_mut_obj_rename_key(handle->m_pDocument_mut.get(), handle->m_pVal_mut, old_key, new_key);
}

static cell_t json_obj_set_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return 0;

	if (!handle1->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON object");
	}

	char* key;
	pContext->LocalToString(params[2], &key);

	yyjson_mut_val* val_copy = nullptr;
	if (handle2->IsMutable()) {
		val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal_mut);
	} else {
		val_copy = yyjson_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal);
	}

	if (!val_copy) {
		return pContext->ThrowNativeError("Failed to copy JSON value");
	}

	return yyjson_mut_obj_put(handle1->m_pVal_mut, yyjson_mut_strcpy(handle1->m_pDocument_mut.get(), key), val_copy);
}

static cell_t json_obj_set_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON object");
	}

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), key), yyjson_mut_bool(handle->m_pDocument_mut.get(), params[3]));
}

static cell_t json_obj_set_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON object");
	}

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), key), yyjson_mut_float(handle->m_pDocument_mut.get(), sp_ctof(params[3])));
}

static cell_t json_obj_set_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON object");
	}

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), key), yyjson_mut_int(handle->m_pDocument_mut.get(), params[3]));
}

static cell_t json_obj_set_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON object");
	}

	char* key, * value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	char* endptr;
	errno = 0;
	long long num = strtoll(value, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0') {
		return pContext->ThrowNativeError("Invalid integer64 value: %s", value);
	}

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), key), yyjson_mut_int(handle->m_pDocument_mut.get(), num));
}

static cell_t json_obj_set_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON object");
	}

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), key), yyjson_mut_null(handle->m_pDocument_mut.get()));
}

static cell_t json_obj_set_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON object");
	}

	char* key, * value;
	pContext->LocalToString(params[2], &key);
	pContext->LocalToString(params[3], &value);

	return yyjson_mut_obj_put(handle->m_pVal_mut, yyjson_mut_strcpy(handle->m_pDocument_mut.get(), key), yyjson_mut_strcpy(handle->m_pDocument_mut.get(), value));
}

static cell_t json_obj_remove(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot remove value from an immutable JSON object");
	}

	char* key;
	pContext->LocalToString(params[2], &key);

	return yyjson_mut_obj_remove_key(handle->m_pVal_mut, key) != nullptr;
}

static cell_t json_ptr_get_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	auto pYYJsonWrapper = CreateWrapper();
	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (!val) {
			return pContext->ThrowNativeError("Failed to get value at path '%s'", path);
		}

		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		pYYJsonWrapper->m_pVal_mut = val;

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (!val) {
			return pContext->ThrowNativeError("Failed to get value at path '%s'", path);
		}

		pYYJsonWrapper->m_pDocument = handle->m_pDocument;
		pYYJsonWrapper->m_pVal = val;

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle)
	{
		return pContext->ThrowNativeError("Failed to create handle for JSON pointer value (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_ptr_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;
	
	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_mut_is_bool(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected boolean value, got %s", path, yyjson_mut_get_type_desc(val));
		}

		return yyjson_mut_get_bool(val);
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_is_bool(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected boolean value, got %s", path, yyjson_get_type_desc(val));
		}

		return yyjson_get_bool(val);
	}
}

static cell_t json_ptr_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_mut_is_real(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected float value, got %s", path, yyjson_mut_get_type_desc(val));
		}

		return sp_ftoc(static_cast<float>(yyjson_mut_get_real(val)));
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_is_real(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected float value, got %s", path, yyjson_get_type_desc(val));
		}

		return sp_ftoc(static_cast<float>(yyjson_get_real(val)));
	}
}

static cell_t json_ptr_get_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_mut_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected integer value, got %s", path, yyjson_mut_get_type_desc(val));
		}

		return yyjson_mut_get_int(val);
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected integer value, got %s", path, yyjson_get_type_desc(val));
		}

		return yyjson_get_int(val);
	}
}

static cell_t json_ptr_get_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;
	int64_t value;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_mut_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected integer64 value, got %s", path, yyjson_mut_get_type_desc(val));
		}
		value = yyjson_mut_get_sint(val);
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_is_int(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected integer64 value, got %s", path, yyjson_get_type_desc(val));
		}

		value = yyjson_get_sint(val);
	}

	char result[21];
	snprintf(result, sizeof(result), "%" PRId64, value);
	pContext->StringToLocalUTF8(params[3], params[4], result, nullptr);

	return 1;
}

static cell_t json_ptr_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_mut_is_str(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected string value, got %s", path, yyjson_mut_get_type_desc(val));
		}

		const char* str = yyjson_mut_get_str(val);
		size_t len = yyjson_mut_get_len(val) + 1;
		size_t maxlen = static_cast<size_t>(params[4]);

		if (len > maxlen) {
			return pContext->ThrowNativeError("Buffer is too small (need %d, have %d)", len, maxlen);
		}

		pContext->StringToLocalUTF8(params[3], maxlen, str, nullptr);

		return 1;
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (!yyjson_is_str(val)) {
			return pContext->ThrowNativeError("Type mismatch at path '%s': expected string value, got %s", path, yyjson_get_type_desc(val));
		}

		const char* str = yyjson_get_str(val);
		size_t len = yyjson_get_len(val) + 1;
		size_t maxlen = static_cast<size_t>(params[4]);

		if (len > maxlen) {
			return pContext->ThrowNativeError("Buffer is too small (need %d, have %d)", len, maxlen);
		}

		pContext->StringToLocalUTF8(params[3], maxlen, str, nullptr);

		return 1;
	}
}

static cell_t json_ptr_get_is_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		return yyjson_mut_is_null(val);
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		return yyjson_is_null(val);
	}
}

static cell_t json_ptr_get_length(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		// +1 for the null terminator
		if (yyjson_mut_is_str(val)) {
			return static_cast<cell_t>(yyjson_mut_get_len(val) + 1);
		}

		return static_cast<cell_t>(yyjson_mut_get_len(val));
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (ptrGetError.code) {
			return pContext->ThrowNativeError("Failed to resolve JSON pointer: %s (error code: %u, position: %d)", ptrGetError.msg, ptrGetError.code, ptrGetError.pos);
		}

		if (yyjson_is_str(val)) {
			return static_cast<cell_t>(yyjson_get_len(val) + 1);
		}

		return static_cast<cell_t>(yyjson_get_len(val));
	}
}

static cell_t json_ptr_set_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return 0;

	if (!handle1->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_mut_val* val_copy = nullptr;
	if (handle2->IsMutable()) {
		val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal_mut);
	} else {
		val_copy = yyjson_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal);
	}

	if (!val_copy) {
		return pContext->ThrowNativeError("Failed to copy JSON value");
	}

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle1->m_pDocument_mut.get(), path, strlen(path), val_copy, true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		return pContext->ThrowNativeError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
	}

	return success;
}

static cell_t json_ptr_set_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_bool(handle->m_pDocument_mut.get(), params[3]), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		return pContext->ThrowNativeError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
	}

	return success;
}

static cell_t json_ptr_set_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_float(handle->m_pDocument_mut.get(), sp_ctof(params[3])), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		return pContext->ThrowNativeError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
	}

	return success;
}

static cell_t json_ptr_set_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut.get(), params[3]), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		return pContext->ThrowNativeError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
	}

	return success;
}

static cell_t json_ptr_set_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON document using pointer");
	}

	char* path, * value;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &value);

	char* endptr;
	errno = 0;
	long long num = strtoll(value, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0') {
		return pContext->ThrowNativeError("Invalid integer64 value: %s", value);
	}

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut.get(), num), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		return pContext->ThrowNativeError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
	}

	return success;
}

static cell_t json_ptr_set_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON document using pointer");
	}

	char* path, * str;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &str);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_strcpy(handle->m_pDocument_mut.get(), str), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		return pContext->ThrowNativeError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
	}

	return success;
}

static cell_t json_ptr_set_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot set value in an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrSetError;
	bool success = yyjson_mut_doc_ptr_setx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_null(handle->m_pDocument_mut.get()), true, nullptr, &ptrSetError);

	if (ptrSetError.code) {
		return pContext->ThrowNativeError("Failed to set JSON pointer: %s (error code: %u, position: %d)", ptrSetError.msg, ptrSetError.code, ptrSetError.pos);
	}

	return success;
}

static cell_t json_ptr_add_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle1 = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	YYJsonWrapper* handle2 = g_JsonExtension.GetJSONPointer(pContext, params[3]);

	if (!handle1 || !handle2) return 0;

	if (!handle1->IsMutable()) {
		return pContext->ThrowNativeError("Cannot add value to an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_mut_val* val_copy = nullptr;
	if (handle2->IsMutable()) {
		val_copy = yyjson_mut_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal_mut);
	} else {
		val_copy = yyjson_val_mut_copy(handle1->m_pDocument_mut.get(), handle2->m_pVal);
	}

	if (!val_copy) {
		return pContext->ThrowNativeError("Failed to copy JSON value");
	}

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle1->m_pDocument_mut.get(), path, strlen(path), val_copy, true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		return pContext->ThrowNativeError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
	}

	return success;
}

static cell_t json_ptr_add_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot add value to an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_bool(handle->m_pDocument_mut.get(), params[3]), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		return pContext->ThrowNativeError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
	}

	return success;
}

static cell_t json_ptr_add_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot add value to an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_float(handle->m_pDocument_mut.get(), sp_ctof(params[3])), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		return pContext->ThrowNativeError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
	}

	return success;
}

static cell_t json_ptr_add_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot add value to an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut.get(), params[3]), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		return pContext->ThrowNativeError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
	}

	return success;
}

static cell_t json_ptr_add_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot add value to an immutable JSON document using pointer");
	}

	char* path, * value;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &value);

	char* endptr;
	errno = 0;
	long long num = strtoll(value, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0') {
		return pContext->ThrowNativeError("Invalid integer64 value: %s", value);
	}

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_int(handle->m_pDocument_mut.get(), num), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		return pContext->ThrowNativeError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
	}

	return success;
}

static cell_t json_ptr_add_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot add value to an immutable JSON document using pointer");
	}

	char* path, * str;
	pContext->LocalToString(params[2], &path);
	pContext->LocalToString(params[3], &str);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_strcpy(handle->m_pDocument_mut.get(), str), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		return pContext->ThrowNativeError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
	}

	return success;
}

static cell_t json_ptr_add_null(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot add value to an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrAddError;
	bool success = yyjson_mut_doc_ptr_addx(handle->m_pDocument_mut.get(), path, strlen(path), yyjson_mut_null(handle->m_pDocument_mut.get()), true, nullptr, &ptrAddError);

	if (ptrAddError.code) {
		return pContext->ThrowNativeError("Failed to add JSON pointer: %s (error code: %u, position: %d)", ptrAddError.msg, ptrAddError.code, ptrAddError.pos);
	}

	return success;
}

static cell_t json_ptr_remove_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot remove value from an immutable JSON document using pointer");
	}

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrRemoveError;
	bool success = yyjson_mut_doc_ptr_removex(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrRemoveError) != nullptr;

	if (ptrRemoveError.code) {
		return pContext->ThrowNativeError("Failed to remove JSON pointer: %s (error code: %u, position: %d)", ptrRemoveError.msg, ptrRemoveError.code, ptrRemoveError.pos);
	}

	return success;
}

static cell_t json_ptr_try_get_val(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	auto pYYJsonWrapper = CreateWrapper();
	char* path;
	pContext->LocalToString(params[2], &path);
	yyjson_ptr_err ptrGetError;

	cell_t* addr;
	pContext->LocalToPhysAddr(params[3], &addr);

	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (!val) {
			return 0;
		}

		if (ptrGetError.code) {
			return 0;
		}

		pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
		pYYJsonWrapper->m_pVal_mut = val;
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (!val) {
			return 0;
		}

		if (ptrGetError.code) {
			return 0;
		}

		pYYJsonWrapper->m_pDocument = handle->m_pDocument;
		pYYJsonWrapper->m_pVal = val;
	}

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle) {
		return 0;
	}

	*addr = pYYJsonWrapper.release()->m_handle;
	return 1;
}

static cell_t json_ptr_try_get_bool(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);

	cell_t* addr;
	pContext->LocalToPhysAddr(params[3], &addr);

	yyjson_ptr_err ptrGetError;
	
	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (!val) {
			return 0;
		}

		if (ptrGetError.code) {
			return 0;
		}

		if (!yyjson_mut_is_bool(val)) {
			return 0;
		}

		*addr = yyjson_mut_get_bool(val);
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);
		
		if (!val) {
			return 0;
		}

		if (ptrGetError.code) {
			return 0;
		}

		if (!yyjson_is_bool(val)) {
			return 0;
		}

		*addr = yyjson_get_bool(val);
	}
	
	return 1;
}

static cell_t json_ptr_try_get_float(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);

	cell_t* addr;
	pContext->LocalToPhysAddr(params[3], &addr);

	yyjson_ptr_err ptrGetError;
	
	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (!val) {
			return 0;
		}

		if (ptrGetError.code) {
			return 0;
		}

		if (!yyjson_mut_is_num(val)) {
			return 0;
		}

		*addr = sp_ftoc(static_cast<float>(yyjson_mut_get_num(val)));
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (!val) {
			return 0;
		}

		if (ptrGetError.code) {
			return 0;
		}

		if (!yyjson_is_num(val)) {
			return 0;
		}

		*addr = sp_ftoc(static_cast<float>(yyjson_get_num(val)));
	}
	
	return 1;
}

static cell_t json_ptr_try_get_int(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);

	cell_t* addr;
	pContext->LocalToPhysAddr(params[3], &addr);

	yyjson_ptr_err ptrGetError;
	
	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (!val || ptrGetError.code || !yyjson_mut_is_int(val)) {
			return 0;
		}
		*addr = yyjson_mut_get_int(val);
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (!val || ptrGetError.code || !yyjson_is_int(val)) {
			return 0;
		}
		*addr = yyjson_get_int(val);
	}
	
	return 1;
}

static cell_t json_ptr_try_get_integer64(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);
	
	int64_t value;
	yyjson_ptr_err ptrGetError;
	
	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);
		if (!val || ptrGetError.code || !yyjson_mut_is_int(val)) {
			return 0;
		}
		value = yyjson_mut_get_sint(val);
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);
		if (!val || ptrGetError.code || !yyjson_is_int(val)) {
			return 0;
		}
		value = yyjson_get_sint(val);
	}

	size_t maxlen = static_cast<size_t>(params[4]);

	char result[21];
	snprintf(result, sizeof(result), "%" PRId64, value);
	pContext->StringToLocalUTF8(params[3], maxlen, result, nullptr);
	return 1;
}

static cell_t json_ptr_try_get_str(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	char* path;
	pContext->LocalToString(params[2], &path);

	yyjson_ptr_err ptrGetError;
	const char* str;
	size_t len;
	
	if (handle->IsMutable()) {
		yyjson_mut_val* val = yyjson_mut_doc_ptr_getx(handle->m_pDocument_mut.get(), path, strlen(path), nullptr, &ptrGetError);

		if (!val || ptrGetError.code || !yyjson_mut_is_str(val)) {
			return 0;
		}

		str = yyjson_mut_get_str(val);
		len = yyjson_mut_get_len(val) + 1;
	} else {
		yyjson_val* val = yyjson_doc_ptr_getx(handle->m_pDocument.get(), path, strlen(path), &ptrGetError);

		if (!val || ptrGetError.code || !yyjson_is_str(val)) {
			return 0;
		}

		str = yyjson_get_str(val);
		len = yyjson_get_len(val) + 1;
	}

	size_t maxlen = static_cast<size_t>(params[4]);
	if (len > maxlen) {
		return 0;
	}

	pContext->StringToLocalUTF8(params[3], maxlen, str, nullptr);
	
	return 1;
}

static cell_t json_obj_foreach(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_obj(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected object value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_mut_obj_iter_init(handle->m_pVal_mut, &handle->m_iterObj)) {
				return pContext->ThrowNativeError("Failed to initialize object iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_mut_val* key = yyjson_mut_obj_iter_next(&handle->m_iterObj);
		if (key) {
			yyjson_mut_val* val = yyjson_mut_obj_iter_get_val(key);

			pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_str(key), nullptr);

			auto pYYJsonWrapper = CreateWrapper();
			pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
			pYYJsonWrapper->m_pVal_mut = val;

			HandleError err;
			HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
			pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

			if (!pYYJsonWrapper->m_handle) {
				return pContext->ThrowNativeError("Failed to create handle for JSON object value (error code: %d)", err);
			}

			cell_t* valHandle;
			pContext->LocalToPhysAddr(params[4], &valHandle);
			*valHandle = pYYJsonWrapper.release()->m_handle;

			return true;
		}
	} else {
		if (!yyjson_is_obj(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected object value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_obj_iter_init(handle->m_pVal, &handle->m_iterObjImm)) {
				return pContext->ThrowNativeError("Failed to initialize object iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_val* key = yyjson_obj_iter_next(&handle->m_iterObjImm);
		if (key) {
			yyjson_val* val = yyjson_obj_iter_get_val(key);

			pContext->StringToLocalUTF8(params[2], params[3], yyjson_get_str(key), nullptr);

			auto pYYJsonWrapper = CreateWrapper();
			pYYJsonWrapper->m_pDocument = handle->m_pDocument;
			pYYJsonWrapper->m_pVal = val;

			HandleError err;
			HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
			pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

			if (!pYYJsonWrapper->m_handle) {
				return pContext->ThrowNativeError("Failed to create handle for JSON object value (error code: %d)", err);
			}

			cell_t* valHandle;
			pContext->LocalToPhysAddr(params[4], &valHandle);
			*valHandle = pYYJsonWrapper.release()->m_handle;

			return true;
		}
	}

	handle->ResetObjectIterator();
	return false;
}

static cell_t json_arr_foreach(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_mut_arr_iter_init(handle->m_pVal_mut, &handle->m_iterArr)) {
				return pContext->ThrowNativeError("Failed to initialize array iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_mut_val* val = yyjson_mut_arr_iter_next(&handle->m_iterArr);
		if (val) {
			cell_t* index;
			pContext->LocalToPhysAddr(params[2], &index);
			*index = static_cast<cell_t>(handle->m_arrayIndex);

			auto pYYJsonWrapper = CreateWrapper();
			pYYJsonWrapper->m_pDocument_mut = handle->m_pDocument_mut;
			pYYJsonWrapper->m_pVal_mut = val;

			HandleError err;
			HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
			pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

			if (!pYYJsonWrapper->m_handle) {
				return pContext->ThrowNativeError("Failed to create handle for JSON array value (error code: %d)", err);
			}

			cell_t* valHandle;
			pContext->LocalToPhysAddr(params[3], &valHandle);
			*valHandle = pYYJsonWrapper.release()->m_handle;

			handle->m_arrayIndex++;
			return true;
		}
	} else {
		if (!yyjson_is_arr(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_arr_iter_init(handle->m_pVal, &handle->m_iterArrImm)) {
				return pContext->ThrowNativeError("Failed to initialize array iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_val* val = yyjson_arr_iter_next(&handle->m_iterArrImm);
		if (val) {
			cell_t* index;
			pContext->LocalToPhysAddr(params[2], &index);
			*index = static_cast<cell_t>(handle->m_arrayIndex);

			auto pYYJsonWrapper = CreateWrapper();
			pYYJsonWrapper->m_pDocument = handle->m_pDocument;
			pYYJsonWrapper->m_pVal = val;

			HandleError err;
			HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
			pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

			if (!pYYJsonWrapper->m_handle) {
				return pContext->ThrowNativeError("Failed to create handle for JSON array value (error code: %d)", err);
			}

			cell_t* valHandle;
			pContext->LocalToPhysAddr(params[3], &valHandle);
			*valHandle = pYYJsonWrapper.release()->m_handle;

			handle->m_arrayIndex++;
			return true;
		}
	}

	handle->ResetArrayIterator();
	return false;
}

static cell_t json_obj_foreach_key(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_obj(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected object value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_mut_obj_iter_init(handle->m_pVal_mut, &handle->m_iterObj)) {
				return pContext->ThrowNativeError("Failed to initialize object iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_mut_val* key = yyjson_mut_obj_iter_next(&handle->m_iterObj);
		if (key) {
			yyjson_mut_val* val = yyjson_mut_obj_iter_get_val(key);
			pContext->StringToLocalUTF8(params[2], params[3], yyjson_mut_get_str(key), nullptr);
			return true;
		}
	} else {
		if (!yyjson_is_obj(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected object value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_obj_iter_init(handle->m_pVal, &handle->m_iterObjImm)) {
				return pContext->ThrowNativeError("Failed to initialize object iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_val* key = yyjson_obj_iter_next(&handle->m_iterObjImm);
		if (key) {
			yyjson_val* val = yyjson_obj_iter_get_val(key);
			pContext->StringToLocalUTF8(params[2], params[3], yyjson_get_str(key), nullptr);
			return true;
		}
	}

	handle->ResetObjectIterator();
	return false;
}

static cell_t json_arr_foreach_index(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);
	if (!handle) return 0;

	if (handle->IsMutable()) {
		if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_mut_get_type_desc(handle->m_pVal_mut));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_mut_arr_iter_init(handle->m_pVal_mut, &handle->m_iterArr)) {
				return pContext->ThrowNativeError("Failed to initialize array iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_mut_val* val = yyjson_mut_arr_iter_next(&handle->m_iterArr);
		if (val) {
			cell_t* index;
			pContext->LocalToPhysAddr(params[2], &index);
			*index = static_cast<cell_t>(handle->m_arrayIndex);
			handle->m_arrayIndex++;
			return true;
		}
	} else {
		if (!yyjson_is_arr(handle->m_pVal)) {
			return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
				yyjson_get_type_desc(handle->m_pVal));
		}

		if (!handle->m_iterInitialized) {
			if (!yyjson_arr_iter_init(handle->m_pVal, &handle->m_iterArrImm)) {
				return pContext->ThrowNativeError("Failed to initialize array iterator");
			}
			handle->m_iterInitialized = true;
		}

		yyjson_val* val = yyjson_arr_iter_next(&handle->m_iterArrImm);
		if (val) {
			cell_t* index;
			pContext->LocalToPhysAddr(params[2], &index);
			*index = static_cast<cell_t>(handle->m_arrayIndex);
			handle->m_arrayIndex++;
			return true;
		}
	}

	handle->ResetArrayIterator();
	return false;
}

static cell_t json_arr_sort(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot sort an immutable JSON array");
	}

	if (!yyjson_mut_is_arr(handle->m_pVal_mut)) {
		return pContext->ThrowNativeError("Type mismatch: expected array value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
	}

	YYJSON_SORT_ORDER sort_mode = static_cast<YYJSON_SORT_ORDER>(params[2]);
	if (sort_mode < YYJSON_SORT_ASC || sort_mode > YYJSON_SORT_RANDOM) {
		return pContext->ThrowNativeError("Invalid sort mode: %d (expected 0=ascending, 1=descending, 2=random)", sort_mode);
	}

	size_t arr_size = yyjson_mut_arr_size(handle->m_pVal_mut);
	if (arr_size <= 1) return true;

	static thread_local std::vector<yyjson_mut_val*> values;
	values.clear();
	values.reserve(arr_size);

	size_t idx, max;
	yyjson_mut_val *val;
	yyjson_mut_arr_foreach(handle->m_pVal_mut, idx, max, val) {
		values.push_back(val);
	}

	if (sort_mode == YYJSON_SORT_RANDOM) {
		std::shuffle(values.begin(), values.end(), g_randomGenerator);
	}
	else {
		auto compare = [sort_mode](yyjson_mut_val* a, yyjson_mut_val* b) {
			if (a == b) return false;

			uint8_t type_a = yyjson_mut_get_type(a);
			uint8_t type_b = yyjson_mut_get_type(b);
			if (type_a != type_b) {
				return sort_mode == YYJSON_SORT_ASC ? type_a < type_b : type_a > type_b;
			}

			switch (type_a) {
			case YYJSON_TYPE_STR: {
				const char* str_a = yyjson_mut_get_str(a);
				const char* str_b = yyjson_mut_get_str(b);
				int cmp = strcmp(str_a, str_b);
				return sort_mode == YYJSON_SORT_ASC ? cmp < 0 : cmp > 0;
			}
			case YYJSON_TYPE_NUM: {
				if (yyjson_mut_is_int(a) && yyjson_mut_is_int(b)) {
					int64_t num_a = yyjson_mut_get_int(a);
					int64_t num_b = yyjson_mut_get_int(b);
					return sort_mode == YYJSON_SORT_ASC ? num_a < num_b : num_a > num_b;
				}
				double num_a = yyjson_mut_get_num(a);
				double num_b = yyjson_mut_get_num(b);
				return sort_mode == YYJSON_SORT_ASC ? num_a < num_b : num_a > num_b;
			}
			case YYJSON_TYPE_BOOL: {
				bool val_a = yyjson_mut_get_bool(a);
				bool val_b = yyjson_mut_get_bool(b);
				return sort_mode == YYJSON_SORT_ASC ? val_a < val_b : val_a > val_b;
			}
			default:
				return false;
			}
			};

		std::sort(values.begin(), values.end(), compare);
	}

	yyjson_mut_arr_clear(handle->m_pVal_mut);
	for (auto val : values) {
		yyjson_mut_arr_append(handle->m_pVal_mut, val);
	}

	return true;
}

static cell_t json_obj_sort(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Cannot sort an immutable JSON object");
	}

	if (!yyjson_mut_is_obj(handle->m_pVal_mut)) {
		return pContext->ThrowNativeError("Type mismatch: expected object value, got %s",
			yyjson_mut_get_type_desc(handle->m_pVal_mut));
	}

	YYJSON_SORT_ORDER sort_mode = static_cast<YYJSON_SORT_ORDER>(params[2]);
	if (sort_mode < YYJSON_SORT_ASC || sort_mode > YYJSON_SORT_RANDOM) {
		return pContext->ThrowNativeError("Invalid sort mode: %d (expected 0=ascending, 1=descending, 2=random)", sort_mode);
	}

	size_t obj_size = yyjson_mut_obj_size(handle->m_pVal_mut);
	if (obj_size <= 1) return true;

	static thread_local std::vector<std::pair<yyjson_mut_val*, yyjson_mut_val*>> pairs;
	pairs.clear();
	pairs.reserve(obj_size);

	size_t idx, max;
	yyjson_mut_val *key, *val;
	yyjson_mut_obj_foreach(handle->m_pVal_mut, idx, max, key, val) {
		pairs.emplace_back(key, val);
	}

	if (sort_mode == YYJSON_SORT_RANDOM) {
		std::shuffle(pairs.begin(), pairs.end(), g_randomGenerator);
	}
	else {
		auto compare = [sort_mode](const auto& a, const auto& b) {
			const char* key_a = yyjson_mut_get_str(a.first);
			const char* key_b = yyjson_mut_get_str(b.first);
			int cmp = strcmp(key_a, key_b);
			return sort_mode == YYJSON_SORT_ASC ? cmp < 0 : cmp > 0;
			};

		std::sort(pairs.begin(), pairs.end(), compare);
	}

	yyjson_mut_obj_clear(handle->m_pVal_mut);
	for (const auto& pair : pairs) {
		yyjson_mut_obj_add(handle->m_pVal_mut, pair.first, pair.second);
	}

	return true;
}

static cell_t json_doc_to_mutable(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (handle->IsMutable()) {
		return pContext->ThrowNativeError("Document is already mutable");
	}

	auto pYYJsonWrapper = CreateWrapper();
	pYYJsonWrapper->m_pDocument_mut = CopyDocument(handle->m_pDocument.get());
	pYYJsonWrapper->m_pVal_mut = yyjson_mut_doc_get_root(pYYJsonWrapper->m_pDocument_mut.get());

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle) {
		return pContext->ThrowNativeError("Failed to create handle for mutable JSON document (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

static cell_t json_doc_to_immutable(IPluginContext* pContext, const cell_t* params)
{
	YYJsonWrapper* handle = g_JsonExtension.GetJSONPointer(pContext, params[1]);

	if (!handle) return 0;

	if (!handle->IsMutable()) {
		return pContext->ThrowNativeError("Document is already immutable");
	}

	auto pYYJsonWrapper = CreateWrapper();
	yyjson_doc* mdoc = yyjson_mut_doc_imut_copy(handle->m_pDocument_mut.get(), nullptr);
	pYYJsonWrapper->m_pDocument = WrapImmutableDocument(mdoc);
	pYYJsonWrapper->m_pVal = yyjson_doc_get_root(pYYJsonWrapper->m_pDocument.get());

	HandleError err;
	HandleSecurity sec(pContext->GetIdentity(), myself->GetIdentity());
	pYYJsonWrapper->m_handle = handlesys->CreateHandleEx(g_htJSON, pYYJsonWrapper.get(), &sec, nullptr, &err);

	if (!pYYJsonWrapper->m_handle) {
		return pContext->ThrowNativeError("Failed to create handle for immutable JSON document (error code: %d)", err);
	}

	return pYYJsonWrapper.release()->m_handle;
}

const sp_nativeinfo_t json_natives[] =
{
	// JSONObject
	{"YYJSONObject.YYJSONObject", json_obj_init},
	{"YYJSONObject.FromStrings", json_obj_init_with_str},
	{"YYJSONObject.Size.get", json_obj_get_size},
	{"YYJSONObject.Get", json_obj_get_val},
	{"YYJSONObject.GetBool", json_obj_get_bool},
	{"YYJSONObject.GetFloat", json_obj_get_float},
	{"YYJSONObject.GetInt", json_obj_get_int},
	{"YYJSONObject.GetInt64", json_obj_get_integer64},
	{"YYJSONObject.GetString", json_obj_get_str},
	{"YYJSONObject.IsNull", json_obj_is_null},
	{"YYJSONObject.GetKey", json_obj_get_key},
	{"YYJSONObject.GetValueAt", json_obj_get_val_at},
	{"YYJSONObject.HasKey", json_obj_has_key},
	{"YYJSONObject.RenameKey", json_obj_rename_key},
	{"YYJSONObject.Set", json_obj_set_val},
	{"YYJSONObject.SetBool", json_obj_set_bool},
	{"YYJSONObject.SetFloat", json_obj_set_float},
	{"YYJSONObject.SetInt", json_obj_set_int},
	{"YYJSONObject.SetInt64", json_obj_set_integer64},
	{"YYJSONObject.SetNull", json_obj_set_null},
	{"YYJSONObject.SetString", json_obj_set_str},
	{"YYJSONObject.Remove", json_obj_remove},
	{"YYJSONObject.Clear", json_obj_clear},
	{"YYJSONObject.FromString", json_obj_parse_str},
	{"YYJSONObject.FromFile", json_obj_parse_file},
	{"YYJSONObject.Sort", json_obj_sort},

	// JSONArray
	{"YYJSONArray.YYJSONArray", json_arr_init},
	{"YYJSONArray.FromStrings", json_arr_init_with_str},
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
	{"YYJSONArray.PushInt64", json_arr_append_integer64},
	{"YYJSONArray.PushNull", json_arr_append_null},
	{"YYJSONArray.PushString", json_arr_append_str},
	{"YYJSONArray.Remove", json_arr_remove},
	{"YYJSONArray.RemoveFirst", json_arr_remove_first},
	{"YYJSONArray.RemoveLast", json_arr_remove_last},
	{"YYJSONArray.RemoveRange", json_arr_remove_range},
	{"YYJSONArray.Clear", json_arr_clear},
	{"YYJSONArray.FromString", json_arr_parse_str},
	{"YYJSONArray.FromFile", json_arr_parse_file},
	{"YYJSONArray.IndexOfBool", json_arr_index_of_bool},
	{"YYJSONArray.IndexOfString", json_arr_index_of_str},
	{"YYJSONArray.IndexOfInt", json_arr_index_of_int},
	{"YYJSONArray.IndexOfInt64", json_arr_index_of_integer64},
	{"YYJSONArray.IndexOfFloat", json_arr_index_of_float},
	{"YYJSONArray.Sort", json_arr_sort},

	// JSON
	{"YYJSON.ToString", json_doc_write_to_str},
	{"YYJSON.ToFile", json_doc_write_to_file},
	{"YYJSON.Parse", json_doc_parse},
	{"YYJSON.Equals", json_doc_equals},
	{"YYJSON.DeepCopy", json_doc_copy_deep},
	{"YYJSON.GetTypeDesc", json_val_get_type_desc},
	{"YYJSON.GetSerializedSize", json_val_get_serialized_size},
	{"YYJSON.ReadSize.get", json_val_get_read_size},
	{"YYJSON.Type.get", json_val_get_type},
	{"YYJSON.SubType.get", json_val_get_subtype},
	{"YYJSON.IsArray.get", json_val_is_array},
	{"YYJSON.IsObject.get", json_val_is_object},
	{"YYJSON.IsInt.get", json_val_is_int},
	{"YYJSON.IsUint.get", json_val_is_uint},
	{"YYJSON.IsSint.get", json_val_is_sint},
	{"YYJSON.IsNum.get", json_val_is_num},
	{"YYJSON.IsBool.get", json_val_is_bool},
	{"YYJSON.IsTrue.get", json_val_is_true},
	{"YYJSON.IsFalse.get", json_val_is_false},
	{"YYJSON.IsFloat.get", json_val_is_float},
	{"YYJSON.IsStr.get", json_val_is_str},
	{"YYJSON.IsNull.get", json_val_is_null},
	{"YYJSON.IsCtn.get", json_val_is_ctn},
	{"YYJSON.IsMutable.get", json_val_is_mutable},
	{"YYJSON.IsImmutable.get", json_val_is_immutable},
	{"YYJSON.ForeachObject", json_obj_foreach},
	{"YYJSON.ForeachArray", json_arr_foreach},
	{"YYJSON.ForeachKey", json_obj_foreach_key},
	{"YYJSON.ForeachIndex", json_arr_foreach_index},
	{"YYJSON.ToMutable", json_doc_to_mutable},
	{"YYJSON.ToImmutable", json_doc_to_immutable},

	// JSON CREATE & GET
	{"YYJSON.Pack", json_val_pack},
	{"YYJSON.CreateBool", json_val_create_bool},
	{"YYJSON.CreateFloat", json_val_create_float},
	{"YYJSON.CreateInt", json_val_create_int},
	{"YYJSON.CreateInt64", json_val_create_integer64},
	{"YYJSON.CreateNull", json_val_create_null},
	{"YYJSON.CreateString", json_val_create_str},
	{"YYJSON.GetBool", json_val_get_bool},
	{"YYJSON.GetFloat", json_val_get_float},
	{"YYJSON.GetInt", json_val_get_int},
	{"YYJSON.GetInt64", json_val_get_integer64},
	{"YYJSON.GetString", json_val_get_str},

	// JSON POINTER
	{"YYJSON.PtrGet", json_ptr_get_val},
	{"YYJSON.PtrGetBool", json_ptr_get_bool},
	{"YYJSON.PtrGetFloat", json_ptr_get_float},
	{"YYJSON.PtrGetInt", json_ptr_get_int},
	{"YYJSON.PtrGetInt64", json_ptr_get_integer64},
	{"YYJSON.PtrGetString", json_ptr_get_str},
	{"YYJSON.PtrGetIsNull", json_ptr_get_is_null},
	{"YYJSON.PtrGetLength", json_ptr_get_length},
	{"YYJSON.PtrSet", json_ptr_set_val},
	{"YYJSON.PtrSetBool", json_ptr_set_bool},
	{"YYJSON.PtrSetFloat", json_ptr_set_float},
	{"YYJSON.PtrSetInt", json_ptr_set_int},
	{"YYJSON.PtrSetInt64", json_ptr_set_integer64},
	{"YYJSON.PtrSetString", json_ptr_set_str},
	{"YYJSON.PtrSetNull", json_ptr_set_null},
	{"YYJSON.PtrAdd", json_ptr_add_val},
	{"YYJSON.PtrAddBool", json_ptr_add_bool},
	{"YYJSON.PtrAddFloat", json_ptr_add_float},
	{"YYJSON.PtrAddInt", json_ptr_add_int},
	{"YYJSON.PtrAddInt64", json_ptr_add_integer64},
	{"YYJSON.PtrAddString", json_ptr_add_str},
	{"YYJSON.PtrAddNull", json_ptr_add_null},
	{"YYJSON.PtrRemove", json_ptr_remove_val},
	{"YYJSON.PtrTryGetVal", json_ptr_try_get_val},
	{"YYJSON.PtrTryGetBool", json_ptr_try_get_bool},
	{"YYJSON.PtrTryGetFloat", json_ptr_try_get_float},
	{"YYJSON.PtrTryGetInt", json_ptr_try_get_int},
	{"YYJSON.PtrTryGetInt64", json_ptr_try_get_integer64},
	{"YYJSON.PtrTryGetString", json_ptr_try_get_str},
	{nullptr, nullptr}
};