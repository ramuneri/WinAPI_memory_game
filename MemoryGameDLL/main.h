#ifndef MEMORY_GAME_DLL_H
#define MEMORY_GAME_DLL_H

#include <windows.h>

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Start a shuffled game.
DLL_EXPORT void NewGame(unsigned int seed);

// Flip a card.
// Returns:
// 0 = click ignored
// 1 = first card revealed
// 2 = matching pair found
// 3 = different cards revealed
// 4 = all pairs found
DLL_EXPORT int FlipCard(int index);

// Hide the two cards after an unsuccessful attempt.
DLL_EXPORT void HideCards();

// Read the game state.
DLL_EXPORT int GetCardValue(int index);
DLL_EXPORT int IsCardVisible(int index);
DLL_EXPORT int GetAttempts();
DLL_EXPORT int GetMatchedPairs();

#ifdef __cplusplus
}
#endif

#endif
