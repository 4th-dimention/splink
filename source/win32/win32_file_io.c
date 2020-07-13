// TODO(allen): Changed durring this jam
// HACKED: threw in DialogueSavePath not sure it would actually want to live here.
//         or even be included at all?

internal char *
W32_FrameCStringFromString(String8 string)
{
    char *buffer = 0;
    buffer = M_ArenaPushZero(&os->frame_arena, string.size+1);
    MemoryCopy(buffer, string.str, string.size);
    return buffer;
}

internal b32
W32_SaveToFile(String8 path, void *data, u64 data_len)
{
    b32 result = 0;
    HANDLE file = {0};
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = CREATE_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(W32_FrameCStringFromString(path),
                              desired_access,
                              share_mode,
                              &security_attributes,
                              creation_disposition,
                              flags_and_attributes,
                              template_file)) != INVALID_HANDLE_VALUE)
        {
            
            void *data_to_write = data;
            DWORD data_to_write_size = (DWORD)data_len;
            DWORD bytes_written = 0;
            
            WriteFile(file, data_to_write, data_to_write_size, &bytes_written, 0);
            
            CloseHandle(file);
            
            result = 1;
        }
        else
        {
            W32_OutputError("File I/O Error", "Could not save to \"%s\"", path);
        }
    }
    return(result);
}

internal void
W32_AppendToFile(String8 path, void *data, u64 data_len)
{
    HANDLE file = {0};
    {
        DWORD desired_access = FILE_APPEND_DATA;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFileA(W32_FrameCStringFromString(path),
                               desired_access,
                               share_mode,
                               &security_attributes,
                               creation_disposition,
                               flags_and_attributes,
                               template_file)) != INVALID_HANDLE_VALUE)
        {
            
            void *data_to_write = data;
            DWORD data_to_write_size = (DWORD)data_len;
            DWORD bytes_written = 0;
            
            SetFilePointer(file, 0, 0, FILE_END);
            WriteFile(file, data_to_write, data_to_write_size, &bytes_written, 0);
            
            CloseHandle(file);
        }
        else
        {
            W32_OutputError("File I/O Error", "Could not save to \"%s\"", path);
        }
    }
}

internal void
W32_LoadEntireFile(M_Arena *arena, String8 path, void **data, u64 *data_len)
{
    *data = 0;
    *data_len = 0;
    
    HANDLE file = {0};
    
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_EXISTING;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(W32_FrameCStringFromString(path), desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE)
        {
            
            DWORD read_bytes = GetFileSize(file, 0);
            if(read_bytes)
            {
                void *read_data = M_ArenaPush(arena, read_bytes+1);
                DWORD bytes_read = 0;
                OVERLAPPED overlapped = {0};
                
                ReadFile(file, read_data, read_bytes, &bytes_read, &overlapped);
                
                ((u8 *)read_data)[read_bytes] = 0;
                
                *data = read_data;
                *data_len = (u64)bytes_read;
            }
            CloseHandle(file);
        }
    }
}

internal char *
W32_LoadEntireFileAndNullTerminate(M_Arena *arena, String8 path)
{
    char *result = 0;
    
    HANDLE file = {0};
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_EXISTING;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(W32_FrameCStringFromString(path), desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE)
        {
            
            DWORD read_bytes = GetFileSize(file, 0);
            if(read_bytes)
            {
                result = M_ArenaPush(arena, read_bytes+1);
                DWORD bytes_read = 0;
                OVERLAPPED overlapped = {0};
                
                ReadFile(file, result, read_bytes, &bytes_read, &overlapped);
                
                result[read_bytes] = 0;
            }
            CloseHandle(file);
        }
        else
        {
            W32_OutputError("File I/O Error", "Could not read from \"%s\"", path);
        }
    }
    
    return result;
}

internal void
W32_FreeFileMemory(void *data)
{
    W32_HeapFree(data);
}

internal void
W32_DeleteFile(String8 path)
{
    DeleteFileA(W32_FrameCStringFromString(path));
}

internal b32
W32_MakeDirectory(String8 path)
{
    b32 result = 1;
    if(!CreateDirectoryA(W32_FrameCStringFromString(path), 0))
    {
        result = 0;
    }
    return result;
}

internal b32
W32_DoesFileExist(String8 path)
{
    b32 found = GetFileAttributesA(W32_FrameCStringFromString(path)) != INVALID_FILE_ATTRIBUTES;
    return found;
}

internal b32
W32_DoesDirectoryExist(String8 path)
{
    DWORD file_attributes = GetFileAttributesA(W32_FrameCStringFromString(path));
    b32 found = (file_attributes != INVALID_FILE_ATTRIBUTES &&
                 !!(file_attributes & FILE_ATTRIBUTE_DIRECTORY));
    return found;
}

internal b32
W32_CopyFile(String8 dest, String8 source)
{
    b32 success = 0;
    success = CopyFile(W32_FrameCStringFromString(source), W32_FrameCStringFromString(dest), 0);
    return success;
}

internal OS_DirectoryList
W32_DirectoryListLoad(M_Arena *arena, String8 path, i32 flags)
{
    OS_DirectoryList list = {0};
    
    return list;
}


internal u8*
_W32_InitFilter(M_Arena *arena, String8 *fixed_ext){
    u8 *filter = 0;
    if (fixed_ext != 0){
        String8_List list = {0};
        StringListPush(arena, &list, fixed_ext[0]);
        StringListPush(arena, &list, S8Lit("\0"));
        StringListPush(arena, &list, S8Lit("*."));
        StringListPush(arena, &list, fixed_ext[1]);
        StringListPush(arena, &list, S8Lit("\0\0"));
        String8 filter_str = StringListJoin(arena, &list, 0);
        filter = filter_str.str;
    }
    return(filter);
}

internal String8
W32_DialogueSavePath(M_Arena *arena, String8 *fixed_ext)
{
    M_Arena *scratch = OS_GetScratch1(arena);
    
    OPENFILENAMEA openfilename = {sizeof(openfilename)};
    openfilename.lpstrFilter = (LPSTR)_W32_InitFilter(scratch, fixed_ext);
    openfilename.nMaxFile = Kilobytes(32);
    openfilename.lpstrFile = (LPSTR)PushArray(arena, u8, openfilename.nMaxFile);
    openfilename.lpstrFile[0] = 0;
    openfilename.lpstrTitle = "Save";
    openfilename.Flags = OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    
    String8 result = {0};
    if (GetSaveFileNameA(&openfilename)){
        result.str = openfilename.lpstrFile;
        result.size = 0;
        for (;openfilename.lpstrFile[result.size] != 0; result.size += 1);
    }
    
    OS_ReleaseScratch(scratch);
    return(result);
}

internal String8
W32_DialogueLoadPath(M_Arena *arena, String8 *fixed_ext)
{
    M_Arena *scratch = OS_GetScratch1(arena);
    
    OPENFILENAMEA openfilename = {sizeof(openfilename)};
    openfilename.lpstrFilter = (LPSTR)_W32_InitFilter(scratch, fixed_ext);
    openfilename.nMaxFile = Kilobytes(32);
    openfilename.lpstrFile = (LPSTR)PushArray(arena, u8, openfilename.nMaxFile);
    openfilename.lpstrFile[0] = 0;
    openfilename.lpstrTitle = "Open";
    openfilename.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
    
    String8 result = {0};
    if (GetOpenFileNameA(&openfilename)){
        result.str = openfilename.lpstrFile;
        result.size = 0;
        for (;openfilename.lpstrFile[result.size] != 0; result.size += 1);
    }
    
    OS_ReleaseScratch(scratch);
    return(result);
}

