#include "StudentWorld.h"
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

// functions that help to determine the initialization of values that depend on level number
int min(int n1, int n2)
{
    if (n1 < n2)
        return n1;
    return n2;
}

int max(int n1, int n2)
{
    if (n1 > n2)
        return n1;
    return n2;
}

void StudentWorld::addBoulders()
{
    int B = min(getLevel() / 2 + 2, 6); // how many boulders to add
    
    while (B != 0) // while you can add more boulders
    {
        int x = 0;
        int y = 0;
        
        bool validSpot = false;
        while (!validSpot)
        {
             // find a random, valid coordinate for the boulder to be placed in
            x = rand() % 61;
            y = rand() % 37 + 20;
        
            if (x < 27 || x > 33) // make sure the boulder is not in the mineshaft
                validSpot = true;
            
            if (overlapsAnotherActor(x, y)) // make sure the boulder doesn't overlap another actor
                validSpot = false;
        }
        
        actors.push_back(new Boulder(this, x, y)); // add the boulder to the vector of actors
        B--;
    }
}


void StudentWorld::addBarrels()
{
    int L = min(2 + getLevel(), 20); // how many barrels to add
    barrelsLeft = L;
    
    while (L != 0) // while you can add more barrels of oil
    {
        int x = 0;
        int y = 0;
        
        bool validSpot = false;
        while (!validSpot)
        {
            // find a random, valid spot to add the barrel of oil
            x = rand() % 61;
            y = rand() % 57;
            
            if (x < 27 || x > 33) // make sure the barrel is not in the mineshaft
                validSpot = true;
            
            if (overlapsAnotherActor(x, y)) // make sure the barrel doesn't overlap another actor
                validSpot = false;
        }
        
        actors.push_back(new Barrel(this, x, y)); // add the barrel to the vector of actors
        L--;
    }
}


void StudentWorld::addGold()
{
    int G = max(5-getLevel() / 2, 2); // how many gold pieces should be added
    
    while (G != 0) // while you can still add more gold pieces
    {
        int x = 0;
        int y = 0;
        
        bool validSpot = false;
        while (!validSpot)
        {
            // find a random, valid spot to place the gold piece
            x = rand() % 61;
            y = rand() % 57;
            
            if (x < 27 || x > 33) // make sure the barrel is not in the mineshaft
                validSpot = true;
            
            if (overlapsAnotherActor(x, y)) // make sure the barrel doesn't overlap another actor
                validSpot = false;
        }
        
        actors.push_back(new Gold(this, x, y)); // add the gold piece to the vector of actors
        G--;
    }
}

void StudentWorld::addProtester()
{
    if (!recentlyAddedProtester) // if you have not recently added a protester
    {
        if (numProtestersPresent < maxProtesters) // if you have not reached the maximum number of protesters
        {
            Protester* newProtester;
            
            // randomly determine whether protester is regular or hardcore
            int probabilityOfHardcore = min(90, getLevel() * 10 + 30);
            int randNum = rand() % 100;
            
            if (randNum < probabilityOfHardcore)
            { // add a hardcore protester
                int timeToStare = max(50, 100 - getLevel() * 10);
                int signalRange = 16 + getLevel() * 2;
                newProtester = new HardcoreProtester(this, timeToStare, signalRange);
            }
            else
            { // add a regular protester
                newProtester = new Protester(this, IID_PROTESTER, 5);
            }
            newProtester->setTicksToWaitBetweenMoves( max(0, 3 - getLevel()/4) );
            newProtester->setStunTime( max(50, 100 - getLevel()*10) );
            
            actors.push_back(newProtester); // add the protester to the vector of actors
            numProtestersPresent++;
            
            ticksBetweenProtesters = max(25, 200 - getLevel()); // tell next ticks that you have recently added a protester
            recentlyAddedProtester = true;
        }
        
    }
    else // you have recently added a protester
    {
        ticksBetweenProtesters--; // decrement the counter and determine whether you can add a protester next tick
        if (ticksBetweenProtesters == 0)
            recentlyAddedProtester = false;
    }
}



