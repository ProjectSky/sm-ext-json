#if defined _json_included
#endinput
#endif
#define _json_included

// JSON types
enum JSONType
{
	JSONError = -1,
	JSONNull = 1,
	JSONString = 2,
	JSONNumber = 3,
	JSONObject = 4,
	JSONArray = 5,
	JSONBoolean = 6,
	JSONInteger = 7,
}

// JSON Result
enum JSONResult
{
	JSONSuccess = 0,
	JSONFailure = -1
}

// JSON invalid handle
enum JSON
{
	Invalid_JSON = -1
}

// Generic type for encoding JSON
methodmap JSON < Handle
{
	// Writes the JSON string representation to a file
	//
	// @param file       File to write to
	// @param pretty     True to count size for pretty format, false to not
	// @return           True on success, false on failure
	public native bool ToFile(const char[] file, bool pretty = false);

	// Retrieves the JSON string representation
	//
	// @param buffer     String buffer to write to
	// @param maxlength  Maximum length of the string buffer
	// @param pretty     True to count size for pretty format, false to not
	// @return           True on success, false on failure
	public native bool ToString(char[] buffer, int maxlength, bool pretty = false);

	// Parses JSON string or a file that contains JSON
	//
	// @note                    Must be freed via delete or CloseHandle()
	//
	// @param string            String to parse
	// @param is_file           True to treat string param as filename, false otherwise
	// @param with_comments     True if parsing JSON includes comments (it will ignore them), false otherwise
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	public static native JSON Parse(const char[] string, bool is_file = false, bool with_comments = false);

	// Checks if the first value is the same as the second one
	//
	// @param value1            JSON handle
	// @param value2            JSON handle
	//
	// @return                  True if they are the same, false otherwise
	// @error                   If passed value is not a valid handle
	public static native bool Equals(const JSON value1, const JSON value2);

	// Validates json by checking if object have identically named
	// fields with matching types
	//
	// @note                    Schema {"name":"", "age":0} will validate
	//                          {"name":"Joe", "age":25} and {"name":"Joe", "age":25, "gender":"m"},
	//                          but not {"name":"Joe"} or {"name":"Joe", "age":"Cucumber"}
	//
	// @note                    In case of arrays, only first value in schema
	//                          is checked against all values in tested array
	//
	// @note                    Empty objects ({}) validate all objects,
	//                          empty arrays ([]) validate all arrays,
	//                          null validates values of every type
	//
	// @param schema            JSON handle
	// @param value             JSON handle
	//
	// @return                  True if passed value is valid, false otherwise
	// @error                   If a schema handle or value handle is invalid
	public static native bool Validate(const JSON schema, const JSON value);

	// Gets value's parent handle
	//
	// @note                  Parent's handle must be freed via delete or CloseHandle()
	//
	// @param value           JSON handle
	//
	// @return                Parent's handle
	public static native JSON GetParent(const JSON value);

	// Inits string data
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @param value             String that the handle will be initialized with
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	public static native JSON String(const char[] value);

	// Inits a integer
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @param value             Integer that the handle will be initialized with
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	public static native JSON Int(int value);

	// Inits a 64-bit integer
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @param value             64-bit integer that the handle will be initialized with
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	public static native JSON Int64(const char[] value);

	// Inits a real number
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @param value             Real number that the handle will be initialized with
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	public static native JSON Float(float value);

	// Inits a boolean value
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @param value             Boolean value that the handle will be initialized with
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	public static native JSON Bool(bool value);

	// Inits a null
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	public static native JSON Null();

	// Creates deep copy of passed value
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @param value             JSON handle to be copied
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	// @error                   If passed value is not a valid handle
	public static native JSON DeepCopy(const JSON value);

	// Gets string data
	//
	// @param value             JSON handle
	// @param buffer            Buffer to copy string to
	// @param maxlength         Maximum size of the buffer
	//
	// @return                  True on success, false on failure
	// @error                   If passed value is not a valid handle
	public static native bool GetString(const JSON value, char[] buffer, int maxlength);

	// Gets a integer
	//
	// @param value             JSON handle
	//
	// @return                  integer
	// @error                   If passed value is not a valid handle
	public static native int GetInt(const JSON value);

	// Gets a 64-bit integer
	//
	// @param value             JSON handle
	//
	// @return                  64-bit integer
	// @error                   If passed value is not a valid handle
	public static native int GetInt64(const JSON value, char[] buffer, int maxlength);

	// Gets a real number
	//
	// @param value             JSON handle
	//
	// @return                  Real number
	// @error                   If passed value is not a valid handle
	public static native float GetFloat(const JSON value);

	// Gets a boolean value
	//
	// @param value             JSON handle
	//
	// @return                  Boolean value
	// @error                   If passed value is not a valid handle
	public static native bool GetBool(const JSON value);

	// Gets size of serialization
	//
	// @param value             JSON handle
	// @param pretty            True to count size for pretty format, false to not
	// @param null_byte         True to include null byte, false to not
	//
	// @return                  Size of serialized string
	// @error                   If passed handle is not a valid value
	public static native int SerialSize(const JSON value, bool pretty = false, bool null_byte = false);

	// Sets if slashes should be escaped or not when serializing JSON.
	// By default slashes are escaped.
	// This function sets a global setting
	//
	// @param escape_slashes   True escape slashes, false to not
	public static native void EscapeSlashes(bool escape_slashes);

	// Sets float format used for serialization of numbers.
	// Make sure it can't serialize to a string longer than PARSON_NUM_BUF_SIZE.
	// If format is null then the default format (%1.17g) is used.
	//
	// @param format             float serialization formatter
	public static native void FloatSerialize(const char[] format);

	// Retrieves the type of the json
	property int Type {
		public native get();
	}
};

