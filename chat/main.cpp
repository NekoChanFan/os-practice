#include <Windows.h>
#include <fstream>
#include <iostream>
#include <thread>

using namespace std;

int user = 0;

HANDLE sem1 = CreateSemaphore(NULL, 0, 1, L"sem1");
HANDLE sem2 = CreateSemaphore(NULL, 0, 1, L"sem2");

void reader() {

  int anotherUser;
  if (user == 1)
    anotherUser = 2;
  else
    anotherUser = 1;

  while (true) {

    if (user == 1) {
      if (WaitForSingleObject(sem1, INFINITE) == WAIT_OBJECT_0) {
        ifstream i_file("text.txt");
        string text;
        i_file >> text;
        cout << "User " << anotherUser << ": " << text << "\n";
        i_file.close();
      }

    } else {

      if (WaitForSingleObject(sem2, INFINITE) == WAIT_OBJECT_0) {
        ifstream i_file("text.txt");
        string text;
        i_file >> text;
        cout << "User " << anotherUser << ": " << text << "\n";
        i_file.close();
      }
    }
  }
}

void writer() {

  while (true) {
    string text;
    // cout << "User " << user << ": ";
    cin >> text;
    ofstream o_file("text.txt");
    o_file << text << "\n";
    o_file.close();
    if (user == 1) {
      ReleaseSemaphore(sem2, 1, NULL);
    } else {
      ReleaseSemaphore(sem1, 1, NULL);
    }
  }
}

int main(int argc, char **args) {
  // user = 1;
  if (argc == 2)
    user = atoi(args[1]);
  else {
    cout << "wrong arguments";
    return 1;
  }
  thread rThread(reader);
  thread wThread(writer);
  rThread.join();
  wThread.join();
}
