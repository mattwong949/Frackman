#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

#include <vector>
#include <queue>
#include "Actor.h"

int min(int n1, int n2); // functions that help with determining some of the numbers that vary with the level number
int max(int n1, int n2);

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
	 : GameWorld(assetDir)
	{
        // initialize all variables and data structures to default values
        barrelsLeft = 0;
        
        ticksBetweenProtesters = 0;
        recentlyAddedProtester = false;
        numProtestersPresent = 0;
        maxProtesters = 0;
        
        for (int i = 0; i < 61; i++)
            for (int j = 0; j < 61; j++)
                empty4s[i][j].m_isEmpty = false;
	}

	virtual int init()
	{
        // set the values for which coordinates represent empty 4x4 squares
        for (int i = 0; i < 61; i++)
            for (int j = 0; j < 61; j++)
                empty4s[i][j].m_isEmpty = false;
        
        // create all dirt actors
        for (int x = 0; x < 64; x++)
            for (int y = 0; y < 60; y++)
            {
                if ( 30 <= x && x <= 33 && 4 <= y && y <= 59) // no dirt starting mine shaft
                {
                    dirtGrid[x][y] = nullptr;
                    if (x == 30)
                        empty4s[x][y].m_isEmpty = true;
                }
                else
                    dirtGrid[x][y] = new Dirt(this, x, y);
            }
        
        // the top row is all empty 4x4 squares
        for (int x = 0; x < 61; x++)
            empty4s[x][60].m_isEmpty = true;
        
        ticksBetweenProtesters = 0;
        recentlyAddedProtester = false;
        maxProtesters = min(15, 2 + getLevel() * 1.5);
        numProtestersPresent = 0;
        
        // create FrackMan
        player = new FrackMan(this);
        
        // create actors...
        addBoulders();
        addBarrels();
        addGold();
        addProtester();
        
        return GWSTATUS_CONTINUE_GAME;
	}

	virtual int move()
    {
        // Update the Game Status Line
        updateDisplayText(); // update the score/lives/level text at screen top
        
        // update the directions that allow protesters to leave the field
        addDirectionToLeaveToEmpty4s();
        // udpate the directions that allow hardcore protesters to track frackman's cellphone
        updateSignalDirection();
        
        // possibly add a goodie or a protester this tick
        addGoodie();
        addProtester();
         
        // The vector actors refers to all the boulders, gold, barrels, goodies, protesters, and water squirts
        // Give each Actor a chance to do something
        for(int i = 0; i < actors.size(); i++)
        {
            actors[i]->doSomething();
                
            if (player->isAlive() == false)
            {
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
        }
        
        // give the frackman a chance to do something
        player->doSomething();
        
        // Remove newly-dead actors after each tick
        removeDeadGameObjects();
                 
        // the player died this tick
        if (player->isAlive() == false)
        {
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }
        
        // If the player has collected all of the Barrels on the level, then
        // return the result that the player finished the level
        if (getBarrelsLeft() == 0)
        {
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
        
        // the player hasn’t completed the current level and hasn’t died
        // let them continue playing the current level
        return GWSTATUS_CONTINUE_GAME;
	}

	virtual void cleanUp()
	{
        // delete any other dirt objects
        for (int x = 0; x < 64; x++)
            for (int y = 0; y < 60; y++)
                delete dirtGrid[x][y];
        
        // delete FrackMan
        delete player;
        
        // delete remaining actors
        std::vector<Actor*>::iterator it;
        it = actors.begin();
        while (it != actors.end())
        {
            delete (*it);
            it++;
        }
        while (!actors.empty())
        {
            actors.pop_back();
        }
	}
    
    //destructor
    ~StudentWorld()
    {
        // delete any other dirt objects
        for (int x = 0; x < 64; x++)
            for (int y = 0; y < 60; y++)
                delete dirtGrid[x][y];
        
        // delete FrackMan
        delete player;
        
        // delete remaining actors
        std::vector<Actor*>::iterator it;
        it = actors.begin();
        while (it != actors.end())
        {
            delete (*it);
            it++;
        }
        while (!actors.empty())
        {
            actors.pop_back();
        }
    }
    
    
    
    void addBoulders();
    void addBarrels();
    void addGold();
    
    void addProtester();
    void addGoodie();
    
    void addSquirt(int x, int y, Actor::Direction dir);
    
    double computeDistance(int x1, int y1, int x2, int y2);
    bool overlapsAnotherActor(int x, int y);
    bool boulderBlockingPath(int x, int y);
    
    bool annoyProtesters(int x, int y, int damage, double maxDistance);
    void annoyFrackMan(int x, int y, int damage, double maxDistance);
    
    bool protesterHasLineOfSightOfPlayer(int x, int y);
    bool isPathBlocked(int x, int y, Actor::Direction dir);
    int canMakePerpendicularTurn(int x, int y, Actor::Direction dir);
    
    bool protestersPickedUpGold(int x, int y);
    
    void playerDroppedGold(int x, int y);
    void useSonarCharge(int x, int y);
    
    void decreaseNumBarrels()
    { barrelsLeft--;}
    int getBarrelsLeft()
    { return barrelsLeft; }
    
    void decreaseNumProtesters()
    { numProtestersPresent--; }
    
    FrackMan* getFrackMan()
    { return player; }

    
    void updateDisplayText();
    std::string formatDisplayText(int score, int level, int lives, int health, int squirts, int gold, int sonar, int barrelsLeft);
    
    bool isDirtPresent(int x, int y); // does grid space have dirt
    void digDirtSquare(int x, int y); // remove Dirt from dirtGrid
    bool isEmptySpace(int x, int y); // is a 4x4 grid space empty
    void updateEmpty4s(int x, int y); // update empty 4x4 array to add new true values
    void addDirectionToLeaveToEmpty4s();
    Actor::Direction getDirectionToLeave(int x, int y);
    void undiscoverEmpty4s();
    void updateSignalDirection();
    Actor::Direction getDirectionToPlayer(int x, int y);
    int getDistanceFromPlayer(int x, int y);
    
    void removeDeadGameObjects();

    
private:
    
    // a Node in the 2D array represents the data associated with a 4x4 square in the oil field
    // the node's row and col in the 2D array represent the lower left coordinate of the 4x4 square in the oil field
    struct Node
    {
    public:
        Node(bool value = false)
        {
            m_isEmpty = value;
            m_dirToLeave = Actor::none;
            m_discovered = false;
            m_distanceFromPlayer = 0;
            m_dirToPlayer = Actor::none;
        }
        
        bool m_isEmpty; // is the full 4x4 square free from dirt
        Actor::Direction m_dirToLeave; // which direction does a protester move to leave the field from this coordinate
        bool m_discovered; // used for purposes of the breadth-first search
        
        int m_distanceFromPlayer; // how many steps from the frackman is this coordinate
        Actor::Direction m_dirToPlayer; // which direction should a hardcore protester use to move towards the frackman
    };
    
    Dirt* dirtGrid[64][60]; // holds all dirt pointers
    Node empty4s[61][61]; // records all x,y where there is an empty 4x4 square
    FrackMan* player;
    std::vector<Actor*> actors;
    
    int barrelsLeft;
    
    int ticksBetweenProtesters;
    bool recentlyAddedProtester;
    int numProtestersPresent;
    int maxProtesters;
};

#endif // STUDENTWORLD_H_