methodmap JSONObject < JSON
{
	// Creates a JSON object A JSON object maps strings (called "keys") to values Keys in a
	// JSON object are unique That is, there is at most one entry in the map for a given key
	//
	// The JSONObject must be freed via delete or CloseHandle()
	public native JSONObject();

	// Loads a JSON object from a file
	//
	// @param file              File to read from
 	// @param with_comments     True if parsing JSON includes comments (it will ignore them), false otherwise
	// @return                  Object handle, or null on failure
	// @error                   Invalid JSON
	public static native JSONObject FromFile(const char[] file, bool with_comments = false);

	// Loads a JSON object from a string
	//
	// @param buffer            String buffer to load into the JSON object
 	// @param with_comments     True if parsing JSON includes comments (it will ignore them), false otherwise
	// @return                  Object handle, or null on failure
	// @error                   Invalid JSON
	public static native JSONObject FromString(const char[] buffer, bool with_comments = false);

	// Gets a value from the object
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	//
	// @param key               Key name
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	// @error                   If passed handle is not a valid object
	public native JSON Get(const char[] key, bool dot_not = false);

	// Gets a boolean value from the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	//
	// @param key               Key name
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  Boolean value
	// @error                   If passed handle is not a valid object
	public native bool GetBool(const char[] key, bool dot_not = false);

	// Gets a float value from the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	//
	// @param key               Key name
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  Float value
	// @error                   If passed handle is not a valid object
	public native float GetFloat(const char[] key, bool dot_not = false);

	// Gets a integer value from the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	//
	// @param key               Key name
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  Integer value
	// @error                   If passed handle is not a valid object
	public native int GetInt(const char[] key, bool dot_not = false);

	// Retrieves a 64-bit integer value from the object.
	//
	// @param key               Key string.
	// @param buffer            String buffer to store value.
	// @param maxlength         Maximum length of the string buffer.
	// @param dot_not           True to use dot notation, false to not
	// @return                  True on success, false if the key was not found.
	public native bool GetInt64(const char[] key, char[] buffer, int maxlength, bool dot_not = false);

	// Gets name of the object's key
	//
	// @param index        Position from which get key name
	// @param buffer       Buffer to copy string to
	// @param maxlength    Maximum size of the buffer
	//
	// @return             True on success, false on failure
	// @error              If passed handle is not a valid object
	public native bool GetName(int index, char[] buffer, int maxlength);

	// Gets a value at the specified position from the object
	//
	// @note               Needs to be freed using delete or CloseHandle()
	//
	// @param index        Position from which get key name
	//
	// @return             JSON handle, Invalid_JSON if error occurred
	// @error              If passed handle is not a valid object
	public native JSON GetValueAt(int index);

	// Returns whether or not a key exists in the object
	//
	// @param key        Key string
	// @param dot_not    True to use dot notation, false to not
	// @return           True if the key exists, false otherwise
	public native bool HasKey(const char[] key, bool dot_not = false);

	// Checks if the object has a value with a specific name and type
	//
	// @param key           Key name
	// @param type          Type of value, if JSONError type will not be checked
	// @param dot_not       True to use dot notation, false to not
	//
	// @return              True if has, false if not
	// @error               If passed handle is not a valid object
	public native bool HasValue(const char[] name, JSONType type = JSONError, bool dot_not = false);

	// Gets string data from the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	//
	// @param key               Key name
	// @param buffer            Buffer to copy string to
	// @param maxlength         Maximum size of the buffer
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True on success, false on failure
	// @error                   If passed handle is not a valid object
	public native bool GetString(const char[] key, char[] buffer, int maxlength, bool dot_not = false);

	// Returns whether or not a value in the object is null
	//
	// @param key               Key string
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if the value is null, false otherwise
	// @error                   Invalid key
	public native bool IsNull(const char[] key, bool dot_not = false);

	// Sets a value in the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	// @note                    It also removes the old value if any
	//
	// @param key               Key name
	// @param value             JSON handle to set
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool Set(const char[] key, JSON value, bool dot_not = false);

	// Sets a boolean value in the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	// @note                    It also removes the old value if any
	//
	// @param key               Key name
	// @param value             Boolean value to set
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool SetBool(const char[] key, bool value, bool dot_not = false);

	// Sets a float value in the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	// @note                    It also removes the old value if any
	//
	// @param key               Key name
	// @param value             Real number to set
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool SetFloat(const char[] key, float value, bool dot_not = false);

	// Sets a integer value in the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	// @note                    It also removes the old value if any
	//
	// @param key               Key name
	// @param value             integer to set
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool SetInt(const char[] key, int value, bool dot_not = false);

	// Sets a 64-bit integer value in the object, either inserting a new entry or replacing an old one.
	//
	// @param key               Key string.
	// @param value             Value to store at this key.
	// @param dot_not           True to use dot notation, false to not
	// @return                  True on success, false on failure.
	public native bool SetInt64(const char[] key, const char[] value, bool dot_not = false);

	// Sets a null in the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	// @note                    It also removes the old value if any
	//
	// @param key               Key name
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool SetNull(const char[] key, bool dot_not = false);

	// Sets string data in the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	// @note                    It also removes the old value if any
	//
	// @param key               Key name
	// @param value             String to copy
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool SetString(const char[] key, const char[] value, bool dot_not = false);

	// Removes a key and its value in the object
	//
	// @note                    If dot notation is used some values may be inaccessible
	//                          because valid names in JSON can contain dots
	//
	// @param key               Key name
	// @param dot_not           True to use dot notation, false to not
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool Remove(const char[] key, bool dot_not = false);

	// Removes all keys and their values in the object
	//
	// @param object            Object handle
	//
	// @return                  True if succeed, false otherwise
	// @error                   If passed handle is not a valid object
	public native bool Clear();

	// Retrieves the size of the object
	property int Size {
		public native get();
	}
};

