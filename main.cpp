#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "version.h"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

template <u32 add, u32 mult>
class LCRNG
{
public:
    LCRNG(uint32_t seed = 0) : seed(seed)
    {
    }

    void advanceFrames(u32 frames)
    {
        for (u32 frame = 0; frame < frames; frame++)
        {
            nextUInt();
        }
    }

    u16 nextUShort()
    {
        return nextUInt() >> 16;
    }

    u32 nextUInt()
    {
        return seed = seed * mult + add;
    }

    u32 getSeed() const
    {
        return seed;
    }

private:
    u32 seed;
};

using XDRNG = LCRNG<0x269EC3, 0x343FD>;
using XDRNGR = LCRNG<0xA170F641, 0xB9B33155>;

bool validateJirachi(u32 seed);
bool validateMenu(u32 seed);
void advanceMenu(XDRNG &rng, u32 &count);
void advanceJirachi(XDRNG &rng, u32 &count);
void advanceCutscene(XDRNG &rng, u32 &count);
void advanceTitleScreen(XDRNG &rng, u32 &count);
u32 calculateFrame(u32 currentSeed, u32 targetSeed);
std::vector<u8> calculateActions(u32 currentSeed, u32 targetFrame, u32 bruteForce);
void increment_vector(std::vector<u8> &actions);

int main()
{
    std::cout << "Jirachi Advancer" << std::endl;
    std::cout << "Branch: " << GIT_BRANCH << std::endl;
    std::cout << "Commit: " << GIT_COMMIT << std::endl;
    std::cout << std::endl << std::endl;

    int go = 1;

    while (go == 1)
    {
        std::cout << "Current seed: 0x";
        std::string input;
        std::cin >> input;
        u32 currentSeed = std::stoul(input, nullptr, 16);

        if (!validateMenu(currentSeed))
        {
            std::cout << "Current seed is invalid" << std::endl;
        }
        else
        {

            std::cout << "Target seed: 0x";
            std::cin >> input;
            u32 targetSeed = std::stoul(input, nullptr, 16);

            if (!validateJirachi(targetSeed))
            {
                std::cout << "Target seed is invalid" << std::endl;
            }
            else
            {
                std::cout << "Max frame: ";
                std::cin >> input;
                u32 maxFrame = std::stoul(input, nullptr, 10);

                u32 targetFrame = calculateFrame(currentSeed, targetSeed);
                if (targetFrame > maxFrame)
                {
                    std::cout << "Frame range is greater than " << maxFrame << std::endl;
                }
                else
                {
                    std::cout << "Brute force range: ";
                    std::cin >> input;
                    u32 bruteForce = std::stoul(input, nullptr, 10);

                    std::vector<u8> path = calculateActions(currentSeed, targetFrame, bruteForce);
                    if (path.empty())
                    {
                        std::cout << "Target seed is unreachable" << std::endl;
                    }
                    else
                    {
                        std::cout << std::endl;

                        int number = 1;
                        if (path[0] != 255)
                        {
                            for (u8 action : path)
                            {
                                if (action == 0)
                                {
                                    std::cout << number++ << ": Reload Menu";
                                }
                                else if (action == 1)
                                {
                                    std::cout << number++ << ": Reject Jirachi";
                                }
                                else
                                {
                                    std::cout << number++ << ": Exit Special cutscene";
                                }

                                if (number % 5 == 0)
                                {
                                    std::cout << std::endl;
                                }
                                else
                                {
                                    std::cout << " ";
                                }
                            }
                        }
                        std::cout << number++ << ": Accept Jirachi" << std::endl;
                    }
                }
            }
        }

        std::cout << std::endl << "Go again? 1/0" << std::endl;
        std::cin >> go;
        std::cout << std::endl;
    }

    return 0;
}

// Working backwards this validates if a Jirachi seed is obtainable
// There are 3 different patterns for this (6/7/8 advances) plus menu checking
bool validateJirachi(u32 seed)
{
    XDRNGR rng(seed);

    uint16_t num1 = rng.nextUShort();
    uint16_t num2 = rng.nextUShort();
    uint16_t num3 = rng.nextUShort();

    rng.advanceFrames(3);
    if (num1 <= 0x4000) // 6 advances
    {
        if (validateMenu(rng.getSeed()))
        {
            return true;
        }
    }

    rng.advanceFrames(1);
    if (num2 > 0x4000 && num1 <= 0x547a) // 7 advances
    {
        if (validateMenu(rng.getSeed()))
        {
            return true;
        }
    }

    rng.advanceFrames(1);
    if (num3 > 0x4000 && num2 > 0x547a) // 8 advances
    {
        if (validateMenu(rng.getSeed()))
        {
            return true;
        }
    }

    return false;
}

// Working backwards from a seed check if the menu sequence will end on said seed
// Menu will advance the prng until it collects a 1, 2, and 3
bool validateMenu(u32 seed)
{
    // Impossible to stop 0
    u8 target = seed >> 30;
    if (target == 0)
    {
        return false;
    }
    
    u8 mask = 1 << target;
    XDRNGR rng(seed);
    do
    {
        u8 num = rng.nextUInt() >> 30;

        // Basically this check means that while rolling for 1, 2, and 3
        // We hit our original target meaning that we can't land on the target
        if (num == target)
        {
            return false;
        }

        mask |= 1 << num;
    } while ((mask & 14) != 14);

    return true;
}

