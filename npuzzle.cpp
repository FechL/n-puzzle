#include <algorithm>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

// Simple encryption key
const string ENCRYPTION_KEY = "N-PUZZLE-KEY";

int sizeN;
vector<vector<int>> arr;
vector<vector<int>> arrSave;
int emptyX, emptyY, emptyXSave, emptyYSave;
int moves = 0;
time_t startTime;
bool gameStarted = false;
bool gameRunning = true;
bool gameWon = false;
bool isInGame = false;
bool restart = false;
bool five_seconds = false;

mutex gameMutex;

// Simple encryption/decryption function using XOR
string encryptDecrypt(const string &input) {
    string output = input;
    for (size_t i = 0; i < input.length(); i++) {
        output[i] = input[i] ^ ENCRYPTION_KEY[i % ENCRYPTION_KEY.length()];
    }
    return output;
}

void title(bool inGame = false, int count = 25) {
    system("cls");
    cout << " N-Puzzle by @FechL" << endl;
    if (!inGame) {
        for (int i = 0; i < count; i++)
            cout << "-";
        cout << endl;
    }
}

void fillArray() {
    vector<int> numbers;
    for (int i = 0; i < sizeN * sizeN; i++)
        numbers.push_back(i);

    srand(time(0));
    random_shuffle(numbers.begin(), numbers.end());

    int idx = 0;
    for (int i = 0; i < sizeN; i++) {
        for (int j = 0; j < sizeN; j++) {
            arr[i][j] = numbers[idx++];
            if (arr[i][j] == 0) {
                emptyX = i;
                emptyY = j;
            }
        }
    }
    gameStarted = false;
    startTime = 0;
    moves = 0;
}

bool isSolved() {
    int idx = 1;
    for (int i = 0; i < sizeN; i++) {
        for (int j = 0; j < sizeN; j++) {
            if (arr[i][j] != idx++ && idx <= sizeN * sizeN)
                return false;
        }
    }
    return true;
}

void swapTile(int p, int q) {
    if (!gameStarted) {
        startTime = time(0);
        gameStarted = true;
    }
    swap(arr[emptyX][emptyY], arr[p][q]);
    emptyX = p;
    emptyY = q;
    moves++;
}

void draw(bool win = false, int timeTaken = 0) {
    int timer = time(0) - startTime;
    title(true);
    for (int i = 0; i < sizeN * 3; i++)
        cout << "-";
    cout << "-------------------------" << endl;
    for (int i = 0; i < sizeN; i++) {
        for (int j = 0; j < sizeN; j++) {
            if (arr[i][j] == 0)
                cout << "   ";
            else
                cout << (arr[i][j] < 10 ? " " : "") << arr[i][j] << " ";
        }
        if (i == 0)
            cout << "         Moves: " << moves;
        if (i == 1)
            cout << "         Time : " << (gameStarted ? timer : 0) << "s";
        cout << endl;
    }
    for (int i = 0; i < sizeN * 3; i++)
        cout << "-";
    cout << "-------------------------" << endl;
    if (win) {
        cout << "You win! in " << timeTaken
             << (timeTaken == 1 ? " second." : " seconds.") << endl;
    } else {
        cout << "tips: ";
        if (timer % 5 == 0)
            five_seconds = !five_seconds;
        if (five_seconds)
            cout << "Refill if its impossible." << endl;
        else
            cout << "Use arrow key to move!" << endl;
        for (int i = 0; i < sizeN * 3; i++)
            cout << "-";
        cout << "-------------------------" << endl;
        cout << "[R] Restart [E] Refill" << endl;
        cout << "[B] Back    [Esc] Exit" << endl;
    }
}

