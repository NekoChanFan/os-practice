#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#define MAX_FILES 10
#define MAX_FILENAME_LENGTH 32
#define MAX_DATA_LENGTH 24

struct File {
  std::string name;
  int size;
  char *data;
  std::mutex mt;

public:
  File(const char *name) {
    this->name = std::string(name);
    this->data = nullptr;
    this->size = 0;
  };
  ~File() { delete data; }
};

struct Filesystem {
  std::map<std::string, File *> files;
  unsigned int size;
  std::mutex createDeleteMt;

public:
  Filesystem() { this->size = 0; }
  ~Filesystem() {
    for (auto f : files) {
      delete f.second;
    }
  }

  void f_find(const char *name);
  void f_write(const char *name, const char *data);
  void f_read(const char *name);
  bool f_delete(const char *name);
  bool f_create(const char *name);
  void fs_print_info();
};

Filesystem FS = Filesystem();

bool Filesystem::f_create(const char *name) {
  printf("create: ");
  if (strlen(name) > 32 || strlen(name) < 1) {
    printf("Inapropriate name\n");
    return false;
  }
  File *f = new File(name);
  this->createDeleteMt.lock();
  if (this->size + 1 > MAX_FILES) {
    this->createDeleteMt.unlock();
    printf("Too many files, creation failed\n");
    delete f;
    return false;
  }

  if (this->files.emplace(std::string(name), f).second == false) {
    this->createDeleteMt.unlock();
    printf("File with this name already exists\n");
    delete f;
    return false;
  };
  printf("File %s created\n", name);
  this->size++;

  this->createDeleteMt.unlock();
  return true;
}

bool Filesystem::f_delete(const char *name) {
  printf("delete: ");
  char *temp = new char[strlen(name) + 1];
  strcpy(temp, name);
  this->createDeleteMt.lock();
  auto it = this->files.find(temp);
  if (it == this->files.end()) {
    this->createDeleteMt.unlock();
    printf("No file with such name\n");
    return false;
  }

  delete it->second->data;
  printf("File %s deleted\n", name);
  this->files.erase(name);
  this->size--;
  this->createDeleteMt.unlock();
  return true;
}

void Filesystem::f_read(const char *name) {
  printf("read: ");
  char *temp = new char[strlen(name) + 1];
  strcpy(temp, name);
  this->createDeleteMt.lock();
  auto it = this->files.find(temp);
  if (it == this->files.end()) {
    this->createDeleteMt.unlock();
    printf("No file with such name\n");
    return;
  }

  it->second->mt.lock();
  printf("Reading file %s...\n"
         "Contents: %s\n",
         name, it->second->data);
  it->second->mt.unlock();
  this->createDeleteMt.unlock();
}

void Filesystem::f_write(const char *name, const char *data) {
  printf("write: ");
  char *temp = new char[strlen(name) + 1];
  strcpy(temp, name);
  this->createDeleteMt.lock();
  auto it = this->files.find(temp);
  if (it == this->files.end()) {
    this->createDeleteMt.unlock();
    printf("No file with such name\n");
    return;
  }

  it->second->mt.lock();
  delete it->second->data;
  it->second->data = new char[strlen(data) + 1];
  it->second->size = strlen(data);
  if (!it->second->data) {
    it->second->mt.unlock();
    this->createDeleteMt.unlock();
    perror("In write");
    return;
  }
  strcpy(it->second->data, data);
  printf("File %s was successfully written.\n"
         "Contents:%s,\tSize: %d\n",
         name, data, it->second->size);
  it->second->mt.unlock();
  this->createDeleteMt.unlock();
}

void Filesystem::f_find(const char *name) {
  printf("find: ");
  char *temp = new char[strlen(name) + 1];
  strcpy(temp, name);
  this->createDeleteMt.lock();
  auto it = this->files.find(temp);
  if (it == this->files.end()) {
    printf("No file with such name\n");
    this->createDeleteMt.unlock();
    return;
  }
  it->second->mt.lock();
  printf("File %s was successfully found. Size: %d\n", name, it->second->size);
  it->second->mt.unlock();
  this->createDeleteMt.unlock();
}

void Filesystem::fs_print_info() {
  printf("print_info: ");
  printf("FILESYSTEM INFO\n"
         "name \t|\t size\n");
  this->createDeleteMt.lock();
  for (auto it : this->files) {
    printf("%s \t|\t %d\n", it.second->name.c_str(), it.second->size);
  }
  this->createDeleteMt.unlock();
}

char *random_string(int maxLength, bool spaces) {
  int randNum = 0;
  int length = rand() % maxLength + 1;
  char *str = new char[length];
  for (int i = 0; i < length; ++i) {
    if (spaces) {
      randNum = rand() % 28;
    } else {
      randNum = rand() % 26;
    }
    if (randNum < 26) {
      str[i] = randNum + 'a';
    } else if (randNum == 26) {
      str[i] = '\n';
    } else {
      str[i] = ' ';
    }
  }
  str[length] = '\0';

  return str;
}

char *existing_name() {
  FS.createDeleteMt.lock();
  if (FS.size == 0) {
    FS.createDeleteMt.unlock();
    return random_string(MAX_FILENAME_LENGTH, 0);
  }
  int pos = rand() % FS.files.size();
  int i = 0;
  for (auto it : FS.files) {
    if (i == pos) {
      const char *str = it.first.c_str();
      char *res = new char[it.first.length() + 1];
      strcpy(res, str);
      FS.createDeleteMt.unlock();
      return res;
    }
    ++i;
  }
  FS.createDeleteMt.unlock();
  return nullptr;
}

void make_random_action() {
  int action = rand() % 6;
  char *name;
  if (rand() % 2) {
    name = random_string(MAX_FILENAME_LENGTH, 0);
  } else {
    name = existing_name();
  }

  switch (action) {
  case 0:
    FS.fs_print_info();
    break;
  case 1:
    FS.f_find(name);
    break;
  case 2:
    FS.f_read(name);
    break;
  case 3:
    FS.f_write(name, random_string(MAX_DATA_LENGTH, 1));
    break;
  case 4:
    FS.f_create(name);
    break;
  case 5:
    FS.f_delete(name);
    break;
  }
  delete name;
}

void user(int actionsAmount) {
  for (int i = 0; i < actionsAmount; ++i) {
    printf("%d)", i);
    make_random_action();
  }
}

int main() {
  srand(time(NULL));
  // srand(4);
  auto th1 = std::thread(user, 10);
  int a = 10;
  auto th2 = std::thread(user, 10);
  th1.join();
  th2.join();
  return 0;
}
