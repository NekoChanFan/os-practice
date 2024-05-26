#include <Windows.h>
#include <fstream>
#include <iostream>
#include <thread>

constexpr auto MSIZE = 10000;

using namespace std;

int user = 0;

HANDLE sem1 = CreateSemaphore(NULL, 0, 1, L"sem1");
HANDLE sem2 = CreateSemaphore(NULL, 0, 1, L"sem2");
HANDLE hFile = CreateFile(
    L"C:\\Users\\Guolinang\\source\\repos\\OpSys3\\x64\\Debug\\text.txt",
    GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
    NULL);
const LPCWSTR shared_memory_name = L"MySharedMemory";
const int shared_memory_size = 1024;
HANDLE shared_memory_handle =
    CreateFileMapping(INVALID_HANDLE_VALUE, // используется для файла
                      NULL,                 // не защищается
                      PAGE_READWRITE, // доступ к
                      0, // максимальный размер, используется 0
                      shared_memory_size, // размер разделяемой памяти
                      shared_memory_name);
char *shared_memory =
    (char *)MapViewOfFile(shared_memory_handle, // дескриптор разделяемой памяти
                          FILE_MAP_ALL_ACCESS, // доступ к странице
                          0,                   // offset high
                          0,                   // offset low
                          0);                  // number of bytes to map

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
    cout << "wrong parametr";
    return 1;
  }

  thread readThread(reader);
  thread writeThread(writer);
  readThread.join();
  writeThread.join();
}