// Modified function to save score to a single encrypted database file
void saveScore(string name, int timeTaken, int moves) {
    name.resize(6, ' ');
    const string filename = "highscores.dat";

    // Map to store all highscores by size
    map<int, vector<pair<int, pair<int, string>>>> allScores;

    // Try to read existing scores
    ifstream fileIn(filename, ios::binary);
    if (fileIn.is_open()) {
        string encryptedData((istreambuf_iterator<char>(fileIn)),
                             istreambuf_iterator<char>());
        fileIn.close();

        if (!encryptedData.empty()) {
            string decryptedData = encryptDecrypt(encryptedData);
            stringstream ss(decryptedData);

            int size, count;
            while (ss >> size >> count) {
                vector<pair<int, pair<int, string>>> sizeScores;
                for (int i = 0; i < count; i++) {
                    string playerName;
                    int t, m;
                    ss >> playerName >> t >> m;
                    sizeScores.push_back({t, {m, playerName}});
                }
                allScores[size] = sizeScores;
            }
        }
    }

    // Add new score
    allScores[sizeN].push_back({timeTaken, {moves, name}});

    // Sort scores for current size
    sort(allScores[sizeN].begin(), allScores[sizeN].end());

    // Prepare data for saving
    stringstream dataToSave;
    for (const auto &sizeEntry : allScores) {
        dataToSave << sizeEntry.first << " " << sizeEntry.second.size() << " ";
        for (const auto &score : sizeEntry.second) {
            dataToSave << score.second.second << " " << score.first << " "
                       << score.second.first << " ";
        }
    }

    // Encrypt and save
    string encryptedData = encryptDecrypt(dataToSave.str());
    ofstream fileOut(filename, ios::binary);
    fileOut.write(encryptedData.c_str(), encryptedData.size());
    fileOut.close();
}

// Modified function to show high scores from the unified database
void showHighScores() {
    const string filename = "highscores.dat";
    vector<pair<int, pair<int, string>>> scores;

    ifstream file(filename, ios::binary);
    if (file.is_open()) {
        string encryptedData((istreambuf_iterator<char>(file)),
                             istreambuf_iterator<char>());
        file.close();

        if (!encryptedData.empty()) {
            string decryptedData = encryptDecrypt(encryptedData);
            stringstream ss(decryptedData);

            int size, count;
            while (ss >> size >> count) {
                if (size == sizeN) {
                    for (int i = 0; i < count; i++) {
                        string name;
                        int timeTaken, moves;
                        ss >> name >> timeTaken >> moves;
                        scores.push_back({timeTaken, {moves, name}});
                    }
                    break; // Found our size, no need to continue
                } else {
                    // Skip scores for other sizes
                    for (int i = 0; i < count; i++) {
                        string name;
                        int timeTaken, moves;
                        ss >> name >> timeTaken >> moves;
                    }
                }
            }
        }
    }

    sort(scores.begin(), scores.end());

    cout << "      High Scores (Size " << sizeN << ")" << endl;
    cout << "--------------------------------" << endl;
    int idx = 1;
    for (auto &score : scores) {
        for (int i = 0; score.second.second.length() < 6; i++)
            score.second.second += " ";
        cout << idx++ << (idx < 11 ? "  | " : " | ") << score.second.second
             << (score.first < 10
                     ? " |    "
                     : (score.first < 100
                            ? " |   "
                            : (score.first < 1000 ? " |  " : " | ")))
             << score.first
             << (score.second.first < 10
                     ? "s |    "
                     : (score.second.first < 100
                            ? "s |   "
                            : (score.second.first < 1000 ? "s |  " : "s | ")))
             << score.second.first << " moves" << endl;
        if (idx > 10)
            break;
    }
    if (idx == 1)
        cout << "No high scores yet." << endl;
}