// Advances prng of menu as described in validateMenu
void advanceMenu(XDRNG &rng, u32 &count)
{
    u8 mask = 0;
    do
    {
        u8 num = rng.nextUInt() >> 30;
        count++;

        mask |= 1 << num;
    } while ((mask & 14) != 14);
}

// Advances prng of jirachi flying on screen
void advanceJirachi(XDRNG &rng, u32 &count)
{
    rng.advanceFrames(4);
    count += 4;

    bool flag = false;

    count++;
    if (rng.nextUShort() <= 0x4000)
    {
        flag = true;
    }
    else
    {
        count++;
        flag = rng.nextUShort() <= 0x547a;
    }

    if (flag)
    {
        rng.advanceFrames(1);
        count++;
    }
    else
    {
        rng.advanceFrames(2);
        count += 2;
    }
}

// Advances prng of special cutscene playing
void advanceCutscene(XDRNG &rng, u32 &count)
{
    rng.advanceFrames(1);
    count++;
}

// Advances prng of title screen reloading
void advanceTitleScreen(XDRNG &rng, u32 &count)
{
    rng.advanceFrames(1);
    count++;
}

// Determines how far apart two prng states are
u32 calculateFrame(u32 currentSeed, u32 targetSeed)
{
    XDRNG rng(currentSeed);
    u32 count = 0;

    while (rng.getSeed() != targetSeed)
    {
        rng.nextUInt();
        count++;
    }

    return count;
}

// Attempts to calculate an action plan to get to a target seed given a current seed
std::vector<u8> calculateActions(u32 currentSeed, u32 targetFrame, u32 bruteForce)
{
    // Not possible, fail early
    if (targetFrame < 6)
    {
        return {};
    }

    // Special handling for if we only need to accept
    // Jirachi can only advance 6, 7, or 8 frames so bound the check
    if (targetFrame >= 6 && targetFrame <= 8)
    {
        XDRNG rng(currentSeed);
        u32 count = 0;
        advanceJirachi(rng, count);

        if (count == targetFrame)
        {
            return { 255 };
        }
    }

    XDRNG menu(currentSeed);
    u32 menuFrame = 0;
    int menuCount = 0;

    // Use menu advances to get to a brute forcable range
    while (targetFrame > bruteForce + menuFrame)
    {
        menuCount++;
        advanceMenu(menu, menuFrame);
    }

    // Now that we are within our brute force range we brute force search
    // We have 3 ways to advance frames
    for (int i = 1;; i++)
    {
        // This variable handles checking if all of the possibilities of the current search size exceed the target seed
        // This is preferred to guessing what value of 'i' that is
        bool done = true;

        std::vector<u8> searchActions(static_cast<size_t>(i), 0);
        while (true)
        {
            u32 searchFrame = menuFrame;
            XDRNG rng(menu.getSeed());

            bool flag = false;
            for (u8 action : searchActions)
            {
                if (action == 0) // Reload menu
                {
                    advanceMenu(rng, searchFrame);
                }
                else if (action == 1) // Reject jirachi
                {
                    advanceJirachi(rng, searchFrame);
                    advanceTitleScreen(rng, searchFrame);
                    advanceMenu(rng, searchFrame);
                }
                else // Special cutscene
                {
                    advanceCutscene(rng, searchFrame);
                    advanceTitleScreen(rng, searchFrame);
                    advanceMenu(rng, searchFrame);
                }

                // Make sure didn't go over frame
                // Add 6 since that is the minimum an accept can advance
                if ((searchFrame + 6) > targetFrame)
                {
                    flag = true;
                    break;
                }
            }

            // Verify we didn't go over target seed
            if (!flag)
            {
                done = false;

                // Advance the frames of accepting the jirachi
                advanceJirachi(rng, searchFrame);

                // If we land on target seed then return the actions to get to it
                if (searchFrame == targetFrame)
                {
                    // Vector is constructed in the way that the initial menu advances are already set
                    std::vector<u8> actions(static_cast<size_t>(menuCount) + searchActions.size(), 0);

                    // Copy over the search actions
                    std::copy(searchActions.cbegin(), searchActions.cend(), actions.begin() + menuCount);

                    return actions;
                }
            }

            // Exit loop once all possibilities have been attempted
            if (std::count(searchActions.cbegin(), searchActions.cend(), 2) == i)
            {
                break;
            }

            increment_vector(searchActions);
        }

        if (done)
        {
            break;
        }
    }

    // If we get to this point then it is extremely unlikely to get to the the target seed from the current seed
    return {};
}

void increment_vector(std::vector<u8> &actions)
{
    size_t size = actions.size();
    if (size == 1)
    {
        actions[0]++;
        return;
    }

    bool increment = true;
    for (int i = 0; i < size; i++)
    {
        u8 compare = i == 0 ? 2 : 3;
        if (actions[i] >= compare)
        {
            increment = false;

            actions[i] = 0;
            if (i != size - 1)
            {
                actions[i + 1]++;
            }
        }
        else if (increment)
        {
            actions[i]++;
            break;
        }
    }
}