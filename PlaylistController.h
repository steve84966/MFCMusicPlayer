#pragma once
#include "pch.h"
#include "framework.h"

class PlaylistController
{
protected:
    CStringArray playlist;
    int index = 0;
    int nextIndex = -1;
    PlaylistPlayMode mode = PlaylistPlayMode::Sequential;
public:
    PlaylistController() = default;
    ~PlaylistController() = default;
    explicit PlaylistController(const CStringArray& arr);

    void AddMusicFile(const CString& file_path);
    void ClearPlaylist();
    [[nodiscard]] size_t GetPlaylistSize() const;
    [[nodiscard]] CString GetMusicFileAt(int index_in) const;
    [[nodiscard]] bool CanMoveNext() const;
    [[nodiscard]] bool CanMovePrevious() const;
    void GenerateNextIndex();
    bool MoveNext();
    bool MovePrevious();
    void ResetIndex();
    [[nodiscard]] int GetCurrentIndex() const { return index; }
    void MoveItem(int fromIndex, int toIndex);
    void SetNextIndex(int next_index);
    void SetIndex (int index_in);
    void SetPlayMode(PlaylistPlayMode mode_in) { mode = mode_in; GenerateNextIndex(); }
};

