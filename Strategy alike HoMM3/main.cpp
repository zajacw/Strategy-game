#include <allegro.h>
#include <almp3.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <winalleg.h>

//////Timer
volatile long speed = 0;
void increment_speed()
{
    speed++;
}
END_OF_FUNCTION( increment_speed );
LOCK_VARIABLE( speed );
LOCK_FUNCTION( increment_speed );

using namespace std;

#include "music.h"
#include "graphics.h"

#define WHITE makecol(255, 255, 255)
#define BLACK makecol(0, 0, 0)
#define RED makecol(255, 0, 0)
#define GREEN makecol(0, 255, 0)
#define BLUE makecol(0, 0, 255)
#define BROWN makecol(147, 39, 36)

enum {RIGHT_DIR=0, LEFT_DIR, DOWN_DIR, UP_DIR};
enum {ARIADOR=0, BELLASEE_FALLS, KHARAMDOOR, ELVENMORT}; //Towns
enum {TOWN_HALL=0, CITY_WALLS, TAVERN, FORGE, MARKSMAN, HUNTSMAN, MAGE_TOWER, CHAPEL, LUMBER_MILL, QUARRIES, LARGE_HUT}; //Buildings
enum {PIKEMEN=0, ARCHERS, SCOUTS, MAGES, WARRIORS, PRIESTS}; // Units
int UnitsTypeNumber = 6;

long framemem=0, frame=0;  //for the perception of time
int enableCommand=0; //0-not enabled, 1-enabled, 2-command line buffer to be cleared
string  commandLine, command;
string::iterator iter = commandLine.begin(); // string iterator
int     caret  = 0;                       // tracks the text caret
bool    insert = true; 

////// keyboard operations
bool KeyPressed(int keyValue);
bool KeyReleased(int keyValue);
bool KeyStillPressed(int keyValue);
int keyPress = 0;
int prevKeyPress = 0;

////// STRUCTURES
struct Basics
{
       int Health, MaxHealth;
       int Attack, Defence, Leadership;
};

struct Character:Basics
{
     int Diplomacy;
     int Level, Exp;
     int Gold, Wood, Ore;
};

struct Building
{
       int ID;
       string name;
       int OreCost, WoodCost;
       string descr;
       bool built;
} BuildingList[11];

struct Weapon
{
       string name;
       string Damage;
       bool Equipped;
       int Level;
       int cost;
};

struct Camp
{
       int x, y;
};

////// CLASSES
class Unit : public Basics
{
      public:
       string name;
       int AttackUpg, DefenceUpg, MaxHealthUpg;
       int ID;
       int cost, costUpg;
       struct Weapon UnitsWeapon;
       Unit(int ID);
       Unit(string name);
       bool Recruit(int GoldAmount, int LeadPoints, BITMAP* unit_bmp, bool lesser_cost);
};

class Player
{
      public:
             int x, y, direction;
             string name;
             bool isPassword;
             string pass;
             bool Dead;
             int Quest[10];
             string Place;
             struct Character attributes;
             vector <class Unit> Units;
             int Army[6];
             vector <struct Weapon> Weapons;
             vector <struct Weapon> PlayersItems;
             struct Weapon PlayersWeapon;
             vector <struct Camp> CampsCleared;
             void LoadBuildings();
             class Town
                    {
                          public:
                                  BITMAP* town_screen;
                                  Player* player_ptr;
                                  int ID;
                                  string name;
                                  string musicfile;
                                  bool visitable;
                                  vector <struct Building> buildings;
                                  vector <class Unit> units;
                                  Town() {ID = TownID; TownID++;}
                                  bool Build(int BuildingID);
                                  bool Built(int BuildingID);
                                  void Load();
                                  void TownMenu(int &Gold, int &Wood, int &Ore, int &Leadership, int* Army, vector <struct Weapon> &PlayersItems);
                                  void CommandInterpretTown(int &Gold, int &Wood, int &Ore, int &Leadership, int* Army, vector <struct Weapon> &PlayersItems);
                                  void Tavern(vector <struct Weapon> &PlayersItems , int &Gold);
                                 
                    } town[4];
             void ThrowPtr() {for(int i=0;i<4;i++) town[i].player_ptr=this;}
             Player(bool &esc);
             Player(string nam) {name = nam; load(); ThrowPtr();}
             bool passCheck(string check) { return strcmp(check.c_str(), pass.c_str());}
             void save();
             void load();
             void Card();
             void UnitStats();
             void CameraFollow(class Map &map);
             void Move(class Map &map, class MusicList &BackgroundMusic);
             void Visit(int placeID);
             void Camping();
             bool Battle(int Enemy[]);
             void LevelUp();
             void Death();
             void WinGame();
};

class Map
{
      public:
              int MapFields[32][32];
              int ID, MapWidth, MapHeight;
              int CampsNumber;
              BITMAP* MapBMP;
              bool LoadMap(int MapID, class Player &player);
              Map() {MapWidth=32*80; MapHeight=32*80; CampsNumber=0; for(int i=0; i<32; i++) for(int j=0; j<32; j++) MapFields[i][j]=0; DrawMap();}
              Map(int MapID, class Player &player) {LoadMap(MapID, player);}
              ~Map() {destroy_bitmap(MapBMP);}
              void ClearMap() {for(int i=0; i<32; i++) for(int j=0; j<32; j++) MapFields[i][j]=0;}
              void DrawMap();
              void OnScreen(class Player &player);
              bool Movable(int x, int y);
};


////// FUNCTIONS' DECLARE
void Gameplay(class Player player, class MusicList &MenuMusic);
void Help();
void CommandLineON();
void CommandReset(int state); //0-CommandLineOFF, 1-Commands enabled, 2-Only numbers enabled, 3-Turn off commands
void CommandInterpret(class Player &player, class MusicList &BackgroundMusic);
int level(int exp);
int diceroll (string dice);
void split(const string& s, char c, vector<string>& v);
int zawijaj(const string& s, vector <string> &v, FONT* fnt, int SubLength);
void framing (BITMAP* bmp, int x, int y, FONT* fnt, char text[], int col);
bool YesNo (char question[]);
int getWhole(float a) {return (int)trunc(a);}
string UpperAll(string text);


///// METHODS
Unit::Unit(int ID)
{
     this->ID=ID;
     switch(ID)
     {
               case -1: name=(""); break;
               case PIKEMEN: name=string("Pikemen"); break;
               case ARCHERS: name=string("Archers"); break;
               case SCOUTS: name=string("Scouts"); break;
               case MAGES: name=string("Mages"); break;
               case WARRIORS: name=string("Warriors"); break;
               case PRIESTS: name=string("Priests"); break;
               default: cout<<"Wrong UnitID..."<<endl;
     }
}

Unit::Unit(string name)
{
     this->name=name;
     if(strcmp(UpperAll(name).c_str(),"PIKEMEN")==0) ID = PIKEMEN;
     else if(strcmp(UpperAll(name).c_str(),"ARCHERS")==0) ID = ARCHERS;
     else if(strcmp(UpperAll(name).c_str(),"SCOUTS")==0) ID = SCOUTS;
     else if(strcmp(UpperAll(name).c_str(),"MAGES")==0) ID = MAGES;
     else if(strcmp(UpperAll(name).c_str(),"WARRIORS")==0) ID = WARRIORS;
     else if(strcmp(UpperAll(name).c_str(),"PRIESTS")==0) ID = PRIESTS;
}

bool Unit::Recruit(int GoldAmount, int LeadPoints, BITMAP* unit_bmp, bool lesser_cost)
{
     int MaxNumber, Amount=0;
     stringstream ss, ssAmount;
     if(lesser_cost) ss<<"Leadership points "<<Leadership<<", Cost "<<costUpg;
     else ss<<"Leadership points "<<Leadership<<", Cost "<<cost;
     if(lesser_cost) for(MaxNumber=0; (GoldAmount>=(MaxNumber+1)*costUpg&&LeadPoints>=Leadership*(MaxNumber+1)); MaxNumber++);
     else for(MaxNumber=0; (GoldAmount>=(MaxNumber+1)*cost&&LeadPoints>=Leadership*(MaxNumber+1)); MaxNumber++);
     CommandReset(2);
     while(true)
     {
         prevKeyPress = keyPress;
         clear(buffer);
         draw_sprite(buffer, unit_bmp, 0, 50);
         draw_sprite(buffer, background, 0, 0);
         textprintf_centre_ex(buffer, font1, 600, 100, BLACK, -1, name.c_str());
         textprintf_ex(buffer, font, 450, 150, BLACK, -1, "HP %d, ATK %d, DEF %d,", MaxHealth, Attack, Defence);
         textprintf_ex(buffer, font, 450, 170, BLACK, -1, ss.str().c_str());
         draw_sprite(buffer, gold, 455+text_length(font, ss.str().c_str()), 165);
         textprintf_centre_ex(buffer, font, 500+text_length(font, ss.str().c_str()), 170, BLACK, -1, "/unit");
         textprintf_centre_ex(buffer, font1, 600, 300, BLACK, -1, "How many units to recruit:");
         CommandLineON();
         ssAmount<<commandLine;
         ssAmount>>Amount;
         ssAmount.clear();
         if(Amount<=MaxNumber) textout_centre_ex(buffer, font1, commandLine.c_str(), 600, 350, GREEN, -1);
         else textout_centre_ex(buffer, font1, commandLine.c_str(), 600, 350, RED, -1);
         textprintf_centre_ex(buffer, font, 600, 450, BLACK, -1, "Max number of units you can afford: %d", MaxNumber);
         textprintf_ex(buffer, font, 435, 500, BLACK, -1, "You have:   %d and %d Leadership points", GoldAmount, LeadPoints);
         draw_sprite(buffer, gold, 435+text_length(font, "You have:"), 495);
         if(KeyReleased(KEY_ESC)) return false;
         if(KeyReleased(KEY_ENTER)&&Amount<=MaxNumber) return true;
         draw_sprite(screen, buffer, 0, 0);
     }
}

void Player::Town::Load()
{
     ifstream townfile;
     stringstream ss, ss2;
     ss << "data\\towns\\town_"<<ID<<".txt";
     townfile.open(ss.str().c_str(), ios::in);
     getline (townfile,name);
     string bmp_title;
     getline (townfile,bmp_title);
     ss2 << "data\\towns\\" << bmp_title;
     town_screen = load_bitmap(ss2.str().c_str(), NULL);
     
     vector <string> v;
     string line, building;
     int tempid;
     getline (townfile,line);
     split(line, ',', v);
     for (int k = 0; k < v.size( ); ++k) 
     {
          istringstream iss(v[k]);
          iss >> building;
          if(building=="Town_hall") tempid = TOWN_HALL;
          else if(building=="City_walls") tempid = CITY_WALLS;
          else if(building=="Tavern") tempid = TAVERN;
          else if(building=="Forge") tempid = FORGE;
          else if(building=="Marksman") tempid = MARKSMAN;
          else if(building=="Huntsman") tempid = HUNTSMAN;
          else if(building=="Mage_tower") tempid = MAGE_TOWER;
          else if(building=="Chapel") tempid = CHAPEL;
          else if(building=="Lumber_mill") tempid = LUMBER_MILL;
          else if(building=="Quarries") tempid = QUARRIES;
          else if(building=="Large_hut") tempid = LARGE_HUT;
          for(int j=0; j<11; j++)
                if(tempid==BuildingList[j].ID)
                     buildings.push_back(BuildingList[j]);

     }
     v.clear();
     
     getline (townfile,line);
     split(line, ',', v);
     string unit;
     for (int k = 0; k < v.size( ); ++k) 
     {
          istringstream iss(v[k]);
          iss >> unit;
          class Unit tempUnit(unit);
          units.push_back(tempUnit);
     }
     v.clear();
     getline (townfile,line);
     musicfile = string("data\\music\\"+line);
     townfile.close();
}

void Player::Town::TownMenu(int &Gold, int &Wood, int &Ore, int &Leadership, int* Army, vector <struct Weapon> &PlayersItems)
{
     cout<<"Entering "<<name<<endl;
     MusicList TownMusic(musicfile, 'u');
     TownMusic.PlayStream(-1);
     CommandReset(0);
     clear_keybuf();
     while(true)
     {
          clear(buffer);
          prevKeyPress = keyPress;
          TownMusic.PlayStream(0);
          
          draw_sprite(buffer, town_screen, 0, 0);
          draw_sprite(buffer, background, 0, 0);
          
          if(KeyReleased(KEY_BACKSLASH)&&enableCommand==0) {clear_keybuf(); enableCommand=1;}
          if(KeyReleased(KEY_ESC))
          {
              if (enableCommand==1) CommandReset(0);
              else break;
          }
          if(KeyReleased(KEY_S)) break;
          else if(KeyReleased(KEY_ENTER)&&enableCommand==1)
          {
               time(&rawtime);
               char timeS[10];
               strftime(timeS, 10, "%X ", localtime(&rawtime));
               cout<<timeS<<commandLine<<endl;
               command=commandLine;
               CommandInterpretTown(Gold, Wood, Ore, Leadership, Army, PlayersItems);
               CommandReset(0);
          }
          if(KeyReleased(KEY_C)&&enableCommand==0) player_ptr->Card();// open player's card
          if(KeyReleased(KEY_U)&&enableCommand==0) player_ptr->UnitStats();
          
          CommandLineON();
          textout_centre_ex(buffer, font1, name.c_str(), screenW-115, 20, WHITE, -1);
          textout_centre_ex(buffer, font, "BUILDINGS:", screenW-115, 100, WHITE, -1);
          for(int i=0; i<buildings.size(); i++)
          {
                  if(buildings[i].built==true) textout_centre_ex(buffer, font, buildings[i].name.c_str(), screenW-115, 120+i*12, GREEN, -1);
                  else if(Wood-buildings[i].WoodCost>=0&&Ore-buildings[i].OreCost>=0) textout_centre_ex(buffer, font, buildings[i].name.c_str(), screenW-115, 120+i*12, WHITE, -1);
                  else textout_centre_ex(buffer, font, buildings[i].name.c_str(), screenW-115, 120+i*12, RED, -1);
          }
          textout_centre_ex(buffer, font, "UNITS:", screenW-115, 300, WHITE, -1);
          if (Built(TOWN_HALL)) for(int i=0; i<units.size(); i++)
                  textout_centre_ex(buffer, font, units[i].name.c_str(), screenW-115, 320+i*12, WHITE, -1);
          else textout_centre_ex(buffer, font, "Not available now", screenW-115, 320, RED, -1);
                  
          textout_centre_ex(buffer, font1, player_ptr->name.c_str(), screenW-115, 580, WHITE, -1);
          draw_sprite(buffer, gold, screenW-200, 610);
          textprintf_ex(buffer, font, screenW-160, 615, WHITE, -1, "%d", Gold);
          draw_sprite(buffer, wood, screenW-200, 630);
          textprintf_ex(buffer, font, screenW-160, 635, WHITE, -1, "%d", Wood);
          draw_sprite(buffer, ore, screenW-200, 650);
          textprintf_ex(buffer, font, screenW-160, 655, WHITE, -1, "%d", Ore);
          
          textout_ex(buffer, font, commandLine.c_str(), 20, 740, WHITE, -1);
          if(command[0]!=0) textout_ex(buffer, font1, string("Last command: "+command).c_str(), 20, 710, WHITE, -1);
          if(enableCommand==1) vline(buffer, caret * 8+20, 738, 748, WHITE); // draw the caret
          
         draw_sprite(screen, buffer, 0, 0);
     }
     CommandReset(0);
     command = commandLine;
     cout<<"Leaving "<<name<<endl;
     TownMusic.MusicList::~MusicList();
}

