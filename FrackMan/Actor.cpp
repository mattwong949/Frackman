#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp


// ***** FRACKMAN ********

void FrackMan::doSomething()
{
    if (!isAlive())
        return;
    
    int xPos = getX();
    int yPos = getY();
    bool didDig = false;
    
    // remove any overlapping dirt
    for (int x = xPos; x < xPos + 4; x++)
        for (int y = yPos; y < yPos + 4; y++)
            if (getWorld()->isDirtPresent(x, y))
            {
                getWorld()->digDirtSquare(x, y);
                didDig = true;
            }
    
    if (didDig) // if the frackman removed dirt this tick, then play the sound and update the StudentWorld's data structure
    {
        getWorld()->playSound(SOUND_DIG);
        getWorld()->updateEmpty4s(xPos, yPos);
    }
    
    int key;
    if (getWorld()->getKey(key) == true)
    {
        // user hit a key this tick!
        switch (key)
        {
            case KEY_PRESS_LEFT:
                if (getDirection() != left) // set direction if not facing left
                    setDirection(left);
                else if (!getWorld()->boulderBlockingPath(xPos-1, yPos)) // check that nothing is blocking path
                {
                    if (xPos > 0)
                        moveTo(xPos-1, yPos);
                    else
                        moveTo(xPos, yPos);
                }
                break;
            case KEY_PRESS_RIGHT:
                if (getDirection() != right)
                    setDirection(right);
                else if (!getWorld()->boulderBlockingPath(xPos+1, yPos))
                {
                    if (xPos < 60)
                        moveTo(xPos+1, yPos);
                    else
                        moveTo(xPos, yPos);
                }
                break;
            case KEY_PRESS_DOWN:
                if (getDirection() != down)
                    setDirection(down);
                else if (!getWorld()->boulderBlockingPath(xPos, yPos-1))
                {
                    if (yPos > 0)
                        moveTo(xPos, yPos-1);
                    else
                        moveTo(xPos, yPos);
                }
                break;
            case KEY_PRESS_UP:
                if (getDirection() != up)
                    setDirection(up);
                else if (!getWorld()->boulderBlockingPath(xPos, yPos+1))
                {
                    if (yPos < 60)
                        moveTo(xPos, yPos+1);
                    else
                        moveTo(xPos, yPos);
                }
                break;
            case KEY_PRESS_SPACE:
                if (getSquirts() > 0) // squirt the area in front of you if you have water squirts available
                {
                    shootWater();
                    getWorld()->addSquirt(xPos, yPos, getDirection());
                }
                break;
                
            case KEY_PRESS_TAB:
                if (getGold() > 0) // drop a gold piece where you are standing if you have gold pieces available
                {
                    dropGold();
                    getWorld()->playerDroppedGold(xPos, yPos);
                }
                break;
            case 'z':
            case 'Z':
                if (getSonar() > 0) // use a sonar charge if it is available
                {
                    useSonar();
                    getWorld()->useSonarCharge(xPos, yPos);
                    getWorld()->playSound(SOUND_SONAR);
                }
                
                break;
            case KEY_PRESS_ESCAPE:
                hasDied(); // end the current level
                break;
        }
    }
}

// ******* BOULDER *********

Boulder::Boulder(StudentWorld* swPtr, int xPos, int yPos)
: Actor(swPtr, IID_BOULDER, xPos, yPos, down, 1, 1, true)
{
    // remove dirt from under the boulder
    for (int x = xPos; x < xPos + 4; x++)
        for (int y = yPos; y < yPos + 4; y++)
            if (getWorld()->isDirtPresent(x, y))
            {
                getWorld()->digDirtSquare(x, y);
            }
    
    state = 0; // boulder starts the level in stable state
    tickCount = 0;
}