void StudentWorld::addGoodie()
{
    // determine randomly if a sonar or water pickup can be added this tick
    int G = getLevel() * 25 + 300;
    int randNum = rand() % G + 1;
    
    if (randNum == 73) // because Sheldon Cooper says 73 is the best number. The probability is 1/G and G > 73
    {
        // randomly determine if the added goodie is to be sonar or water
        randNum = rand() % 5 + 1;
        Goodie* goodie;
        
        if (randNum == 1) // 1, which is sonar
        {
            goodie = new Sonar(this);
        }
        else // 2, 3, 4, 5, which are water
        {
            int x = 0;
            int y = 0;
            
            // find valid coordinates such that the water can appear in an empty 4x4 square that has no dirt
            bool yIsValid = false;
            while (!yIsValid)
            {
                y = rand() % 57;
                
                for (int i = 0; i <= 60; i++)
                    if (empty4s[i][y].m_isEmpty)
                        yIsValid = true;
            }
            
            bool xIsValid = false;
            while (!xIsValid)
            {
                x = rand() % 61;
                
                if (empty4s[x][y].m_isEmpty)
                    xIsValid = true;
            }
            
            goodie = new Water(this, x, y);
        }
        
        // add the goodie to the vector of actors
        int time = max(100, 300 - 10 * getLevel());
        goodie->setTickCount(time);
        actors.push_back(goodie);
    }
}



void StudentWorld::addSquirt(int x, int y, Actor::Direction dir)
{
    switch (dir) // squirt travels in the direction which the frackman faces
    {
        case Actor::up:
            if ( y+4 <= 60 && empty4s[x][y+4].m_isEmpty) // determine if the squirt has room to be created
            {
                actors.push_back(new Squirt(this, x, y+4, dir)); // add the squirt and play sound if the squirt has enough empty space
                playSound(SOUND_PLAYER_SQUIRT);
            }
            break;
            
        case Actor::down:
            if ( y-4 >= 0 && empty4s[x][y-4].m_isEmpty)
            {
                actors.push_back(new Squirt(this, x, y-4, dir));
                playSound(SOUND_PLAYER_SQUIRT);
            }
            break;
            
        case Actor::right:
            if ( x+4 <= 60 && empty4s[x+4][y].m_isEmpty)
            {
                actors.push_back(new Squirt(this, x+4, y, dir));
                playSound(SOUND_PLAYER_SQUIRT);
            }
            break;
            
        case Actor::left:
            if ( x-4 >= 0 && empty4s[x-4][y].m_isEmpty)
            {
                actors.push_back(new Squirt(this, x-4, y, dir));
                playSound(SOUND_PLAYER_SQUIRT);
            }
            break;
            
        default:
            break;
    }
}

double StudentWorld::computeDistance(int x1, int y1, int x2, int y2) // compute the distance between two coordinates
{
    int xDiff = x1 - x2;
    int yDiff = y1 - y2;
    return sqrt(xDiff * xDiff + yDiff * yDiff);
}

bool StudentWorld::overlapsAnotherActor(int x, int y) // determine if an actor is too close to another actor when it is added
{
    for (int i = 0; i < actors.size(); i++)
    {
        double distance = computeDistance(x, y, actors[i]->getX(), actors[i]->getY());
        
        if (distance <= 6.0)
            return true; // the actor is too close, so return true
    }
    return false; // the actor is the appropriate distance from all other actors
}

bool StudentWorld::boulderBlockingPath(int x, int y)
{
    for (int i = 0; i < actors.size(); i++)
    {
        if (actors[i]->getID() == "boulder") // if it's a boulder
        {
            double distance = computeDistance(x + 2, y + 2, actors[i]->getX() + 2, actors[i]->getY() + 2);
            if (distance <= 3)
                return true; // there is at least one boulder in your way
        }
    }
    return false; // there are no boulders blocking the path
}

bool StudentWorld::annoyProtesters(int x, int y, int damage, double maxDistance)
{
    bool didAnnoy = false;
    for (int i = 0; i < actors.size(); i++)
    {
        if (actors[i]->getID() == "protester" || actors[i]->getID() == "hardcore") // if the actor is a protester
        {
            double distanceToProtester = computeDistance(x, y, actors[i]->getX(), actors[i]->getY());
            
            if (distanceToProtester <= maxDistance && !actors[i]->getIsLeavingField() ) // if the protester is within range to be annoyed
            {
                actors[i]->beAnnoyed(damage); // annoy the protester
                didAnnoy = true; // notify that at least one protester was annoyed
            }
        }
    }
    return didAnnoy;
}

