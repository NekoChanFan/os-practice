#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <unistd.h>      // Заменить на <windows.h> при запуске в Windows. Содержит sleep()

#define ITERATIONS_COUNT 20000

// Обыкновенное прибавление единицы при помощи операции инкремента
void increment(int &i) {
  sleep(1);
  ++i;
}

// Прибавление единицы, контролируя доступ к ресурсам при помощи мьютекса
void increment_with_mutex(int &i, std::mutex &mut) {
  sleep(1);
  mut.lock();
  ++i;
  mut.unlock();
}

// Прибавление единицы, используя атомарное сложение
void atomic_increment(std::atomic<int> &i) { 
  sleep(1);
  i.fetch_add(1);
}

// Прибавление 1 при помощи обыкновенного неатомарного оператора инкремента.
// Возможно отклонение от реального числа итераций
int basic_increment_case() {
  int i = 0;
  std::vector<std::thread> threads;
  for (int j = 0; j < ITERATIONS_COUNT; ++j) {
    threads.push_back(std::thread(increment, std::ref(i)));
  }

  for (auto &th : threads) {
    th.join();
  }
  return i;
}

// Контроль доступа к ресурсам при помощи мьютекса. Не возникает отклонений от
// реального числа итераций
int increment_with_mutex_case() {
  int i = 0;
  std::mutex mut;
  std::vector<std::thread> threads;
  for (int j = 0; j < ITERATIONS_COUNT; ++j) {
    threads.push_back(
        std::thread(increment_with_mutex, std::ref(i), std::ref(mut)));
  }

  for (auto &th : threads) {
    th.join();
  }
  return i;
}

// Использование атомарной операции сложения. Не возникает отклонений от
// реального числа итераций
int atomic_increment_case() {
  std::vector<std::thread> threads;
  std::atomic<int> i = 0;
  for (int j = 0; j < ITERATIONS_COUNT; ++j) {
    threads.push_back(std::thread(atomic_increment, ref(i)));
  }

  for (auto &th : threads) {
    th.join();
  }
  return i;
}

void compare_solutions() {
  int i = 0;
  std::vector<std::thread> threads;

  std::cout << "Ожидаемое значение счётчика " << ITERATIONS_COUNT;

  // Обыкновенный инкремент в многопотоке
  auto start = std::chrono::steady_clock::now();
  i = basic_increment_case();
  auto end = std::chrono::steady_clock::now();
  std::cout << "\nЗначение счётчика при наивном использовании инкремента " << i;
  std::cout << "\nЗатрачено времени: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                   .count();

  // Инкремент в многопотоке с использованием мьютекса
  start = std::chrono::steady_clock::now();
  i = increment_with_mutex_case();
  end = std::chrono::steady_clock::now();
  std::cout << "\nЗначение счётчика в случае использования мьютексов " << i;
  std::cout << "\nЗатрачено времени: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                   .count();

  // Инкремент в многопотоке с использованием std::atomic
  start = std::chrono::steady_clock::now();
  i = atomic_increment_case();
  end = std::chrono::steady_clock::now();
  std::cout << "\nЗначение счётчика в случае использования std::atomic " << i;
  std::cout << "\nЗатрачено времени: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                   .count();
}

int main() {
  compare_solutions();

  return 0;
}