void Boulder::doSomething()
{
    if (!isAlive())
        return;
    
    int x = getX();
    int y = getY();
    
    if (state == 0) // checking for dirt underneath
    {
        bool nothingBelow = true;
        
        for (int i = 0; i < 4; i++)
            if (getWorld()->isDirtPresent(x+i, y-1))
                nothingBelow = false;
        
        if (nothingBelow) // the boulder will be in a 'waiting state' next tick
            state = 1;
    }
    else if (state == 1) // waiting state
    {
        tickCount++;
        if (tickCount == 30) // when it has waited 30 ticks
        {
            state = 2; // the boulder switches to falling state
            for (int i = 0; i < 4; i++)
                getWorld()->updateEmpty4s(x, y-i); // boulder leaves an empty 4x4 space
            getWorld()->playSound(SOUND_FALLING_ROCK);
        }
    }
    else if (state == 2) // falling state
    {
        getWorld()->annoyProtesters(x, y, 100, 3.0); // annoy any protesters it comes in contact with
        getWorld()->annoyFrackMan(x, y, 100, 3.0); // annoy the frackman if he comes in contact
        
        if (getWorld()->isEmptySpace(x, y-1)) // fall as long as there is empty space below
            moveTo(x, y-1);
        else
            hasDied(); // no more room to fall, so the boulder dies and will be removed at the end of this tick
    }
}

// ********** BARREL **********

void Barrel::doSomething()
{
    if (!isAlive())
        return;
    
    double distanceToPlayer = getWorld()->computeDistance(getX(), getY(), getWorld()->getFrackMan()->getX(), getWorld()->getFrackMan()->getY());
    if (distanceToPlayer <= 3.0) // the barrel is close enough to the player to be picked up
    {
        hasDied();
        getWorld()->playSound(SOUND_FOUND_OIL);
        getWorld()->increaseScore(1000);
        getWorld()->decreaseNumBarrels();
    }
    if (distanceToPlayer <= 4.0) // the barrel is close enough to the player to be discovered
    {
        setVisible(true);
        return;
    }
}

// ************ GOLD ************

void Gold::doSomething()
{
    if (!isAlive())
        return;
    
    if (playerCanPickUp) // if the gold is in it's original state (hasn't be picked up by frackman)
    {
        double distanceToPlayer = getWorld()->computeDistance(getX(), getY(), getWorld()->getFrackMan()->getX(), getWorld()->getFrackMan()->getY());
        if (distanceToPlayer <= 3.0) // the gold is close enough to the player to be picked up
        {
            hasDied();
            getWorld()->playSound(SOUND_GOT_GOODIE);
            getWorld()->increaseScore(10);
            getWorld()->getFrackMan()->addGold();
        }
        if (distanceToPlayer <= 4.0) // the gold is close enough to the player to be discovered
        {
            setVisible(true);
            return;
        }
    }
    else // the gold was placed by the frackman and can be picked up by protesters
    {
        if (getWorld()->protestersPickedUpGold(getX(), getY()) ) // determine if a protester is close enough to pick up the gold
        {
            hasDied(); // protester picked up the gold
        }
        
        timer--;
        if (timer == 0) // gold has been present for more than 100 ticks, so it must go away
            hasDied();
    }
}

// ********** SONAR *************

void Sonar::doSomething()
{
    if (!isAlive())
        return;
    
    double distanceToPlayer = getWorld()->computeDistance(getX(), getY(), getWorld()->getFrackMan()->getX(), getWorld()->getFrackMan()->getY());
    if (distanceToPlayer <= 3.0) // determine if the gold is close enough to the player to be picked up
    {
        hasDied();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(75);
        getWorld()->getFrackMan()->addSonar();
    }
    
    decrementTicks();
    if (getTickCount() == 0) // the sonar has been present for too long and must go away this tick
    {
        hasDied();
    }
}

// ********** WATER *************