void StudentWorld::annoyFrackMan(int x, int y, int damage, double maxDistance)
{
    double distanceToPlayer = computeDistance(x, y, player->getX(), player->getY());
    
    if (distanceToPlayer <= maxDistance) // if the player is within range
    {
        player->beAnnoyed(damage); // annoy the player
        
        if (!player->isAlive()) // determine if player died from the damage applied to him
            playSound(SOUND_PLAYER_GIVE_UP);
    }
}

bool StudentWorld::protesterHasLineOfSightOfPlayer(int x, int y)
{
    int playerX = player->getX();
    int playerY = player->getY();
    
    if (x == playerX) // x coordinates match, so need to check if protester is able to move straight in the y-direction
    {
        int yDiff = y - playerY;
        
        if (yDiff > 0)
        {
            for (int yCheck = y; yCheck > playerY; yCheck--)
                if (!empty4s[x][yCheck].m_isEmpty)
                    return false; // the protester is obstructed by some dirt or a boulder
        }
        else if (yDiff < 0)
        {
            for (int yCheck = y; yCheck < playerY; yCheck++)
                if (!empty4s[x][yCheck].m_isEmpty)
                    return false;// the protester is obstructed by some dirt or a boulder
        }
        return true; // the protester has a clear straight path to reach the player
    }
    else if (y == playerY)// y coordinates match, so need to check if protester is able to move straight in the x-direction
    {
        int xDiff = x - playerX;
        if (xDiff > 0)
        {
            for (int xCheck = x; xCheck > playerX; xCheck--)
                if (!empty4s[xCheck][y].m_isEmpty)
                    return false;// the protester is obstructed by some dirt or a boulder
        }
        else if (xDiff < 0)
        {
            for (int xCheck = x; xCheck < playerX; xCheck++)
                if (!empty4s[xCheck][y].m_isEmpty)
                    return false;// the protester is obstructed by some dirt or a boulder
        }
        return true; // the protester has a clear straight path to reach the player
    }
    return false;
}

bool StudentWorld::isPathBlocked(int x, int y, Actor::Direction dir)
{
    switch (dir) // determine if specific 4x4 square in said direction is free from dirt or boulders
    {
        case Actor::up:
            return !(y+1 <= 60 && empty4s[x][y+1].m_isEmpty);
        case Actor::down:
            return !(y-1 >= 0 && empty4s[x][y-1].m_isEmpty);
        case Actor::left:
            return !(x-1 >= 0 && empty4s[x-1][y].m_isEmpty);
        case Actor::right:
            return !(x+1 <= 60 && empty4s[x+1][y].m_isEmpty);
        default:
            return true;
    }
}

int StudentWorld::canMakePerpendicularTurn(int x, int y, Actor::Direction dir)
{
    // return values for possible turns:
    // 0 - no turns
    // 1 - right
    // 2 - left
    // 3 - right and left
    // 4 - up
    // 5 - down
    // 9 - up and down
    int count = 0;
    switch (dir)
    {
        case Actor::left: // check if up or down is possible
        case Actor::right:
            if (y+1 <= 60 && empty4s[x][y+1].m_isEmpty)
                count+=4;
            if (y-1 >= 0 && empty4s[x][y-1].m_isEmpty)
                count+=5;
            return count;
            break;
        case Actor::up: // check if left or right is possible
        case Actor::down:
            if (x+1 <= 60 && empty4s[x+1][y].m_isEmpty)
                count+=1;
            if (x-1 >= 0 && empty4s[x-1][y].m_isEmpty)
                count+=2;
            return count;
            break;
        default:
            return 0;
            break;
    }
}

