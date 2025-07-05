#ifndef SM_EXT_YYJSON_YYJSONWRAPPER_H_
#define SM_EXT_YYJSON_YYJSONWRAPPER_H_

#include "extension.h"
#include "am-float.h"

class YYJsonWrapper {
public:
	YYJsonWrapper() = default;
	~YYJsonWrapper() {
		if (m_pDocument_mut.unique()) {
			yyjson_mut_doc_free(m_pDocument_mut.get());
		}

		if (m_pDocument.unique()) {
			yyjson_doc_free(m_pDocument.get());
		}
	}

	YYJsonWrapper(const YYJsonWrapper&) = delete;
	YYJsonWrapper& operator=(const YYJsonWrapper&) = delete;

	void ResetObjectIterator() {
		m_iterInitialized = false;
	}

	void ResetArrayIterator() {
		m_iterInitialized = false;
		m_arrayIndex = 0;
	}

	bool IsMutable() const {
		return m_pDocument_mut != nullptr;
	}

	bool IsImmutable() const {
		return m_pDocument != nullptr;
	}

	// mutable document
	std::shared_ptr<yyjson_mut_doc> m_pDocument_mut;
	yyjson_mut_val* m_pVal_mut{ nullptr };

	// immutable document
	std::shared_ptr<yyjson_doc> m_pDocument;
	yyjson_val* m_pVal{ nullptr };

	// mutable document iterators
	yyjson_mut_obj_iter m_iterObj;
	yyjson_mut_arr_iter m_iterArr;

	// immutable document iterators
	yyjson_obj_iter m_iterObjImm;
	yyjson_arr_iter m_iterArrImm;

	Handle_t m_handle{ BAD_HANDLE };
	size_t m_arrayIndex{ 0 };
	size_t m_readSize{ 0 };
	bool m_iterInitialized{ false };
};

// Helper functions
inline std::unique_ptr<YYJsonWrapper> CreateWrapper() {
	return std::make_unique<YYJsonWrapper>();
}

inline std::shared_ptr<yyjson_mut_doc> WrapDocument(yyjson_mut_doc* doc) {
	return std::shared_ptr<yyjson_mut_doc>(doc, [](yyjson_mut_doc*){});
}

inline std::shared_ptr<yyjson_mut_doc> CopyDocument(yyjson_doc* doc) {
	return WrapDocument(yyjson_doc_mut_copy(doc, nullptr));
}

inline std::shared_ptr<yyjson_mut_doc> CreateDocument() {
	return WrapDocument(yyjson_mut_doc_new(nullptr));
}

inline std::shared_ptr<yyjson_doc> WrapImmutableDocument(yyjson_doc* doc) {
	return std::shared_ptr<yyjson_doc>(doc, [](yyjson_doc*){});
}

/**
 * Compare two floating-point numbers with an epsilon values.
 * This function uses both relative and absolute epsilon to handle edge cases.
 * @param a First floating-point number to compare.
 * @param b Second floating-point number to compare.
 * @param rel_epsilon Relative epsilon for comparison (default: 1e-6).
 * @param abs_epsilon Absolute epsilon for comparison (default: 1e-15).
 * @return true if the numbers are considered equal, false otherwise.
 */
template <typename T,
		  typename = std::enable_if_t<std::is_same_v<T, float> || std::is_same_v<T, double>>>
bool EqualsFp(T a, T b, T rel_epsilon = 1e-6, T abs_epsilon = 1e-15) {
	// handle NaN cases first since NaN != NaN
	if (ke::IsNaN(a) || ke::IsNaN(b)) return false;

	// fast path for exact equality
	// this also handles the case where both a and b are +0 or -0
	// IEEE 754 guarantees that +0 == -0 is true
	if (a == b) return true;

	// handle infinity cases
	if (ke::IsInfinite(a) || ke::IsInfinite(b)) {
		return ke::IsInfinite(a) && ke::IsInfinite(b) && (a > 0) == (b > 0);
	}

	double diff = fabs(a - b);
	if (diff <= abs_epsilon) return true;

	double larger = fmax(fabs(a), fabs(b));
	return diff <= larger * rel_epsilon;
}

#endif // SM_EXT_YYJSON_YYJSONWRAPPER_H_