void Water::doSomething()
{
    if (!isAlive())
        return;
    
    double distanceToPlayer = getWorld()->computeDistance(getX(), getY(), getWorld()->getFrackMan()->getX(), getWorld()->getFrackMan()->getY());
    if (distanceToPlayer <= 3.0) // determine if the water is close enough to the player to be picked up
    {
        hasDied();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(100);
        getWorld()->getFrackMan()->addWater();
    }
    
    decrementTicks();
    if (getTickCount() == 0) // the water has been present for too long and must go away this tick
    {
        hasDied();
    }
    
}

// ********** PROTESTER ***********

void Protester::doSomething()
{
    if (!isAlive())
        return;
    
    int xPos = getX();
    int yPos = getY();
    
    if (!leavingField) // if the protester is still searching for the frackman
    {
        if (isStunnedTick()) // has been stunned and is still stunned by the player
            return;
        
        else if (isRestingTick()) // is on a resting tick
            return;

        else // is on a nonresting tick
        {
            startNonRestingTick(); // update protester so that next ticks will be resting ticks
            
            if (hasRecentlyShouted()) // if the protester has recently shouted, do nothing
                return;
            
            if (shoutAtPlayer()) // if the protester is close enough and is facing the player, shout at him
                return;
            
            else if (takeStepTowardsPlayer()) // if the protester has line-of-sight of the player, take a step toward him
                return;
            
            else
                takeNormalStep(); // take a regular step in the current direction or in a new direction
        }
    }
    else // protester has given up and is leaving field
    {
        if (isRestingTick()) // is on a resting tick
            return;
        
        if (xPos == 60 && yPos == 60) // the protester has made it to the top right corner, so he must be removed this tick
        {
            hasDied();
            return;
        }
        
        takeStepToLeave(); // the protester hasn't made it to the top right corner, so he must take a step in the direction to leave the field
        return;
    }
}

bool Protester::isRestingTick()
{
    if (isResting) // if the protester is on a resting tick
    {
        restCounter++;
        if (restCounter > ticksToWaitBetweenMoves) // update the counter and determine if the next tick is a resting tick
        {
            isResting = false;
            restCounter = 0;
        }
    }
    return isResting; //  return true if the protester is supposed to rest this tick, false otherwise
}

bool Protester::isStunnedTick()
{
    if (isStunned) // if the protester is still stunned by the player's water squirt
    {
        stunCounter++;
        if (stunCounter > ticksToBeStunned) // update the counter and determine if the next tick is a stunned tick
        {
            isStunned = false;
            stunCounter = 0;
        }
    }
    return isStunned; // return true if the protester is supposed to be stunned this tick, false otherwise
}

void Protester::startNonRestingTick()
{
    isResting = true; // the next tick will be a resting tick
    ticksSincePerpendicular++; // if this value ever reaches 200, then the protester needs to make a perpendicular turn when possible
}

bool Protester::hasRecentlyShouted()
{
    if (recentlyShouted) // if the protester recently shouted at the player
    {
        ticksAfterShouting++;
        if (ticksAfterShouting == 15) // update the counter and determine when the protester may start moving again
            recentlyShouted = false;
    }
    return recentlyShouted; // return true if the protester has recently shouted at the player, false otherwise
}

