#include <algorithm>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

using namespace std;

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

void saveScore(string name, int timeTaken, int moves) {
    name.resize(6, ' ');
    string filename = "highscores_" + to_string(sizeN) + ".txt";
    vector<pair<int, pair<int, string>>> scores;

    ifstream fileIn(filename);
    string playerName;
    int t, m;
    while (fileIn >> playerName >> t >> m) {
        scores.push_back({t, {m, playerName}});
    }
    fileIn.close();

    scores.push_back({timeTaken, {moves, name}});
    sort(scores.begin(), scores.end());

    ofstream fileOut(filename);
    for (auto &score : scores)
        fileOut << score.second.second << " " << score.first << " "
                << score.second.first << endl;
}

void showHighScores() {
    string filename = "highscores_" + to_string(sizeN) + ".txt";
    ifstream file(filename);
    vector<pair<int, pair<int, string>>> scores;
    string name;
    int timeTaken, moves;

    while (file >> name >> timeTaken >> moves) {
        while (name.length() < 6)
            name += ' ';
        scores.push_back({timeTaken, {moves, name}});
    }
    file.close();

    sort(scores.begin(), scores.end());

    cout << "      High Scores (Size " << sizeN << ")" << endl;
    cout << "--------------------------------" << endl;
    int idx = 1;
    for (auto &score : scores) {
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
}

void updateTimeDisplay() {
    while (gameRunning) {
        {
            lock_guard<mutex> lock(gameMutex);
            if (gameStarted && !gameWon && isInGame) {
                draw();
            }
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
                cout << "Size n-puzzle (2-9): ";
                cin >> sizeN;
            } while (sizeN < 2 || sizeN > 9);
            return false;
            break;
        case 'w':
            do {
                title();
                cout << "High Scores (2-9): ";
                cin >> sizeN;
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
                                key = getch();
                                if (key == 'b' || key == 'B')
                                    menu();
                            }
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