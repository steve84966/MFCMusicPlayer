#pragma once
#include "pch.h"

struct NcmMusicMeta
{
    CString musicName;
    std::vector<std::vector<CString>> artist;
    CString format;
    CString album;
    CString albumPic;
};

struct DecryptResult
{
    CString title;
    CString artist;
    CString album;
    CString ext;
    CString pictureUrl;
    std::vector<uint8_t> audioData;
    CString mime;
};

class NcmDecryptor
{
public:
    NcmDecryptor(const std::vector<uint8_t>& data, const CString& filename);
    DecryptResult Decrypt();

private:
    const std::vector<uint8_t>& m_raw;
    size_t m_offset = 0;
    CString m_filename;

    NcmMusicMeta m_oriMeta;
    std::vector<uint8_t> m_audio;
    CString m_format;
    CString m_mime;

    std::vector<uint8_t> GetKeyData();
    std::vector<uint8_t> GetKeyBox();
    NcmMusicMeta GetMetaData();
    std::vector<uint8_t> GetAudio(const std::vector<uint8_t>& keyBox);

    static CString Utf8ToWstring(const CString& s);
};