methodmap JSONArray < JSON
{
	// Creates a JSON array
	//
	// The JSONArray must be freed via delete or CloseHandle()
	public native JSONArray();

	// Loads a JSON array from a file
	//
	// @param file              File to read from
 	// @param with_comments     True if parsing JSON includes comments (it will ignore them), false otherwise
	// @return                  Array handle, or null on failure
	// @error                   Invalid JSON
	public static native JSONArray FromFile(const char[] file, bool with_comments = false);

	// Loads a JSON array from a string
	//
	// @param buffer            String buffer to load into the JSON array
 	// @param with_comments     True if parsing JSON includes comments (it will ignore them), false otherwise
	// @return                  Array handle, or null on failure
	// @error                   Invalid JSON
	public static native JSONArray FromString(const char[] buffer, bool with_comments = false);

	// Gets a value from the array
	//
	// @note                    Needs to be freed using delete or CloseHandle()
	//
	// @param index             Position in the array (starting from 0)
	//
	// @return                  JSON handle, Invalid_JSON if error occurred
	// @error                   If passed handle is not a valid array
	public native JSON Get(int index);

	// Gets a boolean value from the array
	//
	// @param index             Position in the array (starting from 0)
	//
	// @return                  Boolean value
	// @error                   If passed handle is not a valid array
	public native bool GetBool(int index);

	// Gets a float value from the array
	//
	// @param index             Position in the array (starting from 0)
	//
	// @return                  The number as float
	// @error                   If passed handle is not a valid array
	public native float GetFloat(int index);

	// Gets a integer value from the array
	//
	// @param index             Position in the array (starting from 0)
	//
	// @return                  integer value
	// @error                   If passed handle is not a valid array
	public native int GetInt(int index);

	// Retrieves an 64-bit integer value from the array.
	//
	// @param index      Index in the array.
	// @param buffer     Buffer to copy to.
	// @param maxlength  Maximum size of the buffer.
	// @error            Invalid index.
	public native void GetInt64(int index, char[] buffer, int maxlength);

	// Gets string data from the array
	//
	// @param index             Position in the array (starting from 0)
	// @param buffer            Buffer to copy string to
	// @param maxlength         Maximum size of the buffer
	//
	// @return                  True on success, false on failure
	// @error                   If passed handle is not a valid array
	public native bool GetString(int index, char[] buffer, int maxlength);

	// Returns whether or not a value in the object is null
	//
	// @param index             Position in the array (starting from 0)
	//
	// @return                  True if the value is null, false otherwise
	// @error                   Invalid Index
	public native bool IsNull(int index);

	// Sets an array or object value in the array
	//
	// @param index      Index in the array
	// @param value      Value to set
	// @return           True on success, false on failure
	public native bool Set(int index, JSON value);

	// Sets a boolean value in the array
	//
	// @param index      Index in the array
	// @param value      Value to set
	// @return           True on success, false on failure
	public native bool SetBool(int index, bool value);

	// Sets a float value in the array
	//
	// @param index      Index in the array
	// @param value      Value to set
	// @return           True on success, false on failure
	public native bool SetFloat(int index, float value);

	// Sets an integer value in the array
	//
	// @param index      Index in the array
	// @param value      Value to set
	// @return           True on success, false on failure
	public native bool SetInt(int index, int value);

	// Sets a 64 bit integer value in the array.
	//
	// @param index      Index in the array.
	// @param value      64-bit integer value to set.
	// @return           True on success, false on failure.
	public native bool SetInt64(int index, const char[] value);

	// Sets a null value in the array
	//
	// @param index      Index in the array
	// @return           True on success, false on failure
	public native bool SetNull(int index);

	// Sets a string value in the array
	//
	// @param index      Index in the array
	// @param value      String value to set
	// @return           True on success, false on failure
	public native bool SetString(int index, const char[] value);

	// Pushes an array or object value onto the end of the array, adding a new index
	//
	// @param value      Value to push
	// @return           True on success, false on failure
	public native bool Push(JSON value);

	// Pushes a boolean value onto the end of the array, adding a new index
	//
	// @param value      Value to push
	// @return           True on success, false on failure
	public native bool PushBool(bool value);

	// Pushes a float value onto the end of the array, adding a new index
	//
	// @param value      Value to push
	// @return           True on success, false on failure
	public native bool PushFloat(float value);

	// Pushes an integer value onto the end of the array, adding a new index
	//
	// @param value      Value to push
	// @return           True on success, false on failure
	public native bool PushInt(int value);

	// Pushes a 64-bit integer value onto the end of the array, adding a new index.
	//
	// @param value      64-bit integer value to push.
	// @return           True on success, false on failure.
	public native bool PushInt64(const char[] value);

	// Pushes a null value onto the end of the array, adding a new index
	// @return           True on success, false on failure
	public native bool PushNull();

	// Pushes a string value onto the end of the array, adding a new index
	//
	// @param value      String value to push
	// @return           True on success, false on failure
	public native bool PushString(const char[] value);

	// Removes an entry from the array
	//
	// @param index      Index in the array to remove
	// @return           True on success, false on invalid index
	public native bool Remove(int index);

	// Clears the array of all entries
	// @return           True on success, false on failure
	public native bool Clear();

	// Retrieves the size of the array
	property int Length {
		public native get();
	}
};

