#pragma once
#include "pch.h"
#include "framework.h"

class PlaylistController
{
protected:
    CStringArray playlist;
    size_t index = 0;
public:
    PlaylistController() = default;
    ~PlaylistController() = default;
    explicit PlaylistController(const CStringArray& arr);

    void AddMusicFile(const CString& file_path);
    void ClearPlaylist();
    [[nodiscard]] size_t GetPlaylistSize() const;
    [[nodiscard]] CString GetMusicFileAt(size_t index) const;
    bool CanMoveNext();
    bool CanMovePrevious();
    bool MoveNext();
    bool MovePrevious();
    void ResetIndex();
    [[nodiscard]] size_t GetCurrentIndex() const { return index; }
};

