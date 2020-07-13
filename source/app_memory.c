// NOTE(allen): memory

#define M_ARENA_MAX          Gigabytes(4)
#define M_ARENA_COMMIT_SIZE  Kilobytes(4)

internal M_Arena
M_ArenaInitializeWithAlign(u64 auto_align)
{
    M_Arena arena = {0};
    arena.max = M_ARENA_MAX;
    arena.base = os->Reserve(arena.max);
    arena.alloc_position = 0;
    arena.commit_position = 0;
    arena.auto_align = auto_align;
    return arena;
}

internal M_Arena
M_ArenaInitialize(void)
{
    return(M_ArenaInitializeWithAlign(8));
}

internal void *
M_ArenaPush(M_Arena *arena, u64 size)
{
    void *memory = 0;
    if(arena->alloc_position + size > arena->commit_position)
    {
        u64 commit_size = size;
        commit_size += M_ARENA_COMMIT_SIZE-1;
        commit_size -= commit_size % M_ARENA_COMMIT_SIZE;
        os->Commit((u8 *)arena->base + arena->commit_position, commit_size);
        arena->commit_position += commit_size;
    }
    memory = (u8 *)arena->base + arena->alloc_position;
    u64 p = arena->alloc_position + size;
    arena->alloc_position = (p + arena->auto_align - 1)&(~(arena->auto_align - 1));
    return memory;
}

internal void *
M_ArenaPushZero(M_Arena *arena, u64 size)
{
    void *memory = M_ArenaPush(arena, size);
    MemorySet(memory, 0, size);
    return memory;
}

internal void
M_ArenaSetPosBack(M_Arena *arena, u64 pos)
{
    if (pos <= arena->alloc_position)
    {
        arena->alloc_position = pos;
    }
}

internal void
M_ArenaSetPosBackByPtr(M_Arena *arena, void *ptr)
{
    u8 *uptr = (u8*)ptr;
    u64 pos = (uptr - (u8*)arena->base);
    if ((u8*)arena->base <= uptr)
    {
        M_ArenaSetPosBack(arena, pos);
    }
}

internal void
M_ArenaPop(M_Arena *arena, u64 size)
{
    size = ClampTop(size, arena->alloc_position);
    arena->alloc_position -= size;
}

internal void
M_ArenaClear(M_Arena *arena)
{
    M_ArenaPop(arena, arena->alloc_position);
}

internal void
M_ArenaRelease(M_Arena *arena)
{
    os->Release(arena->base);
}

internal M_Temp
M_BeginTemp(M_Arena *arena)
{
    M_Temp temp = {arena, arena->alloc_position};
    return(temp);
}

internal void
M_EndTemp(M_Temp temp)
{
    M_ArenaSetPosBack(temp.arena, temp.pos);
}

