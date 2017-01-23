#include "stdafx.h"
#include "CFile.h"

int CFile::OpenFile( wchar_t *fileName, FileAccess desiredAccess ) {
    DWORD access;

    switch( desiredAccess ) {
        case FileWrite:
            access = GENERIC_WRITE;
            break;
        case FileRead:
            access = GENERIC_READ;
            break;
        case FileReadWrite:
        default:
            access = GENERIC_WRITE | GENERIC_READ;
            break;
    }

    dataSizeInReadBuffer = 0;
    file = CreateFile( fileName, access, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( file == INVALID_HANDLE_VALUE ) {
        file = 0;
    }

    return (int) file;
}

void CFile::CloseFile( ) {
    if ( file ) {
        CloseHandle( file );
        file = 0;
    }
}

int CFile::printf( const wchar_t *format, ... ) {
    DWORD bytesWritten;
    int size;
    va_list arguments;
    va_start( arguments, format );
    size = vswprintf( buffer, format, arguments );
    va_end( arguments );

    wchar2char( buffer, buffer_ansi );
    WriteFile( file, ( wchar_t* ) buffer_ansi, size, &bytesWritten, NULL );

    return bytesWritten;
}

int CFile::scanf( const wchar_t *format, ... ) {
    DWORD bytesRead;
    wchar_t character;
    int parametersFilled;
    void *arg1, *arg2, *arg3, *arg4, *arg5, *arg6, *arg7, *arg8;
    va_list arguments;
    va_start( arguments, format );
    arg1 = va_arg( arguments, void* );
    arg2 = va_arg( arguments, void* );
    arg3 = va_arg( arguments, void* );
    arg4 = va_arg( arguments, void* );
    arg5 = va_arg( arguments, void* );
    arg6 = va_arg( arguments, void* );
    arg7 = va_arg( arguments, void* );
    arg8 = va_arg( arguments, void* );
    va_end( arguments );

    dataSizeInReadBuffer = 0;
    character = 0;

    do {
        ReadFile( file, &character, 1, &bytesRead, NULL );
        read_buffer_ansi[dataSizeInReadBuffer] = character;
        dataSizeInReadBuffer++;
    } while( bytesRead && character != '\n' );
    read_buffer_ansi[dataSizeInReadBuffer] = 0;
    char2wchar( read_buffer_ansi, read_buffer );

    bytesRead = swscanf( read_buffer, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 );

    return parametersFilled;
}

//

unsigned int CFile::char2wchar( char *ansi, wchar_t *unicode, unsigned int ansiSize ) {
    unsigned int i;
    for( i = 0; i < ansiSize; i++ ) {
        if ( ansi[i] == 0 ) break;
        unicode[i] = ansi[i];
    }
    unicode[i] = 0;
    return i;
}

unsigned int CFile::wchar2char( wchar_t *unicode, char *ansi, unsigned int unicodeSize ) {
    unsigned int i;
    for( i = 0; i < unicodeSize; i++ ) {
        if ( unicode[i] == 0 ) break;
        ansi[i] = unicode[i]&0x00ff;
    }
    ansi[i] = 0;
    return i;
}

int CFile::OpenFilename( std::wstring ext, std::wstring desc ) {
    browseDialogStruct.lStructSize = sizeof (browseDialogStruct );
    browseDialogStruct.hInstance = NULL;
    browseDialogStruct.hwndOwner = NULL;
    browseDialogStruct.lpstrFilter = L"*.*\0";
    browseDialogStruct.lpstrCustomFilter = NULL;
    browseDialogStruct.nMaxCustFilter = 0;
    browseDialogStruct.nFilterIndex = 0;
    browseDialogStruct.lpstrFile = fileName;
    browseDialogStruct.nMaxFile = MAX_FILENAME_PATH;
    browseDialogStruct.lpstrFileTitle = NULL;
    browseDialogStruct.nMaxFileTitle = 0;
    browseDialogStruct.lpstrInitialDir = NULL;
    browseDialogStruct.lpstrTitle = NULL;
    browseDialogStruct.Flags = 0;
    //browseDialogStruct.nFileOffset;
    //browseDialogStruct.nFileExtension;
    browseDialogStruct.lpstrDefExt = NULL;
    //browseDialogStruct.lCustData;
    browseDialogStruct.lpfnHook = NULL;
    browseDialogStruct.lpTemplateName = NULL;
    //browseDialogStruct.FlagsEx = NULL;


    return GetOpenFileName( &browseDialogStruct );
}

int CFile::SaveFilename( std::wstring ext, std::wstring desc ) {
    int retVal;
    wchar_t temp[511];
    wchar_t *ext_c_str;

    if ( ext.length( ) > 0 ) {
        if ( desc.length( ) > 0 ) {
            wsprintf( temp, L"%s (*.%s)%c*.%s%cAll Files (*.*)%c*.*%c%c", desc.c_str( ), ext.c_str( ), NULL, ext.c_str( ), NULL, NULL, NULL, NULL );
        } else {
            wsprintf( temp, L"(*.%s)%c*.%s%cAll Files (*.*)%c*.*%c%c", ext.c_str( ), NULL, ext.c_str( ), NULL, NULL, NULL, NULL );
        }
        browseDialogStruct.lpstrFilter = temp;
    } else {
        browseDialogStruct.lpstrFilter = L"All Files (*.*)\0*.*\0\0";
    }

    browseDialogStruct.lStructSize = sizeof (browseDialogStruct );
    browseDialogStruct.hInstance = NULL;
    browseDialogStruct.hwndOwner = NULL;

    browseDialogStruct.lpstrCustomFilter = NULL;
    browseDialogStruct.nMaxCustFilter = 0;
    browseDialogStruct.nFilterIndex = 0;
    browseDialogStruct.lpstrFile = fileName;
    browseDialogStruct.nMaxFile = MAX_FILENAME_PATH;
    browseDialogStruct.lpstrFileTitle = NULL;
    browseDialogStruct.nMaxFileTitle = 0;
    browseDialogStruct.lpstrInitialDir = NULL;
    browseDialogStruct.lpstrTitle = NULL;
    browseDialogStruct.Flags = OFN_OVERWRITEPROMPT;
    //browseDialogStruct.nFileOffset;
    //browseDialogStruct.nFileExtension;
    browseDialogStruct.lpstrDefExt = NULL;
    //browseDialogStruct.lCustData;
    browseDialogStruct.lpfnHook = NULL;
    browseDialogStruct.lpTemplateName = NULL;
    //browseDialogStruct.FlagsEx = NULL;


    retVal = GetSaveFileName( &browseDialogStruct );
    if ( ext.length( ) > 0 && browseDialogStruct.nFilterIndex == 1 && browseDialogStruct.nFileExtension == 0 ) {
        int i;
        i = wcslen( fileName );
        ext_c_str = ( wchar_t* ) ext.c_str( );
        fileName[i++] = '.';
        for( int j = 0; ext_c_str[j] != 0; j++ ) {
            fileName[i] = ext_c_str[j];
            i++;
        }
        fileName[i] = '\0';
    }

    return retVal;
}

int CFile::SelectAndOpen( std::wstring ext, std::wstring desc ) {
    if ( OpenFilename( ext, desc ) ) {
        return OpenFile( fileName, FileRead );
    } else {
        return 0;
    }
}

int CFile::SelectAndSave( std::wstring ext, std::wstring desc ) {
    if ( SaveFilename( ext, desc ) ) {
        return OpenFile( fileName, FileWrite );
    } else {
        return 0;
    }
}
