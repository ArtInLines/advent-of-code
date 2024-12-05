#include "../deps/ail/src/base/ail_base_all.h"
#include "../deps/ail/src/fs/ail_file.h"
#include <stdio.h> // @TODO: Replace with custom stdio functions once implemented in ail

global AIL_Allocator al;

typedef AIL_SA(u8) Bitset;

inline_func b32 is_bit_set(Bitset bs, u32 i)
{
	u64 idx = i / 8;
	ail_assert(idx < bs.len);
	return (bs.data[idx] >> (i % 8)) & 1;
}

inline_func void set_bit(Bitset bs, u32 i)
{
	u64 idx = i / 8;
	ail_assert(idx < bs.len);
	bs.data[idx] |= (1 << (i % 8));
}

inline_func void unset_bit(Bitset bs, u32 i)
{
	u64 idx = i / 8;
	ail_assert(idx < bs.len);
	bs.data[idx] &= ~(1 << (i % 8));
}

internal print_updates(AIL_DA(u64) updates)
{
	printf("[");
	for (u64 i = 0; i < updates.len; i++) {
		if (i) printf(",");
		printf("%lld", updates.data[i]);
	}
	printf("]");
}

internal b32 is_sorted(AIL_DA(u64) updates, Bitset ordering)
{
	for (u64 j = 0; j < updates.len - 1; j++) {
		for (u64 k = j+1; k < updates.len; k++) {
			u64 first  = updates.data[j];
			u64 second = updates.data[k];
			ail_assert(first  < 100);
			ail_assert(second < 100);
			if (is_bit_set(ordering, second*100 + first)) {
				// printf("Not sorted because of ordering %lld|%lld (Updates: ", second, first);
				// print_updates(updates);
				// printf(")\n");
				return false;
			}
		}
	}
	return true;
}

internal void sort(AIL_DA(u64) updates, Bitset ordering)
{
	for (u64 i = 1; i < updates.len; i++) {
		u64 cur = updates.data[i];
		for (i64 j = i-1; j >= 0; j--) {
			u64 prev = updates.data[j];
			if (is_bit_set(ordering, prev*100 + cur)) break;
			else ail_swap(u64, updates.data[j], updates.data[j+1]);
		}
	}
	ail_assert(is_sorted(updates, ordering));
}

internal u64 get_middle(AIL_DA(u64) updates)
{
	ail_assert(updates.len % 2 == 1);
	return updates.data[updates.len / 2];
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Missing input file\n");
        printf("Usage: %s <input-file>\n", argv[0]);
        return 1;
    } else if (argc > 2) {
        printf("Too many inputs\n");
        printf("Usage: %s <input-file>\n", argv[0]);
        return 1;
    } else if (!ail_fs_is_file(argv[1])) {
        printf("Invalid input: '%s' is not a file\n", argv[1]);
        printf("Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

	u8 *buf = ail_call_alloc(ail_alloc_pager, AIL_MB(8));
    al = ail_alloc_buffer_new(AIL_MB(8), buf);
    u64 size;
    u8 *txt = ail_fs_read_entire_file(argv[1], &size, al);
    AIL_SV input = ail_sv_from_parts(txt, size);
    AIL_DA(AIL_SV) lines = ail_sv_split_lines_a(input, false, al);
	u8 *arr = ail_call_alloc(al, 100*100/8);
	Bitset ordering = ail_sa_from_parts(arr, 100*100/8);

	u64 i = 0;
	u32 len;
	for (; i < lines.len && ail_sv_contains_char(lines.data[i], '|'); i++) {
		AIL_SV line = lines.data[i];
		u64 first = ail_sv_parse_unsigned(line, &len);
		ail_assert(len > 0);
		ail_assert(line.str[len] == '|');
		u64 second = ail_sv_parse_unsigned(ail_sv_offset(line, len+1), &len);
		ail_assert(len > 0);
		set_bit(ordering, first*100 + second);
	}

	u64 part1 = 0;
	u64 part2 = 0;
	AIL_DA(u64) updates = ail_da_new_with_alloc(u64, 32, al);
	for (; i < lines.len; i++) {
		AIL_SV line = lines.data[i];
		if (!ail_sv_ltrim(line).len) continue;
		updates.len = 0;
		AIL_DA(AIL_SV) unparsed_updates = ail_sv_split_char_a(line, ',', true, al);
		for (u64 j = 0; j < unparsed_updates.len; j++) {
			ail_da_push(&updates, ail_sv_parse_unsigned(unparsed_updates.data[j], &len));
		}
		if (is_sorted(updates, ordering)) {
			// printf("Part1 += %lld\n", get_middle(updates));
			part1 += get_middle(updates);
		} else {
			sort(updates, ordering);
			part2 += get_middle(updates);
		}
	}
	printf("Part 1: %lld\n", part1);
	printf("Part 2: %lld\n", part2);

    return 0;
}