void updateTimeDisplay() {
    while (gameRunning) {
        {
            lock_guard<mutex> lock(gameMutex);
            if (gameStarted && !gameWon && isInGame)
                draw();
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
}

bool menu() {
    title();
    cout << "[q] Play" << endl;
    cout << "[w] High Scores" << endl;
    cout << "[Esc] Exit" << endl;
    while (true) {
        int key = getch();
        switch (key) {
        case 'q':
            do {
                title();
                cout << "Choose size n-puzzle\n[2][3][4][5][6][7][8][9]"
                     << endl;
                key = getch();
                sizeN = key - '0';
            } while (sizeN < 2 || sizeN > 9);
            return false;
            break;
        case 'w':
            do {
                title();
                cout << "Choose size n-puzzle\n[2][3][4][5][6][7][8][9]"
                     << endl;
                key = getch();
                sizeN = key - '0';
                cout << "-------------------------" << endl;
            } while (sizeN < 2 || sizeN > 9);
            title(false, 32);
            showHighScores();
            cout << "--------------------------------" << endl;
            cout << "[B] Back" << endl;
            key = getch();
            if (key == 'b' || key == 'B')
                menu();
            return false;
            break;
        case 27:
            return true;
            break;
        }
    }
}

int main() {
    thread timeThread(updateTimeDisplay);

    while (true) {
        if (!restart) {
            if (menu()) {
                gameRunning = false;
                timeThread.join();
                return 0;
            }
            arr.assign(sizeN, vector<int>(sizeN));
            fillArray();
            arrSave = arr;
            emptyXSave = emptyX;
            emptyYSave = emptyY;
        } else {
            arr = arrSave;
            gameStarted = false;
            moves = 0;
            emptyX = emptyXSave;
            emptyY = emptyYSave;
            restart = false;
        }
        {
            lock_guard<mutex> lock(gameMutex);
            isInGame = true;
            gameWon = false;
            gameStarted = false;
        }

        while (true) {
            {
                lock_guard<mutex> lock(gameMutex);

                draw();
                if (isSolved()) {
                    gameWon = true;
                    int timeTaken = time(0) - startTime;
                    (timeTaken > 170000000 ? timeTaken = 0 : timeTaken);
                    draw(true, timeTaken);
                    cout << "-------------------------------" << endl;
                    cout << "Enter name: ";
                    string name;
                    cin >> name;
                    saveScore(name, timeTaken, moves);
                    cout << "-------------------------------" << endl;
                    cout << "[H] Show highscores" << endl;
                    cout << "[B] Back [Esc] Exit" << endl;
                    while (true) {
                        int key = getch();
                        if (key == 'h' || key == 'H') {
                            title(false, 32);
                            showHighScores();
                            cout << "-------------------------------" << endl;
                            cout << "[B] Back" << endl;
                            while (true) {
                                int key = getch();
                                if (key == 'b' || key == 'B')
                                    break;
                            }
                            break;
                        }
                        if (key == 27) {
                            gameRunning = false;
                            timeThread.join();
                            return 0;
                        }
                        if (key == 'b' || key == 'B') {
                            isInGame = false;
                            break;
                        }
                    }
                    break;
                }
            }

            int key = getch();
            if (key == 'e' || key == 'E') {
                lock_guard<mutex> lock(gameMutex);
                fillArray();
                arrSave = arr;
                emptyXSave = emptyX;
                emptyYSave = emptyY;
            } else if (key == 'r' || key == 'R') {
                lock_guard<mutex> lock(gameMutex);
                restart = true;
                break;
            } else if (key == 'b' || key == 'B') {
                lock_guard<mutex> lock(gameMutex);
                isInGame = false;
                gameStarted = false;
                break;
            } else if (key == 27) {
                gameRunning = false;
                timeThread.join();
                return 0;
            } else {
                lock_guard<mutex> lock(gameMutex);
                switch (key) {
                case 72: // Up
                    if (emptyX < sizeN - 1)
                        swapTile(emptyX + 1, emptyY);
                    break;
                case 75: // Left
                    if (emptyY < sizeN - 1)
                        swapTile(emptyX, emptyY + 1);
                    break;
                case 77: // Right
                    if (emptyY > 0)
                        swapTile(emptyX, emptyY - 1);
                    break;
                case 80: // Down
                    if (emptyX > 0)
                        swapTile(emptyX - 1, emptyY);
                    break;
                }
            }
        }
    }
}