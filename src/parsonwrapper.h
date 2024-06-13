#include "extension.h"

class ParsonWrapper {
public:
	ParsonWrapper();
	~ParsonWrapper();

	ParsonWrapper(const ParsonWrapper&) = delete;
	ParsonWrapper& operator=(const ParsonWrapper&) = delete;

public:
	JSON_Value *m_pValue;
	JSON_Array *m_pArray;
	JSON_Object *m_pObject;

	Handle_t m_handle = BAD_HANDLE;
	bool m_bAllowFree;
};