// Helper macros for checking type
#define json_is_object(%1)   (%1 != Invalid_JSON && json_get_type(%1) == JSONObject)
#define json_is_array(%1)    (%1 != Invalid_JSON && json_get_type(%1) == JSONArray)
#define json_is_string(%1)   (%1 != Invalid_JSON && json_get_type(%1) == JSONString)
#define json_is_number(%1)   (%1 != Invalid_JSON && json_get_type(%1) == JSONNumber)
#define json_is_bool(%1)     (%1 != Invalid_JSON && json_get_type(%1) == JSONBoolean)
#define json_is_null(%1)     (%1 != Invalid_JSON && json_get_type(%1) == JSONNull)
#define json_is_true(%1)     (%1 != Invalid_JSON && json_is_bool(%1) && json_get_bool(%1))
#define json_is_false(%1)    (%1 != Invalid_JSON && json_is_bool(%1) && !json_get_bool(%1))

 // Parses JSON string or a file that contains JSON
 //
 // @note                    Must be freed via delete or CloseHandle()
 //
 // @param string            String to parse
 // @param is_file           True to treat string param as filename, false otherwise
 // @param with_comments     True if parsing JSON includes comments (it will ignore them), false otherwise
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_parse(const char[] string, bool is_file = false, bool with_comments = false);

 // Checks if the first value is the same as the second one
 //
 // @param value1            JSON handle
 // @param value2            JSON handle
 //
 // @return                  True if they are the same, false otherwise
 // @error                   If passed value is not a valid handle