void Player::Town::CommandInterpretTown(int &Gold, int &Wood, int &Ore, int &Leadership, int* Army, vector <struct Weapon> &PlayersItems)
{
     int space=0;
     for(int i=0; commandLine[i]; i++) if(commandLine[i]==' ') space++;
          if(space>0)
          {
             vector <string> v;
             split(commandLine, ' ', v);
             if (strcmp(UpperAll(v[0]).c_str(),"BUILD")==0)
             {
                       cout<<"Command \"build\" recognized..."<<endl;
                       int id;
                          if(strcmp(UpperAll(v[1]).c_str(),"TOWN_HALL")==0||(strcmp(UpperAll(v[1]).c_str(),"TOWN")==0&&strcmp(UpperAll(v[2]).c_str(),"HALL")==0)) id = TOWN_HALL;
                          else if(strcmp(UpperAll(v[1]).c_str(),"CITY_WALLS")==0||(strcmp(UpperAll(v[1]).c_str(),"CITY")==0&&strcmp(UpperAll(v[2]).c_str(),"WALLS")==0)) id = CITY_WALLS;
                          else if(strcmp(UpperAll(v[1]).c_str(),"TAVERN")==0) id = TAVERN;
                          else if(strcmp(UpperAll(v[1]).c_str(),"FORGE")==0) id = FORGE;
                          else if(strcmp(UpperAll(v[1]).c_str(),"MARKSMAN")==0) id = MARKSMAN;
                          else if(strcmp(UpperAll(v[1]).c_str(),"HUNTSMAN")==0) id = HUNTSMAN;
                          else if(strcmp(UpperAll(v[1]).c_str(),"MAGE_TOWER")==0||(strcmp(UpperAll(v[1]).c_str(),"MAGE")==0&&strcmp(UpperAll(v[2]).c_str(),"TOWER")==0)) id = MAGE_TOWER;
                          else if(strcmp(UpperAll(v[1]).c_str(),"CHAPEL")==0) id = CHAPEL;
                          else if(strcmp(UpperAll(v[1]).c_str(),"LUMBER_MILL")==0||(strcmp(UpperAll(v[1]).c_str(),"LUMBER")==0&&strcmp(UpperAll(v[2]).c_str(),"MILL")==0)) id = LUMBER_MILL;
                          else if(strcmp(UpperAll(v[1]).c_str(),"QUARRIES")==0) id = QUARRIES;
                          else if(strcmp(UpperAll(v[1]).c_str(),"LARGE_HUT")==0||(strcmp(UpperAll(v[1]).c_str(),"LARGE")==0&&strcmp(UpperAll(v[2]).c_str(),"HUT")==0)) id = LARGE_HUT;
                          else if(strcmp(UpperAll(v[1]).c_str(),"ALL")==0)
                          {
                               for(int j=0; j<buildings.size(); j++)
                               {
                                       buildings[j].built=true;
                                       if(buildings[j].ID==LARGE_HUT)
                                       {
                                               Leadership+=50;
                                               player_ptr->attributes.Leadership+=50;
                                       }
                               }
                               command = "Cheater!";
                          }
                          
                       for(int j=0; j<buildings.size(); j++)
                       {
                               if(buildings[j].ID==id)
                               {
                                      if(buildings[j].built) cout<<"BuildingID="<<buildings[j].ID<<" already built."<<endl;
                                      else 
                                      {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           { 
                                                 if(Build(buildings[j].ID))
                                                 {
                                                       Wood-=buildings[j].WoodCost;
                                                       Ore-=buildings[j].OreCost;
                                                       buildings[j].built=true;
                                                       if(buildings[j].ID==LARGE_HUT)
                                                       {
                                                            Leadership+=50;
                                                            player_ptr->attributes.Leadership+=50;
                                                       }
                                                 }
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                      }
                                      break;
                               }
                               else if(j==buildings.size()-1) cout<<"BuildingID="<<buildings[j].ID<<" not available in this town."<<endl;
                       }
             }
             else if (strcmp(UpperAll(v[0]).c_str(),"DESTROY")==0)
             {
                  cout<<"Command \"destroy\" recognized..."<<endl;
                  if(strcmp(UpperAll(v[1]).c_str(),"ALL")==0) for(int j=0; j<buildings.size(); j++) buildings[j].built=false;
             }
             else if(strcmp(UpperAll(v[0]).c_str(),"TOWN_HALL")==0||(strcmp(UpperAll(v[0]).c_str(),"TOWN")==0&&strcmp(UpperAll(v[1]).c_str(),"HALL")==0))
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==TOWN_HALL)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(v[0]).c_str(),"CITY_WALLS")==0||(strcmp(UpperAll(v[0]).c_str(),"CITY")==0&&strcmp(UpperAll(v[1]).c_str(),"WALLS")==0))
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==CITY_WALLS)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(v[0]).c_str(),"MAGE_TOWER")==0||(strcmp(UpperAll(v[0]).c_str(),"MAGE")==0&&strcmp(UpperAll(v[1]).c_str(),"TOWER")==0))
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==MAGE_TOWER)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(v[0]).c_str(),"LUMBER_MILL")==0||(strcmp(UpperAll(v[0]).c_str(),"LUMBER")==0&&strcmp(UpperAll(v[1]).c_str(),"MILL")==0))
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==LUMBER_MILL)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(v[0]).c_str(),"LARGE_HUT")==0||(strcmp(UpperAll(v[0]).c_str(),"LARGE")==0&&strcmp(UpperAll(v[1]).c_str(),"HUT")==0))
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==MAGE_TOWER)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                               Leadership+=50;
                                               player_ptr->attributes.Leadership+=50;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
             else if (strcmp(UpperAll(v[0]).c_str(),"RECRUIT")==0)
             {
                  cout<<"Command \"recruit\" recognized..."<<endl;
                  if(!Built(TOWN_HALL)) {cout<<"You have to build Town_Hall first, to recruit town units."<<endl; return;}
                  int id=-1;
                  if(strcmp(UpperAll(v[1]).c_str(),"PIKEMEN")==0) id = PIKEMEN;
                  else if(strcmp(UpperAll(v[1]).c_str(),"ARCHERS")==0) id = ARCHERS;
                  else if(strcmp(UpperAll(v[1]).c_str(),"SCOUTS")==0) id = SCOUTS;
                  else if(strcmp(UpperAll(v[1]).c_str(),"MAGES")==0) id = MAGES;
                  else if(strcmp(UpperAll(v[1]).c_str(),"WARRIORS")==0) id = WARRIORS;
                  else if(strcmp(UpperAll(v[1]).c_str(),"PRIESTS")==0) id = PRIESTS;
                  switch (id)
                  {
                         case PIKEMEN:
                         {
                              for(int i=0; i<units.size(); i++)
                                   if(units[i].ID==PIKEMEN)
                                   {
                                          if(player_ptr->Units[PIKEMEN].Recruit(Gold, Leadership, unit_pikemen, Built(CITY_WALLS)))
                                          {
                                               int Amount;
                                               stringstream ssAmount;
                                               ssAmount<<commandLine;
                                               if(commandLine.c_str()!=0) ssAmount>>Amount;
                                               else Amount =0 ;
                                               if(Built(CITY_WALLS)) Gold-=Amount*player_ptr->Units[PIKEMEN].costUpg;
                                               else Gold-=Amount*player_ptr->Units[PIKEMEN].cost;
                                               Leadership-=Amount*player_ptr->Units[PIKEMEN].Leadership;
                                               Army[PIKEMEN]+=Amount;
                                               cout<<"You have successfully recruited "<<Amount<<" "<<player_ptr->Units[PIKEMEN].name<<endl;
                                          }
                                          break;
                                   }
                         } break;
                         case ARCHERS:
                         {
                              
                              for(int i=0; i<units.size(); i++)
                                   if(units[i].ID==ARCHERS)
                                   {
                                          if(player_ptr->Units[ARCHERS].Recruit(Gold, Leadership, unit_archers, Built(CITY_WALLS)))
                                          {
                                               int Amount;
                                               stringstream ssAmount;
                                               ssAmount<<commandLine;
                                               if(commandLine.c_str()!=0) ssAmount>>Amount;
                                               else Amount =0 ;
                                               if(Built(CITY_WALLS)) Gold-=Amount*player_ptr->Units[ARCHERS].costUpg;
                                               else Gold-=Amount*player_ptr->Units[ARCHERS].cost;
                                               Leadership-=Amount*player_ptr->Units[ARCHERS].Leadership;
                                               Army[ARCHERS]+=Amount;
                                               cout<<"You have successfully recruited "<<Amount<<" "<<player_ptr->Units[ARCHERS].name<<endl;
                                          }
                                          break;
                                   }
                         } break;
                         case SCOUTS:
                         {
                              for(int i=0; i<units.size(); i++)
                                   if(units[i].ID==SCOUTS)
                                   {
                                          if(player_ptr->Units[SCOUTS].Recruit(Gold, Leadership, unit_scouts, Built(CITY_WALLS)))
                                          {
                                               int Amount;
                                               stringstream ssAmount;
                                               ssAmount<<commandLine;
                                               if(commandLine.c_str()!=0) ssAmount>>Amount;
                                               else Amount =0 ;
                                               if(Built(CITY_WALLS)) Gold-=Amount*player_ptr->Units[SCOUTS].costUpg;
                                               else Gold-=Amount*player_ptr->Units[SCOUTS].cost;
                                               Leadership-=Amount*player_ptr->Units[SCOUTS].Leadership;
                                               Army[SCOUTS]+=Amount;
                                               cout<<"You have successfully recruited "<<Amount<<" "<<player_ptr->Units[SCOUTS].name<<endl;
                                          }
                                          break;
                                   }
                         } break;
                         case MAGES:
                         {
                              for(int i=0; i<units.size(); i++)
                                   if(units[i].ID==MAGES)
                                   {
                                          if(player_ptr->Units[MAGES].Recruit(Gold, Leadership, unit_mages, Built(CITY_WALLS)))
                                          {
                                               int Amount;
                                               stringstream ssAmount;
                                               ssAmount<<commandLine;
                                               if(commandLine.c_str()!=0) ssAmount>>Amount;
                                               else Amount =0 ;
                                               if(Built(CITY_WALLS)) Gold-=Amount*player_ptr->Units[MAGES].costUpg;
                                               else Gold-=Amount*player_ptr->Units[MAGES].cost;
                                               Leadership-=Amount*player_ptr->Units[MAGES].Leadership;
                                               Army[MAGES]+=Amount;
                                               cout<<"You have successfully recruited "<<Amount<<" "<<player_ptr->Units[MAGES].name<<endl;
                                          }
                                          break;
                                   }
                         } break;
                         case WARRIORS:
                         {
                              for(int i=0; i<units.size(); i++)
                                   if(units[i].ID==WARRIORS)
                                   {
                                          if(player_ptr->Units[WARRIORS].Recruit(Gold, Leadership, unit_warriors, Built(CITY_WALLS)))
                                          {
                                               int Amount;
                                               stringstream ssAmount;
                                               ssAmount<<commandLine;
                                               if(commandLine.c_str()!=0) ssAmount>>Amount;
                                               else Amount =0 ;
                                               if(Built(CITY_WALLS)) Gold-=Amount*player_ptr->Units[WARRIORS].costUpg;
                                               else Gold-=Amount*player_ptr->Units[WARRIORS].cost;
                                               Leadership-=Amount*player_ptr->Units[WARRIORS].Leadership;
                                               Army[WARRIORS]+=Amount;
                                               cout<<"You have successfully recruited "<<Amount<<" "<<player_ptr->Units[WARRIORS].name<<endl;
                                          }
                                          break;
                                   }
                         } break;
                         case PRIESTS:
                         {
                              for(int i=0; i<units.size(); i++)
                                   if(units[i].ID==PRIESTS)
                                   {
                                          if(player_ptr->Units[PRIESTS].Recruit(Gold, Leadership, unit_priests, buildings[CITY_WALLS].built))
                                          {
                                               int Amount;
                                               stringstream ssAmount;
                                               ssAmount<<commandLine;
                                               if(commandLine.c_str()!=0) ssAmount>>Amount;
                                               else Amount =0 ;
                                               if(Built(CITY_WALLS)) Gold-=Amount*player_ptr->Units[PRIESTS].costUpg;
                                               else Gold-=Amount*player_ptr->Units[PRIESTS].cost;
                                               Leadership-=Amount*player_ptr->Units[PRIESTS].Leadership;
                                               Army[PRIESTS]+=Amount;
                                               cout<<"You have successfully recruited "<<Amount<<" "<<player_ptr->Units[PRIESTS].name<<endl;
                                          }
                                          break;
                                   }
                         } break;
                         default:
                         {
                         } break;
                  }
             }
             //else cout<<"Command not recognized..."<<endl;
             v.clear();
          }
          else
          {
              if(strcmp(UpperAll(commandLine).c_str(), "TAVERN")==0)
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==TAVERN)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                                   Tavern(PlayersItems, Gold);
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(commandLine).c_str(),"FORGE")==0)
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==FORGE)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(commandLine).c_str(),"MARKSMAN")==0)
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==MARKSMAN)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(commandLine).c_str(),"HUNTSMAN")==0)
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==HUNTSMAN)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(commandLine).c_str(),"CHAPEL")==0)
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==CHAPEL)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(commandLine).c_str(),"QUARRIES")==0)
              {
                   for(int j=0; j<buildings.size(); j++)
                   {
                        if(buildings[j].ID==QUARRIES)
                        {
                              if(!buildings[j].built)
                              {
                                    if(Build(buildings[j].ID))
                                    {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           {
                                               Wood-=buildings[j].WoodCost;
                                               Ore-=buildings[j].OreCost;
                                               buildings[j].built=true;
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                    }
                              }
                              else //if the building is already built
                              {
                              }
                        }
                   }
              }
              else if(strcmp(UpperAll(commandLine).c_str(), "BUILD")==0)
              {
                   cout<<"What should be built? ";
                   string toBeBuilt;
                   cin>>toBeBuilt;
                   int id;
                   if(strcmp(UpperAll(toBeBuilt).c_str(),"TOWN_HALL")==0) id = TOWN_HALL;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"CITY_WALLS")==0) id = CITY_WALLS;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"TAVERN")==0) id = TAVERN;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"FORGE")==0) id = FORGE;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"MARKSMAN")==0) id = MARKSMAN;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"HUNTSMAN")==0) id = HUNTSMAN;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"MAGE_TOWER")==0) id = MAGE_TOWER;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"CHAPEL")==0) id = CHAPEL;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"LUMBER_MILL")==0) id = LUMBER_MILL;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"QUARRIES")==0) id = QUARRIES;
                   else if(strcmp(UpperAll(toBeBuilt).c_str(),"LARGE_HUT")==0) id = LARGE_HUT;
                        for(int j=0; j<buildings.size(); j++)
                       {
                               if(buildings[j].ID==id)
                               {
                                      if(buildings[j].built) cout<<"BuildingID="<<buildings[j].ID<<" already built."<<endl;
                                      else 
                                      {
                                           if(Wood-buildings[j].WoodCost>=0&&Ore-buildings[j].OreCost>=0)
                                           { 
                                                 if(Build(buildings[j].ID))
                                                 {
                                                       Wood-=buildings[j].WoodCost;
                                                       Ore-=buildings[j].OreCost;
                                                       buildings[j].built=true;
                                                       if(buildings[j].ID==LARGE_HUT)
                                                       {
                                                            Leadership+=50;
                                                            player_ptr->attributes.Leadership+=50;
                                                       }
                                                 }
                                           }
                                           else while(true)
                                           {
                                                 prevKeyPress = keyPress;
                                                 framing(buffer, screenWidth/2-150, screenHeight/2-200, font1, "You don't have enough sources.", BLUE);
                                                 framing(buffer, screenWidth/2-130, screenHeight/2-150, font1, "Press ENTER to continue", BLACK);
                                                 if(KeyPressed(KEY_ENTER)) break;
                                                 if(KeyPressed(KEY_ESC)) break;
                                                 draw_sprite(screen, buffer, 0, 0);
                                           }
                                      }
                                      break;
                               }
                               else if(j==buildings.size()-1) cout<<"BuildingID="<<buildings[j].ID<<" not available in this town."<<endl;
                       }
                       
              }
          }
}

bool Player::Town::Build(int BuildingID)
{
     for(int i=0; i<buildings.size(); i++)
     {
             if(buildings[i].ID==BuildingID)
             {
                    if(text_length(font1, buildings[i].descr.c_str())>700) //descr should be displayed in two rows
                    {
                          // splitting into 2 rows
                    }
                    while(true)
                    {
                        prevKeyPress = keyPress;
                        if(KeyReleased(KEY_Y)) {return true; break;}
                        if(KeyReleased(KEY_ENTER)) {return true; break;}
                        if(KeyReleased(KEY_N)) {return false; break;}
                        if(KeyReleased(KEY_ESC)) {return false; break;}
                        
                        draw_sprite(buffer, builder, 30, 100);
                        textout_centre_ex(buffer, MenuFont, buildings[i].name.c_str(), screenWidth/2, screenHeight/2-100, RED, -1);
                        textout_centre_ex(buffer, font1, buildings[i].descr.c_str(), screenWidth/2, screenHeight/2-60, RED, -1);
                        textprintf_centre_ex(buffer, font1, screenWidth/2, screenHeight/2, BLACK, -1, "Do you want to build %s?(Y/N)", buildings[i].name.c_str());
                        draw_sprite(buffer, wood, screenWidth/2-100, screenHeight/2+40);
                        textprintf_ex(buffer, font1, screenWidth/2-60, screenHeight/2+35, BLACK, -1, "cost: %d", buildings[i].WoodCost);
                        draw_sprite(buffer, ore, screenWidth/2+30, screenHeight/2+40);
                        textprintf_ex(buffer, font1, screenWidth/2+70, screenHeight/2+35, BLACK, -1, "cost: %d", buildings[i].OreCost);
                        draw_sprite(screen, buffer, 0, 0);
                    }
             }
     }
}

