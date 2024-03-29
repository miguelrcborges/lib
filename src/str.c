#include "lib.h"
#include <stdarg.h>

bool string_equal(string s1, string s2) {
	if (s1.len != s2.len)
		return 0;

	for (usize i = 0; i < s1.len; ++i)
		if (s1.str[i] != s2.str[i])
			return 0;
	
	return 1;
}

i8 string_compare(string pos, string neg) {
	i8 longer;
	usize limit;
	if (pos.len == neg.len) {
		longer = 0;
		limit = neg.len;
	} else if (pos.len > neg.len) {
		longer = 1;
		limit = neg.len;
	} else {
		longer = -1;
		limit = pos.len;
	}

	for (usize i = 0; i < limit; ++i)
		if (pos.str[i] != neg.str[i])
			// May need to change this later.
			// This wont work properly if values >= 128
			return pos.str[i] - neg.str[i];

	return longer;
}

string_format_result string_fmtu64(Arena *a, u64 n) {
	string_format_result ret;
	// Max u64 is 18446744073709551615 : 20 characters
	SafePointer sp = Arena_alloc(a, 20, 1);
	if (unlikely(sp._ptr == NULL)) {
		ret.err.err = errFailedToAllocate;
		return ret;
	}

	u8 *end = sp._ptr + 19;
	u8 *cursor = end;
	do {
		*cursor = '0' + n % 10;
		cursor--;
		n /= 10;
	} while (n > 0);
	ret.err.err = NULL;
	ret.s.str = cursor + 1;
	ret.s.len = end - cursor;
	return ret;
}

string_format_result string_fmti64(Arena *a, i64 n) {
	string_format_result ret;
	// Widest i64 is -9223372036854775808 : 20 characters
	SafePointer sp = Arena_alloc(a, 20, 1);
	if (unlikely(sp._ptr == NULL)) {
		ret.err.err = errFailedToAllocate;
		return ret;
	}

	u64 n2;
	bool is_neg;
	u8 *end = sp._ptr + 19;
	u8 *cursor = end;
	if (n < 0) {
		is_neg = 1;
		n2 = -n;
	} else {
		is_neg = 0;
		n2 = n;
	}
	
	do {
		*cursor = '0' + n2 % 10;
		cursor--;
		n2 /= 10;
	} while (n2 > 0);
	if (is_neg) {
		*cursor = '-';
		cursor--;
	}
	ret.err.err = NULL;
	ret.s.str = cursor + 1;
	ret.s.len = end - cursor;
	return ret;
}

bool StringBuilder_create(StringBuilder *sb, Arena *a, string start) {
	SafePointer sp = Arena_alloc(a, sizeof(StringNode), sizeof(void*));
	if (unlikely(sp._ptr == NULL))
		return 1;

	StringNode *n = sp._ptr;
	n->string = start;
	n->next = NULL;
	sb->str_len = start.len;
	sb->first = n;
	sb->last = n;
	return 0;
};

bool StringBuilder_append(StringBuilder *sb, Arena *a, string s) {
	SafePointer sp = Arena_alloc(a, sizeof(StringNode), sizeof(void*));
	if (unlikely(sp._ptr == NULL))
		return 1;

	StringNode *n = sp._ptr;
	n->next = NULL;
	n->string = s;
	sb->str_len += s.len;
	sb->last->next = n;
	sb->last = sb->last->next;
	return 0;
}

string_format_result StringBuilder_build(StringBuilder *sb, Arena *a) {
	string_format_result ret;
	if (unlikely(sb->str_len == 0)) {
		ret.err.err = NULL;
		ret.s = str("");
	}

	SafePointer sp = Arena_alloc(a, sb->str_len + 1, 1);
	if (unlikely(sp._ptr == NULL))
		return ret;

	u8 *cursor = sp._ptr;
	ret.s.str = cursor;
	ret.s.len = sb->str_len;
	for (StringNode *n = sb->first; n; n = n->next) {
		for (usize i = 0; i < n->string.len; ++i) {
			*cursor = n->string.str[i];
			++cursor;
		}
	}
	return ret;
};

string_format_result string_fmtb16(Arena *a, u64 n) {
	string_format_result ret;
	enum {
		ALLOC_LEN = 64/4,
	};
	static u8 lookup_table[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	SafePointer sp = Arena_alloc(a, ALLOC_LEN, 1);
	if (unlikely(sp._ptr == NULL)) {
		ret.err.err = errFailedToAllocate;
		return ret;
	}

	u8 *end = sp._ptr + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = lookup_table[n&0b1111];
		cursor--;
		n = n >> 4;
	} while (n > 0);
	ret.err.err = NULL;
	ret.s.str = cursor + 1;
	ret.s.len = end - cursor;
	return ret;
}

string_format_result string_fmtb8(Arena *a, u64 n) {
	string_format_result ret;
	enum {
		ALLOC_LEN = 64/3,
	};
	SafePointer sp = Arena_alloc(a, ALLOC_LEN, 1);
	if (unlikely(sp._ptr == NULL)) {
		ret.err.err = errFailedToAllocate;
		return ret;
	}

	u8 *end = sp._ptr + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = '0' + (n & 0b111);
		cursor--;
		n = n >> 3;
	} while (n > 0);
	ret.err.err = NULL;
	ret.s.str = cursor + 1;
	ret.s.len = end - cursor;
	return ret;
}

string_format_result _string_build(Arena *a, usize n, ...) {
	string_format_result ret;

	usize len = 1;
	va_list args;
	va_start(args, n);
	for (usize i = 0; i < n; ++i) {
		len += va_arg(args, string).len;
	}
	va_end(args);

	SafePointer sp = Arena_alloc(a, len, 1);
	if (unlikely(sp._ptr == NULL)) {
		ret.err.err = errFailedToAllocate;
		return ret;
	}
	ret.s.str = sp._ptr;
	ret.s.len = len - 1;
	u8 *writter = sp._ptr;

	va_start(args, n);
	for (usize i = 0; i < n; ++i) {
		string t = va_arg(args, string);
		for (usize ii = 0; ii < t.len; ++ii) {
			*writter = t.str[ii];
			writter++;
		}
	}
	va_end(args);
	*writter = '\0';

	return ret;
}
