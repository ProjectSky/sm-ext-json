#include "extension.h"

ParsonWrapper::ParsonWrapper() : m_pValue(NULL), m_pArray(NULL), m_pObject(NULL), m_bAllowFree(true) {}

ParsonWrapper::~ParsonWrapper() {
	if (this->m_bAllowFree && this->m_pValue) {
		json_value_free(this->m_pValue);
		this->m_pValue = NULL;
		this->m_pArray = NULL;
		this->m_pObject = NULL;
		this->m_bAllowFree = false;
	}
}