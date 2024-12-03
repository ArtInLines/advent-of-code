#include "../deps/ail/src/base/ail_base_all.h"
#include "../deps/ail/src/fs/ail_file.h"
#include <stdio.h> // @TODO: Replace with custom stdio functions once implemented in ail

internal void sort(AIL_DA(u64) l)
{
    for (u64 i = 0; i < l.len - 1; i++) {
        u64 min_idx = i;
        for (u64 j = i + 1; j < l.len; j++) {
            if (l.data[j] < l.data[min_idx]) min_idx = j;
        }
        ail_swap(u64, l.data[i], l.data[min_idx]);
    }
}

inline_func u64 diff(u64 a, u64 b)
{
    return (a - b)*(a >= b) + (b - a)*(a < b);
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

    AIL_Allocator al = ail_alloc_arena_new(AIL_GB(2), &ail_alloc_pager);
    u64 size;
    u8 *txt = ail_fs_read_entire_file(argv[1], &size, al);
    AIL_SV input = ail_sv_from_parts(txt, size);
    AIL_DA(u64) l1 = ail_da_new_with_alloc(u64, size/4, al);
    AIL_DA(u64) l2 = ail_da_new_with_alloc(u64, size/4, al);

    // Collect input into 2 arrays
    for (;;) {
        AIL_SV line = ail_sv_ltrim(ail_sv_split_next_char(&input, '\n', true));
        if (!line.len) break;
        u64 len;
        u64 num = ail_sv_parse_unsigned(line, &len);
        ail_assert(len > 0);
        ail_da_push(&l1, num);
        num = ail_sv_parse_unsigned(ail_sv_ltrim(ail_sv_offset(line, len)), &len);
        ail_assert(len > 0);
        ail_da_push(&l2, num);
    }

    ail_assert(l1.len == l2.len);
    sort(l1);
    sort(l2);

    u64 part1 = 0;
    for (u64 i = 0; i < l1.len; i++) {
        part1 += diff(l1.data[i], l2.data[i]);
    }
    printf("Part 1: %d\n", part1);

    u64 part2 = 0;
    for (u64 i = 0, j = 0; i < l1.len; i++) {
        u64 x = l1.data[i];
        while (j < l2.len && l2.data[j] < x) j++;
        u64 k = 0;
        while (j + k < l2.len && l2.data[j + k] == x) k++;
        part2 += x*k;
    }
    printf("Part 2: %d\n", part2);
    return 0;
}