bool Protester::shoutAtPlayer()
{
    int xPos = getX();
    int yPos = getY();
    int playerX = getWorld()->getFrackMan()->getX();
    int playerY = getWorld()->getFrackMan()->getY();
    
    if (getWorld()->computeDistance(xPos, yPos, playerX, playerY) <= 4.0) // determine if close enough to shout at player
    {
        // now determine if the protester is facing the player
        Direction dir = getDirection();
        bool isFacingPlayer = false;
        switch (dir)
        {
            case up:
                if ( yPos <= playerY && playerX-4 <= xPos && playerX+4 >= xPos)
                    isFacingPlayer = true;
                break;
                
            case down:
                if ( yPos >= playerY && playerX-4 <= xPos && playerX+4 >= xPos)
                    isFacingPlayer = true;
                break;
                
            case left:
                if ( xPos >= playerX && playerY-4 <= yPos && playerY+4 >= yPos)
                    isFacingPlayer = true;
                break;
                
            case right:
                if ( xPos <= playerX && playerY-4 <= yPos && playerY+4 >= yPos)
                    isFacingPlayer = true;
                break;
                
            default:
                break;
        }
        
        if (isFacingPlayer) // if the protester is facing the player
        {
            if (!recentlyShouted) // and if he has not recently shouted
            {
                getWorld()->playSound(SOUND_PROTESTER_YELL); // shout at the player
                getWorld()->annoyFrackMan(xPos, yPos, 2, 4.0);
                recentlyShouted = true;
                ticksAfterShouting = 0;
            }
            else
            {
                ticksAfterShouting++; // otherwise update the counter and determine whether the protester can move again next tick
                if (ticksAfterShouting == 15)
                    recentlyShouted = false;
            }
            return true; // the protester was close enough to shout at the player
        }
    }
    return false; // the protester was not close enough to shout at the player
}

bool Protester::takeStepTowardsPlayer()
{
    int xPos = getX();
    int yPos = getY();
    int playerX = getWorld()->getFrackMan()->getX();
    int playerY = getWorld()->getFrackMan()->getY();
    
    if (getWorld()->protesterHasLineOfSightOfPlayer(xPos, yPos)) // move in player direction if Line of Sight
    {
        if (xPos == playerX && yPos < playerY) // need to face up
        {
            setNewDirection(up);
            moveTo(xPos, yPos+1);
        }
        else if (xPos == playerX && yPos > playerY) // need to face down
        {
            setNewDirection(down);
            moveTo(xPos, yPos-1);
        }
        else if (yPos == playerY && xPos > playerX) // need to face left
        {
            setNewDirection(left);
            moveTo(xPos-1, yPos);
        }
        else if (yPos == playerY && xPos < playerX) // need to face right
        {
            setNewDirection(right);
            moveTo(xPos+1, yPos);
        }
        numSquaresToMoveInCurrentDirection = 0;
        return true; // the protester took a step towards the player
    }
    return false; // the protester did not have line of sight of the player
}

void Protester::takeNormalStep()
{
    int xPos = getX();
    int yPos = getY();
    
    if (numSquaresToMoveInCurrentDirection <= 0) // needs to find new direction
    {
        bool validDir = false;
        Direction newDir = none;
        while (!validDir)
        {
            int randNum = rand() % 4;
            switch (randNum)
            {
                case 0:
                    newDir = up;   break;
                case 1:
                    newDir = down;   break;
                case 2:
                    newDir = left;   break;
                case 3:
                    newDir = right;   break;
                default:   break;
            }
            validDir = !getWorld()->isPathBlocked(xPos, yPos, newDir); // a valid direction needs to be able to take one step in that direction
        }
        setNewDirection(newDir);
    }
    
    if (ticksSincePerpendicular >= 200) // it has been too long since a perpendicular turn and protester needs to make one
    {
        int directions = getWorld()->canMakePerpendicularTurn(xPos, yPos, getDirection()); // determine in which directions the protester can make a perpendicular turn
        int randNum = 0;
        switch (directions) // set the new direction of the protester
        {
            case 1: // right
                setNewDirection(right);
                break;
            case 2: // left
                setNewDirection(left);
                break;
            case 3: // right or left
                randNum = rand() % 2;
                if (randNum == 0)
                    setNewDirection(right);
                else
                    setNewDirection(left);
                break;
            case 4: // up
                setNewDirection(up);
                break;
            case 5: // down
                setNewDirection(down);
                break;
            case 9: // up or down
                randNum = rand() % 2;
                if (randNum == 0)
                    setNewDirection(up);
                else
                    setNewDirection(down);
                break;
                
            default:
                break;
        }
    }
    
    if (numSquaresToMoveInCurrentDirection > 0) // take a step in the protester's current (or new) direction
    {
        switch (getDirection())
        {
            case left:
                moveTo(xPos-1, yPos);
                break;
            case right:
                moveTo(xPos+1, yPos);
                break;
            case up:
                moveTo(xPos, yPos+1);
                break;
            case down:
                moveTo(xPos, yPos-1);
                break;
                
            default:
                break;
        }
        numSquaresToMoveInCurrentDirection--;
        if (getWorld()->isPathBlocked(getX(), getY(), getDirection())) // check if new path blocked
            numSquaresToMoveInCurrentDirection = 0; // if path is blocked, then next tick the protester needs to find a new direction
    }
}

