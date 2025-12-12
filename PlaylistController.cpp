#include "pch.h"
#include "PlaylistController.h"

PlaylistController::PlaylistController(const CStringArray& arr)
{
    for (int i = 0; i < arr.GetCount(); i++) {
        playlist.Add(arr.GetAt(i));
    }
}

void PlaylistController::AddMusicFile(const CString& file_path)
{
    playlist.Add(file_path);
}

void PlaylistController::ClearPlaylist()
{
    playlist.RemoveAll();
}

size_t PlaylistController::GetPlaylistSize() const
{
    return playlist.GetSize();
}

CString PlaylistController::GetMusicFileAt(int index_in) const
{
    if (index_in < playlist.GetSize()) {
        return playlist[index_in];
    }
    return {};
}

bool PlaylistController::CanMoveNext() const
{
    return index + 1 < playlist.GetSize();
}

bool PlaylistController::CanMovePrevious() const
{
    return index > 0;
}

bool PlaylistController::MoveNext()
{
    if (CanMoveNext()) {
        index++;
        return true;
    }
    return false;
}

bool PlaylistController::MovePrevious()
{
    if (CanMovePrevious()) {
        index--;
        return true;
    }
    return false;
}

void PlaylistController::ResetIndex()
{
    index = 0;
}
