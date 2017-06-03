#include <fmod/fmod.h>      // FMOD Header
#include <fmod/fmod_errors.h>

void    InitFMOD ( )
{
    cout<<"Installing FMOD music...";
    FSOUND_SetOutput ( FSOUND_OUTPUT_DSOUND );      // DirectSound
    FSOUND_SetDriver ( 0 );                         // Sound Driver (default 0)
    FSOUND_SetMixer ( FSOUND_MIXER_AUTODETECT );    // Mixer
    FSOUND_Init ( 44100, 32, 0 );                   // 44.1 kHz and 32 channels
    cout<<"Done"<<endl;
}

void DeinitFMOD()
{
     //FSOUND_Sample_Free(MenuMusic);
	FSOUND_Close();
}


class MusicList
{
      public:
             char mode;
             vector <string> members;
             FSOUND_STREAM* streamy;
             int channel;
             int CurrentSong;
             MusicList(string path, char mode);
             MusicList(vector <string> pathList, char mode);
             ~MusicList();
             void PlayStream(int mode);
             bool StreamDone();
             void StopStream();
             void Randomize();
};

MusicList::MusicList(string path, char mode)
{
     this->mode = mode;
     streamy=NULL;
     CurrentSong=0;
     if(mode == 'm' || mode == 'u') //Menu Music mode or Unit mode
     {
          members.push_back(path);
     }
     else if(mode == 'b') //Background Music mode
     {
         ifstream PlaylistFile;
         PlaylistFile.open(path.c_str(), ios::in);
         while(PlaylistFile.good())
         {
              string line;
              getline(PlaylistFile, line);
              members.push_back(string("data\\music\\"+line));
         }
         PlaylistFile.close();
     }
     else //load playlist *.txt from path
     {
         ifstream PlaylistFile;
         PlaylistFile.open(path.c_str(), ios::in);
         while(PlaylistFile.good())
         {
              string line;
              getline(PlaylistFile, line);
              members.push_back(string("data\\music\\"+line));
         }
         PlaylistFile.close();
     }
}

MusicList::MusicList(vector <string> pathList, char mode)
{
     this->mode=mode;
     streamy=NULL;
     CurrentSong=0;
     for(int i=0; i<pathList.size(); i++)
            members.push_back(pathList[i]);
}

void MusicList::PlayStream(int mode) //-1-initialize stream, 0-normal play, 1-next, 2-previous, 3-randomized
{
     switch(mode)
     {
          case -1:
          {
               CurrentSong=0;
               streamy = FSOUND_Stream_Open(members[CurrentSong].c_str(), 0, 0, 0);
          } break;
          case 0: //normal play
          {
             if(StreamDone())
             {
                  FSOUND_Stream_Stop(streamy);
                  FSOUND_Stream_Close(streamy);
                  if(CurrentSong<members.size()-1)
                      streamy = FSOUND_Stream_Open(members[++CurrentSong].c_str(), 0, 0, 0);
                  else
                  {
                      Randomize();
                      streamy = FSOUND_Stream_Open(members[CurrentSong].c_str(), 0, 0, 0);
                  }
             }
             FSOUND_Stream_Play(0, streamy);
             
          } break;
          case 1: //play next
          {
               FSOUND_Stream_Stop(streamy);
               FSOUND_Stream_Close(streamy);
               if(CurrentSong<members.size()-1)
               {
                    streamy = FSOUND_Stream_Open(members[++CurrentSong].c_str(), 0, 0, 0);
                    FSOUND_Stream_Play(0, streamy);
               }
               else
               {
                   Randomize();
                   streamy = FSOUND_Stream_Open(members[CurrentSong].c_str(), 0, 0, 0);
                   FSOUND_Stream_Play(0, streamy);
               }
          } break;
          case 2: //play previous
          {
               FSOUND_Stream_Stop(streamy);
               FSOUND_Stream_Close(streamy);
               if(CurrentSong<=0)
               {
                    cout<<"FMOD Music: Cannot load previous musicfile."<<endl;
                    CurrentSong=0;
               }
               else CurrentSong--;
               streamy = FSOUND_Stream_Open(members[CurrentSong].c_str(), 0, 0, 0);
               FSOUND_Stream_Play(0, streamy);
          } break;
          case 3: //randomized start
          {
               Randomize();
               streamy = FSOUND_Stream_Open(members[CurrentSong].c_str(), 0, 0, 0);
               FSOUND_Stream_Play(0, streamy);
          }break;
          default:
          {
                  cout<<"FMOD Music: Improper StreamPlay mode."<<endl;
          }
     }
}

bool MusicList::StreamDone()
{
     int off = FSOUND_Stream_GetTime(streamy);
     int len = FSOUND_Stream_GetLengthMs(streamy);
     //cout<<off<<"\t"<<len<<endl;
     if(off >= len) return true;
     else return false;
}

void MusicList::StopStream()
{
     FSOUND_Stream_Stop(streamy);
     FSOUND_Stream_Close(streamy);
}

void MusicList::Randomize()
{
     random_shuffle(members.begin(), members.end());
     CurrentSong=0;
}

MusicList::~MusicList()
{
     this->StopStream();
     members.clear();
}
