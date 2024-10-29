#pragma once

#include "extension.h"

class YYJsonWrapper {
public:
	YYJsonWrapper() = default;
	~YYJsonWrapper() {
		if (m_pDocument_mut && !m_isReference) {
			yyjson_mut_doc_free(m_pDocument_mut);
		}
	}

	YYJsonWrapper(const YYJsonWrapper&) = delete;
	YYJsonWrapper& operator=(const YYJsonWrapper&) = delete;

	void ResetIterator() {
		m_iterIndex = 0;
	}

	yyjson_mut_doc* m_pDocument_mut{ nullptr };
	yyjson_mut_val* m_pVal_mut{ nullptr };
	Handle_t m_handle{ BAD_HANDLE };
	size_t m_iterIndex{ 0 };
	bool m_isReference{ false };
};
