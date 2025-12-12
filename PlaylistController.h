#pragma once
#include "pch.h"
#include "framework.h"

class PlaylistController
{
protected:
    CStringArray playlist;
    int index = 0;
public:
    PlaylistController() = default;
    ~PlaylistController() = default;
    explicit PlaylistController(const CStringArray& arr);

    void AddMusicFile(const CString& file_path);
    void ClearPlaylist();
    [[nodiscard]] size_t GetPlaylistSize() const;
    [[nodiscard]] CString GetMusicFileAt(int index_in) const;
    bool CanMoveNext() const;
    bool CanMovePrevious() const;
    bool MoveNext();
    bool MovePrevious();
    void ResetIndex();
    [[nodiscard]] int GetCurrentIndex() const { return index; }
};