bool StudentWorld::protestersPickedUpGold(int x, int y)
{
    for (int i = 0; i < actors.size(); i++)
    {
        if (actors[i]->getID() == "protester" || actors[i]->getID() == "hardcore") // if the actor is a protester
        {
            int distance = computeDistance(x, y, actors[i]->getX(), actors[i]->getY());
            if (distance <= 3.0 && !actors[i]->getIsLeavingField()) // if protester is close enough and is not leaving the field
            {
                actors[i]->pickUpGold(); // have the protester pick up the gold
                return true; // notify that a protester picked up the gold
            }
        }
    }
    return false; // a protester did not pick up the gold
}

void StudentWorld::playerDroppedGold(int x, int y)
{
    actors.push_back(new Gold(this, x, y, false)); // add a new gold piece that can be picked up by protesters
}

void StudentWorld::useSonarCharge(int x, int y)
{
    for (int i = 0; i < actors.size(); i++)
    {
        double distance = computeDistance(x, y, actors[i]->getX(), actors[i]->getY());
        if (distance <= 12.0) // reveal all hidden actors within distance of a sonar charge from the frackman
            actors[i]->setVisible(true);
    }
}

void StudentWorld::updateDisplayText()
{
    int score = getScore();
    int level = getLevel();
    int lives = getLives();
    int health = player->getHealth();
    int squirts = player->getSquirts();
    int gold = player->getGold();
    int sonar = player->getSonar();
    int barrelsLeft = getBarrelsLeft();
    
    string s = formatDisplayText(score, level, lives, health, squirts, gold, sonar, barrelsLeft);
    setGameStatText(s);
}

string StudentWorld::formatDisplayText(int score, int level, int lives, int health, int squirts, int gold, int sonar, int barrelsLeft)
{
    // score
    string text = "";
    text += "Scr: ";
    
    if (score / 10 == 0)
        text += "00000";
    else if (score / 100 == 0)
        text += "0000";
    else if (score / 1000 == 0)
        text += "000";
    else if (score / 10000 == 0)
        text += "00";
    else if (score / 100000 == 0)
        text += "0";
    text += to_string(score) + "  Lvl: ";
    
    // level
    if (level / 10 == 0)
        text += " ";
    text += to_string(level) + "  Lives: ";
    
    // lives
    text += to_string(lives) + "  Hlth: ";
    
    //health
    int percentHealth = health * 10;
    if (percentHealth / 10 == 0) // percentHealth is single digit
        text += "  ";
    else if (percentHealth / 100 == 0) // percentHealth is double digit
        text += " ";
    text += to_string(percentHealth) + "%  Wtr: ";
    
    //water (assume water 0 <= x <= 99)
    if (squirts / 10 == 0)
        text += " ";
    text += to_string(squirts) + "  Gld: ";
    
    //gold (assume gold 0 <= x <= 99)
    if (gold / 10 == 0)
        text += " ";
    text += to_string(gold) + "  Sonar: ";
    
    //sonar (assume sonar 0 <= x <= 99)
    if (sonar / 10 == 0)
        text += " ";
    text += to_string(sonar) + "  Oil Left: ";
    
    //oil (assume oil 0 <= x <= 99)
    if (barrelsLeft / 10 == 0)
        text += " ";
    text += to_string(barrelsLeft);
    
    return text;
}

bool StudentWorld::isDirtPresent(int x, int y)
{
    if (y > 59) // no dirt above ground
        return false;
    return dirtGrid[x][y] != nullptr;
}

void StudentWorld::digDirtSquare(int x, int y)
{
    if (y > 59) // can't dig above ground
        return;
    delete dirtGrid[x][y];
    dirtGrid[x][y] = nullptr;
}

bool StudentWorld::isEmptySpace(int x, int y)
{
    return empty4s[x][y].m_isEmpty;
}