void Protester::takeStepToLeave()
{
    int xPos = getX();
    int yPos = getY();
    
    isResting = true; // the next tick is a resting tick
    
    Direction dir = getWorld()->getDirectionToLeave(xPos, yPos); // access the StudentWorld's data structure to find which direction it should move in to leave
    setNewDirection(dir);
    switch (dir) // move in that direction
    {
        case up:
            moveTo(xPos, yPos+1);
            break;
            
        case down:
            moveTo(xPos, yPos-1);
            break;
            
        case right:
            moveTo(xPos+1, yPos);
            break;
            
        case left:
            moveTo(xPos-1, yPos);
            break;
            
        default:
            break;
    }
}

void Protester::pickUpGold()
{
    startLeaving(); // regular protesters start leaving once they pick up a piece of dropped gold
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(25);
}

void Protester::beAnnoyed(int damage)
{
    if (!leavingField) // if the protester is not leaving the field and thus can take damage
    {
        health -= damage;
        if (health > 0) // if the protester is still alive
        {
            getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
            getStunned();
        }
        if (health <= 0) // if the protester has no more health
        {
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
            leavingField = true; // start leaving the oil field
        }
        if (leavingField) // if the protester is now leaving the oil field (he has been defeated this tick)
        {
            // increment the score according to how the protester was defeated
            if (damage == 100) // annoyed by boulder
                getWorld()->increaseScore(500);
            if (damage == 2) // annoyed by water
                getWorld()->increaseScore(100);
        }
    }
}

void Protester::hasDied()
{
    Actor::hasDied(); // protester will be removed this tick
    getWorld()->decreaseNumProtesters(); // notify StudentWorld that there is one less protester in the game
}

void Protester::setNewDirection(Direction dir) // sets the direction, but also records if the new direction is a perpendicular turn or not
{
    if (getDirection() == up || getDirection() == down) // if it makes a perpendicular turn of any sort, reset the counter of ticksSincePerpendicular
    {
        if (dir == left || dir == right)
            ticksSincePerpendicular = 0;
    }
    else if (getDirection() == left || getDirection() == right)
    {
        if (dir == up || dir == down)
            ticksSincePerpendicular = 0;
    }
    setDirection(dir); // set the new direction
    numSquaresToMoveInCurrentDirection = rand() % 53 + 8; // set a new number of steps to take in that direction
}

// ********** HARDCORE ***************

void HardcoreProtester::doSomething()
{
    if (!isAlive())
        return;
    
    if (isStaringTick()) // if the hardcore protester is staring at gold, do nothing
        return;
    
    int xPos = getX();
    int yPos = getY();
    
    if (!getIsLeavingField()) // if the hardcore protester is still searching for frackman
    {
        if (isStunnedTick()) // has been stunned by the player's water squirt
            return;
        
        else if (isRestingTick()) // is on a resting tick
            return;
        
        else // is on a nonresting tick
        {
            startNonRestingTick(); // next ticks will be resting ticks
            
            if (hasRecentlyShouted()) // if the hardcore protester has recently shouted, do nothing
                return;
            
            if (shoutAtPlayer()) // if he is can shout at the frackman, shout at the frackman
                return;
            
            else if (useCellPhoneToTakeStep()) // if he is close enough to the frackman, take a step using the StudentWorld's data structure
                return;
            
            else if (takeStepTowardsPlayer()) // if frackman is in line of sight, take a step towards the frackman
                return;
            
            else
                takeNormalStep(); // otherwise take a normal step in the current (or new) direction
        }
    }
    else // protester has given up and is leaving field
    {
        if (isRestingTick()) // is on a resting tick
            return;
        
        if (xPos == 60 && yPos == 60) // the protester has made it to the top right corner and must be removed this tick
        {
            hasDied();
            return;
        }
        
        takeStepToLeave(); // use the StudentWorld's data structure to leave the oil field
        return;
    }
}

