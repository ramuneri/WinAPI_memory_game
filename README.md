# Simple memory game with cats

## For Windows API course assignment

---

ENGLISH

#### About
A memory game with 16 cards and eight matching pairs. Reveal two cards at a 
time. Matching cards stay visible; different cards turn back over. Find all
eight pairs to win. Fewer attempts means a better score.

#### How to run
1. Extract the project ZIP before running the game (optional).
2. Open memory_game/bin/Debug and run memory_game.exe.
3. MemoryGameDLL.dll must be in the same folder as memory_game.exe.

#### How to build
Use Code::Blocks with MinGW.
1. Open MemoryGameDLL.cbp and build the Debug target.
2. Open memory_game.cbp and build the Debug target.
3. Copy the built MemoryGameDLL.dll from the DLL project's output folder
   into memory_game/bin/Debug, then run memory_game.exe.
Use the same MinGW compiler and architecture for both projects.

#### Assignment requirements
- WinAPI interface: card buttons, attempt and pair counters, New Game button.
- Resource script: game.rc defines the Game/Help menu, About dialog and images.
- Graphics: eight AI-generated kitten and celestial pictures, drawn with GDI.
- Dynamic DLL: game logic is loaded at runtime using LoadLibrary and
  GetProcAddress; the library is released using FreeLibrary.
- Files: best_score.ini stores the best score beside the executable and is
  read when the game opens. It is created after the first completed game.
- Rules: shuffled pairs, matching checks and detection of a completed game.

The game folder must be writable to save the best score. Images are embedded
in the executable during building; keep the images folder with game.rc to rebuild.

---

LIETUVIŠKAI

#### Apie žaidimą
Atminties žaidimas su 16 kortelių ir aštuoniomis vienodomis poromis. Vienu metu
atverskite dvi korteles. Vienodos kortelės lieka atverstos, o skirtingos vėl
užverčiamos. Suraskite visas poras ir laimėkite. Geresnis rezultatas – mažiau bandymų.

#### Kaip paleisti
1. Prieš paleisdami žaidimą, išarchyvuokite projekto ZIP failą (jei reikia).
2. Atidarykite memory_game/bin/Debug ir paleiskite memory_game.exe.
3. MemoryGameDLL.dll turi būti tame pačiame aplanke kaip memory_game.exe.

#### Kaip sukompiliuoti
Naudokite Code::Blocks su MinGW.
1. Atidarykite MemoryGameDLL.cbp ir sukompiliuokite Debug konfigūraciją.
2. Atidarykite memory_game.cbp ir sukompiliuokite Debug konfigūraciją.
3. Nukopijuokite sukurtą MemoryGameDLL.dll iš DLL projekto rezultatų aplanko
   į memory_game/bin/Debug, tada paleiskite memory_game.exe.
Abiem projektams naudokite tą patį MinGW kompiliatorių ir architektūrą.

#### Užduoties reikalavimai
- WinAPI sąsaja: kortelių mygtukai, bandymų ir porų skaitikliai, New Game mygtukas.
- Resursų scenarijus: game.rc aprašo Game/Help meniu, About dialogą ir paveikslėlius.
- Grafika: aštuoni DI sukurti kačiukų ir dangaus kūnų paveikslėliai, piešiami su GDI.
- Dinaminė DLL: žaidimo logika įkeliama vykdymo metu naudojant LoadLibrary ir
  GetProcAddress; biblioteka atlaisvinama naudojant FreeLibrary.
- Failai: geriausias rezultatas saugomas best_score.ini šalia vykdomojo failo
  ir nuskaitomas paleidžiant žaidimą. Failas sukuriamas pirmą kartą laimėjus.
- Taisyklės: porų išmaišymas, sutapimų tikrinimas ir laimėjimo nustatymas.

Žaidimo aplanke turi būti leidžiama rašyti, kad rezultatas būtų išsaugotas.
Paveikslėliai įterpiami į vykdomąjį failą kompiliuojant; norint perkompiliuoti,
images aplanką reikia laikyti šalia game.rc.