void StudentWorld::updateEmpty4s(int x, int y)
{
    empty4s[x][y] = true;
    
    if ( y - 4 >= 0 && empty4s[x][y-4].m_isEmpty) // account for any overlap and fill in the 'gaps' below
    {
        empty4s[x][y-3].m_isEmpty = true;
        empty4s[x][y-2].m_isEmpty = true;
        empty4s[x][y-1].m_isEmpty = true;
    }
    else if ( y - 3 >= 0 && empty4s[x][y-3].m_isEmpty)
    {
        empty4s[x][y-2].m_isEmpty = true;
        empty4s[x][y-1].m_isEmpty = true;
    }
    else if ( y - 2 >= 0 && empty4s[x][y-2].m_isEmpty)
    {
        empty4s[x][y-1].m_isEmpty = true;
    }
    
    if ( y + 4 <= 60 && empty4s[x][y+4].m_isEmpty) // check for any 'gaps' above
    {
        empty4s[x][y+3].m_isEmpty = true;
        empty4s[x][y+2].m_isEmpty = true;
        empty4s[x][y+1].m_isEmpty = true;
    }
    else if ( y + 3 <= 60 && empty4s[x][y+3].m_isEmpty)
    {
        empty4s[x][y+2].m_isEmpty = true;
        empty4s[x][y+1].m_isEmpty = true;
    }
    else if ( y + 2 <= 60 && empty4s[x][y+2].m_isEmpty)
    {
        empty4s[x][y+1].m_isEmpty = true;
    }
    
    if ( x - 4 >= 0 && empty4s[x-4][y].m_isEmpty) // check for any 'gaps' to the left
    {
        empty4s[x-3][y].m_isEmpty = true;
        empty4s[x-2][y].m_isEmpty = true;
        empty4s[x-1][y].m_isEmpty = true;
    }
    else if ( x - 3 >= 0 && empty4s[x-3][y].m_isEmpty)
    {
        empty4s[x-2][y].m_isEmpty = true;
        empty4s[x-1][y].m_isEmpty = true;
    }
    else if ( x - 2 >= 0 && empty4s[x-2][y].m_isEmpty)
    {
        empty4s[x-1][y].m_isEmpty = true;
    }
    
    if ( x + 4 <= 60 && empty4s[x+4][y].m_isEmpty) // check for any 'gaps' to the right
    {
        empty4s[x+3][y].m_isEmpty = true;
        empty4s[x+2][y].m_isEmpty = true;
        empty4s[x+1][y].m_isEmpty = true;
    }
    else if ( x + 3 <= 60 && empty4s[x+3][y].m_isEmpty)
    {
        empty4s[x+2][y].m_isEmpty = true;
        empty4s[x+1][y].m_isEmpty = true;
    }
    else if ( x + 2 <= 60 && empty4s[x+2][y].m_isEmpty)
    {
        empty4s[x+1][y].m_isEmpty = true;
    }
}

void StudentWorld::addDirectionToLeaveToEmpty4s()
{
    undiscoverEmpty4s();
    
    queue<int> coordQueue;
    
    coordQueue.push(60); // always add x before y
    coordQueue.push(60);
    
    int x;
    int y;
    
    while (!coordQueue.empty())
    {
        x = coordQueue.front();
        coordQueue.pop();
        y = coordQueue.front();
        coordQueue.pop();
        
        empty4s[x][y].m_discovered = true;
        
        if (x-1 >= 0 && (empty4s[x-1][y].m_isEmpty && !empty4s[x-1][y].m_discovered)) // check left
        {
            empty4s[x-1][y].m_discovered = true; // do not want to visit and update this coordinate again
            empty4s[x-1][y].m_dirToLeave = Actor::right; // since we approached this coordinate from the left, the direction to leave must be to the right
            coordQueue.push(x-1);
            coordQueue.push(y);
        }
        
        if (y-1 >= 0 && (empty4s[x][y-1].m_isEmpty && !empty4s[x][y-1].m_discovered)) // check down
        {
            empty4s[x][y-1].m_discovered = true;
            empty4s[x][y-1].m_dirToLeave = Actor::up; // approached from down, so direction to leave must be up
            coordQueue.push(x);
            coordQueue.push(y-1);
        }
        
        if (x+1 <= 60 && (empty4s[x+1][y].m_isEmpty && !empty4s[x+1][y].m_discovered)) // check right
        {
            empty4s[x+1][y].m_discovered = true;
            empty4s[x+1][y].m_dirToLeave = Actor::left; // direction to leave must be left
            coordQueue.push(x+1);
            coordQueue.push(y);
        }
        
        if (y+1 <= 60 && (empty4s[x][y+1].m_isEmpty && !empty4s[x][y+1].m_discovered)) // check up
        {
            empty4s[x][y+1].m_discovered = true;
            empty4s[x][y+1].m_dirToLeave = Actor::down; // direction to leave must be down
            coordQueue.push(x);
            coordQueue.push(y+1);
        }
    }
}