bool HardcoreProtester::isStaringTick()
{
    if (isStaring) // if the protester is staring at a gold piece
    {
        stareTicker++;
        if (stareTicker > ticks_to_stare) // update the counter and determine whether the next tick is also a staring tick
        {
            isStaring = false;
            stareTicker = 0;
        }
    }
    return isStaring; // return true if he is staring at gold, false otherwise
}

bool HardcoreProtester::useCellPhoneToTakeStep()
{
    int xPos = getX();
    int yPos = getY();
    
    if (getWorld()->getDistanceFromPlayer(xPos, yPos) <= signalRange) // if the frackman is close enough to the frackman to use the "cellphone signal"
    {
        Direction newDir = getWorld()->getDirectionToPlayer(xPos, yPos); // access the StudentWorld's data structure to get the direction in which to move
        setDirection(newDir);
        switch (newDir) // move in the direction provided by StudentWorld's data structure
        {
            case up:
                moveTo(xPos, yPos+1);
                break;
                
            case down:
                moveTo(xPos, yPos-1);
                break;
                
            case right:
                moveTo(xPos+1, yPos);
                break;
                
            case left:
                moveTo(xPos-1, yPos);
                break;
                
            default:
                break;
        }
        return true; // the protester took a step using the cellphone signal
    }
    return false; // the protester was too far to use the cellphone signal
}

void HardcoreProtester::pickUpGold()
{
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(50);
    isStaring = true;
}

void HardcoreProtester::beAnnoyed(int damage)
{
    if (!getIsLeavingField()) // if the protester is not leaving the field and thus can be damaged
    {
        receiveDamage(damage); // decrement health accordingly
        if (getHealth() > 0) // if protester is still alive
        {
            getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
            getStunned();
        }
        if (getHealth() <= 0) // if the protester has been defeated this tick
        {
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
            startLeaving();
        }
        if (getIsLeavingField()) // and if the protester is now leaving the field because he was defeated this tick
        {
            // determine how much to increment the score
            if (damage == 100) // annoyed by boulder
                getWorld()->increaseScore(500);
            if (damage == 2) // annoyed by water
                getWorld()->increaseScore(250);
        }
    }
}

// ************ SQUIRT ************

void Squirt::doSomething()
{
    if (!isAlive())
        return;
    
    int xPos = getX();
    int yPos = getY();

    if (getWorld()->annoyProtesters(xPos, yPos, 2, 3.0))
        travelDistance = 0; // the squirt cannot go any further after annoying a protester
    
    if (travelDistance == 0) // squirt cannot go any further so it must be removed
    {
        hasDied();
        return;
    }
    
    switch (getDirection()) // the squirt can still move in the direction the frackman pointed it
    {
        case up:
            moveTo(xPos, yPos+1);
            break;
            
        case down:
            moveTo(xPos, yPos-1);
            break;
            
        case right:
            moveTo(xPos+1, yPos);
            break;
            
        case left:
            moveTo(xPos-1, yPos);
            break;
            
        default:
            break;
    }
    
    travelDistance--;

    if (travelDistance == 0) // the squirt cannot go further and must be removed
        hasDied();
}




