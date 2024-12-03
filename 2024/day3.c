#include "../deps/ail/src/base/ail_base_all.h"
#include "../deps/ail/src/fs/ail_file.h"
#include "../deps/ail/src/pm/ail_pm.h"
#include <stdio.h> // @TODO: Replace with custom stdio functions once implemented in ail

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

    AIL_Allocator al = ail_alloc_arena_new(AIL_KB(8), &ail_alloc_pager);
    ail_pm_init(AIL_MB(1));
    u64 fsize;
    u8 *txt = ail_fs_read_entire_file(argv[1], &fsize, al);
    AIL_SV input = ail_sv_from_parts(txt, fsize);

    AIL_SV p = AIL_SV_FROM_LITERAL("mul(\\d+\\,\\d+)");
    AIL_PM_Comp_Res res = ail_pm_compile_a(p.str, p.len, AIL_PM_EXP_REGEX, al);
    ail_assert(!res.failed);
    // char buf[AIL_KB(2)];
    // u32 n = ail_pm_pattern_to_str(res.pattern, buf, ail_arrlen(buf));
    // printf("Pattern: %.*s\n", n, buf);

    // @Note: Kind of a hacky solution, but does the job with the least amount of work honestly
    // do_toggle_idxs stores the indexes at switches from-do-to-dont or from-dont-to-do
    AIL_SV needles[2] = {
        [0] = AIL_SV_FROM_LITERAL("do()"),
        [1] = AIL_SV_FROM_LITERAL("don't()"),
    };
    AIL_DA(u64) do_toggle_idxs = ail_da_new_with_alloc(u64, AIL_KB(8), al);
    u64 offset = 0;
    for (;;) {
        AIL_SV_Find_Of_Res find_res = ail_sv_find_of(ail_sv_offset(input, offset), needles, ail_arrlen(needles));
        if (find_res.sv_idx < 0) break;
        if (do_toggle_idxs.len % 2 != find_res.needle_idx) {
            ail_da_push(&do_toggle_idxs, offset + find_res.sv_idx);
        }
        offset += find_res.sv_idx + needles[find_res.needle_idx].len;
    }

    u64 part1 = 0;
    u64 part2 = 0;
    u64 toggles_idx = 0;
    offset = 0;
    while (input.len) {
        AIL_PM_Match match = ail_pm_match_sv(res.pattern, input);
        if (!match.len) break;
        while (toggles_idx < do_toggle_idxs.len && offset + match.idx > do_toggle_idxs.data[toggles_idx]) toggles_idx++;
        AIL_SV num_start = ail_sv_offset(input, match.idx + strlen("mul("));
        u32 len;
        u64 n1 = ail_sv_parse_unsigned(num_start, &len);
        ail_assert(len);
        u64 n2 = ail_sv_parse_unsigned(ail_sv_offset(num_start, len + 1), &len);
        u64 mul = n1*n2;
        part1 += mul;
        if (toggles_idx % 2 == 0) part2 += mul;
        input = ail_sv_offset(input, match.idx + match.len);
        offset += match.idx + match.len;
    }
    printf("Part 1: %lld\n", part1);
    printf("Part 2: %lld\n", part2);
    return 0;
}