#include "pch.h"
#include "PlaylistController.h"
#include <random>

PlaylistController::PlaylistController(const CStringArray& arr)
{
    for (int i = 0; i < arr.GetCount(); i++) {
        playlist.Add(arr.GetAt(i));
    }
    GenerateNextIndex();
}

void PlaylistController::AddMusicFile(const CString& file_path)
{
    playlist.Add(file_path);
    GenerateNextIndex();
}

void PlaylistController::ClearPlaylist()
{
    playlist.RemoveAll();
    nextIndex = -1;
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
    return nextIndex != -1;
}

bool PlaylistController::CanMovePrevious() const
{
    return index > 0;
}

void PlaylistController::GenerateNextIndex()
{
    switch (mode)
    {
    case PlaylistPlayMode::Sequential:
        if (index + 1 < playlist.GetSize())
        {
            nextIndex = index + 1;
        }
        break;
    case PlaylistPlayMode::ListLoop:
        nextIndex = index + 1;
        if (nextIndex >= playlist.GetSize())
            nextIndex = 0;
        break;
    case PlaylistPlayMode::SingleLoop:
        nextIndex = index;
        break;
    case PlaylistPlayMode::Random:
        {
            static std::default_random_engine e;
            std::uniform_int_distribution u(0, static_cast<int>(playlist.GetSize()) - 1);
            int rand_index = index;
            // avoid the same index
            while (rand_index == index && playlist.GetSize() > 1)
            {
                rand_index = u(e);
            }
            nextIndex = rand_index;
        }
        break;
    default:
        break;
    }
}

bool PlaylistController::MoveNext()
{
    if (CanMoveNext()) {
        index = nextIndex;
        nextIndex = -1;
        GenerateNextIndex();
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

void PlaylistController::MoveItem(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= playlist.GetSize() ||
        fromIndex == toIndex)
        return;
    if (toIndex < 0 || toIndex >= playlist.GetSize())
    {
        // remove item
        playlist.RemoveAt(fromIndex);
        if (index == fromIndex) {
            index = 0;
        }
        else if (fromIndex < index) {
            index--;
        }
        return;
    }

    CString item = playlist[fromIndex];
    playlist.RemoveAt(fromIndex);
    playlist.InsertAt(toIndex, item);

    if (index == fromIndex) {
        index = toIndex;
    }
    else if (fromIndex < index && toIndex >= index) {
        index--;
    }
    else if (fromIndex > index && toIndex <= index) {
        index++;
    }
}

void PlaylistController::SetNextIndex(int next_index)
{
    if (next_index >= 0 && next_index < playlist.GetSize()) {
        nextIndex = next_index;
    }
}

void PlaylistController::SetIndex(int index_in)
{
    if (index >= 0 && index_in < playlist.GetSize()) {
        index = index_in;
    }
}
