#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unistd.h>

#define i64 long long int

void row_by_col(i64 *row, i64 *col, size_t size, i64 *dest) {
  *dest = 0;
  for (size_t i = 0; i < size; ++i) {
    *dest += row[i] * col[i];
  }
}

// Умножение нескольких строк матрицы А на столбцы матрицы В, в результате ответ
// записан в соответствующие строки матрицы С
void multiple_row_by_col(i64 **A, i64 **B_trans, i64 **C, size_t size,
                         i64 line_from, i64 line_count) {
  for (size_t line = line_from; line < line_from + line_count; ++line) {
    for (size_t i = 0; i < size; ++i) {
      if (line >= size)
        return;
      C[line][i] = 0;

      for (size_t j = 0; j < size; ++j) {
        C[line][i] += A[line][j] * B_trans[i][j];
      }
    }
  }
}

void transpose(i64 **matrix, size_t size) {
  i64 tmp = 0;
  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < i; ++j) {
      tmp = matrix[i][j];
      matrix[i][j] = matrix[j][i];
      matrix[j][i] = tmp;
    }
  }
}

void matrix_allocate_fill_rand(i64 **&A, i64 **&B, size_t size) {
  A = new i64 *[size];
  B = new i64 *[size];
  for (size_t i = 0; i < size; ++i) {
    A[i] = new i64[size];
    B[i] = new i64[size];
  }

  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < size; ++j) {
      A[i][j] = rand() % 100;
      B[i][j] = rand() % 100;
    }
  }
}

void matrix_print(i64 **M, size_t size) {
  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; ++j) {
      std::cout << M[i][j] << " ";
    }
    std::cout << "\n";
  }
}

void matrix_delete(i64 **M, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    delete[] M[i];
  }
  delete[] M;
}

i64 **matrix_multiply_single_th(i64 **A, i64 **B_trans, size_t size) {
  i64 **C = new i64 *[size];
  for (size_t i = 0; i < size; ++i) {
    C[i] = new i64[size];
  }

  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j < size; ++j) {
      row_by_col(A[i], B_trans[j], size, &C[i][j]);
    }
  }

  return C;
}

i64 **matrix_multiply_multith(i64 **A, i64 **B_trans, size_t size,
                              size_t line_count) {
  i64 **C = new i64 *[size];
  for (size_t i = 0; i < size; ++i) {
    C[i] = new i64[size];
  }

  size_t th_amount = ceil((double)(size) / line_count);
  std::thread *th = new std::thread[th_amount];

  for (size_t line_from = 0, j = 0; j < th_amount;
       line_from += line_count, ++j) {
    th[j] = std::thread(multiple_row_by_col, A, B_trans, C, size, line_from,
                        line_count);
  }

  for (size_t i = 0; i < th_amount; ++i) {
    th[i].join();
  }

  return C;
}

int main() {
  srand(time(NULL));
  size_t size = 0, line_count = 0;
  i64 **A = nullptr, **B = nullptr, **C = nullptr, **D = nullptr;
  std::cout << "Введите количество элементов в строке(столбце) матрицы\n";
  std::cin >> size;

  std::cout << "Введите количество строк, вычисляемых в одном потоке\n";
  std::cin >> line_count;

  char single_th;
  std::cout << "Запустить ли однопоточный расчёт(может занять время)?[y/n]\n";
  std::cin >> single_th;
  if (!(line_count && size)) {
    std::cout << "Введены некорректные данные. Перезапустите программу и "
                 "попробуйте ещё раз";
  }

  matrix_allocate_fill_rand(A, B, size);

  // std::cout << "Matrix A:\n";
  // matrix_print(A, size);
  // std::cout << "Matrix B:\n";
  // matrix_print(B, size);

  transpose(B, size);

  auto start = std::chrono::steady_clock::now();
  D = matrix_multiply_multith(A, B, size, line_count);
  auto end = std::chrono::steady_clock::now();

  std::cout << "\nМногопоточная реализация:\n";
  // matrix_print(D, size);
  std::cout << "Затрачено времени: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << " миллисекунд";

  if (single_th == 'y') {
    start = std::chrono::steady_clock::now();
    C = matrix_multiply_single_th(A, B, size);
    end = std::chrono::steady_clock::now();

    std::cout << "\nОднопоточная реализация:\n";
    // matrix_print(C, size);
    std::cout << "Времени затрачено: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       start)
                     .count()
              << " миллисекунд";
    matrix_delete(C, size);
  }

  matrix_delete(A, size);
  matrix_delete(B, size);
  matrix_delete(D, size);
  return 0;
}
