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

string string_fmtu64(Arena *a, u64 n) {
	string ret;
	// Max u64 is 18446744073709551615 : 20 characters
	u8 *p = Arena_alloc(a, 20, 1);
	u8 *end = p + 19;
	u8 *cursor = end;
	do {
		*cursor = '0' + n % 10;
		cursor--;
		n /= 10;
	} while (n > 0);
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

string string_fmti64(Arena *a, i64 n) {
	string ret;

	// Widest i64 is -9223372036854775808 : 20 characters
	u8 *p = Arena_alloc(a, 20, 1);
	u64 n2;
	bool is_neg;
	u8 *end = p + 19;
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
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

StringBuilder StringBuilder_create(void) {
	return (StringBuilder) {0};
};

void StringBuilder_append(StringBuilder *sb, Arena *a, string s) {
	StringNode *n = Arena_alloc(a, sizeof(StringNode), sizeof(void*));
	n->next = NULL;
	n->string = s;
	sb->str_len += s.len;
	if (unlikely(sb->first == NULL)) {
		sb->first = n;
		sb->last = n;
	} else {
		sb->last->next = n;
		sb->last = sb->last->next;
	}
}

string StringBuilder_build(StringBuilder *sb, Arena *a) {
	string ret;
	if (unlikely(sb->str_len == 0)) {
		return str("");
	}

	u8 *cursor = Arena_alloc(a, sb->str_len + 1, 1);
	ret.str = cursor;
	ret.len = sb->str_len;
	for (StringNode *n = sb->first; n; n = n->next) {
		for (usize i = 0; i < n->string.len; ++i) {
			*cursor = n->string.str[i];
			++cursor;
		}
	}
	return ret;
};

string string_fmtb16(Arena *a, u64 n) {
	string ret;
	enum {
		ALLOC_LEN = 64/4,
	};
	static u8 lookup_table[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};

	u8 *p = Arena_alloc(a, ALLOC_LEN, 1);
	u8 *end = p + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = lookup_table[n&0b1111];
		cursor--;
		n = n >> 4;
	} while (n > 0);
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

string string_fmtb8(Arena *a, u64 n) {
	string ret;
	enum {
		ALLOC_LEN = 64/3,
	};

	u8 *p = Arena_alloc(a, ALLOC_LEN, 1);
	u8 *end = p + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = '0' + (n & 0b111);
		cursor--;
		n = n >> 3;
	} while (n > 0);
	ret.str = cursor + 1;
	ret.len = end - cursor;
	return ret;
}

string _string_build(Arena *a, usize n, ...) {
	string ret;

	usize len = 1;
	va_list args;
	va_start(args, n);
	for (usize i = 0; i < n; ++i) {
		len += va_arg(args, string).len;
	}
	va_end(args);

	u8 *writter = Arena_alloc(a, len, 1);
	ret.str = writter; 
	ret.len = len - 1;

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
