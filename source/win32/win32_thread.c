
global DWORD tls_index_thread_context = 0;

internal void
W32_ThreadSystemInit(void)
{
    tls_index_thread_context = TlsAlloc();
}

internal void
W32_ThreadInit(OS_ThreadContext *tctx, OS_ArenaNode *nodes, i32 node_count)
{
    MemoryZeroStruct(tctx);
    MemoryZero(nodes, sizeof(*nodes)*node_count);
    for (i32 i = 0; i < node_count; i += 1)
    {
        OS_ArenaNode *node_ptr = nodes + i;
        SLLStackPush(tctx->free, node_ptr);
        node_ptr->arena = M_ArenaInitialize();
    }
    BOOL success = TlsSetValue(tls_index_thread_context, tctx);
    Assert(success);
}

internal OS_ThreadContext*
W32_GetThreadContext(void)
{
    void *ptr = TlsGetValue(tls_index_thread_context);
    return((OS_ThreadContext*)ptr);
}