void Player::Town::Tavern(vector <struct Weapon> &PlayersItems, int &Gold)
{
     bool left=true, esc=false;
     int position=0;
     int availableItems;
     BITMAP** tavern = NULL;
     if(ID==ARIADOR||ID==BELLASEE_FALLS) tavern=&tavern_human;
     else if(ID==KHARAMDOOR) tavern=&tavern_dwarven;
     else if(ID==ELVENMORT) tavern=&tavern_elven;
     while(true)
     {
                clear(buffer);
                prevKeyPress=keyPress;
                if(KeyReleased(KEY_ESC)) break;
                draw_sprite(buffer, town_screen, 0, 0);
                draw_sprite(buffer, *tavern, 0, 60);
                draw_sprite(buffer, background, 0, 0);
                
                textout_centre_ex(buffer, font, "Press ENTER to buy/sell", screenW-115, 100, WHITE, -1);
                textout_centre_ex(buffer, font, "Press E to equip", screenW-115, 120, WHITE, -1);
                textout_centre_ex(buffer, font, "Press ESC to quit", screenW-115, 140, WHITE, -1);
                for(int i=0; (i<player_ptr->Weapons.size()&&player_ptr->attributes.Level>=player_ptr->Weapons[i].Level); i++) //vendor's deck
                {
                          if(i==position&&left) textprintf_ex(buffer, font, 50, 300+i*20, RED, -1, "%s Damage:%s Cost:%d", player_ptr->Weapons[i].name.c_str(), player_ptr->Weapons[i].Damage.c_str(), player_ptr->Weapons[i].cost);
                          else textprintf_ex(buffer, font, 50, 300+i*20, BLACK, -1, "%s Damage:%s Cost:%d", player_ptr->Weapons[i].name.c_str(), player_ptr->Weapons[i].Damage.c_str(), player_ptr->Weapons[i].cost);
                          stringstream ss;
                          ss<<player_ptr->Weapons[i].name<<" Damage:"<<player_ptr->Weapons[i].Damage<<" Cost:"<<player_ptr->Weapons[i].cost;
                          draw_sprite(buffer, gold, 50+text_length(font, ss.str().c_str()), 295+i*20);
                          ss.clear();
                          availableItems=i+1;
                }
                textprintf_centre_ex(buffer, font1, 600, 100, BLACK, -1, "%s", player_ptr->name.c_str());
                draw_sprite(buffer, gold, 450, 145);
                textprintf_centre_ex(buffer, font, 500, 150, BLACK, -1, "%d", Gold);
                for(int i=0; i<player_ptr->PlayersItems.size(); i++) //player's deck
                {
                          if(i==position&&!left)
                          {
                                    if(PlayersItems[i].Equipped) textprintf_ex(buffer, font, 450, 300+i*20, BROWN, -1, "%s Damage:%s Cost:%d", PlayersItems[i].name.c_str(), PlayersItems[i].Damage.c_str(), PlayersItems[i].cost);
                                    else textprintf_ex(buffer, font, 450, 300+i*20, RED, -1, "%s Damage:%s Cost:%d", PlayersItems[i].name.c_str(), PlayersItems[i].Damage.c_str(), PlayersItems[i].cost);
                          }
                          else
                          {
                                    if(PlayersItems[i].Equipped) textprintf_ex(buffer, font, 450, 300+i*20, GREEN, -1, "%s Damage:%s Cost:%d", PlayersItems[i].name.c_str(), PlayersItems[i].Damage.c_str(), PlayersItems[i].cost);
                                    else textprintf_ex(buffer, font, 450, 300+i*20, BLACK, -1, "%s Damage:%s Cost:%d", PlayersItems[i].name.c_str(), PlayersItems[i].Damage.c_str(), PlayersItems[i].cost);
                          }
                          stringstream ss;
                          ss<<PlayersItems[i].name<<" Damage:"<<PlayersItems[i].Damage<<" Cost:"<<PlayersItems[i].cost;
                          draw_sprite(buffer, gold, 450+text_length(font, ss.str().c_str()), 295+i*20);
                          ss.clear();
                }
                if(KeyReleased(KEY_LEFT)&&!left) {left=true; position=0;}
                if(KeyReleased(KEY_RIGHT)&&left) {left=false; position=0;}
                if(KeyReleased(KEY_UP)&&position>0) position--;
                if(KeyReleased(KEY_DOWN)&&((left&&position<availableItems-1)||(!left&&position<player_ptr->PlayersItems.size()-1))) position++;
                if(KeyReleased(KEY_ENTER))
                {
                     if(left) //Player buys
                     {
                              Gold-=player_ptr->Weapons[position].cost;
                              player_ptr->attributes.Gold-=player_ptr->Weapons[position].cost;
                              PlayersItems.push_back(player_ptr->Weapons[position]);
                              for(int i=0; i<PlayersItems.size(); i++) PlayersItems[i].Equipped=false;
                              PlayersItems[PlayersItems.size()-1].Equipped=true;
                     }
                     else //Player sells
                     {
                          if(PlayersItems.size()>1)
                          {
                                Gold+=PlayersItems[position].cost;
                                player_ptr->attributes.Gold+=PlayersItems[position].cost;
                                PlayersItems.erase(PlayersItems.begin()+position);
                                PlayersItems[0].Equipped=true;
                          }
                          else cout<<"You can't sell the last weapon."<<endl;
                     }
                }
                if(KeyReleased(KEY_E)&&!left)
                {
                      for(int i=0; i<PlayersItems.size(); i++) PlayersItems[i].Equipped=false;
                      PlayersItems[position].Equipped=true;
                }
                                     
                draw_sprite(screen, buffer, 0, 0);
     }
}

bool Player::Town::Built(int BuildingID)
{
     for(int i=0; i<buildings.size(); i++)
             if (buildings[i].ID==BuildingID) return buildings[i].built;
     cout<<"There is no building of ID="<<BuildingID<<" in "<<name<<endl;
     return -1;
}

void Player::LoadBuildings()
{
    ifstream buildsfile;
	buildsfile.open("data\\towns\\buildings.txt", ios::in);
	if(buildsfile)
    {
               int i=0;
               string line;
               vector <string> v;
               while ( buildsfile.good() )
               {
                     getline (buildsfile,line);
                     split(line, ' ', v);
                     BuildingList[i].name = v[0];
                     if(BuildingList[i].name=="TOWN_HALL") BuildingList[i].ID = TOWN_HALL;
                     else if(BuildingList[i].name=="CITY_WALLS") BuildingList[i].ID = CITY_WALLS;
                     else if(BuildingList[i].name=="TAVERN") BuildingList[i].ID = TAVERN;
                     else if(BuildingList[i].name=="FORGE") BuildingList[i].ID = FORGE;
                     else if(BuildingList[i].name=="MARKSMAN") BuildingList[i].ID = MARKSMAN;
                     else if(BuildingList[i].name=="HUNTSMAN") BuildingList[i].ID = HUNTSMAN;
                     else if(BuildingList[i].name=="MAGE_TOWER") BuildingList[i].ID = MAGE_TOWER;
                     else if(BuildingList[i].name=="CHAPEL") BuildingList[i].ID = CHAPEL;
                     else if(BuildingList[i].name=="LUMBER_MILL") BuildingList[i].ID = LUMBER_MILL;
                     else if(BuildingList[i].name=="QUARRIES") BuildingList[i].ID = QUARRIES;
                     else if(BuildingList[i].name=="LARGE_HUT") BuildingList[i].ID = LARGE_HUT;
                     istringstream iss(v[1]);
                     iss >> BuildingList[i].WoodCost;
                     istringstream iss2(v[2]);
                     iss2 >> BuildingList[i].OreCost;
                     v.clear();
                     getline (buildsfile,BuildingList[i].descr);
                     BuildingList[i].built=false;
                     i++;
               }
    }
	else perror("Error with getting buildings data: ");
	buildsfile.close();
}

////// Player constructor
Player::Player(bool &esc)
{
      cout<<"Creating new player character...";
      ThrowPtr();
      bool done=false, option=false, PlExist=false;;
      CommandReset(1);
      while (!PlExist&&!esc)
      {
            clear(buffer);
            prevKeyPress = keyPress;
            CommandLineON();
            textout_centre_ex(buffer, font1, "What's your name, adventurer?", screenW/2, screenH/2, WHITE, -1);
            textout_centre_ex(buffer, font1, commandLine.c_str(), screenW/2, screenH/2+30, WHITE, -1);
            draw_sprite(screen, buffer, 0, 0);
            if(KeyReleased(KEY_ENTER))
            {
                  stringstream ss;
                  ss << "data\\players\\"<<commandLine;
                  if(mkdir(ss.str().c_str())==-1)
                  {
                       if(errno==EEXIST) {cout<<endl<<"Error: Player with that name already exists."<<endl; CommandReset(1);}
                  }
                  
                  else
                  {
                      PlExist = true;
                      name = commandLine;
                  }
            }
            if(KeyReleased(KEY_ESC)) esc = true;
      }
      done=false;
      CommandReset(1);
      if(!esc)
      {
          while(!done)
          {
                clear(buffer);
                prevKeyPress = keyPress;
                isPassword=YesNo("Would you like to set password?");
                stringstream ss;
                if(isPassword)
                              while (!done)
                              {
                                    clear(buffer);
                                    ss.str(string());
                                    prevKeyPress = keyPress;
                                    CommandLineON();
                                    textout_centre_ex(buffer, font1, "Ok, what should be your password?", screenW/2, screenH/2, WHITE, -1);
                                    for(int i=0; i<commandLine.size(); i++) ss<<"*";
                                    textout_centre_ex(buffer, font1, ss.str().c_str(), screenW/2, screenH/2+30, WHITE, -1);
                                    draw_sprite(screen, buffer, 0, 0);
                                    if(KeyReleased(KEY_ENTER))
                                    {
                                         pass = commandLine;
                                         done=true;
                                    }
                              }
                else break;
                draw_sprite(screen, buffer, 0, 0);
          }
          draw_sprite(screen, loader, 0, 0);
          Dead=false;
          attributes.MaxHealth = attributes.Health = 100;
          attributes.Exp = 0;
          attributes.Gold = 100;
          attributes.Wood = 10;
          attributes.Ore = 10;
          attributes.Level = level(attributes.Exp);
          attributes.Leadership=20;
          attributes.Diplomacy=0;
          attributes.Attack=10;
          attributes.Defence=10;
          Place = "map_1";
          LoadBuildings();          
          for(int i=0; i<4; i++)town[i].Load();
          
          vector <string> v;
         string line;
         ifstream weaponfile;
         weaponfile.open("data\\weapons.txt", ios::in);
         while(weaponfile.good())
         {
             struct Weapon temp;
             getline (weaponfile,line);
             split(line, ' ', v);
             for (int k = 0; k < v.size( ); ++k) 
             {
                  istringstream iss(v[k]);
                  switch(k)
                  {
                           case 0: iss >> temp.Level; break;
                           case 1: iss >> temp.name; break;
                           case 2: iss >> temp.Damage; break;
                           case 3: iss >> temp.cost; break;
                  }
             }
             v.clear();
             Weapons.push_back(temp);
         }
         weaponfile.close();
         PlayersWeapon = Weapons[0];
         PlayersWeapon.Equipped=true;
         PlayersItems.push_back(PlayersWeapon);
         ifstream unitfile;
         unitfile.open("data\\units.txt", ios::in);
         while(unitfile.good())
         {
             Unit temp(-1);
             getline (unitfile,line);
             split(line, ' ', v);
             for (int k = 0; k < v.size( ); ++k) 
             {
                  istringstream iss(v[k]);
                  switch(k)
                  {
                           case 0: iss >> temp.ID; break;
                           case 1: iss >> temp.name; break;
                           case 2: iss >> temp.MaxHealth; break;
                           case 3: iss >> temp.MaxHealthUpg; break;
                           case 4: iss >> temp.Attack; break;
                           case 5: iss >> temp.AttackUpg; break;
                           case 6: iss >> temp.Defence; break;
                           case 7: iss >> temp.DefenceUpg; break;
                           case 8: iss >> temp.Leadership; break;
                           case 9: iss >> temp.cost; break;
                           case 10: iss >> temp.costUpg; break;
                           case 11: iss >> temp.UnitsWeapon.name; break;
                  }
             }
             v.clear();
             for(int i=0; i<Weapons.size(); i++)
             {
                     if(strcmp(temp.UnitsWeapon.name.c_str(), Weapons[i].name.c_str())==0)
                     {
                          temp.UnitsWeapon=Weapons[i];
                          break;
                     }
             }
             Units.push_back(temp);
         }
         unitfile.close();
         for(int i=0; i<6; i++) Army[i]=0;
         
          x=0;
          y=0;
          direction=0;
          for(int i=0; i<10; i++) Quest[i]=0;
          cout<<"Success"<<endl;
          save();
      }
}

void Player::save()
{
     cout<<"Saving player character data to file...";
     ofstream ofile;
     
         stringstream ss;
         ss << "data\\players\\"<<name;
         mkdir(ss.str().c_str());
         
     string filPath = ss.str() + "\\PlayerData";
     ofile.open(filPath.c_str(), ios::out | ios::binary);     
     ofile<<name<<endl;
     ofile<<isPassword<<endl;
     ofile<<attributes.MaxHealth<<endl;
     ofile<<attributes.Health<<endl;
     ofile<<attributes.Exp<<endl;
     ofile<<attributes.Gold<<endl;
     ofile<<attributes.Wood<<endl;
     ofile<<attributes.Ore<<endl;
     ofile<<attributes.Level<<endl;
     ofile<<attributes.Leadership<<endl;
     ofile<<attributes.Diplomacy<<endl;
     ofile<<attributes.Attack<<endl;
     ofile<<attributes.Defence<<endl;
     ofile<<Place<<endl;
     ofile<<x<<endl;
     ofile<<y<<endl;
     ofile<<direction<<endl;
     for(int i=0; i<10; i++) ofile<<Quest[i]<<endl;
     ofile<<PlayersItems.size()<<endl;
     for(int i=0; i<PlayersItems.size(); i++) ofile<<PlayersItems[i].name<<endl;
     for(int i=0; i<PlayersItems.size(); i++) if(PlayersItems[i].Equipped) ofile<<i<<endl;
     for(int i=0; i<4; i++)
     {
             for(int j=0; j<town[i].buildings.size(); j++)
             {
                      ofile<<town[i].buildings[j].built<<endl;
             }
     }
     for(int i=0; i<6; i++) ofile<<Army[i]<<endl;
     ofile.close();
     
     ofile.open(string(ss.str()+"\\"+Place).c_str(), ios::out | ios::binary);
     ofile<<CampsCleared.size()<<endl;
     for(int i=0; i<CampsCleared.size(); i++) ofile<<CampsCleared[i].x<<endl<<CampsCleared[i].y<<endl;
     ofile.close();
     
     if(isPassword)
     {
         stringstream ss2;
         ss2 << "data\\players\\"<<name<<"\\pass";
         ofstream f;
         f.open(ss2.str().c_str(), ios::out | ios::binary);
         f<<pass;
         f.close();
     }
     cout<<"Success"<<endl;
}

void Player::load()
{
     cout<<"Loading player character data from file...";
     ifstream ifile;
         stringstream ss;
         ss << "data\\players\\"<<name<<"\\PlayerData";
     
     ifile.open(ss.str().c_str(), ios::in | ios::binary);
     ifile>>name;
     ifile>>isPassword;
     ifile>>attributes.MaxHealth;
     ifile>>attributes.Health;
     ifile>>attributes.Exp;
     ifile>>attributes.Gold;
     ifile>>attributes.Wood;
     ifile>>attributes.Ore;
     ifile>>attributes.Level;
     ifile>>attributes.Leadership;
     ifile>>attributes.Diplomacy;
     ifile>>attributes.Attack;
     ifile>>attributes.Defence;
     ifile>>Place;
     ifile>>x;
     ifile>>y;
     ifile>>direction;
     for(int i=0; i<10; i++) ifile>>Quest[i];
     vector <string> v;
     string line;
     ifstream weaponfile;
         weaponfile.open("data\\weapons.txt", ios::in);
         while(weaponfile.good())
         {
             struct Weapon temp;
             getline (weaponfile,line);
             split(line, ' ', v);
             for (int k = 0; k < v.size( ); ++k) 
             {
                  istringstream iss(v[k]);
                  switch(k)
                  {
                           case 0: iss >> temp.Level; break;
                           case 1: iss >> temp.name; break;
                           case 2: iss >> temp.Damage; break;
                           case 3: iss >> temp.cost; break;
                  }
             }
             v.clear();
             Weapons.push_back(temp);
         }
         weaponfile.close();
     int ItemsAmount;
     ifile>>ItemsAmount;
     for(int i=0; i<ItemsAmount; i++)
     {
             struct Weapon temp;
             ifile>>temp.name;
             for(int i=0; i<Weapons.size(); i++)
             {
                     if(strcmp(Weapons[i].name.c_str(), temp.name.c_str())==0)
                     {
                          temp=Weapons[i];
                          break;
                     }
             }
             temp.Equipped=false;
             PlayersItems.push_back(temp);
     }
     int ItemEquipped;
     ifile>>ItemEquipped;
     PlayersItems[ItemEquipped].Equipped=true;
     PlayersWeapon=PlayersItems[ItemEquipped];
     LoadBuildings();
     for(int i=0; i<4; i++)
     {
             town[i].Load();
             for(int j=0; j<town[i].buildings.size(); j++)
             {
                      ifile>>town[i].buildings[j].built;
             }
     }
     ifstream unitfile;
     unitfile.open("data\\units.txt", ios::in);
     while(unitfile.good())
     {
         Unit temp(-1);
         getline (unitfile,line);
         split(line, ' ', v);
         for (int k = 0; k < v.size( ); ++k) 
         {
              istringstream iss(v[k]);
              switch(k)
              {
                       case 0: iss >> temp.ID; break;
                       case 1: iss >> temp.name; break;
                       case 2: iss >> temp.MaxHealth; break;
                       case 3: iss >> temp.MaxHealthUpg; break;
                       case 4: iss >> temp.Attack; break;
                       case 5: iss >> temp.AttackUpg; break;
                       case 6: iss >> temp.Defence; break;
                       case 7: iss >> temp.DefenceUpg; break;
                       case 8: iss >> temp.Leadership; break;
                       case 9: iss >> temp.cost; break;
                       case 10: iss >> temp.costUpg; break;
                       case 11: iss >> temp.UnitsWeapon.name; break;
              }
         }
         v.clear();
         for(int i=0; i<Weapons.size(); i++)
             {
                     if(strcmp(temp.UnitsWeapon.name.c_str(), Weapons[i].name.c_str())==0)
                     {
                          temp.UnitsWeapon=Weapons[i];
                          break;
                     }
             }
         Units.push_back(temp);
     }
     unitfile.close();
     for(int i=0; i<6; i++) ifile>>Army[i];
     ifile.close();
     if(isPassword)
     {
         stringstream ss2;
         ss2 << "data\\players\\"<<name<<"\\pass";
         ifstream f;
         f.open(ss2.str().c_str(), ios::in | ios::binary);
         getline(f, pass);
         f.close();
     }
     
     ifile.open(string("data\\players\\"+name+"\\"+Place).c_str(), ios::in | ios::binary);
     int Campsno;
     ifile>>Campsno;
     for(int i=0; i<Campsno; i++)
     {
             struct Camp Camptemp;
             ifile>>Camptemp.x;
             ifile>>Camptemp.y;
             CampsCleared.push_back(Camptemp);
     }
     ifile.close();
     
     Dead=false;
     cout<<"Success"<<endl;
}

