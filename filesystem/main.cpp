#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <mutex>
#include <string>

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
    printf("Inapropriate name");
    return false;
  }
  File *f = new File(name);
  this->createDeleteMt.lock();
  if (this->size + 1 > MAX_FILES) {
    this->createDeleteMt.unlock();
    printf("Too many files, creation failed");
    delete f;
    return false;
  }

  if (this->files.emplace(std::string(name), f).second == false) {
    this->createDeleteMt.unlock();
    printf("File with this name already exists");
    delete f;
    return false;
  };
  printf("File %s created\n",name);
  this->size++;

  this->createDeleteMt.unlock();
  return true;
}

bool Filesystem::f_delete(const char *name) {
  printf("delete: ");
  this->createDeleteMt.lock();
  auto it = this->files.find(name);
  if (it == this->files.end()) {
    printf("No file with such name\n");
    this->createDeleteMt.unlock();
    return false;
  }

  delete it->second->data;
  printf("File %s deleted\n", name);
  this->files.erase(name);
  return true;
}

void Filesystem::f_read(const char *name) {
  printf("read: ");
  auto it = this->files.find(name);
  if (it == this->files.end()) {
    printf("No file with such name\n");
    return;
  }

  it->second->mt.lock();
  printf("Reading file %s...\n"
         "Contents: %s\n",
         name, it->second->data);
  it->second->mt.unlock();
}

void Filesystem::f_write(const char *name, const char *data) {
  printf("write: ");
  auto it = this->files.find(name);
  if (it == this->files.end()) {
    printf("No file with such name\n");
    return;
  }

  it->second->mt.lock();
  delete it->second->data;
  it->second->data = new char[strlen(data)];
  it->second->size = strlen(data);
  if (!it->second->data) {
    perror("In write");
    return;
  }
  strcpy(it->second->data, name);
  printf("File %s was successfully written.\n"
         "Contents:%s,\tSize: %d\n",
         name, data, it->second->size);
  it->second->mt.unlock();
}

void Filesystem::f_find(const char *name) {
  printf("find: ");
  auto it = this->files.find(name);
  if (it == this->files.end()) {
    printf("No file with such name\n");
    return;
  }
  it->second->mt.lock();
  printf("File %s was successfully found. Size: %d\n", name, it->second->size);
  it->second->mt.unlock();
}

void Filesystem::fs_print_info() {
  printf("print_info: ");
  this->createDeleteMt.lock();
  printf("FILESYSTEM INFO\n"
         "name \t|\t size\n");
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

void make_random_action() {
  int action = rand() % 6;
  char *name = random_string(MAX_FILENAME_LENGTH, 0);

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
    printf("%d)",i);
    make_random_action();
  }
}

int main() {
  // srand(time(NULL));
  srand(4);
  user(10);
  return 0;
}
