#include "../deps/ail/src/base/ail_base_all.h"
#include "../deps/ail/src/fs/ail_file.h"
#include <stdio.h> // @TODO: Replace with custom stdio functions once implemented in ail

global AIL_Allocator al;

internal AIL_SV vert(AIL_DA(AIL_SV) lines, u64 row, u64 col, u64 size)
{
	u8 *buf = ail_call_alloc(al, size);
	for (u64 i = 0; i < size; i++) {
		buf[i] = lines.data[row + i].str[col];
	}
	return ail_sv_from_parts(buf, size);
}

internal AIL_SV diag_right(AIL_DA(AIL_SV) lines, u64 row, u64 col, u64 size)
{
	u8 *buf = ail_call_alloc(al, size);
	for (u64 i = 0; i < size; i++) {
		buf[i] = lines.data[row + i].str[col + i];
	}
	return ail_sv_from_parts(buf, size);
}

internal AIL_SV diag_left(AIL_DA(AIL_SV) lines, u64 row, u64 col, u64 size)
{
	u8 *buf = ail_call_alloc(al, size);
	for (u64 i = 0; i < size; i++) {
		buf[i] = lines.data[row + i].str[col - i];
	}
	return ail_sv_from_parts(buf, size);
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

    al = ail_alloc_arena_new(AIL_GB(2), &ail_alloc_pager);
    u64 size;
    u8 *txt = ail_fs_read_entire_file(argv[1], &size, al);
    AIL_SV input = ail_sv_from_parts(txt, size);
    AIL_DA(AIL_SV) lines = ail_sv_split_lines_a(input, false, al);

	// Assert all lines are equal size
	for (u64 i = 1; i < lines.len; i++) {
		ail_assert(lines.data[i-1].len == lines.data[i].len);
	}

	AIL_SV xmas = AIL_SV_FROM_LITERAL("XMAS");
	AIL_SV samx = AIL_SV_FROM_LITERAL("SAMX");

	u64 part1 = 0;
	// Horizontal Search
	for (u64 row = 0; row < lines.len; row++) {
		AIL_SV line = lines.data[row];
		for (u64 col = 0; col <= line.len - xmas.len; col++) {
			if (ail_sv_starts_with(ail_sv_offset(line, col), xmas)) part1++;
			if (ail_sv_starts_with(ail_sv_offset(line, col), samx)) part1++;
		}
	}
	// Vertical Search
	for (u64 row = 0; row <= lines.len - xmas.len; row++) {
		for (u64 col = 0; col < lines.data[row].len; col++) {
			AIL_SV vl = vert(lines, row, col, xmas.len);
			if (ail_sv_starts_with(vl, xmas)) part1++;
			if (ail_sv_starts_with(vl, samx)) part1++;
		}
	}
	// Diagonal Right Search
	for (u64 row = 0; row <= lines.len - xmas.len; row++) {
		for (u64 col = 0; col <= lines.data[row].len - xmas.len; col++) {
			AIL_SV dl = diag_right(lines, row, col, xmas.len);
			if (ail_sv_starts_with(dl, xmas)) part1++;
			if (ail_sv_starts_with(dl, samx)) part1++;
		}
	}
	// Diagonal Left Search
	for (u64 row = 0; row <= lines.len - xmas.len; row++) {
		for (u64 col = xmas.len - 1; col < lines.data[row].len; col++) {
			AIL_SV dl = diag_left(lines, row, col, xmas.len);
			if (ail_sv_starts_with(dl, xmas)) part1++;
			if (ail_sv_starts_with(dl, samx)) part1++;
		}
	}
	printf("Part1: %lld\n", part1);

	// . M . S . . . . . .
	// . . A . . M S M S .
	// . M . S . M A A . .
	// . . A . A S M S M .
	// . M . S . M . . . .
	// . . . . . . . . . .
	// S . S . S . S . S .
	// . A . A . A . A . .
	// M . M . M . M . M .
	// . . . . . . . . . .

	AIL_SV mas = AIL_SV_FROM_LITERAL("MAS");
	AIL_SV sam = AIL_SV_FROM_LITERAL("SAM");
	u64 part2 = 0;
	for (u64 row = 0; row <= lines.len - mas.len; row++) {
		for (u64 col = 0; col <= lines.data[row].len - mas.len; col++) {
			AIL_SV dr = diag_right(lines, row, col, mas.len);
			AIL_SV dl = diag_left(lines, row, col + mas.len - 1, mas.len);
			// printf("(%lld, %lld): Right: '%.*s', Left: '%.*s\n", row, col, dr.len, dr.str, dl.len, dl.str);
			if ((ail_sv_starts_with(dr, mas) || ail_sv_starts_with(dr, sam)) &&
				(ail_sv_starts_with(dl, mas) || ail_sv_starts_with(dl, sam))) {
					part2++;
				}
		}
	}
	printf("Part2: %lld\n", part2);

    return 0;
}