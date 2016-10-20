#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

#include <iostream>

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;

// ************ Actor *******************

class Actor: public GraphObject
{
public:
    Actor(StudentWorld* swPtr, int imageID, int startX, int startY, Direction startDirection, float size = 1.0, unsigned int depth = 0, bool stops = false)
    :GraphObject(imageID, startX, startY, startDirection, size, depth)
    {
        setVisible(true);
        m_studentWorld = swPtr;
        m_isAlive = true;
        stopsActors = stops;
    }
    
    virtual void doSomething() = 0;
    virtual void beAnnoyed(int damage) = 0;
    virtual std::string getID() = 0; // return what type of actor the object is
    
    virtual void pickUpGold() // used mainly for the purposes of the protester and hardcore protesters
    { };
    
    virtual bool getIsLeavingField() // used mainly for the purposes of the protester and hardcore protesters
    { return true; }
    
    bool isAlive()
    {
        return m_isAlive;
    }
    
    virtual void hasDied() // virtual because the death of a protester needs to decrease the numProtesters in the StudentWorld object when called
    {
        m_isAlive = false;
    }
    
    bool stopsOtherActors()
    {
        return stopsActors;
    }
    
    StudentWorld* getWorld()
    {
        return m_studentWorld;
    }
    
    virtual ~Actor()
    { }
    
private:
    StudentWorld* m_studentWorld;
    bool m_isAlive;
    bool stopsActors;
};

// ************ DIRT *******************

class Dirt: public Actor
{
public:
    Dirt(StudentWorld* swPtr, int x, int y)
    : Actor(swPtr, IID_DIRT, x, y, right, .25, 3)
    { }
    
    virtual void doSomething() // dirt doesn't do anything
    { return; }
    
    virtual void beAnnoyed(int damage) // dirt can't be annoyed
    { return; }
    
    virtual std::string getID()
    { return "dirt"; }
    
    virtual ~Dirt()
    { }
};

// ************ FrackMan *******************

class FrackMan: public Actor
{
public:
    FrackMan(StudentWorld* swPtr)
    :Actor(swPtr, IID_PLAYER, 30, 60, right, 1.0, 0)
    {
        health = 10;
        waterSquirts = 5;
        sonarCharges = 1;
        goldNuggets = 0;
    }
    
    virtual void doSomething();
    
    virtual void beAnnoyed(int damage)
    {
        health -= damage;
        if (health <= 0)
            hasDied();
    }
    
    virtual std::string getID()
    { return "frackman"; }
    
    void addGold()
    { goldNuggets++; }
    
    void dropGold()
    { goldNuggets--; }
    
    void addSonar()
    { sonarCharges++; }
    
    void useSonar()
    { sonarCharges--; }
    
    void addWater()
    { waterSquirts += 5; }
    
    void shootWater()
    { waterSquirts--; }
    
    int getSquirts()
    { return waterSquirts; }
    
    int getSonar()
    { return sonarCharges; }
    
    int getGold()
    { return goldNuggets; }
    
    int getHealth()
    { return health; }
    
    virtual ~FrackMan()
    {}
    
private:
    int health;
    int waterSquirts;
    int sonarCharges;
    int goldNuggets;
};

// ************ Boulder *******************

class Boulder: public Actor
{
public:
    Boulder(StudentWorld* swPtr, int xPos, int yPos);
    
    virtual void doSomething();
    
    virtual void beAnnoyed(int damage) // boulders can't be annoyed
    { }
    
    virtual std::string getID()
    { return "boulder"; }
    
    virtual ~Boulder()
    { }
    
private:
    int state; // 0 is stable, 1 is waiting, 2 is falling
    int tickCount;
};

// ************* Barrel **************

class Barrel: public Actor
{
public:
    Barrel(StudentWorld* swPtr, int xPos, int yPos)
    :Actor(swPtr, IID_BARREL, xPos, yPos, right, 1.0, 2)
    {
        setVisible(false); // barrels start out invisible
    }
    
    virtual void doSomething();
    
    virtual void beAnnoyed(int damage) // barrels can't be annoyed
    { }
    
    virtual std::string getID()
    { return "barrel"; }
    
    virtual ~Barrel()
    { }
};

// ************** Gold ************

class Gold: public Actor
{
public:
    Gold(StudentWorld* swPtr, int xPos, int yPos, bool startState = true)
    :Actor(swPtr, IID_GOLD, xPos, yPos, right, 1.0, 2)
    {
        if (startState)
            setVisible(false); // if gold is placed at beginning of level, it is not visible
        else
            timer = 100; // else if the frackman places it, it lasts for 100 ticks
        
        playerCanPickUp = startState; // if state is false, then it can be picked up by protestors
    }
    
    virtual void doSomething();
    
    bool protestorCanPickUp() // used for protester collision with gold
    { return !playerCanPickUp; }
    
    virtual void beAnnoyed(int damage) // gold can't be annoyed
    { }
    
    virtual std::string getID()
    { return "gold"; }
    
    virtual ~Gold()
    { }
    
private:
    bool playerCanPickUp; // false means that protestor can pick it up
    int timer;
};

// ********** GOODIE **********

