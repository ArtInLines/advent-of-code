#include "../deps/ail/src/base/ail_base_all.h"
#include "../deps/ail/src/fs/ail_file.h"
#include <stdio.h> // @TODO: Replace with custom stdio functions once implemented in ail

b32 is_safe(AIL_DA(u32) levels)
{
    ail_assert(levels.len >= 2);
    b32 safe = true;
    b32 is_increasing = levels.data[1] > levels.data[0];
    for (u32 i = 1; i < levels.len; i++) {
        u32 prev_num = levels.data[i-1];
        u32 cur_num  = levels.data[i];
        safe &= is_increasing ? (prev_num < cur_num) : (cur_num < prev_num);
        safe &= is_increasing ? (cur_num - prev_num <= 3) : (prev_num - cur_num <= 3);
    }
    return safe;
}

b32 is_approximately_safe(AIL_DA(u32) levels, AIL_DA(u32) tmp)
{
    ail_assert(levels.len >= 3);
    for (u64 i = 0; i < levels.len; i++) {
        tmp.len = 0;
        ail_da_pushn(&tmp, levels.data, i);
        ail_da_pushn(&tmp, levels.data + i + 1, levels.len - i - 1);
        if (is_safe(tmp)) return true;
    }
    return false;
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
    AIL_DA(u32) levels = ail_da_new_with_alloc(u32, 16, al);
    AIL_DA(u32) tmp    = ail_da_new_with_alloc(u32, 16, al);

    u32 part1 = 0;
    u32 part2 = 0;
    for (;;) {
        AIL_SV line = ail_sv_ltrim(ail_sv_split_next_char(&input, '\n', true));
        if (!line.len) break;

        ail_assert(!levels.len);
        u32 len;
        for (;;) {
            u64 num = ail_sv_parse_unsigned(line, &len);
            if (!len) break;
            ail_da_push(&levels, num);
            line = ail_sv_ltrim(ail_sv_offset(line, len));
        }

        if (is_safe(levels)) {
            part1++;
            part2++;
        } else if (is_approximately_safe(levels, tmp)) part2++;

        levels.len = 0;
    }
    printf("Part 1: %d\n", part1);
    printf("Part 2: %d\n", part2);
    return 0;
}