void Player::Card()
{
     while(true)
     {
                     prevKeyPress = keyPress;
                     if(KeyReleased(KEY_ESC)||KeyReleased(KEY_ENTER)) break;
                     int maxexp=0;
                     for(int level=1;level<=attributes.Level; level++) maxexp=maxexp+level*100; //calculating exp needed to lvl up
                     draw_sprite(buffer, book, 100, 100);
                     textprintf_centre_ex(buffer, font1, 250, 130, RED, -1, name.c_str());
                     textprintf_ex(buffer, font, 150, 160, BLACK, -1, "HP %d/%d Lvl %d (%d/%d exp)", attributes.Health, attributes.MaxHealth, attributes.Level, attributes.Exp, maxexp);
                     textprintf_ex(buffer, font, 150, 180, BLACK, -1, "Attack %d", attributes.Attack);
                     textprintf_ex(buffer, font, 150, 200, BLACK, -1, "Defence %d", attributes.Defence);
                     textprintf_ex(buffer, font, 150, 220, BLACK, -1, "Leadership points %d", attributes.Leadership);
                     textprintf_ex(buffer, font, 150, 240, BLACK, -1, "Diplomacy %d", attributes.Diplomacy);
                     textprintf_ex(buffer, font, 150, 260, BLACK, -1, "Equipment:");
                     textprintf_ex(buffer, font, 150, 280, BLACK, -1, "Weapon %s damage:%s", PlayersWeapon.name.c_str(), PlayersWeapon.Damage.c_str());
                     textprintf_ex(buffer, font, 420, 140, BLACK, -1, "Place: %s", Place.c_str());
                     textprintf_ex(buffer, font, 420, 160, BLACK, -1, "Army:");
                     textprintf_ex(buffer, font, 420, 180, BLACK, -1, "Pikemen: %d", Army[PIKEMEN]);
                     textprintf_ex(buffer, font, 420, 200, BLACK, -1, "Archers: %d", Army[ARCHERS]);
                     textprintf_ex(buffer, font, 420, 220, BLACK, -1, "Scouts: %d", Army[SCOUTS]);
                     textprintf_ex(buffer, font, 420, 240, BLACK, -1, "Mages: %d", Army[MAGES]);
                     textprintf_ex(buffer, font, 420, 260, BLACK, -1, "Warriors: %d", Army[WARRIORS]);
                     textprintf_ex(buffer, font, 420, 280, BLACK, -1, "Priests: %d", Army[PRIESTS]);
                     draw_sprite(screen, buffer, 0, 0);
     }
}

void Player::UnitStats()
{
     while(!KeyReleased(KEY_ESC))
     {
                     prevKeyPress = keyPress;
                     draw_sprite(buffer, book, 100, 100);
                     for (int i=0; i<Units.size(); i++)
                     {
                         textprintf_ex(buffer, font, 140, 160+i*55, BLACK, -1, "%s: HP %d, ATK %d, DEF %d,", Units[i].name.c_str(), Units[i].MaxHealth, Units[i].Attack, Units[i].Defence);
                         textprintf_ex(buffer, font, 400, 160+i*55, BLACK, -1, "Leadership points %d, Cost %d", Units[i].Leadership, Units[i].cost);
                         textprintf_ex(buffer, font, 140, 180+i*55, BLACK, -1, "Upgraded: HP %d, ATK %d, DEF %d", Units[i].MaxHealthUpg, Units[i].AttackUpg, Units[i].DefenceUpg);
                         switch(Units[i].ID)
                         {
                               case PIKEMEN: textprintf_ex(buffer, font, 400, 180+i*55, BLACK, -1, "Upgrade in: Ariador's Forge"); break;
                               case ARCHERS: textprintf_ex(buffer, font, 400, 180+i*55, BLACK, -1, "Upgrade in: Marksman"); break;
                               case SCOUTS: textprintf_ex(buffer, font, 400, 180+i*55, BLACK, -1, "Upgrade in: Huntsman"); break;
                               case MAGES: textprintf_ex(buffer, font, 400, 180+i*55, BLACK, -1, "Upgrade in: Mage Tower"); break;
                               case WARRIORS: textprintf_ex(buffer, font, 400, 180+i*55, BLACK, -1, "Upgrade in: Kharamdoor's Forge"); break;
                               case PRIESTS: textprintf_ex(buffer, font, 400, 180+i*55, BLACK, -1, "Upgrade in: Chapel"); break;
                         }
                         draw_sprite(buffer, gold, 625, 155+i*55);
                     }
                     draw_sprite(screen, buffer, 0, 0);
     }
}

void Player::CameraFollow(class Map &map)
{
     //if(x-screenWidth/2<map.MapWidth)
          if((x-screenWidth/2>=0)&&(x+screenWidth/2<map.MapWidth)) cameraX = x - screenWidth / 2;
          else if (x-screenWidth/2<0) cameraX = 0;
          else cameraX = map.MapWidth - screenWidth;
          if((y-screenHeight/2>=0)&&(y+screenHeight/2<map.MapHeight)) cameraY = y - screenHeight / 2;
          else if (y-screenHeight/2<0) cameraY = 0;
          else cameraY = map.MapHeight - screenHeight;
}

void Player::Move(class Map &map, class MusicList &BackgroundMusic)
{
     if(x%80==0&&y%80==0)
     {
         if(map.MapFields[getWhole(y/80)][getWhole(x/80)]==21) //player encounter camp
         {
                 int prevGold=attributes.Gold, prevWood=attributes.Wood, prevOre=attributes.Ore;
                 Camping();
                 draw_sprite(gold, buffer, x-cameraX, y-cameraY);
                 textprintf_ex(buffer, font, x-cameraX+25, y-cameraY, WHITE, -1, "%d", attributes.Gold-prevGold);
                 map.MapFields[getWhole(y/80)][getWhole(x/80)]=1;
         }
         if(map.MapFields[getWhole(y/80)][getWhole(x/80)]==22) //player encounter camp on cobblestone
         {
                 Camping();
                 map.MapFields[getWhole(y/80)][getWhole(x/80)]=3;
         }
     }
     switch(direction)
     {
          case RIGHT_DIR: //heading right
          {
               if(x%80!=0)  //player already in move
               {
                     CameraFollow(map);
                     if(frame%80<10) draw_sprite(buffer, player_right1, x-cameraX, y-cameraY);
                     else if(frame%80>=10&&frame%80<20) draw_sprite(buffer, player_right2, x-cameraX, y-cameraY);
                     else if(frame%80>=20&&frame%80<30) draw_sprite(buffer, player_right3, x-cameraX, y-cameraY);
                     else if(frame%80>=30&&frame%80<40) draw_sprite(buffer, player_right4, x-cameraX, y-cameraY);
                     else if(frame%80>=40&&frame%80<50) draw_sprite(buffer, player_right5, x-cameraX, y-cameraY);
                     else if(frame%80>=50&&frame%80<60) draw_sprite(buffer, player_right6, x-cameraX, y-cameraY);
                     else if(frame%80>=60&&frame%80<70) draw_sprite(buffer, player_right7, x-cameraX, y-cameraY);
                     else if(frame%80>=70&&frame%80<80) draw_sprite(buffer, player_right8, x-cameraX, y-cameraY);
                     x++;
               }
               else if(enableCommand==0) //player standing still
               {
                     draw_sprite(buffer, player_right, x-cameraX, y-cameraY);
                     if(KeyPressed(KEY_D)||KeyStillPressed(KEY_D)) {CameraFollow(map); if(getWhole(x/80)+1<32&&map.Movable(getWhole(x/80)+1, getWhole(y/80))) x++;} //if possible move in right direction
                     else if(KeyPressed(KEY_A)||KeyStillPressed(KEY_A)) {CameraFollow(map); direction = LEFT_DIR;} //turn left
                     else if(KeyPressed(KEY_S)||KeyStillPressed(KEY_S)) {CameraFollow(map); direction = DOWN_DIR;} // turn down
                     else if(KeyPressed(KEY_W)||KeyStillPressed(KEY_W)) {CameraFollow(map); direction = UP_DIR;} //turn up
               }
               else draw_sprite(buffer, player_right, x-cameraX, y-cameraY);
          } break;
          case LEFT_DIR: //heading left
          {
               if(x%80!=0)  //player already in move
               {
                     CameraFollow(map);
                     if(frame%80<10) draw_sprite(buffer, player_left1, x-cameraX, y-cameraY);
                     else if(frame%80>=10&&frame%80<20) draw_sprite(buffer, player_left2, x-cameraX, y-cameraY);
                     else if(frame%80>=20&&frame%80<30) draw_sprite(buffer, player_left3, x-cameraX, y-cameraY);
                     else if(frame%80>=30&&frame%80<40) draw_sprite(buffer, player_left4, x-cameraX, y-cameraY);
                     else if(frame%80>=40&&frame%80<50) draw_sprite(buffer, player_left5, x-cameraX, y-cameraY);
                     else if(frame%80>=50&&frame%80<60) draw_sprite(buffer, player_left6, x-cameraX, y-cameraY);
                     else if(frame%80>=60&&frame%80<70) draw_sprite(buffer, player_left7, x-cameraX, y-cameraY);
                     else if(frame%80>=70&&frame%80<80) draw_sprite(buffer, player_left8, x-cameraX, y-cameraY);
                     x--;
               }
               else if(enableCommand==0) //player standing still
               {
                     draw_sprite(buffer, player_left, x-cameraX, y-cameraY);
                     if(KeyPressed(KEY_D)||KeyStillPressed(KEY_D)) {CameraFollow(map); direction = RIGHT_DIR;}       //turn right
                     else if(KeyPressed(KEY_A)||KeyStillPressed(KEY_A)) {CameraFollow(map); if(getWhole(x/80)-1>=0&&map.Movable(getWhole(x/80)-1, getWhole(y/80))) x--;} //if possible move in left direction
                     else if(KeyPressed(KEY_S)||KeyStillPressed(KEY_S)) {CameraFollow(map); direction = DOWN_DIR;} // turn down
                     else if(KeyPressed(KEY_W)||KeyStillPressed(KEY_W)) {CameraFollow(map); direction = UP_DIR;} //turn up
               }
               else draw_sprite(buffer, player_left, x-cameraX, y-cameraY);
          } break;
          case DOWN_DIR: //heading down
          {
               if(y%80!=0)  //player already in move
               {
                     CameraFollow(map);
                     if(frame%80<10) draw_sprite(buffer, player_down1, x-cameraX, y-cameraY);
                     else if(frame%80>=10&&frame%80<20) draw_sprite(buffer, player_down2, x-cameraX, y-cameraY);
                     else if(frame%80>=20&&frame%80<30) draw_sprite(buffer, player_down3, x-cameraX, y-cameraY);
                     else if(frame%80>=30&&frame%80<40) draw_sprite(buffer, player_down4, x-cameraX, y-cameraY);
                     else if(frame%80>=40&&frame%80<50) draw_sprite(buffer, player_down5, x-cameraX, y-cameraY);
                     else if(frame%80>=50&&frame%80<60) draw_sprite(buffer, player_down6, x-cameraX, y-cameraY);
                     else if(frame%80>=60&&frame%80<70) draw_sprite(buffer, player_down7, x-cameraX, y-cameraY);
                     else if(frame%80>=70&&frame%80<80) draw_sprite(buffer, player_down8, x-cameraX, y-cameraY);
                     y++;
               }
               else if(enableCommand==0) //player standing still
               {
                     draw_sprite(buffer, player_down, x-cameraX, y-cameraY);
                     if(KeyPressed(KEY_D)||KeyStillPressed(KEY_D)) {CameraFollow(map); direction = RIGHT_DIR;}       //turn right
                     else if(KeyPressed(KEY_A)||KeyStillPressed(KEY_A)) {CameraFollow(map); direction = LEFT_DIR;} //turn left
                     else if(KeyPressed(KEY_S)||KeyStillPressed(KEY_S)) {CameraFollow(map); if(getWhole(y/80)+1<32&&map.Movable(getWhole(x/80), getWhole(y/80)+1)) y++;} //if possible move down
                     else if(KeyPressed(KEY_W)||KeyStillPressed(KEY_W)) {CameraFollow(map); direction = UP_DIR;} //turn up
               }
               else draw_sprite(buffer, player_down, x-cameraX, y-cameraY);
          } break;
          case UP_DIR: //heading up
          {
               if(y%80!=0)  //player already in move
               {
                     CameraFollow(map);
                     if(frame%80<10) draw_sprite(buffer, player_up1, x-cameraX, y-cameraY);
                     else if(frame%80>=10&&frame%80<20) draw_sprite(buffer, player_up2, x-cameraX, y-cameraY);
                     else if(frame%80>=20&&frame%80<30) draw_sprite(buffer, player_up3, x-cameraX, y-cameraY);
                     else if(frame%80>=30&&frame%80<40) draw_sprite(buffer, player_up4, x-cameraX, y-cameraY);
                     else if(frame%80>=40&&frame%80<50) draw_sprite(buffer, player_up5, x-cameraX, y-cameraY);
                     else if(frame%80>=50&&frame%80<60) draw_sprite(buffer, player_up6, x-cameraX, y-cameraY);
                     else if(frame%80>=60&&frame%80<70) draw_sprite(buffer, player_up7, x-cameraX, y-cameraY);
                     else if(frame%80>=70&&frame%80<80) draw_sprite(buffer, player_up8, x-cameraX, y-cameraY);
                     y--;
               }
               else if(enableCommand==0) //player standing still
               {
                     draw_sprite(buffer, player_up, x-cameraX, y-cameraY);
                     if(KeyPressed(KEY_D)||KeyStillPressed(KEY_D)) {CameraFollow(map); direction = RIGHT_DIR;}       //turn right
                     else if(KeyPressed(KEY_A)||KeyStillPressed(KEY_A)) {CameraFollow(map); direction = LEFT_DIR;} //turn left
                     else if(KeyPressed(KEY_S)||KeyStillPressed(KEY_S)) {CameraFollow(map); direction = DOWN_DIR;} //if possible move down
                     else if((KeyPressed(KEY_W)||KeyStillPressed(KEY_W))||KeyPressed(KEY_ENTER))
                     {
                          switch(map.MapFields[getWhole((y-1)/80)][getWhole(x/80)])
                          {
                              case 13: case 14: case 15: case 16:
                              {
                                   string lastPlace=Place;
                                   Place = town[map.MapFields[getWhole((y-1)/80)][getWhole(x/80)]-13].name;
                                   ThrowPtr();
                                   town[map.MapFields[getWhole((y-1)/80)][getWhole(x/80)]-13].TownMenu(attributes.Gold, attributes.Wood, attributes.Ore, attributes.Leadership, Army, PlayersItems);
                                   for(int i=0; i<PlayersItems.size(); i++)
                                   {
                                           if(PlayersItems[i].Equipped) PlayersWeapon=PlayersItems[i];
                                   }
                                   clear_keybuf();
                                   direction = 2;
                                   Place = lastPlace;
                              } break;
                              case 6: case 17: case 23:
                              {
                                   Visit(map.MapFields[getWhole((y-1)/80)][getWhole(x/80)]);
                                   clear_keybuf();
                                   direction = 2;
                              } break;
                              default:
                              {
                                  CameraFollow(map);
                                  if(getWhole(y/80)-1>=0&&map.Movable(getWhole(x/80), getWhole(y/80)-1)) y--;
                              } break;
                          }
                     }
               }
               else draw_sprite(buffer, player_up, x-cameraX, y-cameraY);
          } break;
     }
     for(int yt=0; yt<32; yt++) // animations on map
         for(int xt=0; xt<32; xt++) if(map.MapFields[yt][xt]==14)
     {
         if(frame%80<8) draw_sprite(buffer, waterfall0, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=8&&frame%80<16) draw_sprite(buffer, waterfall1, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=16&&frame%80<24) draw_sprite(buffer, waterfall2, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=24&&frame%80<32) draw_sprite(buffer, waterfall3, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=32&&frame%80<40) draw_sprite(buffer, waterfall4, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=40&&frame%80<48) draw_sprite(buffer, waterfall5, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=48&&frame%80<56) draw_sprite(buffer, waterfall6, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=56&&frame%80<64) draw_sprite(buffer, waterfall7, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=64&&frame%80<72) draw_sprite(buffer, waterfall8, xt*80-cameraX, (yt-1)*80-cameraY);
         else if(frame%80>=72&&frame%80<80) draw_sprite(buffer, waterfall9, xt*80-cameraX, (yt-1)*80-cameraY);
     }
}

void Player::Visit(int placeID)
{
     switch(placeID)
     {
           case 6: //Big interactive tree
           {
                while(true)
                {
                           prevKeyPress=keyPress;
                           clear(buffer);
                           draw_sprite(buffer, treescreen, 0, 0);
                           draw_sprite(buffer, background, 0, 0);
                           textout_centre_ex(buffer, font, "There's nothing", screenW-115, 100, WHITE, -1);
                           textout_centre_ex(buffer, font, "to do here right now.", screenW-115, 120, WHITE, -1);
                           draw_sprite(screen, buffer, 0, 0);
                           if(KeyReleased(KEY_ENTER)) break;
                           if(KeyReleased(KEY_ESC)) break;
                }
           } break;
           case 17: //House in the forest
           {
                while(true)
                {
                           prevKeyPress=keyPress;
                           clear(buffer);
                           draw_sprite(buffer, foresthousescreen, 0, 0);
                           draw_sprite(buffer, background, 0, 0);
                           textout_centre_ex(buffer, font, "There's nothing", screenW-115, 100, WHITE, -1);
                           textout_centre_ex(buffer, font, "to do here right now.", screenW-115, 120, WHITE, -1);
                           draw_sprite(screen, buffer, 0, 0);
                           if(KeyReleased(KEY_ENTER)) break;
                           if(KeyReleased(KEY_ESC)) break;
                }
           } break;
           case 23: //Cave
           {
                while(true)
                {
                           prevKeyPress=keyPress;
                           clear(buffer);
                           draw_sprite(buffer, cavescreen, 0, 0);
                           draw_sprite(buffer, background, 0, 0);
                           textout_centre_ex(buffer, font, "There's nothing", screenW-115, 100, WHITE, -1);
                           textout_centre_ex(buffer, font, "to do here right now.", screenW-115, 120, WHITE, -1);
                           draw_sprite(screen, buffer, 0, 0);
                           if(KeyReleased(KEY_ENTER)) break;
                           if(KeyReleased(KEY_ESC)) break;
                }
           } break;
     }
}

