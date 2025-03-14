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

int sizeMatrix;
vector<vector<int>> arr;
int emptyX, emptyY;
int moves = 0;
time_t startTime;
bool gameStarted = false;
bool gameRunning = true;
bool gameWon = false;
bool isInGame = false;

mutex gameMutex;

void fillArray() {
    vector<int> numbers;
    for (int i = 0; i < sizeMatrix * sizeMatrix; i++)
        numbers.push_back(i);

    srand(time(0));
    random_shuffle(numbers.begin(), numbers.end());

    int idx = 0;
    for (int i = 0; i < sizeMatrix; i++) {
        for (int j = 0; j < sizeMatrix; j++) {
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

bool cekMenang() {
    int idx = 1;
    for (int i = 0; i < sizeMatrix; i++) {
        for (int j = 0; j < sizeMatrix; j++) {
            if (arr[i][j] != idx++ && idx <= sizeMatrix * sizeMatrix)
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

void swap(int input) {
    switch (input) {
    case 72: // Up
        if (emptyX < sizeMatrix - 1)
            swapTile(emptyX + 1, emptyY);
        break;
    case 75: // Left
        if (emptyY < sizeMatrix - 1)
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

void draw(bool win = false, int timeTaken = 0) {
    system("cls");
    cout << " N-Puzzle by @FechL" << endl;
    for (int i = 0; i < sizeMatrix * 3; i++)
        cout << "-";
    cout << "----------------" << endl;
    for (int i = 0; i < sizeMatrix; i++) {
        for (int j = 0; j < sizeMatrix; j++) {
            if (arr[i][j] == 0)
                cout << "   ";
            else
                cout << (arr[i][j] < 10 ? " " : "") << arr[i][j] << " ";
        }
        if (i == 0)
            cout << "   Moves: " << moves;
        if (i == 1)
            cout << "   Time : " << (gameStarted ? time(0) - startTime : 0)
                 << "s";
        cout << endl;
    }
    for (int i = 0; i < sizeMatrix * 3; i++)
        cout << "-";
    cout << "----------------" << endl;
    if (win) {
        cout << "You win! in " << timeTaken;
        if (timeTaken == 1)
            cout << " second!" << endl;
        else
            cout << " seconds!" << endl;
    } else {
        cout << "Use arrow key to move!" << endl;
    }

    cout << "[R] Restart [B] Back" << endl;
    cout << "[E] Refill  [Esc] Exit" << endl;
}

void saveScore(string name, int timeTaken, int moves) {
    name.resize(6, ' ');
    string filename = "highscores_" + to_string(sizeMatrix) + ".txt";
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
    string filename = "highscores_" + to_string(sizeMatrix) + ".txt";
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

    cout << "\nHigh Scores (Size " << sizeMatrix << "):" << endl;
    cout << "----------------------" << endl;
    for (auto &score : scores) {
        cout << score.second.second << " - " << score.first << "s - "
             << score.second.first << " moves" << endl;
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

int main() {
    thread timeThread(updateTimeDisplay);

    while (true) {
        do {
            system("cls");
            cout << " N-Puzzle by @FechL" << endl;
            cout << "----------------------" << endl;
            cout << "Size n-puzzle (2-9): ";
            cin >> sizeMatrix;
        } while (sizeMatrix < 2 || sizeMatrix > 9);

        arr.assign(sizeMatrix, vector<int>(sizeMatrix));
        fillArray();
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
                if (cekMenang()) {
                    gameWon = true;
                    int timeTaken = time(0) - startTime;
                    draw(true, timeTaken);
                    cout << "----------------------" << endl;
                    cout << "Enter name: ";
                    string name;
                    cin >> name;
                    saveScore(name, timeTaken, moves);
                    showHighScores();
                    cout << "----------------------" << endl;
                    cout << "[B] Back  [Esc] Exit" << endl;
                    while (true) {
                        int key = getch();
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
            if (key == 'r' || key == 'R' || key == 'e' || key == 'E') {
                lock_guard<mutex> lock(gameMutex);
                fillArray();
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
                swap(key);
            }
        }
    }
}