native bool json_equals(const JSON value1, const JSON value2);

 // Validates json by checking if object have identically named
 // fields with matching types
 //
 // @note                    Schema {"name":"", "age":0} will validate
 //                          {"name":"Joe", "age":25} and {"name":"Joe", "age":25, "gender":"m"},
 //                          but not {"name":"Joe"} or {"name":"Joe", "age":"Cucumber"}
 //
 // @note                    In case of arrays, only first value in schema
 //                          is checked against all values in tested array
 //
 // @note                    Empty objects ({}) validate all objects,
 //                          empty arrays ([]) validate all arrays,
 //                          null validates values of every type
 //
 // @param schema            JSON handle
 // @param value             JSON handle
 //
 // @return                  True if passed value is valid, false otherwise
 // @error                   If a schema handle or value handle is invalid
native bool json_validate(const JSON schema, const JSON value);

 // Gets value's parent handle
 //
 // @note                  Parent's handle must be freed via delete or CloseHandle()
 //
 // @param value           JSON handle
 //
 // @return                Parent's handle
native JSON json_get_parent(const JSON value);

 // Gets JSON type of passed value
 //
 // @param value             JSON handle
 //
 // @return                  JSON type (JSONType constants)
 // @error                   If a value handle is invalid
native JSONType json_get_type(const JSON value);

 // Inits an empty object
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_object();

 // Inits an empty array
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_array();

 // Inits string data
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param value             String that the handle will be initialized with
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_string(const char[] value);

 // Inits a integer
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param value             Integer that the handle will be initialized with
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_integer(int value);

 // Inits a 64-bit integer
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param value             64-bit integer that the handle will be initialized with
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_integer64(const char[] value);

 // Inits a real number
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param value             Real number that the handle will be initialized with
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_real(float value);

 // Inits a boolean value
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param value             Boolean value that the handle will be initialized with
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_bool(bool value);

 // Inits a null
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
native JSON json_init_null();

 // Creates deep copy of passed value
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param value             JSON handle to be copied
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
 // @error                   If passed value is not a valid handle
native JSON json_deep_copy(const JSON value);

 // Gets string data
 //
 // @param value             JSON handle
 // @param buffer            Buffer to copy string to
 // @param maxlength         Maximum size of the buffer
 //
 // @return                  True on success, false on failure
 // @error                   If passed value is not a valid handle
native bool json_get_string(const JSON value, char[] buffer, int maxlength);

 // Gets a integer
 //
 // @param value             JSON handle
 //
 // @return                  integer
 // @error                   If passed value is not a valid handle
native int json_get_integer(const JSON value);

 // Gets a 64-bit integer
 //
 // @param value             JSON handle
 //
 // @return                  64-bit integer
 // @error                   If passed value is not a valid handle
native int json_get_integer64(const JSON value);

 // Gets a real number
 //
 // @param value             JSON handle
 //
 // @return                  Real number
 // @error                   If passed value is not a valid handle
native float json_get_real(const JSON value);

 // Gets a boolean value
 //
 // @param value             JSON handle
 //
 // @return                  Boolean value
 // @error                   If passed value is not a valid handle
native bool json_get_bool(const JSON value);

 // Gets a value from the array
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
 // @error                   If passed handle is not a valid array
native JSON json_array_get_value(const JSON array, int index);

 // Gets string data from the array
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 // @param buffer            Buffer to copy string to
 // @param maxlength         Maximum size of the buffer
 //
 // @return                  True on success, false on failure
 // @error                   If passed handle is not a valid array
native bool json_array_get_string(const JSON array, int index, char[] buffer, int maxlength);

 // Returns whether or not a value in the object is null
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 //
 // @return                  True if the value is null, false otherwise
 // @error                   Invalid Index
native bool json_array_is_null(const JSON array, int index);

 // Gets a integer from the array
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 //
 // @return                  integer
 // @error                   If passed handle is not a valid array
native int json_array_get_integer(const JSON array, int index);

 // Gets a 64-bit integer from the array
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 // @param buffer            Buffer to copy to.
 // @param maxlength         Maximum size of the buffer.
 //
 // @return                  64-bit integer
 // @error                   If passed handle is not a valid array
native int json_array_get_integer64(const JSON array, int index, char[] buffer, int maxlength);

 // Gets a real number from the array
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 //
 // @return                  The number as float
 // @error                   If passed handle is not a valid array
native float json_array_get_real(const JSON array, int index);

 // Gets a boolean value from the array
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 //
 // @return                  Boolean value
 // @error                   If passed handle is not a valid array
native bool json_array_get_bool(const JSON array, int index);

 // Gets count of the elements in the array
 //
 // @param array             Array handle
 //
 // @return                  Number of elements in the array
 // @error                   If passed handle is not a valid array
