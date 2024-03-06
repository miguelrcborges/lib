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
	bool longer;
	usize limit;
	if (pos.len > neg.len) {
		longer = 1;
		limit = neg.len;
	} else {
		longer = -1;
		limit = pos.len;
	}

	for (usize i = 0; i < limit; ++i)
		if (pos.str[i] != neg.str[i])
			return pos.str[i] - neg.str[i];

	return longer;
}

bool string_fmtu64(Arena *a, u64 n, string *out) {
	// Max u64 is 18446744073709551615 : 20 characters
	SafePointer sp = Arena_alloc(a, 20, 1);
	if (sp._ptr == NULL)
		return 1;

	u8 *end = sp._ptr + 19;
	u8 *cursor = end;
	do {
		*cursor = '0' + n % 10;
		cursor--;
		n /= 10;
	} while (n > 0);
	out->str = cursor + 1;
	out->len = end - cursor;
	return 0;
}

bool string_fmti64(Arena *a, i64 n, string *out) {
	// Widest i64 is -9223372036854775808 : 20 characters
	SafePointer sp = Arena_alloc(a, 20, 1);
	if (sp._ptr == NULL)
		return 1;

	u8 *end = sp._ptr + 19;
	u8 *cursor = end;
	bool is_neg = 0;
	if (n < 0) {
		is_neg = 1;
		n = -n;
	}
	
	do {
		*cursor = '0' + n % 10;
		cursor--;
		n /= 10;
	} while (n > 0);
	if (is_neg) {
		*cursor = '-';
		cursor--;
	}
	out->str = cursor + 1;
	out->len = end - cursor;
	return 0;
}

bool StringBuilder_create(StringBuilder *sb, Arena *a, string start) {
	SafePointer sp = Arena_alloc(a, sizeof(StringNode), sizeof(void*));
	if (sp._ptr == NULL)
		return 1;

	StringNode *n = sp._ptr;
	n->string = start;
	n->next = NULL;
	sb->str_len = start.len;
	sb->n_nodes = 1;
	sb->first = n;
	sb->last = n;
	return 0;
};

bool StringBuilder_append(StringBuilder *sb, Arena *a, string s) {
	SafePointer sp = Arena_alloc(a, sizeof(StringNode), sizeof(void*));
	if (sp._ptr == NULL)
		return 1;

	StringNode *n = sp._ptr;
	n->next = NULL;
	n->string = s;
	sb->str_len += s.len;
	sb->n_nodes += 1;
	sb->last->next = n;
	sb->last = sb->last->next;
	return 0;
}

bool StringBuilder_build(StringBuilder *sb, Arena *a, string *out) {
	if (unlikely(sb->n_nodes == 0))
		return 0;

	SafePointer sp = Arena_alloc(a, sb->str_len + 1, 1);
	if (sp._ptr == NULL)
		return 1;

	u8 *cursor = sp._ptr;
	out->str = cursor;
	out->len = sb->str_len;
	for (StringNode *n = sb->first; n; n = n->next) {
		for (usize i = 0; i < n->string.len; ++i) {
			*cursor = n->string.str[i];
			++cursor;
		}
	}
	return 0;
};

bool string_fmtb16(Arena *a, u64 n, string *out) {
	enum {
		ALLOC_LEN = 64/4,
	};
	static u8 lookup_table[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	SafePointer sp = Arena_alloc(a, ALLOC_LEN, 1);
	if (sp._ptr == NULL)
		return 1;

	u8 *end = sp._ptr + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = lookup_table[n&0b1111];
		cursor--;
		n = n << 3;
	} while (n > 0);
	out->str = cursor + 1;
	out->len = end - cursor;
	return 0;
}

bool string_fmtb8(Arena *a, u64 n, string *out) {
	enum {
		ALLOC_LEN = 64/3,
	};
	SafePointer sp = Arena_alloc(a, ALLOC_LEN, 1);
	if (sp._ptr == NULL)
		return 1;

	u8 *end = sp._ptr + (ALLOC_LEN - 1);
	u8 *cursor = end;
	do {
		*cursor = '0' + n & 0b111;
		cursor--;
		n = n << 3;
	} while (n > 0);
	out->str = cursor + 1;
	out->len = end - cursor;
	return 0;
}

string _string_build(Arena *a, usize n, ...) {
	usize len = 1;
	va_list args;
	va_start(args, n);
	for (usize i = 0; i < n; ++i) {
		len += va_arg(args, string).len;
	}
	va_end(args);

	string output;
	output.str = unwrap(Arena_alloc(a, len, 1));
	output.len = len;
	u8 *writter = (u8 *)output.str;

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

	return output;
}
