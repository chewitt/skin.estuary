#pragma once
/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#include "threads/Thread.h"
#include "music/MusicDatabase.h"
#include "MusicAlbumInfo.h"

class CAlbum;
class CArtist;

namespace MUSIC_INFO
{
enum SCAN_STATE { PREPARING = 0, REMOVING_OLD, CLEANING_UP_DATABASE, READING_MUSIC_INFO, DOWNLOADING_ALBUM_INFO, DOWNLOADING_ARTIST_INFO, COMPRESSING_DATABASE, WRITING_CHANGES };

class IMusicInfoScannerObserver
{
public:
  virtual ~IMusicInfoScannerObserver() {}
  virtual void OnStateChanged(SCAN_STATE state) = 0;
  virtual void OnDirectoryChanged(const CStdString& strDirectory) = 0;
  virtual void OnDirectoryScanned(const CStdString& strDirectory) = 0;
  virtual void OnSetProgress(int currentItem, int itemCount)=0;
  virtual void OnFinished() = 0;
};

class CMusicInfoScanner : CThread, public IRunnable
{
public:
  CMusicInfoScanner();
  virtual ~CMusicInfoScanner();

  void Start(const CStdString& strDirectory);
  void FetchAlbumInfo(const CStdString& strDirectory, bool refresh=false);
  void FetchArtistInfo(const CStdString& strDirectory, bool refresh=false);
  bool IsScanning();
  void Stop();
  void SetObserver(IMusicInfoScannerObserver* pObserver);

  /*! \brief Categorise songs into albums
   Albums are defined uniquely by the album name and album artist.

   If albumartist is not available in a song, we determine it from the
   common portion of each song's artist list.

   eg the common artist for
     Bob Dylan / Tom Petty / Roy Orbison
     Bob Dylan / Tom Petty
   would be "Bob Dylan / Tom Petty".

   If all songs that share an album
    1. have a non-empty album name
    2. have at least two different primary artists
    3. have no album artist set
    4. and no track numbers overlap
   we assume it is a various artists album, and set the albumartist field accordingly.

   \param songs [in/out] list of songs to categorise - albumartist field may be altered.
   \param albums [out] albums found within these songs.
   */
  static void CategoriseAlbums(VECSONGS &songs, VECALBUMS &albums);

  static void CheckForVariousArtists(VECSONGS &songs);
  static bool HasSingleAlbum(const VECSONGS &songs, CStdString &album, CStdString &artist);

  bool DownloadAlbumInfo(const CStdString& strPath, const CStdString& strArtist, const CStdString& strAlbum, bool& bCanceled, MUSIC_GRABBER::CMusicAlbumInfo& album, CGUIDialogProgress* pDialog=NULL);
  bool DownloadArtistInfo(const CStdString& strPath, const CStdString& strArtist, bool& bCanceled, CGUIDialogProgress* pDialog=NULL);

  std::map<std::string, std::string> GetArtistArtwork(long id, const CArtist *artist = NULL);
protected:
  virtual void Process();
  int RetrieveMusicInfo(CFileItemList& items, const CStdString& strDirectory);
  void UpdateFolderThumb(const VECSONGS &songs, const CStdString &folderPath);
  int GetPathHash(const CFileItemList &items, CStdString &hash);
  void GetAlbumArtwork(long id, const CAlbum &artist);

  bool DoScan(const CStdString& strDirectory);

  virtual void Run();
  int CountFiles(const CFileItemList& items, bool recursive);
  int CountFilesRecursively(const CStdString& strPath);

protected:
  IMusicInfoScannerObserver* m_pObserver;
  int m_currentItem;
  int m_itemCount;
  bool m_bRunning;
  bool m_bCanInterrupt;
  bool m_needsCleanup;
  int m_scanType; // 0 - load from files, 1 - albums, 2 - artists
  CMusicDatabase m_musicDatabase;

  std::set<CStdString> m_pathsToScan;
  std::set<CAlbum> m_albumsToScan;
  std::set<CArtist> m_artistsToScan;
  std::set<CStdString> m_pathsToCount;
  std::vector<long> m_artistsScanned;
  std::vector<long> m_albumsScanned;
};
}