native int json_array_get_count(const JSON array);

 // Replaces an element in the array with value
 //
 // @param array             Array handle
 // @param index             Position in the array to be replaced
 // @param value             JSON handle to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_replace_value(JSON array, int index, const JSON value);

 // Replaces an element in the array with string data
 //
 // @param array             Array handle
 // @param index             Position in the array to be replaced
 // @param string            String to copy
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_replace_string(JSON array, int index, const char[] string);

 // Replaces an element in the array with integer
 //
 // @param array             Array handle
 // @param index             Position in the array to be replaced
 // @param integer           integer to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_replace_integer(JSON array, int index, int integer);

 // Replaces an element in the array with 64-bit integer
 //
 // @param array             Array handle
 // @param index             Position in the array to be replaced
 // @param value             64-bit integer value to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_replace_integer64(JSON array, int index, const char[] value);

 // Replaces an element in the array with real number
 //
 // @param array             Array handle
 // @param index             Position in the array to be replaced
 // @param number            Real number to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_replace_real(JSON array, int index, float number);

 // Replaces an element in the array with boolean value
 //
 // @param array             Array handle
 // @param index             Position in the array to be replaced
 // @param boolean           Boolean value to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_replace_bool(JSON array, int index, bool boolean);

 // Replaces an element in the array with null
 //
 // @param array             Array handle
 // @param index             Position in the array to be replaced
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_replace_null(JSON array, int index);

 // Appends a value in the array
 //
 // @param array             Array handle
 // @param value             JSON handle to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_append_value(JSON array, const JSON value);

 // Appends string data in the array
 //
 // @param array             Array handle
 // @param string            String to copy
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_append_string(JSON array, const char[] string);

 // Appends a integer in the array
 //
 // @param array             Array handle
 // @param integer           integer to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_append_integer(JSON array, int integer);

 // Appends a 64-bit integer in the array
 //
 // @param array             Array handle
 // @param value             64-bit integer value
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_append_integer64(JSON array, const char[] value);

 // Appends a real number in the array
 //
 // @param array             Array handle
 // @param number            Real number to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_append_real(JSON array, float number);

 // Appends a boolean value in the array
 //
 // @param array             Array handle
 // @param boolean           Boolean value to set
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_append_bool(JSON array, bool boolean);

 // Appends a null in the array
 //
 // @param array             Array handle
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_append_null(JSON array);

 // Removes an element from the array
 //
 // @note                    Order of values in array may change during execution
 //
 // @param array             Array handle
 // @param index             Position in the array (starting from 0)
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_remove(JSON array, int index);

 // Removes all elements from the array
 //
 // @param array             Array handle
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid array
native bool json_array_clear(JSON array);

 // Gets a value from the object
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 //
 // @param object            Object handle
 // @param name              Key name
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
 // @error                   If passed handle is not a valid object
native JSON json_object_get_value(const JSON obj, const char[] name, bool dot_not = false);

 // Gets string data from the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 //
 // @param object            Object handle
 // @param name              Key name
 // @param buffer            Buffer to copy string to
 // @param maxlength         Maximum size of the buffer
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True on success, false on failure
 // @error                   If passed handle is not a valid object
native bool json_object_get_string(const JSON obj, const char[] name, char[] buffer, int maxlength, bool dot_not = false);

 // Gets a integer from the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 //
 // @param object            Object handle
 // @param name              Key name
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  integer
 // @error                   If passed handle is not a valid object
native int json_object_get_integer(const JSON obj, const char[] name, bool dot_not = false);

 // Gets a 64-bit integer from the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 //
 // @param object            Object handle
 // @param name              Key name
 // @param buffer            String buffer to store key.
 // @param maxlength         Maximum length of the string buffer.
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  64-bit integer
 // @error                   If passed handle is not a valid object
native int json_object_get_integer64(const JSON obj, const char[] name, char[] buffer, int maxlength, bool dot_not = false);

 // Gets a real number from the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 //
 // @param object            Object handle
 // @param name              Key name
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  Real number
 // @error                   If passed handle is not a valid object
native float json_object_get_real(const JSON obj, const char[] name, bool dot_not = false);

 // Gets a boolean value from the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 //
 // @param object            Object handle
 // @param name              Key name
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  Boolean value
 // @error                   If passed handle is not a valid object
native bool json_object_get_bool(const JSON obj, const char[] name, bool dot_not = false);

 // Returns whether or not a value in the object is null
 //
 // @param object            Object handle
 // @param key               Key string
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if the value is null, false otherwise
 // @error                   Invalid key
