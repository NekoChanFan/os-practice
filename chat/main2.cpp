#include <Windows.h>
#include <iostream>
#include <thread>

#define SHARED_FILE_PATH L"C:\\Users\\nekochan\\source\\repos\\OS\\x64\\Debug\\text.txt"

constexpr auto MSIZE = 10000;

using namespace std;

int user = 0;

HANDLE sem1 = CreateSemaphore(NULL, 0, 1, L"sem1");
HANDLE sem2 = CreateSemaphore(NULL, 0, 1, L"sem2");
HANDLE hFile;// = CreateFile(
    // SHARED_FILE_PATH,
    // GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
    // NULL);
// HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, MSIZE,
// NULL); char* shared_memory = (char*)MapViewOfFile(hMapFile,
// FILE_MAP_ALL_ACCESS, 0, 0, 0);
HANDLE hMapFile;
char *shared_memory;
char str[MSIZE];
int str_size;

void reader() {
  while (true) {
    if (user == 1) {
      if (WaitForSingleObject(sem1, INFINITE) == WAIT_OBJECT_0) {

        cout << "Second user "
             << ": " << shared_memory << endl;
      }

    } else {

      if (WaitForSingleObject(sem2, INFINITE) == WAIT_OBJECT_0) {

        cout << "Second user "
             << ": " << shared_memory << endl;
      }
    }
  }
}

void writer() {

  while (true) {
    // string text;
    // cout << "User " << user << ": ";
    // cin >> text;xc
    // str_size = text.length() * (sizeof(char))+1;
    // WriteFile(hFile,text.c_str(), str_size ,NULL,NULL);
    // shared_memory = (char*)text.c_str();
    // strcpy_s(shared_memory, str_size, text.c_str());
    cin >> shared_memory;
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

  if (user == 1) {
    hFile = CreateFile(L"text.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL,
                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, MSIZE,
                                 L"SharedMemory");
  } else {
    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"SharedMemory");
  }
  shared_memory = (char *)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

  thread readThread(reader);
  thread writeThread(writer);
  readThread.join();
  writeThread.join();
}