Actor::Direction StudentWorld::getDirectionToLeave(int x, int y)
{
    return empty4s[x][y].m_dirToLeave;
}

void StudentWorld::undiscoverEmpty4s() // reset 2D node array for next tick update
{
    for (int x = 0; x < 61; x++)
        for (int y = 0; y < 61; y++)
        {
            empty4s[x][y].m_discovered = false;
            empty4s[x][y].m_distanceFromPlayer = 0;
        }
}

void StudentWorld::updateSignalDirection()
{
    undiscoverEmpty4s();
    
    queue<int> coordQueue;
    
    coordQueue.push(player->getX()); // always add x before y
    coordQueue.push(player->getY()); // start the breadth-first search from the position of the frackman
    
    int x;
    int y;
    int distance;
    
    while (!coordQueue.empty())
    {
        x = coordQueue.front();
        coordQueue.pop();
        y = coordQueue.front();
        coordQueue.pop();
        
        distance = empty4s[x][y].m_distanceFromPlayer + 1; // the distance of the coordinates we check next must be one greater than the distance of the coordinate we just popped
        
        empty4s[x][y].m_discovered = true;
        
        if (x-1 >= 0 && (empty4s[x-1][y].m_isEmpty && !empty4s[x-1][y].m_discovered)) // check left
        {
            empty4s[x-1][y].m_discovered = true; // we do not want to visit this coordinate more than once
            empty4s[x-1][y].m_dirToPlayer = Actor::right; // since the coordinate was discovered by going left, then the direction to the frackman must be to the right
            empty4s[x-1][y].m_distanceFromPlayer = distance; // record number of steps from the frackman
            coordQueue.push(x-1);
            coordQueue.push(y);
        }
        
        if (y-1 >= 0 && (empty4s[x][y-1].m_isEmpty && !empty4s[x][y-1].m_discovered)) // check down
        {
            empty4s[x][y-1].m_discovered = true;
            empty4s[x][y-1].m_dirToPlayer = Actor::up; // went down to discover coordinate, so frackman is in the up direction
            empty4s[x][y-1].m_distanceFromPlayer = distance;
            coordQueue.push(x);
            coordQueue.push(y-1);
        }
        
        if (x+1 <= 60 && (empty4s[x+1][y].m_isEmpty && !empty4s[x+1][y].m_discovered)) // check right
        {
            empty4s[x+1][y].m_discovered = true;
            empty4s[x+1][y].m_dirToPlayer = Actor::left; // frackman must be in the left direction
            empty4s[x+1][y].m_distanceFromPlayer = distance;
            coordQueue.push(x+1);
            coordQueue.push(y);
        }
        
        if (y+1 <= 60 && (empty4s[x][y+1].m_isEmpty && !empty4s[x][y+1].m_discovered)) // check up
        {
            empty4s[x][y+1].m_discovered = true;
            empty4s[x][y+1].m_dirToPlayer = Actor::down; // frackman must be in the down direction
            empty4s[x][y+1].m_distanceFromPlayer = distance;
            coordQueue.push(x);
            coordQueue.push(y+1);
        }
    }
}

Actor::Direction StudentWorld::getDirectionToPlayer(int x, int y)
{
    return empty4s[x][y].m_dirToPlayer;
}

int StudentWorld::getDistanceFromPlayer(int x, int y)
{
    return empty4s[x][y].m_distanceFromPlayer;
}

void StudentWorld::removeDeadGameObjects()
{
    int indexesToDelete[10]; // assume that no more than 10 actors died within one tick
    int deleteCount = 0;
    
    for (int i = 0; i < actors.size(); i++)
        if (!(actors[i]->isAlive()))
        {
            indexesToDelete[deleteCount] = i;
            deleteCount++;
        }
    
    for (int i = deleteCount - 1; i >= 0; i--)
    {
        int toBeDeleted = indexesToDelete[i];
        delete actors[toBeDeleted]; // delete the actor
        actors[toBeDeleted] = actors.back(); // overwrite pointer with last pointer in actors
        actors.pop_back(); // pop the last pointer in actors
    }
}