native bool json_object_is_null(const JSON obj, const char[] key, bool dot_not = false);

 // Gets count of the keys in the object
 //
 // @param object            Object handle
 //
 // @return                  Keys count
 // @error                   If passed handle is not a valid object
native int json_object_get_count(const JSON obj);

 // Gets name of the object's key
 //
 // @param object            Object handle
 // @param index             Position from which get key name
 // @param buffer            Buffer to copy string to
 // @param maxlength         Maximum size of the buffer
 //
 // @return                  True on success, false on failure
 // @error                   If passed handle is not a valid object
native bool json_object_get_name(const JSON obj, int index, char[] buffer, int maxlength);

 // Gets a value at the specified position from the object
 //
 // @note                    Needs to be freed using delete or CloseHandle()
 //
 // @param object            Object handle
 // @param index             Position from which get key name
 //
 // @return                  JSON handle, Invalid_JSON if error occurred
 // @error                   If passed handle is not a valid object
native JSON json_object_get_value_at(const JSON obj, int index);

 // Checks if the object has a value with a specific name and type
 //
 // @param object            Object handle
 // @param name              Key name
 // @param type              Type of value, if JSONError type will not be checked
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if has, false if not
 // @error                   If passed handle is not a valid object
native bool json_object_has_value(const JSON obj, const char[] name, JSONType type = JSONError, bool dot_not = false);

 // Checks if the object has a value with a specific key
 //
 // @param object            Object handle
 // @param name              Key name
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if has, false if not
 // @error                   If passed handle is not a valid object
native bool json_object_has_key(const JSON obj, const char[] name, bool dot_not = false);

 // Sets a value in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 // @note                    It also removes the old value if any
 //
 // @param object            Object handle
 // @param name              Key name
 // @param value             JSON handle to set
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_set_value(JSON obj, const char[] name, const JSON value, bool dot_not = false);

 // Sets string data in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 // @note                    It also removes the old value if any
 //
 // @param object            Object handle
 // @param name              Key name
 // @param string            String to copy
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_set_string(JSON obj, const char[] name, const char[] string, bool dot_not = false);

 // Sets a number in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 // @note                    It also removes the old value if any
 //
 // @param object            Object handle
 // @param name              Key name
 // @param integer           integer to set
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_set_integer(JSON obj, const char[] name, int integer, bool dot_not = false);

 // Sets a 64-bit integer in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 // @note                    It also removes the old value if any
 //
 // @param object            Object handle
 // @param name              Key name
 // @param value             Value to store at this key.
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_set_integer64(JSON obj, const char[] name, const char[] value, bool dot_not = false);

 // Sets a real number in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 // @note                    It also removes the old value if any
 //
 // @param object            Object handle
 // @param name              Key name
 // @param number            Real number to set
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_set_real(JSON obj, const char[] name, float number, bool dot_not = false);

 // Sets a boolean value in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 // @note                    It also removes the old value if any
 //
 // @param object            Object handle
 // @param name              Key name
 // @param boolean           Boolean value to set
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_set_bool(JSON obj, const char[] name, bool boolean, bool dot_not = false);

 // Sets a null in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 // @note                    It also removes the old value if any
 //
 // @param object            Object handle
 // @param name              Key name
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_set_null(JSON obj, const char[] name, bool dot_not = false);

 // Removes a key and its value in the object
 //
 // @note                    If dot notation is used some values may be inaccessible
 //                          because valid names in JSON can contain dots
 //
 // @param object            Object handle
 // @param name              Key name
 // @param dot_not           True to use dot notation, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_remove(JSON obj, const char[] name, bool dot_not = false);

 // Removes all keys and their values in the object
 //
 // @param object            Object handle
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid object
native bool json_object_clear(JSON obj);

 // Gets size of serialization
 //
 // @param value             JSON handle
 // @param pretty            True to count size for pretty format, false to not
 // @param null_byte         True to include null byte, false to not
 //
 // @return                  Size of serialized string
 // @error                   If passed handle is not a valid value
native int json_serial_size(const JSON value, bool pretty = false, bool null_byte = false);

 // Copies serialized string to the buffer
 //
 // @param value             JSON handle
 // @param buffer            Buffer to copy string to
 // @param maxlength         Maximum size of the buffer
 // @param pretty            True to format pretty JSON string, false to not
 //
 // @return                  True on success, false on failure
 // @error                   If passed handle is not a valid value
native bool json_serial_to_string(const JSON value, char[] buffer, int maxlength, bool pretty = false);

 // Copies serialized string to the file
 //
 // @param value             JSON handle
 // @param file              Path to the file
 // @param pretty            True to format pretty JSON string, false to not
 //
 // @return                  True if succeed, false otherwise
 // @error                   If passed handle is not a valid value
