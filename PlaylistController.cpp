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

CString PlaylistController::GetMusicFileAt(size_t index) const
{
    if (index < playlist.GetSize()) {
        return playlist[index];
    }
    return CString();
}

bool PlaylistController::CanMoveNext()
{
    return index + 1 < playlist.GetSize();
}

bool PlaylistController::CanMovePrevious()
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
