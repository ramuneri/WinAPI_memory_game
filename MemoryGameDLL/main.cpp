#include "main.h"
#include <cstdlib>

namespace
{
    const int CARD_COUNT = 16;
    const int PAIR_COUNT = 8;

    int cards[CARD_COUNT] = {};
    bool matched[CARD_COUNT] = {};

    int firstCard = -1;
    int secondCard = -1;
    int attempts = 0;
    int matchedPairs = 0;
    bool gameStarted = false;

    bool IsValidIndex(int index)
    {
        return index >= 0 && index < CARD_COUNT;
    }
}

extern "C" DLL_EXPORT void NewGame(unsigned int seed)
{
    std::srand(seed);

    // Each value appears twice: 1, 1, 2, 2, ..., 8, 8.
    for (int i = 0; i < CARD_COUNT; ++i)
    {
        cards[i] = i / 2 + 1;
        matched[i] = false;
    }

    // Shuffle the cards.
    for (int i = CARD_COUNT - 1; i > 0; --i)
    {
        int randomIndex = std::rand() % (i + 1);

        int temporary = cards[i];
        cards[i] = cards[randomIndex];
        cards[randomIndex] = temporary;
    }

    firstCard = -1;
    secondCard = -1;
    attempts = 0;
    matchedPairs = 0;
    gameStarted = true;
}

extern "C" DLL_EXPORT int FlipCard(int index)
{
    if (!gameStarted || !IsValidIndex(index))
        return 0;

    // Ignore matched cards and clicking the first card again.
    if (matched[index] || index == firstCard)
        return 0;

    // Wait until an unsuccessful pair has been hidden.
    if (secondCard != -1)
        return 0;

    if (firstCard == -1)
    {
        firstCard = index;
        return 1;
    }

    secondCard = index;
    ++attempts;

    if (cards[firstCard] == cards[secondCard])
    {
        matched[firstCard] = true;
        matched[secondCard] = true;
        ++matchedPairs;

        firstCard = -1;
        secondCard = -1;

        if (matchedPairs == PAIR_COUNT)
            return 4;

        return 2;
    }

    // Keep the different cards visible until HideCards is called.
    return 3;
}

extern "C" DLL_EXPORT void HideCards()
{
    if (secondCard != -1)
    {
        firstCard = -1;
        secondCard = -1;
    }
}

extern "C" DLL_EXPORT int GetCardValue(int index)
{
    if (!gameStarted || !IsValidIndex(index))
        return 0;

    return cards[index];
}

extern "C" DLL_EXPORT int IsCardVisible(int index)
{
    if (!gameStarted || !IsValidIndex(index))
        return 0;

    return matched[index] ||
           index == firstCard ||
           index == secondCard;
}

extern "C" DLL_EXPORT int GetAttempts()
{
    return attempts;
}

extern "C" DLL_EXPORT int GetMatchedPairs()
{
    return matchedPairs;
}

extern "C" BOOL APIENTRY DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}