native bool json_serial_to_file(const JSON value, const char[] file, bool pretty = false);

 // Sets if slashes should be escaped or not when serializing JSON.
 // By default slashes are escaped.
 // This function sets a global setting
 //
 // @param escape_slashes             True escape slashes, false to not
native void json_set_escape_slashes(bool escape_slashes);

 // Sets float format used for serialization of numbers.
 // Make sure it can't serialize to a string longer than PARSON_NUM_BUF_SIZE.
 // If format is null then the default format (%1.17g) is used.
 //
 // @param format             float serialization formatter
native void json_set_float_serialize(const char[] format);

public Extension __ext_json =
{
	name = "json",
	file = "json.ext",
	autoload = 1,
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_json_SetNTVOptional()
{
	MarkNativeAsOptional("json_parse");
	MarkNativeAsOptional("json_equals");
	MarkNativeAsOptional("json_validate");
	MarkNativeAsOptional("json_get_parent");
	MarkNativeAsOptional("json_get_type");
	MarkNativeAsOptional("json_init_object");
	MarkNativeAsOptional("json_init_array");
	MarkNativeAsOptional("json_init_string");
	MarkNativeAsOptional("json_init_integer");
	MarkNativeAsOptional("json_init_integer64");
	MarkNativeAsOptional("json_init_real");
	MarkNativeAsOptional("json_init_bool");
	MarkNativeAsOptional("json_init_null");
	MarkNativeAsOptional("json_deep_copy");
	MarkNativeAsOptional("json_get_string");
	MarkNativeAsOptional("json_get_integer");
	MarkNativeAsOptional("json_get_integer64");
	MarkNativeAsOptional("json_get_real");
	MarkNativeAsOptional("json_get_bool");
	MarkNativeAsOptional("json_array_get_value");
	MarkNativeAsOptional("json_array_get_string");
	MarkNativeAsOptional("json_array_get_count");
	MarkNativeAsOptional("json_array_get_integer");
	MarkNativeAsOptional("json_array_get_integer64");
	MarkNativeAsOptional("json_array_get_real");
	MarkNativeAsOptional("json_array_get_bool");
	MarkNativeAsOptional("json_array_is_null");
	MarkNativeAsOptional("json_array_replace_value");
	MarkNativeAsOptional("json_array_replace_string");
	MarkNativeAsOptional("json_array_replace_integer");
	MarkNativeAsOptional("json_array_replace_integer64");
	MarkNativeAsOptional("json_array_replace_real");
	MarkNativeAsOptional("json_array_replace_bool");
	MarkNativeAsOptional("json_array_replace_null");
	MarkNativeAsOptional("json_array_append_value");
	MarkNativeAsOptional("json_array_append_string");
	MarkNativeAsOptional("json_array_append_integer");
	MarkNativeAsOptional("json_array_append_integer64");
	MarkNativeAsOptional("json_array_append_real");
	MarkNativeAsOptional("json_array_append_bool");
	MarkNativeAsOptional("json_array_append_null");
	MarkNativeAsOptional("json_array_remove");
	MarkNativeAsOptional("json_array_clear");
	MarkNativeAsOptional("json_object_get_value");
	MarkNativeAsOptional("json_object_get_string");
	MarkNativeAsOptional("json_object_get_integer");
	MarkNativeAsOptional("json_object_get_integer64");
	MarkNativeAsOptional("json_object_get_real");
	MarkNativeAsOptional("json_object_get_bool");
	MarkNativeAsOptional("json_object_get_count");
	MarkNativeAsOptional("json_object_get_name");
	MarkNativeAsOptional("json_object_get_value_at");
	MarkNativeAsOptional("json_object_has_value");
	MarkNativeAsOptional("json_object_has_key");
	MarkNativeAsOptional("json_object_is_null");
	MarkNativeAsOptional("json_object_set_value");
	MarkNativeAsOptional("json_object_set_string");
	MarkNativeAsOptional("json_object_set_integer");
	MarkNativeAsOptional("json_object_set_integer64");
	MarkNativeAsOptional("json_object_set_real");
	MarkNativeAsOptional("json_object_set_bool");
	MarkNativeAsOptional("json_object_set_null");
	MarkNativeAsOptional("json_object_remove");
	MarkNativeAsOptional("json_object_clear");
	MarkNativeAsOptional("json_serial_size");
	MarkNativeAsOptional("json_serial_to_string");
	MarkNativeAsOptional("json_serial_to_file");
	MarkNativeAsOptional("json_set_escape_slashes");
	MarkNativeAsOptional("json_set_float_serialize");
}
#endif