#include "../deps/ail/src/base/ail_base_all.h"
#include "../deps/ail/src/fs/ail_file.h"
#include <stdio.h> // @TODO: Replace with custom stdio functions once implemented in ail

global AIL_Allocator al;

typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT,
} Dir;

global char dirChars[4] = {
    [UP]    = '^',
    [RIGHT] = '>',
    [DOWN]  = 'v',
    [LEFT]  = '<',
};

typedef enum {
    VISITED = 'X',
    WALL    = '#',
} Field;

typedef struct {
    i64 x, y;
    Dir dir;
} Guard;

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

internal Guard get_guard(AIL_DA(AIL_SV) lines)
{
    AIL_SV dirs = ail_sv_from_parts(dirChars, ail_arrlen(dirChars));
    for (i64 y = 0; y < lines.len; y++) {
        for (i64 x = 0; x < lines.data[y].len; x++) {
            char c = lines.data[y].str[x];
            if (ail_sv_contains_char(dirs, c)) {
                return (Guard) {
                    .x   = x,
                    .y   = y,
                    .dir = ail_sv_find_char(dirs, c),
                };
            }
        }
    }
    AIL_UNREACHABLE();
    return (Guard) { 0 };
}

internal Dir rotate(Dir dir)
{
    switch (dir) {
        case UP:
            return RIGHT;
        case RIGHT:
            return DOWN;
        case DOWN:
            return LEFT;
        case LEFT:
            return UP;
        default:
            AIL_UNREACHABLE();
            return 0;
    }
}

internal Guard move_guard(Guard guard, AIL_DA(AIL_SV) lines)
{
    Guard og_guard = guard;
    switch (guard.dir) {
        case UP:
            guard.y--;
            break;
        case RIGHT:
            guard.x++;
            break;
        case DOWN:
            guard.y++;
            break;
        case LEFT:
            guard.x--;
            break;
        default:
            AIL_UNREACHABLE();
    }
    if (guard.y >= 0 && guard.x < lines.data[guard.y].len && lines.data[guard.y].str[guard.x] == WALL) {
        og_guard.dir = rotate(og_guard.dir);
        return move_guard(og_guard, lines);
    }
    return guard;
}

internal b32 does_guard_loop(Guard guard, AIL_DA(AIL_SV) lines)
{
    static Bitset bitsets[4];
    u64 width = lines.data[0].len;
    if (AIL_UNLIKELY(!bitsets[0].data)) {
        for (u64 i = 0; i < ail_arrlen(bitsets); i++) {
            AIL_DA(u8) buf = ail_da_new_with_alloc_t(u8, lines.len*width/8 + 1, al);
            bitsets[i] = ail_sa_from_parts_t(u8, buf.data, buf.cap);
        }
    }
    for (u64 i = 0; i < ail_arrlen(bitsets); i++) {
        ail_mem_set(bitsets[i].data, bitsets[i].len, 0);
    }

    u64 moves_counter = 0;
    while (guard.y < lines.len && guard.x < lines.data[guard.y].len) {
        u64 cur_idx = guard.y*width + guard.x;
        if (is_bit_set(bitsets[guard.dir], cur_idx)) return true;
        set_bit(bitsets[guard.dir], cur_idx);
        guard = move_guard(guard, lines);
        moves_counter++;
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

    u8 *buf = ail_call_alloc(ail_alloc_pager, AIL_MB(8));
    al = ail_alloc_buffer_new(AIL_MB(8), buf);
    u64 size;
    u8 *txt = ail_fs_read_entire_file(argv[1], &size, al);
    AIL_SV input = ail_sv_from_parts(txt, size);
    AIL_DA(AIL_SV) lines = ail_sv_split_lines_a(input, false, al);
    Guard guard = get_guard(lines);
    Guard og_guard = guard;

    u64 part1 = 0;
    while (guard.y < lines.len && guard.x < lines.data[guard.y].len) {
        if (lines.data[guard.y].str[guard.x] != VISITED) part1++;
        lines.data[guard.y].str[guard.x] = VISITED;
        guard = move_guard(guard, lines);
    }
    printf("Part 1: %lld\n", part1);

    u64 part2 = 0;
    for (u64 y = 0; y < lines.len; y++) {
        for (u64 x = 0; x < lines.data[y].len; x++) {
            if (lines.data[y].str[x] == VISITED) {
                lines.data[y].str[x] = WALL;
                if (does_guard_loop(og_guard, lines)) part2++;
                lines.data[y].str[x] = VISITED;
            }
        }
    }
    printf("Part 2: %lld\n", part2);
    return 0;
}