void Player::Camping()
{
     int WinGold = (rand()%3+1)*100;
     int WinWood, WinOre;
     int Enemy[6];
     int Exp=0;
     if(town[ARIADOR].Built(LUMBER_MILL)) WinWood=rand()%4+1;
     else WinWood=rand()%3;
     if(town[KHARAMDOOR].Built(QUARRIES)) WinOre=rand()%4+1;
     else WinOre=rand()%3;
     for(int i=0; i<6; i++)
     {
             stringstream ss;
             ss<<attributes.Level<<"d"<<getWhole(10/Units[i].Leadership);
             Enemy[i]=rand()%diceroll(ss.str());
             ss.str(string());
             ss.clear();
             Exp+=5*Enemy[i]*Units[i].Leadership;
     }
     bool battle=true;
     while(true)
     {
         clear(buffer);
         prevKeyPress = keyPress;
         draw_sprite(buffer, fight, 0, 0);
         draw_sprite(buffer, background, 0, 0);
         textprintf_centre_ex(buffer, font, screenW-115, 180, WHITE, -1, "You come upon:");
         for(int i=0; i<6; i++) textprintf_ex(buffer, font, screenW-200, 200+i*20, WHITE, -1, "%d %s", Enemy[i], Units[i].name.c_str());
         if(battle)
         {
               textprintf_ex(buffer, font, screenW-200, 400, RED, -1, ">>FIGHT<<");
               textprintf_ex(buffer, font, screenW-200, 450, WHITE, -1, "  FLEE");
         }
         else
         {
             textprintf_ex(buffer, font, screenW-200, 400, WHITE, -1, "  FIGHT");
             textprintf_ex(buffer, font, screenW-200, 450, RED, -1, ">>FLEE<<");
         }
         if(KeyReleased(KEY_ENTER)||KeyReleased(KEY_E)) break;
         if(KeyReleased(KEY_ESC)) {battle=false; break;}
         if(!battle&&(KeyReleased(KEY_UP)||KeyReleased(KEY_W))) battle=true;
         if(battle&&(KeyReleased(KEY_DOWN)||KeyReleased(KEY_S))) battle = false;
         draw_sprite(screen, buffer, 0, 0);
     }
     
     if(battle)
     {
         int prevArmy[UnitsTypeNumber];
         for(int i=0; i<UnitsTypeNumber; i++) prevArmy[i]=Army[i];
         if(Battle(Enemy))
         {
             while(true)
             {
                    clear(buffer);
                    prevKeyPress = keyPress;
                    if(KeyReleased(KEY_ENTER)||KeyReleased(KEY_ESC)) break;
                    draw_sprite(buffer, camping, 0, 0);
                    draw_sprite(buffer, background, 0, 0);
                    textprintf_centre_ex(buffer, font, screenW-115, 50, WHITE, -1, "In the battle you lost:");
                    for(int i=0; i<UnitsTypeNumber; i++) textprintf_centre_ex(buffer, font, screenW-115, 70+i*20, WHITE, -1, "%d %s", prevArmy[i]-Army[i], Units[i].name.c_str());
                    textprintf_centre_ex(buffer, font, screenW-115, 200, WHITE, -1, "After searching the camp");
                    textprintf_centre_ex(buffer, font, screenW-115, 220, WHITE, -1, "you find:");
                    textprintf_ex(buffer, font, screenW-200, 250, WHITE, -1, "%d", WinGold);
                    draw_sprite(buffer, gold, screenW-170, 245);
                    textprintf_ex(buffer, font, screenW-200, 270, WHITE, -1, "%d", WinWood);
                    draw_sprite(buffer, wood, screenW-180, 265);
                    textprintf_ex(buffer, font, screenW-200, 290, WHITE, -1, "%d", WinOre);
                    draw_sprite(buffer, ore, screenW-180, 285);
                    textprintf_centre_ex(buffer, font, screenW-115, 350, WHITE, -1, "You gained %d Exp points.", Exp);
                    draw_sprite(screen, buffer, 0, 0);
                    
             }
             attributes.Gold+=WinGold;
             attributes.Wood+=WinWood;
             attributes.Ore+=WinOre;
             attributes.Exp+=Exp;
                struct Camp temp;
                temp.x=getWhole(x/80);
                temp.y=getWhole(y/80);
                CampsCleared.push_back(temp);
             attributes.Leadership=10+attributes.Level*10;
             if(town[ELVENMORT].Built(LARGE_HUT))
                     {attributes.Leadership+=50;cout<<town[ELVENMORT].name<<"uku";}
             for(int i=0; i<UnitsTypeNumber; i++) 
                     attributes.Leadership-=Army[i]*Units[i].Leadership;
             if(attributes.Level<level(attributes.Exp))
                  while(attributes.Level<level(attributes.Exp)) LevelUp();//levelup
         }
         else Death(); //Player is dead
     }
     else
     {
         switch(direction)
         {
               case RIGHT_DIR: x-=80; break;
               case LEFT_DIR: x+=80; break;
               case DOWN_DIR: y-=80; break;
               case UP_DIR: y+=80; break;
         }
     }
     clear_keybuf();
}