class Goodie: public Actor // serves as an abstract class for the temporary pick-ups: the Sonar and the Water
{
public:
    Goodie(StudentWorld* swPtr, int imageID, int xPos, int yPos)
    :Actor(swPtr, imageID, xPos, yPos, right, 1.0, 2)
    { };
    
    virtual void doSomething() = 0;
    
    virtual void beAnnoyed(int damage) // goodies can't be annoyed
    { };
    
    virtual std::string getID() = 0;
    
    void setTickCount(int timer) // set how long the pick-ups last for
    {
        tickCount = timer;
    }
    
    void decrementTicks()
    {
        tickCount--;
    }
    
    int getTickCount()
    {
        return tickCount;
    }
    
    virtual ~Goodie()
    { }
    
private:
    int tickCount;
};



// ********** SONAR *********

class Sonar: public Goodie
{
public:
    Sonar(StudentWorld* swPtr)
    :Goodie(swPtr, IID_SONAR, 0, 60)
    { }
    
    virtual std::string getID()
    { return "sonar"; }
    
    virtual ~Sonar()
    { }
    
    virtual void doSomething();
    
};

// ************ WATER ***********

class Water: public Goodie
{
public:
    Water(StudentWorld* swPtr, int xPos, int yPos)
    :Goodie(swPtr, IID_WATER_POOL, xPos, yPos)
    { }
    
    virtual std::string getID()
    { return "water"; }
    
    virtual ~Water()
    { }
    
    virtual void doSomething();
    
};

// ************ REGULAR PROTESTER *************

class Protester: public Actor // represents a regular protester. This is the base class for hardcore protesters
{
public:
    Protester(StudentWorld* swPtr, int img, int hp)
    : Actor(swPtr, img, 60, 60, left, 1, 0, true)
    {
        health = hp;
        leavingField = false;
        numSquaresToMoveInCurrentDirection = rand() % 53 + 8;
        
        isResting = false;
        restCounter = 0;
        
        isStunned = false;
        stunCounter = 0;
        
        recentlyShouted = false;
        ticksAfterShouting = 0;
        
        ticksSincePerpendicular = 0;
    }
    
    virtual void doSomething();
    
    // methods for the general protester movement that is called in doSomething()
    bool isRestingTick();
    bool isStunnedTick();
    bool hasRecentlyShouted();
    bool shoutAtPlayer();
    bool takeStepTowardsPlayer();
    void takeNormalStep();
    void takeStepToLeave();
    
    void startNonRestingTick();
    
    virtual void pickUpGold();
    
    virtual void beAnnoyed(int damage);
    
    virtual void hasDied();
    
    virtual std::string getID()
    { return "protester"; }
    
    void setNewDirection(Direction dir); // keeps track of perpendicular turns
    
    void getStunned()
    { isStunned = true; }
    
    void setTicksToWaitBetweenMoves(int time)
    { ticksToWaitBetweenMoves = time; }
    
    void setStunTime(int time)
    { ticksToBeStunned = time; }
    
    int getHealth()
    { return health; }
    
    void receiveDamage(int damage)
    { health -= damage; }
    
    bool getIsLeavingField()
    { return leavingField; }
    
    void startLeaving()
    { leavingField = true; }
    
    virtual ~Protester()
    { }
    
private:
    int health;
    bool leavingField;
    int numSquaresToMoveInCurrentDirection;
    
    bool isResting;
    int restCounter;

    bool isStunned;
    int stunCounter;
    
    bool recentlyShouted;
    int ticksAfterShouting;
    
    int ticksSincePerpendicular;
    
    int ticksToWaitBetweenMoves;
    int ticksToBeStunned;
};

// ************** HARDCORE PROTESTOR **************

class HardcoreProtester: public Protester // a hardcore protester is a regular protester with a few modifications to its behavior
{
public:
    HardcoreProtester(StudentWorld* swPtr, int tick2stare, int cellSignalRange)
    : Protester(swPtr, IID_HARD_CORE_PROTESTER, 20)
    {
        ticks_to_stare = tick2stare; // hardcore protesters stare at gold
        signalRange = cellSignalRange; // hardcore protesters use the frackman's cell phone to track his location when frackman is nearby
        
        stareTicker = 0;
        isStaring = false;
    }
    
    virtual void doSomething();
    
    virtual void pickUpGold();
    
    virtual void beAnnoyed(int damage);
    
    bool useCellPhoneToTakeStep();
    
    bool isStaringTick();
    
    virtual std::string getID()
    { return "hardcore"; }
    
    int getSignalRange()
    { return signalRange; }
    
    virtual ~HardcoreProtester()
    { }
    
private:
    int stareTicker;
    int ticks_to_stare;
    int signalRange;
    bool isStaring;
};

// ******** SQUIRT ***************

class Squirt: public Actor
{
public:
    Squirt(StudentWorld* swPtr, int x, int y, Direction dir)
    : Actor(swPtr, IID_WATER_SPURT, x, y, dir, 1, 1)
    {
        travelDistance = 4;
    }
    
    virtual void doSomething();
    
    virtual void beAnnoyed(int damage) // squirts can't be annoyed
    { }
    
    virtual std::string getID()
    { return "squirt"; }
    
    virtual ~Squirt()
    { }
    
private:
    int travelDistance;
    
};






#endif // ACTOR_H_