bool Player::Battle(int Enemy[])
{
     int turn = 1;
     int initArmy[UnitsTypeNumber], initEnemy[UnitsTypeNumber];
     vector <class Unit> tempUnits;
     for (int i=0; i<UnitsTypeNumber; i++) 
     {
         tempUnits.push_back(Units[i]);
         tempUnits[i].Health=tempUnits[i].MaxHealth;
         initArmy[i]=Army[i];
         initEnemy[i]=Enemy[i];
         switch(i)
         {
                  case PIKEMEN: 
                  { 
                       if(town[ARIADOR].Built(FORGE)) Units[i].Health=Units[i].MaxHealthUpg;
                       else Units[i].Health=Units[i].MaxHealth;
                  } break;
                  case ARCHERS:
                  {
                       if(town[BELLASEE_FALLS].Built(MARKSMAN)) Units[i].Health=Units[i].MaxHealthUpg;
                       else Units[i].Health=Units[i].MaxHealth;
                  } break;
                  case SCOUTS:
                  {
                       if(town[ELVENMORT].Built(HUNTSMAN)) Units[i].Health=Units[i].MaxHealthUpg;
                       else Units[i].Health=Units[i].MaxHealth;
                  } break;
                  case MAGES:
                  {
                       if(town[ELVENMORT].Built(MAGE_TOWER)) Units[i].Health=Units[i].MaxHealthUpg;
                       else Units[i].Health=Units[i].MaxHealth;
                  } break;
                  case WARRIORS:
                  {
                       if(town[KHARAMDOOR].Built(FORGE)) Units[i].Health=Units[i].MaxHealthUpg;
                       else Units[i].Health=Units[i].MaxHealth;
                  } break;
                  case PRIESTS:
                  {
                       if(town[KHARAMDOOR].Built(CHAPEL)) Units[i].Health=Units[i].MaxHealthUpg;
                       else Units[i].Health=Units[i].MaxHealth;
                  } break;
         }
     }
     while(true)
     {
          if(attributes.Health<=0)
          {
               cout<<"You loose."<<endl;
               return false;
               break;
          }
          cout<<turn<<". turn starts:"<<endl;
          int EnemyNumber = 0;
          for (int i=0; i<UnitsTypeNumber; i++)
          {
              cout<<Units[i].name<<" "<<Enemy[i]<<endl;
              EnemyNumber+=Enemy[i];
          }
          if(EnemyNumber>0) //Player's turn
          {
              for(int i=0; i<UnitsTypeNumber; i++) //Hero's move
              {
                      if(Enemy[i]>0)
                      {
                          int HeroAtk = diceroll("1d20") + attributes.Attack;
                          int EnemyDef = diceroll("1d20") + tempUnits[i].Defence;
                          if(HeroAtk-attributes.Attack==20||(HeroAtk>=EnemyDef&&EnemyDef-tempUnits[i].Defence!=20)) //Hero hits
                          {
                               int DamageLeft = diceroll(PlayersWeapon.Damage);
                               cout<<name<<" do "<<DamageLeft<<" damage points."<<endl;
                               int AmountBefore = Enemy[i];
                               while(tempUnits[i].Health<DamageLeft&&Enemy[i]>0)
                               {
                                     DamageLeft -= tempUnits[i].Health;
                                     Enemy[i]--;
                                     tempUnits[i].Health=tempUnits[i].MaxHealth;
                               }
                               tempUnits[i].Health-=DamageLeft;
                               if(Enemy[i]==0) cout<<tempUnits[i].name<<" perish."<<endl;
                               else cout<<AmountBefore-Enemy[i]<<" "<<tempUnits[i].name<<" perish."<<endl;
                          }
                          else cout<<Units[i].name<<" dodge "<<name<<"'s attack."<<endl;
                      }
              }
              if(Army[SCOUTS]>0) //Player's Scouts turn
              {
                  int TargetID; //Targetting Enemy
                  if(Enemy[PRIESTS]>0) TargetID = PRIESTS;
                  else if(Enemy[MAGES]>0) TargetID = MAGES;
                  else if(Enemy[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Enemy[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Enemy[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Enemy[WARRIORS]>0) TargetID = WARRIORS;
                  
                  int UnitAtk;
                  int chance = diceroll("1d20");
                  if(town[ELVENMORT].Built(HUNTSMAN)) UnitAtk = chance + Units[SCOUTS].AttackUpg;
                  else UnitAtk = chance + Units[SCOUTS].Attack;
                  int EnemyDef = diceroll("1d20") + tempUnits[TargetID].Defence;
                  if(chance==20||(UnitAtk>=EnemyDef&&EnemyDef-tempUnits[TargetID].Defence!=20)) //Scouts hit
                  {
                       int DamageLeft = Army[SCOUTS]*diceroll(Units[SCOUTS].UnitsWeapon.Damage);
                       cout<<"Your Scouts do "<<DamageLeft<<" damage points."<<endl;
                       int AmountBefore = Enemy[TargetID];
                       while(tempUnits[TargetID].Health<DamageLeft&&Enemy[TargetID]>0)
                       {
                            DamageLeft -= tempUnits[TargetID].Health;
                            Enemy[TargetID]--;
                            tempUnits[TargetID].Health=tempUnits[TargetID].MaxHealth;
                       }
                       tempUnits[TargetID].Health-=DamageLeft;
                       if(Enemy[TargetID]==0) cout<<tempUnits[TargetID].name<<" perish."<<endl;
                       else cout<<AmountBefore-Enemy[TargetID]<<" "<<tempUnits[TargetID].name<<" perish."<<endl;
                  }
                  else cout<<Units[TargetID].name<<" dodge your Scouts attack."<<endl;
                  
                  if(town[ELVENMORT].Built(HUNTSMAN)) //Scouts do the Double-shot
                  {
                      chance = diceroll("1d20");
                      if(town[ELVENMORT].Built(HUNTSMAN)) UnitAtk = chance + Units[SCOUTS].AttackUpg;
                      else UnitAtk = chance + Units[SCOUTS].Attack;
                      int EnemyDef = diceroll("1d20") + tempUnits[TargetID].Defence;
                      if(chance==20||(UnitAtk>=EnemyDef&&EnemyDef-tempUnits[TargetID].Defence!=20)) //Scouts hit
                      {
                           int DamageLeft = Army[SCOUTS]*diceroll(Units[SCOUTS].UnitsWeapon.Damage);
                           cout<<"Your Scouts do "<<DamageLeft<<" damage points."<<endl;
                           int AmountBefore = Enemy[TargetID];
                           while(tempUnits[TargetID].Health<DamageLeft&&Enemy[TargetID]>0)
                           {
                                DamageLeft -= tempUnits[TargetID].Health;
                                Enemy[TargetID]--;
                                tempUnits[TargetID].Health=tempUnits[TargetID].MaxHealth;
                           }
                           tempUnits[TargetID].Health-=DamageLeft;
                           if(Enemy[TargetID]==0) cout<<tempUnits[TargetID].name<<" perish."<<endl;
                           else cout<<AmountBefore-Enemy[TargetID]<<" "<<tempUnits[TargetID].name<<" perish."<<endl;
                      }
                      else cout<<Units[TargetID].name<<" dodge your Scouts attack."<<endl;
                  }
              }
              if(Enemy[SCOUTS]>0) //Enemy Scouts turn
              {
                  int TargetID; //Targetting
                  if(Army[PRIESTS]>0) TargetID = PRIESTS;
                  else if(Army[MAGES]>0) TargetID = MAGES;
                  else if(Army[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Army[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Army[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Army[WARRIORS]>0) TargetID = WARRIORS;
                  else TargetID = -1;
                  
                  int UnitDef;
                  int chance = diceroll("1d20");
                  switch(TargetID)
                  {
                         case PIKEMEN:
                         {
                              if(town[ARIADOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case ARCHERS:
                         {
                              if(town[BELLASEE_FALLS].Built(MARKSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case SCOUTS:
                         {
                              if(town[ELVENMORT].Built(HUNTSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case MAGES:
                         {
                              if(town[ELVENMORT].Built(MAGE_TOWER)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case WARRIORS:
                         {
                              if(town[KHARAMDOOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case PRIESTS:
                         {
                              if(town[KHARAMDOOR].Built(CHAPEL)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case -1:
                         {
                              UnitDef = chance + attributes.Defence;

                         } break;
                  }
                  int EnemyAtk = diceroll("1d20") + tempUnits[SCOUTS].Attack;
                  if(EnemyAtk-tempUnits[SCOUTS].Attack==20||EnemyAtk>UnitDef) //Scouts hit
                  {
                       int DamageLeft = Enemy[SCOUTS]*diceroll(Units[SCOUTS].UnitsWeapon.Damage);
                       cout<<"Enemy Scouts do "<<DamageLeft<<" damage points."<<endl;
                       if(TargetID!=-1)
                       {
                           int AmountBefore = Army[TargetID];
                           while(Units[TargetID].Health<DamageLeft&&Army[TargetID]>0)
                           {
                                DamageLeft -= Units[TargetID].Health;
                                Army[TargetID]--;
                                switch(TargetID)
                                 {
                                          case PIKEMEN: 
                                          { 
                                               if(town[ARIADOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case ARCHERS:
                                          {
                                               if(town[BELLASEE_FALLS].Built(MARKSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case SCOUTS:
                                          {
                                               if(town[ELVENMORT].Built(HUNTSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case MAGES:
                                          {
                                               if(town[ELVENMORT].Built(MAGE_TOWER)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case WARRIORS:
                                          {
                                               if(town[KHARAMDOOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case PRIESTS:
                                          {
                                               if(town[KHARAMDOOR].Built(CHAPEL)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                 }
                           }
                           Units[TargetID].Health-=DamageLeft;
                           if(Army[TargetID]==0) cout<<"Your "<<Units[TargetID].name<<" perish."<<endl;
                           else cout<<AmountBefore-Army[TargetID]<<" of your "<<Units[TargetID].name<<" perish."<<endl;
                       }
                       else
                       {
                           if(attributes.Health>DamageLeft)
                                  attributes.Health-=DamageLeft;
                           else
                           {
                               cout<<"Your Hero is dead."<<endl;
                               attributes.Health=0;
                               return false;
                               break;
                           }
                       }
                  }
                  else
                  {
                      if(TargetID!=-1) cout<<"Your "<<Units[TargetID].name<<" dodge enemy Scouts attack."<<endl;
                      else cout<<name<<" dodges enemy Scouts attack."<<endl;
                  }
              }
              if(Army[ARCHERS]>0) //Player's Archers turn
              {
                  int TargetID; //Targetting Enemy
                  if(Enemy[PRIESTS]>0) TargetID = PRIESTS;
                  else if(Enemy[MAGES]>0) TargetID = MAGES;
                  else if(Enemy[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Enemy[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Enemy[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Enemy[WARRIORS]>0) TargetID = WARRIORS;
                  
                  int UnitAtk;
                  int chance = diceroll("1d20");
                  if(town[BELLASEE_FALLS].Built(MARKSMAN)) UnitAtk = chance + Units[ARCHERS].AttackUpg;
                  else UnitAtk = chance + Units[ARCHERS].Attack;
                  int EnemyDef = diceroll("1d20") + tempUnits[TargetID].Defence;
                  if(chance==20||(UnitAtk>=EnemyDef&&EnemyDef-tempUnits[TargetID].Defence!=20)) //Archers hit
                  {
                       int DamageLeft = Army[ARCHERS]*diceroll(Units[ARCHERS].UnitsWeapon.Damage);
                       cout<<"Your Archers do "<<DamageLeft<<" damage points."<<endl;
                       int AmountBefore = Enemy[TargetID];
                       while(tempUnits[TargetID].Health<DamageLeft&&Enemy[TargetID]>0)
                       {
                            DamageLeft -= tempUnits[TargetID].Health;
                            Enemy[TargetID]--;
                            tempUnits[TargetID].Health=tempUnits[TargetID].MaxHealth;
                       }
                       tempUnits[TargetID].Health-=DamageLeft;
                       if(Enemy[TargetID]==0) cout<<tempUnits[TargetID].name<<" perish."<<endl;
                       else cout<<AmountBefore-Enemy[TargetID]<<" "<<tempUnits[TargetID].name<<" perish."<<endl;
                  }
                  else cout<<Units[TargetID].name<<" dodge your Archers attack."<<endl;
              }
              if(Enemy[ARCHERS]>0) //Enemy Archers turn
              {
                  int TargetID; //Targetting
                  if(Army[PRIESTS]>0) TargetID = PRIESTS;
                  else if(Army[MAGES]>0) TargetID = MAGES;
                  else if(Army[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Army[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Army[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Army[WARRIORS]>0) TargetID = WARRIORS;
                  else TargetID = -1;
                  
                  int UnitDef;
                  int chance = diceroll("1d20");
                  switch(TargetID)
                  {
                         case PIKEMEN:
                         {
                              if(town[ARIADOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case ARCHERS:
                         {
                              if(town[BELLASEE_FALLS].Built(MARKSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case SCOUTS:
                         {
                              if(town[ELVENMORT].Built(HUNTSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case MAGES:
                         {
                              if(town[ELVENMORT].Built(MAGE_TOWER)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case WARRIORS:
                         {
                              if(town[KHARAMDOOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case PRIESTS:
                         {
                              if(town[KHARAMDOOR].Built(CHAPEL)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case -1:
                         {
                              UnitDef = chance + attributes.Defence;

                         } break;
                  }
                  int EnemyAtk = diceroll("1d20") + tempUnits[ARCHERS].Attack;
                  if(EnemyAtk-tempUnits[ARCHERS].Attack==20||EnemyAtk>UnitDef) //Archers hit
                  {
                       int DamageLeft = Enemy[ARCHERS]*diceroll(tempUnits[ARCHERS].UnitsWeapon.Damage);
                       cout<<"Enemy Archers do "<<DamageLeft<<" damage points."<<endl;
                       if(TargetID!=-1)
                       {
                           int AmountBefore = Army[TargetID];
                           while(Units[TargetID].Health<DamageLeft&&Army[TargetID]>0)
                           {
                                DamageLeft -= Units[TargetID].Health;
                                Army[TargetID]--;
                                switch(TargetID)
                                 {
                                          case PIKEMEN: 
                                          { 
                                               if(town[ARIADOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case ARCHERS:
                                          {
                                               if(town[BELLASEE_FALLS].Built(MARKSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case SCOUTS:
                                          {
                                               if(town[ELVENMORT].Built(HUNTSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case MAGES:
                                          {
                                               if(town[ELVENMORT].Built(MAGE_TOWER)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case WARRIORS:
                                          {
                                               if(town[KHARAMDOOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case PRIESTS:
                                          {
                                               if(town[KHARAMDOOR].Built(CHAPEL)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                 }
                           }
                           Units[TargetID].Health-=DamageLeft;
                           if(Army[TargetID]==0) cout<<"Your "<<Units[TargetID].name<<" perish."<<endl;
                           else cout<<AmountBefore-Army[TargetID]<<" of your "<<Units[TargetID].name<<" perish."<<endl;
                       }
                       else
                       {
                           if(attributes.Health>DamageLeft)
                                  attributes.Health-=DamageLeft;
                           else
                           {
                               cout<<"Your Hero is dead."<<endl;
                               attributes.Health=0;
                               return false;
                               break;
                           }
                       }
                  }
                  else
                  {
                      if(TargetID!=-1) cout<<"Your "<<Units[TargetID].name<<" dodge enemy Archers attack."<<endl;
                      else cout<<name<<" dodges enemy Archers attack."<<endl;
                  }
              }
              if(Army[PIKEMEN]>0) //Player's Pikemen turn
              {
                  int TargetID; //Targetting Enemy
                  if(Enemy[WARRIORS]>0) TargetID = WARRIORS;
                  else if(Enemy[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Enemy[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Enemy[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Enemy[MAGES]>0) TargetID = MAGES;
                  else if(Enemy[PRIESTS]>0) TargetID = PRIESTS;
                  
                  int UnitAtk;
                  int chance = diceroll("1d20");
                  if(town[ARIADOR].Built(FORGE)) UnitAtk = chance + Units[PIKEMEN].AttackUpg;
                  else UnitAtk = chance + Units[PIKEMEN].Attack;
                  int EnemyDef = diceroll("1d20") + tempUnits[TargetID].Defence;
                  if(chance==20||(UnitAtk>=EnemyDef&&EnemyDef-tempUnits[TargetID].Defence!=20)) //Pikemen hit
                  {
                       int DamageLeft = Army[PIKEMEN]*diceroll(Units[PIKEMEN].UnitsWeapon.Damage);
                       cout<<"Your Pikemen do "<<DamageLeft<<" damage points."<<endl;
                       int AmountBefore = Enemy[TargetID];
                       while(tempUnits[TargetID].Health<DamageLeft&&Enemy[TargetID]>0)
                       {
                            DamageLeft -= tempUnits[TargetID].Health;
                            Enemy[TargetID]--;
                            tempUnits[TargetID].Health=tempUnits[TargetID].MaxHealth;
                       }
                       tempUnits[TargetID].Health-=DamageLeft;
                       if(Enemy[TargetID]==0) cout<<tempUnits[TargetID].name<<" perish."<<endl;
                       else cout<<AmountBefore-Enemy[TargetID]<<" "<<tempUnits[TargetID].name<<" perish."<<endl;
                  }
                  else cout<<Units[TargetID].name<<" parry your Pikemen attack."<<endl;
              }
              if(Enemy[PIKEMEN]>0) //Enemy Pikemen turn
              {
                  int TargetID; //Targetting
                  if(Army[WARRIORS]>0) TargetID = WARRIORS;
                  else if(Army[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Army[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Army[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Army[MAGES]>0) TargetID = MAGES;
                  else if(Army[PRIESTS]>0) TargetID = PRIESTS;
                  else TargetID = -1;
                  
                  int UnitDef;
                  int chance = diceroll("1d20");
                  switch(TargetID)
                  {
                         case PIKEMEN:
                         {
                              if(town[ARIADOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case ARCHERS:
                         {
                              if(town[BELLASEE_FALLS].Built(MARKSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case SCOUTS:
                         {
                              if(town[ELVENMORT].Built(HUNTSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case MAGES:
                         {
                              if(town[ELVENMORT].Built(MAGE_TOWER)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case WARRIORS:
                         {
                              if(town[KHARAMDOOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case PRIESTS:
                         {
                              if(town[KHARAMDOOR].Built(CHAPEL)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case -1:
                         {
                              UnitDef = chance + attributes.Defence;

                         } break;
                  }
                  int EnemyAtk = diceroll("1d20") + tempUnits[PIKEMEN].Attack;
                  if(EnemyAtk-tempUnits[ARCHERS].Attack==20||EnemyAtk>UnitDef) //Pikemen hit
                  {
                       int DamageLeft = Enemy[PIKEMEN]*diceroll(tempUnits[PIKEMEN].UnitsWeapon.Damage);
                       cout<<"Enemy Pikemen do "<<DamageLeft<<" damage points."<<endl;
                       if(TargetID!=-1)
                       {
                           int AmountBefore = Army[TargetID];
                           while(Units[TargetID].Health<DamageLeft&&Army[TargetID]>0)
                           {
                                DamageLeft -= Units[TargetID].Health;
                                Army[TargetID]--;
                                switch(TargetID)
                                 {
                                          case PIKEMEN: 
                                          { 
                                               if(town[ARIADOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case ARCHERS:
                                          {
                                               if(town[BELLASEE_FALLS].Built(MARKSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case SCOUTS:
                                          {
                                               if(town[ELVENMORT].Built(HUNTSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case MAGES:
                                          {
                                               if(town[ELVENMORT].Built(MAGE_TOWER)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case WARRIORS:
                                          {
                                               if(town[KHARAMDOOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case PRIESTS:
                                          {
                                               if(town[KHARAMDOOR].Built(CHAPEL)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                 }
                           }
                           Units[TargetID].Health-=DamageLeft;
                           if(Army[TargetID]==0) cout<<"Your "<<Units[TargetID].name<<" perish."<<endl;
                           else cout<<AmountBefore-Army[TargetID]<<" of your "<<Units[TargetID].name<<" perish."<<endl;
                       }
                       else
                       {
                           if(attributes.Health>DamageLeft)
                                  attributes.Health-=DamageLeft;
                           else
                           {
                               cout<<"Your Hero is dead."<<endl;
                               attributes.Health=0;
                               return false;
                               break;
                           }
                       }
                  }
                  else
                  {
                      if(TargetID!=-1) cout<<"Your "<<Units[TargetID].name<<" dodge enemy Pikemen attack."<<endl;
                      else cout<<name<<" dodges enemy Pikemen attack."<<endl;
                  }
              }
              if(Army[WARRIORS]>0) //Player's Warriors turn
              {
                  int TargetID; //Targetting Enemy
                  if(Enemy[WARRIORS]>0) TargetID = WARRIORS;
                  else if(Enemy[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Enemy[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Enemy[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Enemy[MAGES]>0) TargetID = MAGES;
                  else if(Enemy[PRIESTS]>0) TargetID = PRIESTS;
                  
                  int UnitAtk;
                  int chance = diceroll("1d20");
                  if(town[KHARAMDOOR].Built(FORGE)) UnitAtk = chance + Units[WARRIORS].AttackUpg;
                  else UnitAtk = chance + Units[WARRIORS].Attack;
                  int EnemyDef = diceroll("1d20") + tempUnits[TargetID].Defence;
                  if(chance==20||(UnitAtk>=EnemyDef&&EnemyDef-tempUnits[TargetID].Defence!=20)) //Warriors hit
                  {
                       int DamageLeft = Army[WARRIORS]*diceroll(Units[PIKEMEN].UnitsWeapon.Damage);
                       cout<<"Your Warriors do "<<DamageLeft<<" damage points."<<endl;
                       int AmountBefore = Enemy[TargetID];
                       while(tempUnits[TargetID].Health<DamageLeft&&Enemy[TargetID]>0)
                       {
                            DamageLeft -= tempUnits[TargetID].Health;
                            Enemy[TargetID]--;
                            tempUnits[TargetID].Health=tempUnits[TargetID].MaxHealth;
                       }
                       tempUnits[TargetID].Health-=DamageLeft;
                       if(Enemy[TargetID]==0) cout<<tempUnits[TargetID].name<<" perish."<<endl;
                       else cout<<AmountBefore-Enemy[TargetID]<<" "<<tempUnits[TargetID].name<<" perish."<<endl;
                  }
                  else cout<<Units[TargetID].name<<" parry your Warriors attack."<<endl;
              }
              if(Enemy[WARRIORS]>0) //Enemy Warriors turn
              {
                  int TargetID; //Targetting
                  if(Army[WARRIORS]>0) TargetID = WARRIORS;
                  else if(Army[PIKEMEN]>0) TargetID = PIKEMEN;
                  else if(Army[SCOUTS]>0) TargetID = SCOUTS;
                  else if(Army[ARCHERS]>0) TargetID = ARCHERS;
                  else if(Army[MAGES]>0) TargetID = MAGES;
                  else if(Army[PRIESTS]>0) TargetID = PRIESTS;
                  else TargetID = -1;
                  
                  int UnitDef;
                  int chance = diceroll("1d20");
                  switch(TargetID)
                  {
                         case PIKEMEN:
                         {
                              if(town[ARIADOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case ARCHERS:
                         {
                              if(town[BELLASEE_FALLS].Built(MARKSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case SCOUTS:
                         {
                              if(town[ELVENMORT].Built(HUNTSMAN)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case MAGES:
                         {
                              if(town[ELVENMORT].Built(MAGE_TOWER)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case WARRIORS:
                         {
                              if(town[KHARAMDOOR].Built(FORGE)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case PRIESTS:
                         {
                              if(town[KHARAMDOOR].Built(CHAPEL)) UnitDef = chance + Units[TargetID].DefenceUpg;
                              else UnitDef = chance + Units[TargetID].Defence;
                         } break;
                         case -1:
                         {
                              UnitDef = chance + attributes.Defence;

                         } break;
                  }
                  int EnemyAtk = diceroll("1d20") + tempUnits[WARRIORS].Attack;
                  if(EnemyAtk-tempUnits[WARRIORS].Attack==20||EnemyAtk>UnitDef) //Warriors hit
                  {
                       int DamageLeft = Enemy[WARRIORS]*diceroll(tempUnits[WARRIORS].UnitsWeapon.Damage);
                       cout<<"Enemy Warriors do "<<DamageLeft<<" damage points."<<endl;
                       if(TargetID!=-1)
                       {
                           int AmountBefore = Army[TargetID];
                           while(Units[TargetID].Health<DamageLeft&&Army[TargetID]>0)
                           {
                                DamageLeft -= Units[TargetID].Health;
                                Army[TargetID]--;
                                switch(TargetID)
                                 {
                                          case PIKEMEN: 
                                          { 
                                               if(town[ARIADOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case ARCHERS:
                                          {
                                               if(town[BELLASEE_FALLS].Built(MARKSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case SCOUTS:
                                          {
                                               if(town[ELVENMORT].Built(HUNTSMAN)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case MAGES:
                                          {
                                               if(town[ELVENMORT].Built(MAGE_TOWER)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case WARRIORS:
                                          {
                                               if(town[KHARAMDOOR].Built(FORGE)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                          case PRIESTS:
                                          {
                                               if(town[KHARAMDOOR].Built(CHAPEL)) Units[TargetID].Health=Units[TargetID].MaxHealthUpg;
                                               else Units[TargetID].Health=Units[TargetID].MaxHealth;
                                          } break;
                                 }
                           }
                           Units[TargetID].Health-=DamageLeft;
                           if(Army[TargetID]==0) cout<<"Your "<<Units[TargetID].name<<" perish."<<endl;
                           else cout<<AmountBefore-Army[TargetID]<<" of your "<<Units[TargetID].name<<" perish."<<endl;
                       }
                       else
                       {
                           if(attributes.Health>DamageLeft)
                                  attributes.Health-=DamageLeft;
                           else
                           {
                               cout<<"Your Hero is dead."<<endl;
                               attributes.Health=0;
                               return false;
                               break;
                           }
                       }
                  }
                  else
                  {
                      if(TargetID!=-1) cout<<"Your "<<Units[TargetID].name<<" parry enemy Warriors attack."<<endl;
                      else cout<<name<<" parries enemy Warriors attack."<<endl;
                  }
              }
              if(Army[MAGES]>0) //Player's Mages turn
              {
                  stringstream magic;
                  magic<<attributes.Level<<"d4";
                  if(!town[ELVENMORT].Built(MAGE_TOWER))
                  {
                      int TargetID; //Targetting Enemy
                      if(Enemy[MAGES]>0) TargetID = MAGES;
                      else if(Enemy[SCOUTS]>0) TargetID = SCOUTS;
                      else if(Enemy[ARCHERS]>0) TargetID = ARCHERS;
                      else if(Enemy[PIKEMEN]>0) TargetID = PIKEMEN;
                      else if(Enemy[WARRIORS]>0) TargetID = WARRIORS;
                      
                           int DamageLeft = Army[MAGES]*diceroll(magic.str());
                           cout<<"Your Mages do "<<DamageLeft<<" damage points."<<endl;
                           int AmountBefore = Enemy[TargetID];
                           while(tempUnits[TargetID].Health<DamageLeft&&Enemy[TargetID]>0)
                           {
                                DamageLeft -= tempUnits[TargetID].Health;
                                Enemy[TargetID]--;
                                tempUnits[TargetID].Health=tempUnits[TargetID].MaxHealth;
                           }
                           tempUnits[TargetID].Health-=DamageLeft;
                           if(Enemy[TargetID]==0) cout<<tempUnits[TargetID].name<<" perish."<<endl;
                           else cout<<AmountBefore-Enemy[TargetID]<<" "<<tempUnits[TargetID].name<<" perish."<<endl;
                  }
                  else
                  {
                      for(int i=0; i<UnitsTypeNumber; i++)
                      {
                              if(Enemy[i]>0&&i!=PRIESTS)
                              {
                                           int DamageLeft = Army[MAGES]*diceroll(magic.str());
                                           cout<<"Your Mages do "<<DamageLeft<<" damage points."<<endl;
                                           int AmountBefore = Enemy[i];
                                           while(tempUnits[i].Health<DamageLeft&&Enemy[i]>0)
                                           {
                                                DamageLeft -= tempUnits[i].Health;
                                                Enemy[i]--;
                                                tempUnits[i].Health=tempUnits[i].MaxHealth;
                                           }
                                           tempUnits[i].Health-=DamageLeft;
                                           if(Enemy[i]==0) cout<<tempUnits[i].name<<" perish."<<endl;
                                           else cout<<AmountBefore-Enemy[i]<<" "<<tempUnits[i].name<<" perish."<<endl;
                              }
                      }
                  }
                  magic.clear();
              }
              if(Enemy[MAGES]>0) //Enemy Mages turn
              {
                  stringstream magic;
                  magic<<attributes.Level<<"d4";
                  for(int i=0; i<UnitsTypeNumber; i++)
                  {
                              if(Army[i]>0&&i!=PRIESTS)
                              {
                                           int DamageLeft = Enemy[MAGES]*diceroll(magic.str());
                                           cout<<"Enemy Mages do "<<DamageLeft<<" damage points."<<endl;
                                           int AmountBefore = Army[i];
                                           while(Units[i].Health<DamageLeft&&Army[i]>0)
                                           {
                                                DamageLeft -= Units[i].Health;
                                                Army[i]--;
                                                switch(i)
                                                 {
                                                          case PIKEMEN: 
                                                          { 
                                                               if(town[ARIADOR].Built(FORGE)) Units[i].Health=Units[i].MaxHealthUpg;
                                                               else Units[i].Health=Units[i].MaxHealth;
                                                          } break;
                                                          case ARCHERS:
                                                          {
                                                               if(town[BELLASEE_FALLS].Built(MARKSMAN)) Units[i].Health=Units[i].MaxHealthUpg;
                                                               else Units[i].Health=Units[i].MaxHealth;
                                                          } break;
                                                          case SCOUTS:
                                                          {
                                                               if(town[ELVENMORT].Built(HUNTSMAN)) Units[i].Health=Units[i].MaxHealthUpg;
                                                               else Units[i].Health=Units[i].MaxHealth;
                                                          } break;
                                                          case MAGES:
                                                          {
                                                               if(town[ELVENMORT].Built(MAGE_TOWER)) Units[i].Health=Units[i].MaxHealthUpg;
                                                               else Units[i].Health=Units[i].MaxHealth;
                                                          } break;
                                                          case WARRIORS:
                                                          {
                                                               if(town[KHARAMDOOR].Built(FORGE)) Units[i].Health=Units[i].MaxHealthUpg;
                                                               else Units[i].Health=Units[i].MaxHealth;
                                                          } break;
                                                          case PRIESTS:
                                                          {
                                                               if(town[KHARAMDOOR].Built(CHAPEL)) Units[i].Health=Units[i].MaxHealthUpg;
                                                               else Units[i].Health=Units[i].MaxHealth;
                                                          } break;
                                                 }
                                           }
                                           Units[i].Health-=DamageLeft;
                                           if(Army[i]==0) cout<<"Your "<<Units[i].name<<" perish."<<endl;
                                           else cout<<AmountBefore-Army[i]<<" of your "<<Units[i].name<<" perish."<<endl;
                              }
                  }
                  magic.clear();
              }
          if (Army[PRIESTS]>0) //Player's Priests turn
          {
              if(!town[KHARAMDOOR].Built(CHAPEL))
              {
                   int TargetID; //Targetting Unit to heal
                   if(Army[MAGES]<initArmy[MAGES]) TargetID = MAGES;
                   else if(Army[SCOUTS]<initArmy[SCOUTS]) TargetID = SCOUTS;
                   else if(Army[ARCHERS]<initArmy[ARCHERS]) TargetID = ARCHERS;
                   else if(Army[PIKEMEN]<initArmy[PIKEMEN]) TargetID = PIKEMEN;
                   else if(Army[WARRIORS]<initArmy[WARRIORS]) TargetID = WARRIORS;
                   
                   int DamageHeal = Army[PRIESTS]*diceroll("2d4");
                   cout<<"Your Priests heal "<<Units[TargetID].name<<" for "<<DamageHeal<<" damage points."<<endl;
                   int TargetMaxHealth, prevUnitNumber=Army[TargetID];
                   switch(TargetID)
                   {
                          case PIKEMEN: 
                          { 
                                   if(town[ARIADOR].Built(FORGE)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                   else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case ARCHERS:
                          {
                                   if(town[BELLASEE_FALLS].Built(MARKSMAN)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                   else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case SCOUTS:
                          {
                                   if(town[ELVENMORT].Built(HUNTSMAN)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                   else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case MAGES:
                          {
                                    if(town[ELVENMORT].Built(MAGE_TOWER)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                    else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case WARRIORS:
                          {
                                    if(town[KHARAMDOOR].Built(FORGE)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                    else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case PRIESTS:
                          {
                                    if(town[KHARAMDOOR].Built(CHAPEL)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                    else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                   }
                   while(DamageHeal>0&&Army[TargetID]<initArmy[TargetID])
                   {
                       DamageHeal-=TargetMaxHealth-Units[TargetID].Health;
                       Army[TargetID]++;
                       Units[TargetID].Health=1;
                   }
                   Units[TargetID].Health+=DamageHeal;
                   if(Army[TargetID]>prevUnitNumber) cout<<Army[TargetID]-prevUnitNumber<<" "<<Units[TargetID].name<<" are resurrected."<<endl;
              }
              else //Massive heal
              {
                for(int TargetID=0; TargetID<UnitsTypeNumber; TargetID++)
                {
                  int DamageHeal = Army[PRIESTS]*diceroll("1d4");
                   cout<<"Your Priests heal "<<Units[TargetID].name<<" for "<<DamageHeal<<" damage points."<<endl;
                   int TargetMaxHealth, prevUnitNumber=Army[TargetID];
                   switch(TargetID)
                   {
                          case PIKEMEN: 
                          { 
                                   if(town[ARIADOR].Built(FORGE)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                   else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case ARCHERS:
                          {
                                   if(town[BELLASEE_FALLS].Built(MARKSMAN)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                   else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case SCOUTS:
                          {
                                   if(town[ELVENMORT].Built(HUNTSMAN)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                   else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case MAGES:
                          {
                                    if(town[ELVENMORT].Built(MAGE_TOWER)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                    else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case WARRIORS:
                          {
                                    if(town[KHARAMDOOR].Built(FORGE)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                    else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                          case PRIESTS:
                          {
                                    if(town[KHARAMDOOR].Built(CHAPEL)) TargetMaxHealth=Units[TargetID].MaxHealthUpg;
                                    else TargetMaxHealth=Units[TargetID].MaxHealth;
                          } break;
                   }
                   while(DamageHeal>0&&Army[TargetID]<initArmy[TargetID])
                   {
                       DamageHeal-=TargetMaxHealth-Units[TargetID].Health;
                       Army[TargetID]++;
                       Units[TargetID].Health=1;
                   }
                   Units[TargetID].Health+=DamageHeal;
                   if(Army[TargetID]>prevUnitNumber) cout<<Army[TargetID]-prevUnitNumber<<" "<<Units[TargetID].name<<" are resurrected."<<endl;
              }
            }
          }
          if (Enemy[PRIESTS]>0) //Enemy Priests turn
          {
                for(int TargetID=0; TargetID<UnitsTypeNumber; TargetID++)
                {
                  int DamageHeal = Enemy[PRIESTS]*diceroll("1d4");
                   cout<<"Enemy Priests heal "<<tempUnits[TargetID].name<<" for "<<DamageHeal<<" damage points."<<endl;
                   int TargetMaxHealth = tempUnits[TargetID].MaxHealth;
                   int prevUnitNumber = Enemy[TargetID];
                   
                   while(DamageHeal>0&&Enemy[TargetID]<initEnemy[TargetID])
                   {
                       DamageHeal-=TargetMaxHealth-tempUnits[TargetID].Health;
                       Enemy[TargetID]++;
                       tempUnits[TargetID].Health=1;
                   }
                   tempUnits[TargetID].Health+=DamageHeal;
                   if(Enemy[TargetID]>prevUnitNumber) cout<<Enemy[TargetID]-prevUnitNumber<<" "<<tempUnits[TargetID].name<<" are resurrected."<<endl;
              }
          }
          }
          else
          {
              cout<<"The enemy is defeated!"<<endl;
              return true;
              break;
          }
          turn++;
     }
}

void Player::LevelUp()
{
     attributes.Leadership+=10;
     attributes.Level++;
     attributes.MaxHealth+=10;
     attributes.Health=attributes.MaxHealth;
     int option = 0;
     string choice[] = {"Attack", "Defence", "Diplomacy"};
     while(true)
     {
                prevKeyPress = keyPress;
                clear(buffer);
                draw_sprite(buffer, Level, 0, 0);
                textprintf_centre_ex(buffer, MenuFont, screenW/2, 100, BLACK, -1, "You have raised up to %d Level!", attributes.Level);
                for(int i=0; i<3; i++)
                {
                        if(i==option) textprintf_centre_ex(buffer, MenuFont, screenW/2, (i+1)*screenH/4, RED, -1, choice[i].c_str());
                        else textprintf_centre_ex(buffer, MenuFont, screenW/2, (i+1)*screenH/4, WHITE, -1, choice[i].c_str());
                }
                if(KeyReleased(KEY_ENTER)) break;
                if(KeyReleased(KEY_UP))
                {
                     if(option>0) option--;
                }
                if(KeyReleased(KEY_DOWN))
                {
                    if(option<2) option++;
                }
                draw_sprite(screen, buffer, 0, 0);
     }
     switch(option)
     {
           case 0: attributes.Attack++; break;
           case 1: attributes.Defence++; break;
           case 2: attributes.Diplomacy++; break;
     }
}

void Player::Death()
{
     Dead=true;
     while(true)
     {
                prevKeyPress = keyPress;
                clear(buffer);
                draw_sprite(buffer, death, 0, 0);
                textprintf_centre_ex(buffer, MenuFont, screenW/2, screenH/2, RED, -1, "You loose!");
                if(KeyReleased(KEY_ENTER)||KeyReleased(KEY_ESC)) break;
                draw_sprite(screen, buffer, 0, 0);
     }
}

void Player::WinGame()
{
     while(true)
     {
                prevKeyPress = keyPress;
                clear(buffer);
                draw_sprite(buffer, wingame, 0, 15);
                textprintf_centre_ex(buffer, MenuFont, screenW/2, screenH/2, RED, -1, "YOU WIN!");
                if(KeyReleased(KEY_ENTER)||KeyReleased(KEY_ESC)) break;
                draw_sprite(screen, buffer, 0, 0);
     }
     clear_keybuf();
}

bool Map::LoadMap(int MapID, class Player &player)
{
    ID=MapID;
    ifstream mapfile;
    string line;
    vector<string> v;
    
    stringstream path;
    path<<"data\\maps\\map_"<<MapID<<".txt";
    cout<<"Opening "<<path.str()<<"...";
    mapfile.open(path.str().c_str(), ios::in);
    
    if(mapfile)
    {
               cout<<"Successful"<<endl<<"Reading data...";
               int i=0;
               while ( mapfile.good() )
               {
                     getline (mapfile,line);
                     split(line, ',', v);
                     for (int k = 0; k < v.size( ); ++k) 
                     {
                         istringstream iss(v[k]);
                         iss >> MapFields[i][k];
                         if(MapFields[i][k]==21)
                         {
                             CampsNumber++;
                             for(int j=0; j<player.CampsCleared.size(); j++)
                                  if(player.CampsCleared[j].y==i&&player.CampsCleared[j].x==k)
                                        if(rand()%2==0) MapFields[i][k]=1;
                                        else MapFields[i][k]=2;
                                       
                         }
                         else if(MapFields[i][k]==22)
                         {
                             CampsNumber++;
                             for(int j=0; j<player.CampsCleared.size(); j++)
                                  if(player.CampsCleared[j].y==i&&player.CampsCleared[j].x==k)
                                        MapFields[i][k]=3;
                                       
                         }
                     }
                     v.clear();
                     i++;
               }
               mapfile.close();
               cout<<"Done"<<endl;
               DrawMap();
               return true;
    }
    else return false;
}

void Map::DrawMap()
{
     cout<<"Drawing map fields to a map bitmap...";
     MapBMP = create_bitmap(MapWidth, MapHeight);
     for(int y=0; y<32; y++)
             for(int x=0; x<32; x++)
             {
                     switch(MapFields[y][x])
                     {
                         case 0: draw_sprite(MapBMP, no_tile, x*80, y*80); break; //no tile
                         case 1: draw_sprite(MapBMP, grass, x*80, y*80); break;   // grass
                         case 2: draw_sprite(MapBMP, dirt, x*80, y*80); break;     //dirt
                         case 3: draw_sprite(MapBMP, cobblestone, x*80, y*80); break; //cobblestone route
                         case 4: break; //water (animation)
                         case 5: {draw_sprite(MapBMP, grass, x*80, y*80); draw_sprite(MapBMP, trees, x*80, y*80);} break;    //trees
                         case 6: {draw_sprite(MapBMP, grass, x*80, y*80); draw_sprite(MapBMP, tree2, x*80, y*80);} break;    //interactive tree
                         case 7: {draw_sprite(MapBMP, grass, x*80, y*80); draw_sprite(MapBMP, tree_big, x*80, y*80);} break;    //dry tree
                         case 8: {draw_sprite(MapBMP, dirt, x*80, y*80); draw_sprite(MapBMP, mount, x*80, y*80);} break; //mountain
                         case 11: {draw_sprite(MapBMP, dirt, x*80, y*80); draw_sprite(MapBMP, house, x*80, y*80);} break; // house
                         case 13: {draw_sprite(MapBMP, dirt, x*80, y*80); draw_sprite(MapBMP, castle2, x*80, y*80);} break; // castle (Human town)
                         case 14: draw_sprite(MapBMP, dirt, x*80, y*80); break; // waterfall (Human town)
                         case 15: {draw_sprite(MapBMP, dirt, x*80, y*80); draw_sprite(MapBMP, house_stone, x*80, y*80);} break; // Dwarven town
                         case 16: {draw_sprite(MapBMP, grass, x*80, y*80); draw_sprite(MapBMP, house_forest, x*80, y*80);} break; // Elven tonw
                         case 17: {draw_sprite(MapBMP, grass, x*80, y*80); draw_sprite(MapBMP, house_forest2, x*80, y*80);} break; // house in forest
                         case 21: {if(rand()%2==0) draw_sprite(MapBMP, grass, x*80, y*80); else draw_sprite(MapBMP, dirt, x*80, y*80);} break; // camp (animation)
                         case 22: draw_sprite(MapBMP, cobblestone, x*80, y*80); break; //camp on cobblestone (animation)
                         case 23: {draw_sprite(MapBMP, dirt, x*80, y*80); draw_sprite(MapBMP, cave, x*80, y*80);} break; //cave
                         default: draw_sprite(MapBMP, no_tile, x*80, y*80); break;//no tile
                     }
             }
     cout<<"Successful"<<endl;
}

void Map::OnScreen(class Player &player)
{
     if(enableCommand!=1)
     {
         if(key[KEY_LEFT]) cameraX-=10;
         if(key[KEY_RIGHT]) cameraX+=10;
         if(key[KEY_UP]) cameraY-=10;
         if(key[KEY_DOWN]) cameraY+=10;
     }
     
     if (cameraX < 0) cameraX = 0;
     if (cameraY < 0) cameraY = 0;
     if (cameraX > MapWidth - screenWidth) cameraX = MapWidth - screenWidth;
     if (cameraY > MapHeight - screenHeight) cameraY = MapHeight - screenHeight;
     
     blit(MapBMP, buffer, cameraX, cameraY, 0, 0, screenWidth, screenHeight);
     
     for(int y=0; y<32; y++) // animations on map
         for(int x=0; x<32; x++)
             {
                 switch(MapFields[y][x])
                 {
                         case 4: // water
                         {
                             if(frame%80<20) draw_sprite(buffer, water1, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=20&&frame%80<40) draw_sprite(buffer, water2, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=40&&frame%80<60) draw_sprite(buffer, water3, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=60&&frame%80<80) draw_sprite(buffer, water4, x*80-cameraX, y*80-cameraY);
                             
                             if(x-1>=0&&((MapFields[y][x-1]==1||MapFields[y][x-1]==2)||MapFields[y][x-1]==5))//left waterbank
                                      draw_sprite(buffer, waterbank_left, x*80-cameraX, y*80-cameraY);
                             if(x+1<32&&((MapFields[y][x+1]==1||MapFields[y][x+1]==2)||MapFields[y][x+1]==5))//right waterbank
                                      draw_sprite(buffer, waterbank_right, x*80-cameraX, y*80-cameraY);
                             if(y-1>=0&&((MapFields[y-1][x]==1||MapFields[y-1][x]==2)||MapFields[y-1][x]==5))  //it's not the end of map
                             {
                                       draw_sprite(buffer, waterbank_up, x*80-cameraX, y*80-cameraY); //upper waterbank
                                       if(x-1>=0&&((MapFields[y][x-1]==1||MapFields[y][x-1]==2)||MapFields[y][x-1]==5)) //upper-left waterbank
                                              draw_sprite(buffer, waterbank_ulcorner, x*80-cameraX, y*80-cameraY);
                                       if(x+1<32&&((MapFields[y][x+1]==1||MapFields[y][x+1]==2)||MapFields[y][x+1]==5)) //upper-right waterbank
                                              draw_sprite(buffer, waterbank_urcorner, x*80-cameraX, y*80-cameraY);
                                       
                             }
                             if(y+1<32&&((MapFields[y+1][x]==1||MapFields[y+1][x]==2)||MapFields[y+1][x]==5)) //not the ned of map
                             {
                                       draw_sprite(buffer, waterbank_down, x*80-cameraX, y*80-cameraY); //down waterbank
                                       if(x-1>=0&&((MapFields[y][x-1]==1||MapFields[y][x-1]==2)||MapFields[y][x-1]==5)) //down-left waterbank
                                              draw_sprite(buffer, waterbank_dlcorner, x*80-cameraX, y*80-cameraY);
                                       if(x+1<32&&((MapFields[y][x+1]==1||MapFields[y][x+1]==2)==1||MapFields[y][x+1]==5)) //down-right waterbank
                                              draw_sprite(buffer, waterbank_drcorner, x*80-cameraX, y*80-cameraY);
                             }
                         } break;
                         case 21: case 22: // camp
                         {
                             if(frame%80<10) draw_sprite(buffer, camp1, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=10&&frame%80<20) draw_sprite(buffer, camp2, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=20&&frame%80<30) draw_sprite(buffer, camp3, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=30&&frame%80<40) draw_sprite(buffer, camp4, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=40&&frame%80<50) draw_sprite(buffer, camp5, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=50&&frame%80<60) draw_sprite(buffer, camp6, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=60&&frame%80<70) draw_sprite(buffer, camp7, x*80-cameraX, y*80-cameraY);
                             else if(frame%80>=70&&frame%80<80) draw_sprite(buffer, camp8, x*80-cameraX, y*80-cameraY);
                         } break;
                 }
             }

}

bool Map::Movable(int x, int y)
{
     switch(MapFields[y][x])
     {
          case 0: return false; break;
          case 1: return true; break;
          case 2: return true; break;
          case 3: return true; break;
          //case 13: return true; break;
          case 21: case 22: return true; break;
          default: return false;
     }
}

////////////////////////////////////// MAIN /////////////////////////////////////////////

int main() 
{
    cout<<"Initializing program..."<<endl;
    init();
	loader = LoadBMP("data\\graphics\\alphacoders_113714.bmp", NULL);
	draw_sprite(screen, loader, 0, 0);
    InitFMOD();
    MusicList MenuMusic(string("data\\music\\Thomas_Bergersen-Rada.mp3"), 'm');
    MenuMusic.PlayStream(-1);
	srand(time(NULL));
	buffer = create_bitmap(screenW,screenH);
	MenuBackground = LoadBMP("data\\graphics\\alphacoders_108850.bmp", NULL);
	help = LoadBMP("data\\graphics\\alphacoders_194533.bmp", NULL);
	help1 = load_bitmap("data\\graphics\\help1.bmp", NULL);
	help2 = load_bitmap("data\\graphics\\help2.bmp", NULL);
    font1 = load_font("data\\fonts\\ComicSansMS.pcx", NULL, NULL);
    MenuFont = load_font("data\\fonts\\ScriptS_IV50.pcx", NULL, NULL);
    
	bool done = false, exit = false;
	int option = 1;
	vector <string> MainMenu;
    MainMenu.push_back("1. Load player character");
    MainMenu.push_back("2. Create new player character");
    MainMenu.push_back("3. Help");
    MainMenu.push_back("4. Exit");
    cout<<"Done"<<endl;
    
	while(!exit)
	{
	   while(!done)
	   {
           clear(buffer);
           draw_sprite(buffer, MenuBackground, 0, 0);
           prevKeyPress = keyPress;
           MenuMusic.PlayStream(0);
           
           for (int i=0; i<MainMenu.size(); i++)
           {
               if(i==option-1) textout_centre_ex(buffer, MenuFont, MainMenu[i].c_str(), screenW/2, (i+1)*screenH/(MainMenu.size()+1), RED, -1);
               else textout_centre_ex(buffer, MenuFont, MainMenu[i].c_str(), screenW/2, (i+1)*screenH/(MainMenu.size()+1), WHITE, -1);
           }
           
           if (KeyReleased(KEY_ENTER)||KeyReleased(KEY_E)) done = true;
           if (KeyReleased(KEY_ESC)) {option = 4; break;}
           if (KeyReleased(KEY_UP)||KeyReleased(KEY_W))
           {
                if (option>1) option--;
                else option=MainMenu.size();
           }
           if (KeyReleased(KEY_DOWN)||KeyReleased(KEY_S))
           {
                if (option<MainMenu.size()) option++;
                else option=1;
           }
           if(KeyReleased(KEY_1))
           {
               option=1;
               done=true;
           }
           if(KeyReleased(KEY_2))
           {
               option=2;
               done=true;
           }
           if(KeyReleased(KEY_3))
           {
               option=3;
               done=true;
           }
           if(KeyReleased(KEY_4))
           {
               option=4;
               done=true;
           }
           
           draw_sprite(screen, buffer, 0, 0);
       }
        
        switch(option)
        {
           case 1: //Load game
           {
                done = false;
                CommandReset(1);
                while (!done)
                {
                        clear(buffer);
                        prevKeyPress = keyPress;
                        CommandLineON();
                        textout_centre_ex(buffer, font1, "What was your name, again?", screenW/2, screenH/2, WHITE, -1);
                        textout_centre_ex(buffer, font1, commandLine.c_str(), screenW/2, screenH/2+30, WHITE, -1);
                        draw_sprite(screen, buffer, 0, 0);
                        
                        if(KeyReleased(KEY_ENTER))
                        {
                             draw_sprite(screen, loader, 0, 0);
                             ifstream ifile;
                             stringstream ss;
                             ss << "data\\players\\"<<commandLine<<"\\PlayerData";
                             ifile.open(ss.str().c_str(), ios::in | ios::binary);
                             if(!ifile) {cout<<endl<<"Error: Player with  that name does not exist."<<endl; ifile.close(); break;}
                             ifile.close();
                             class Player player(commandLine);
                             if(player.isPassword==1)
                             {
                                  CommandReset(1);
                                  stringstream ss;
                                  while(true)
                                  {
                                             clear(buffer);
                                             ss.str(string());
                                             prevKeyPress = keyPress;
                                             CommandLineON();
                                             textout_centre_ex(buffer, font1, "Oh, you have set a password! Remember it?", screenW/2, screenH/2, WHITE, -1);
                                             for(int i=0; i<commandLine.size(); i++) ss<<"*";
                                             textout_centre_ex(buffer, font1, ss.str().c_str(), screenW/2, screenH/2+30, WHITE, -1);
                                             draw_sprite(screen, buffer, 0, 0);
                                             if(KeyReleased(KEY_ENTER))
                                             {
                                                 if(player.passCheck(commandLine)!=0)
                                                 {
                                                      cout<<"Error: Wrong password."<<endl;
                                                      enableCommand = 2;
                                                      CommandLineON();
                                                      enableCommand=1;
                                                 }
                                                 else break;
                                             }
                                  }
                             }
                             Gameplay(player, MenuMusic);
                             MenuMusic.PlayStream(-1);
                             done=true;
                        }
                        if(KeyReleased(KEY_ESC)) break;
                }
                done = false;
                option = 1;
           } break;
                 
           case 2: //New game
           {
                 bool esc=false;
                 class Player player(esc);
                 if(!esc)
                 {
                         Gameplay(player, MenuMusic);
                         MenuMusic.PlayStream(-1);
                 }
                 else cout<<"Aborted"<<endl;
                 done=false;
                 option = 1;
           } break;
           case 3: //Help
           {
                Help();
                done=false;
                option = 1;
           } break;
           case 4: //Exit
           {
                exit=true;
           } break;
                 
           default: break;
        }
    }
    
    cout<<"Closing...";
	deinit();
	DeinitFMOD();
	cout<<"Deinitialization done successfully";
	
	return 0;
}
END_OF_MAIN()

/////// FUNCTIONS

void Gameplay(class Player player, class MusicList &MenuMusic)
{
     draw_sprite(screen, loader, 0, 0);
     cout<<"Loading gameplay graphics..."<<endl;
     LoadGraphics();
     Map map;
     int mapID=1;
     if(!map.LoadMap(mapID, player)) cout<<"Could not load map of ID="<<mapID<<endl;
     player.CameraFollow(map);
     show_mouse( screen );
     unscare_mouse();
     bool exit=false;
     CommandReset(0);
     cout<<"Loading music...";
     class MusicList BackgroundMusic(string("data\\music\\list.txt"), 'b');
     MenuMusic.StopStream();
     BackgroundMusic.PlayStream(3);
     cout<<"Done"<<endl;
     cout<<"Ready"<<endl;
     speed=0;
     
     while (!exit&&!player.Dead) 
     {
       while(speed>0)
       {
          clear(buffer);     // clear screen
          prevKeyPress = keyPress;
          if(player.CampsCleared.size()>=map.CampsNumber) {player.WinGame(); break;}
          BackgroundMusic.PlayStream(0);

          if(KeyReleased(KEY_BACKSLASH)&&enableCommand==0) {clear_keybuf(); enableCommand=1;}
          else if(KeyReleased(KEY_ENTER)&&enableCommand==1)
          {
               time(&rawtime);
               char timeS[10];
               strftime(timeS, 10, "%X ", localtime(&rawtime));
               cout<<timeS<<commandLine<<endl;
               command=commandLine;
               CommandInterpret(player, BackgroundMusic);
               CommandReset(0);
          }
          if(KeyReleased(KEY_ESC))
          {
               if(enableCommand==1) CommandReset(0);
               else {exit=true; break;}
          }
          if(KeyReleased(KEY_F)&&enableCommand!=1) player.CameraFollow(map);
          if(KeyReleased(KEY_Z)&&enableCommand!=1) player.save();
          
          map.OnScreen(player);
          player.Move(map, BackgroundMusic);
      
          draw_sprite(buffer, background, 0, 0);
          CommandLineON();
          textout_centre_ex(buffer, font1, player.name.c_str(), screenW-115, 20, WHITE, -1);
          draw_sprite(buffer, gold, screenW-200, 50);
          textprintf_ex(buffer, font, screenW-160, 55, WHITE, -1, "%d", player.attributes.Gold);
          draw_sprite(buffer, wood, screenW-200, 70);
          textprintf_ex(buffer, font, screenW-160, 75, WHITE, -1, "%d", player.attributes.Wood);
          draw_sprite(buffer, ore, screenW-200, 90);
          textprintf_ex(buffer, font, screenW-160, 95, WHITE, -1, "%d", player.attributes.Ore);
          textout_ex(buffer, font, commandLine.c_str(), 20, 740, WHITE, -1);
          if(command[0]!=0) textout_ex(buffer, font1, string("Last command: "+command).c_str(), 20, 710, WHITE, -1);
          
          if(enableCommand==1) vline(buffer, caret * 8+20, 738, 748, WHITE); // draw the caret
          
          if(KeyReleased(KEY_C)&&enableCommand!=1) player.Card();// open player's card
          if(KeyReleased(KEY_U)&&enableCommand!=1) player.UnitStats();

          draw_sprite(screen, buffer, 0, 0);
          //speed--;
          speed=0;
          if (++frame>80) frame=0;
       }
	 }
	 CommandReset(0);
	 command=commandLine;
	 if(!player.Dead) player.save();
     BackgroundMusic.MusicList::~MusicList();
	 TownID = 0;
	 return;
}

void Help()
{
     bool done=false;
     do
     {
         while(true)
         {
                    prevKeyPress=keyPress;
                    clear(buffer);
                    draw_sprite(buffer, help, 0, 0);
                    draw_sprite(buffer, help1, 0, 0);
                    draw_sprite(screen, buffer, 0, 0);
                    if(KeyReleased(KEY_ENTER)) break;
                    if(KeyReleased(KEY_ESC)) {done=true; break;}
                    if(KeyReleased(KEY_RIGHT)) break;
         }
         if(!done) while(true)
         {
                    prevKeyPress=keyPress;
                    clear(buffer);
                    draw_sprite(buffer, help, 0, 0);
                    draw_sprite(buffer, help2, 0, 0);
                    draw_sprite(screen, buffer, 0, 0);
                    if(KeyReleased(KEY_ENTER)||KeyReleased(KEY_ESC)) {done=true; break;}
                    if(KeyReleased(KEY_LEFT)) break;
         }
     } while(!done);
}

void CommandLineON()
{
     if(enableCommand==3)
     {
          iter=commandLine.end();
          while(iter!=commandLine.begin())
          {
                  caret--;
                  iter--;
                  iter = commandLine.erase(iter);
          }
          enableCommand=0;
     }
     else if(enableCommand==2)while(keypressed())
     {
        int  newkey   = readkey();
        char ASCII    = newkey & 0xff;
        char scancode = newkey >> 8;
        
         // a character key was pressed; add it to the string
        if(ASCII>=48&&ASCII<=57)
        {
            // add the new char, inserting or replacing as need be
            if(insert || iter == commandLine.end())
               iter = commandLine.insert(iter, ASCII);
            else
               commandLine.replace(caret, 1, 1, ASCII);
 
            // increment both the caret and the iterator
            caret++;
            iter++;
        }
        else if (scancode==KEY_BACKSPACE)
        {
                  if(iter != commandLine.begin())
                  {
                     caret--;
                     iter--;
                     iter = commandLine.erase(iter);
                  }
        }
     }
     else if(enableCommand==1)while(keypressed())
     {
        int  newkey   = readkey();
        char ASCII    = newkey & 0xff;
        char scancode = newkey >> 8;
        
         // a character key was pressed; add it to the string
        if((ASCII==32||(ASCII>=48&&ASCII<=57)) || ((ASCII>=65&&ASCII<=90)||(ASCII==95||(ASCII>=97&&ASCII<=122))))
        {
            // add the new char, inserting or replacing as need be
            if(insert || iter == commandLine.end())
               iter = commandLine.insert(iter, ASCII);
            else
               commandLine.replace(caret, 1, 1, ASCII);
 
            // increment both the caret and the iterator
            caret++;
            iter++;
        }
        // some other, "special" key was pressed; handle it here
        else
            switch(scancode)
            {
               case KEY_DEL:
                  if(iter != commandLine.end()) iter = commandLine.erase(iter);
               break;
 
               case KEY_BACKSPACE:
                  if(iter != commandLine.begin())
                  {
                     caret--;
                     iter--;
                     iter = commandLine.erase(iter);
                  }
               break;
 
               case KEY_RIGHT:
                  if(iter != commandLine.end())   caret++, iter++;
               break;
 
               case KEY_LEFT:
                  if(iter != commandLine.begin()) caret--, iter--;
               break;
               
               case KEY_UP:
               {
                  iter=commandLine.end();
                  while(iter!=commandLine.begin())
                  {
                          caret--;
                          iter--;
                          iter = commandLine.erase(iter);
                  }
                  commandLine=string(command);
                  iter=commandLine.begin();
                  while(iter < commandLine.end())
                  {
                             caret++;
                             iter++;
                  }
               } break;
 
               case KEY_INSERT:
                  if(insert) insert = 0; else insert = 1;
               break;
 
               default:
 
               break;
            }
     }
}

void CommandReset(int state)
{
     clear_keybuf();
     enableCommand = 3;
     CommandLineON();
     enableCommand = state;
}

void CommandInterpret(class Player &player, class MusicList &BackgroundMusic)
{
     if (strcmp(UpperAll(commandLine).c_str(), "LEVEL UP")==0)
     {
           cout<<"Command 'Level up' recognized..."<<endl;
           commandLine="Cheater!";
           player.LevelUp();
     }
     else if (strcmp(UpperAll(commandLine).c_str(), "WINGAME")==0)
     {
           cout<<"Command 'WinGame' recognized..."<<endl;
           player.WinGame();
     }
     else if (strcmp(UpperAll(commandLine).c_str(), "PLAY NEXT")==0)
     {
           cout<<"Command 'Play next' recognized...\nPlaying next track."<<endl;
           BackgroundMusic.PlayStream(1);
     }
     else if (strcmp(UpperAll(commandLine).c_str(), "PLAY PREVIOUS")==0)
     {
           cout<<"Command 'Play next' recognized...\nPlaying previous track."<<endl;
           BackgroundMusic.PlayStream(2);
     }
     else if (strcmp(UpperAll(commandLine).c_str(), "PLAY SHUFFLE")==0)
     {
           cout<<"Command 'Play next' recognized...\nShuffling tracks."<<endl;
           BackgroundMusic.PlayStream(3);
     }
}

int level(int exp)
{
    int level, maxexp=0;
    for(level=1;level<=10; level++)
    {
         maxexp=maxexp+level*100;
         if (exp<maxexp) break;
    }
    if(level>10) level=10;
    return level;
}

bool KeyPressed(int keyValue)
{
     for(int i=0; i<KEY_MAX; i++)
     {
         if(key[i])
         {
                keyPress = i;
                break;
         }
         else keyPress = 0;
     }
     if(keyPress == keyValue && prevKeyPress != keyValue)
          return true;
     return false;
}

bool KeyReleased(int keyValue)
{
     for(int i=0; i<KEY_MAX; i++)
     {
         if(key[i])
         {
                keyPress = i;
                break;
         }
         else keyPress = 0;
     }
     if(keyPress != keyValue && prevKeyPress == keyValue)
          return true;
     return false;
}

bool KeyStillPressed(int keyValue)
{
     for(int i=0; i<KEY_MAX; i++)
     {
         if(key[i])
         {
                keyPress = i;
                break;
         }
         else keyPress = 0;
     }
     if(keyPress == keyValue && prevKeyPress == keyValue)
          return true;
     return false;
}

int diceroll (string dice)
{
     int roll, edge, result;
     sscanf(dice.c_str(), "%dd%d", &roll, &edge);
     result=0;
     if(edge!=0) for(int i=0; i<roll; i++) result=result+(rand()%edge+1);
     return result;
}

void split(const string& s, char c, vector<string>& v)
{
   string::size_type i = 0;
   string::size_type j = s.find(c);
   while (j != string::npos) {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);
      if (j == string::npos)
         v.push_back(s.substr(i, s.length( )));
   }
}

int zawijaj(const string& s, vector <string> &v, FONT* fnt, int SubLength)
{
    int RowNo=0;
    int pos=0;
    int start=0;
    while(s[pos]!=0)
    {
          
    }
    return RowNo;
}

void framing (BITMAP* bmp, int x, int y, FONT* f, char text[], int col)
{
     int width = text_length(f, text);
     int height = text_height(f);
     rectfill(bmp, x, y, x+width+20, y+height+20, WHITE);
     rect(bmp, x, y, x+width+20, y+height+20, RED);
     rect(bmp, x+10, y+10, x+width+10, y+height+10, RED);
     floodfill(bmp, x+1, y+1, GREEN);
     textout_ex(bmp, f, text, x+10, y+10, col, -1);

}

bool YesNo (char question[])
{
     bool option=-1;
     while(true)
     {
            clear(buffer);
            prevKeyPress = keyPress;
            textout_centre_ex(buffer, font1, question, screenW/2, screenH/3, WHITE, -1);
            switch (option)
            {
                   case true:
                   {
                        textout_centre_ex(buffer, font1, "YES", screenW/3, 2*screenH/3, RED, -1);
                        textout_centre_ex(buffer, font1, "NO", 2*screenW/3, 2*screenH/3, WHITE, -1);
                   } break;
                   case false:
                   {
                        textout_centre_ex(buffer, font1, "YES", screenW/3, 2*screenH/3, WHITE, -1);
                        textout_centre_ex(buffer, font1, "NO", 2*screenW/3, 2*screenH/3, RED, -1);
                   } break;
                   default:
                   {
                        textout_centre_ex(buffer, font1, "YES", screenW/3, 2*screenH/3, WHITE, -1);
                        textout_centre_ex(buffer, font1, "NO", 2*screenW/3, 2*screenH/3, WHITE, -1);
                   } break;
                
            }
            
            if(KeyReleased(KEY_ENTER)&&option!=-1)
            {
                return option;
            }
            if(KeyReleased(KEY_ESC))
            {
                return false;
            }
            if (KeyReleased(KEY_LEFT))
            {
                if (option==-1) option=true;
                else if (!option) option=true;
            }
            if (KeyReleased(KEY_RIGHT))
            {
                if (option==-1) option=false;
                else if (option) option=false;
            }
            
            draw_sprite(screen, buffer, 0, 0);
      }
}

string UpperAll(string text)
{
     for (int i=0; text[i]; i++)
     text[i] = toupper(text[i]);
     return text;
}
