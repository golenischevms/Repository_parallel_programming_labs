#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <complex>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <fftw3.h>
#include <openacc.h>
#include <omp.h>

using namespace std;

vector<double> t, u, f, a;


//////////////////////////////////////////////////////////////////////////////////////
/// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Функция для чтения данных из CSV-файла
bool read_csv(const string &filename, vector<double> &x, vector<double> &y, int max_lines = -1) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл " << filename << endl;
        return false;
    }

    x.clear();
    y.clear();

    string line;
    bool header_skipped = false;
    int line_count = 0;  // Счётчик обработанных строк

    while (getline(file, line)) {
        if (!header_skipped) {
            header_skipped = true;
            continue;  // Пропускаем первую строку (заголовок)
        }

        if (max_lines != -1 && line_count >= max_lines) {
            break;  // Прекращаем чтение, если достигнут лимит строк
        }

        stringstream ss(line);
        string time_str, ch1_str;

        if (getline(ss, time_str, ',') && getline(ss, ch1_str, ',')) {
            try {
                x.push_back(stod(time_str));
                y.push_back(stod(ch1_str));
            } catch (const invalid_argument &e) {
                cerr << "Ошибка: некорректные данные в строке: " << line << endl;
                continue;
            }
        }

        ++line_count;  // Увеличиваем счётчик строк
    }

    file.close();
    return true;
}

// Функция для записи данных в CSV-файл с полной очисткой
bool write_csv(const string &filename, const vector<double> &t, const vector<double> &u, const vector<double> &f, const vector<double> &a) {
    // Открытие файла с флагом ios::trunc для очистки перед записью
    ofstream file(filename, ios::trunc);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл " << filename << endl;
        return false;
    }

    // Запись заголовков в файл
    file << "t,u,f,a\n";

    // Находим максимальный размер векторов
    size_t max_size = max({t.size(), u.size(), f.size(), a.size()});
    for (size_t i = 0; i < max_size; ++i) {
        if (i < t.size()) file << t[i];
        file << ",";
        if (i < u.size()) file << u[i];
        file << ",";
        if (i < f.size()) file << f[i];
        file << ",";
        if (i < a.size()) file << a[i];
        file << "\n";
    }

    // Закрытие файла
    file.close();
    return true;
}

// Искусственный сигнал
double my_signal(double t) {
    return 3 * cos(2 * M_PI * 3 * t + M_PI / 4) + 2 * sin(2 * M_PI * 7 * t - M_PI / 6) +
           1.5 * cos(2 * M_PI * 12 * t) + 0.8 * sin(2 * M_PI * 20 * t + M_PI / 3);
}

// Функция для генерации сигнала
void sample_signal(double (*func)(double), int m, vector<double> &x, vector<double> &y) {
    x.clear();
    y.clear();

    double dt = 1.0 / m;

    for (int i = 0; i < m; ++i) {
        double time = i * dt;
        x.push_back(time);
        y.push_back(func(time));
    }
}


//////////////////////////////////////////////////////////////////////////////////////
/// РЕАЛИЗАЦИЯ ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


// Вычисление БПФ с OpenACC + OpenMP
void my_compute_fft(const vector<double> &u, vector<double> &frequencies, vector<double> &amplitudes, double time_step, int threads_omp, int gangs_acc) {
    int N = u.size();
    vector<complex<double>> fft_result(N);

// Основная обработка с использованием OpenACC и OpenMP
#pragma acc data copyin(u[0:N]) copyout(fft_result[0:N])
omp_set_num_threads(threads_omp);
#pragma omp parallel for
    for (int k = 0; k < N; ++k) {
        complex<double> sum(0.0, 0.0);

// Параллельная обработка по элементам внутри OpenACC
#pragma acc parallel loop num_gangs(gangs_acc) num_workers(32) vector_length(32)
#pragma acc parallel loop reduction(+:sum)
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            sum += u[n] * complex<double>(cos(angle), sin(angle));
        }

        fft_result[k] = sum;
    }

    // Инициализация векторов частот и амплитуд
    frequencies.resize(N);
    amplitudes.resize(N);

    // Расчет шага частоты
    double freq_step = 1.0 / (N * time_step);

    // Построение массивов частот и амплитуд
    for (int k = 0; k < N; ++k) {
        frequencies[k] = k * freq_step;
        amplitudes[k] = abs(fft_result[k]) / N;
    }
}


// БПФ без ускорителей (последовательный)
void serial_compute_fft(const vector<double> &u, vector<double> &frequencies, vector<double> &amplitudes, double delta_t) {
    int N = u.size();
    vector<complex<double>> fft_result(N);

    // Выполнение БПФ
    for (int k = 0; k < N; ++k) {
        complex<double> sum(0.0, 0.0);
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            sum += u[n] * complex<double>(cos(angle), sin(angle));
        }
        fft_result[k] = sum;
    }

    // Расчет частоты и амплитуд
    double freq_step = 1.0 / (N * delta_t); // Шаг частоты
    for (int k = 0; k < N; ++k) {
        frequencies.push_back(k * freq_step);
        amplitudes.push_back(abs(fft_result[k]) / N);
    }
}


// БПФ с использованием FFTW
void fftw_compute_fft(const vector<double> &u, vector<double> &frequencies, vector<double> &amplitudes, double delta_t) {
    int N = u.size();
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Заполнение входного массива
    for (int i = 0; i < N; ++i) {
        in[i][0] = u[i]; // Реальная часть
        in[i][1] = 0.0;  // Мнимая часть
    }

    // Выполнение FFT
    fftw_execute(plan);

    // Расчет частоты и амплитуд
    double freq_step = 1.0 / (N * delta_t); // Шаг частоты
    for (int k = 0; k < N; ++k) {
        frequencies.push_back(k * freq_step); // Частота
        amplitudes.push_back(sqrt(out[k][0] * out[k][0] + out[k][1] * out[k][1]) / N); // Амплитуда
    }

    // Освобождение ресурсов
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}


//////////////////////////////////////////////////////////////////////////////////////
/// ТЕСТИРОВАНИЕ /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Тест 1: Увеличиваем количество потоков для OpenACC + OpenMP и FFTW, фиксированное количество выборки
void test_openacc_omp_fftw_fixed_threads(const vector<double>& u, double time_step) {
    cout << "Тест 1: Увеличение числа потоков для OpenACC + OpenMP и FFTW с фиксированным размером выборки" << endl;

    // Массивы для хранения результатов
    vector<double> f_my, a_my, f_fftw, a_fftw;

    // Увеличиваем количество потоков (10, 50, 100, 500, 1000)
    for (int threads = 10; threads <= 1000; threads += 50) {
        // Вычисляем БПФ с OpenMP + OpenACC
        auto start = chrono::high_resolution_clock::now();
        my_compute_fft(u, f_my, a_my, time_step, threads, threads);
        auto end = chrono::high_resolution_clock::now();
        cout << "OpenACC + OpenMP, потоки: " << threads << ", время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;

        // Вычисляем БПФ с использованием FFTW
        start = chrono::high_resolution_clock::now();
        fftw_compute_fft(u, f_fftw, a_fftw, time_step);
        end = chrono::high_resolution_clock::now();
        cout << "FFTW, потоки: " << threads << ", время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;
    }
}

// Тест 2: Увеличиваем количество потоков для OpenACC (фиксированное количество потоков для OpenMP), FFTW
void test_openacc_omp_increase_openacc_threads(const vector<double>& u, double time_step) {
    cout << "Тест 2: Увеличение числа потоков для OpenACC, фиксированное количество потоков для OpenMP и FFTW" << endl;

    // Массивы для хранения результатов
    vector<double> f_my, a_my, f_fftw, a_fftw;


    // Увеличиваем количество потоков только для OpenACC (10, 50, 100, 500, 1000)
    for (int acc_gangs = 10; acc_gangs <= 1000; acc_gangs += 50) {
        // Вычисляем БПФ с OpenMP + OpenACC
        auto start = chrono::high_resolution_clock::now();
        my_compute_fft(u, f_my, a_my, time_step, 10, acc_gangs);
        auto end = chrono::high_resolution_clock::now();
        cout << "Увеличение gsngs OpenACC, gangs: " << acc_gangs << ", время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;

        // Вычисляем БПФ с использованием FFTW
        start = chrono::high_resolution_clock::now();
        fftw_compute_fft(u, f_fftw, a_fftw, time_step);
        end = chrono::high_resolution_clock::now();
        cout << "FFTW, время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;
    }
}

// Тест 3: Увеличиваем количество потоков для OpenMP, фиксированное количество gangs OpenACC, FFTW
void test_openacc_omp_increase_omp_threads(const vector<double>& u, double time_step) {
    cout << "Тест 3: Увеличение числа потоков для OpenMP, фиксированное количество потоков для OpenACC и FFTW" << endl;

    // Массивы для хранения результатов
    vector<double> f_my, a_my, f_fftw, a_fftw;

    // Увеличиваем количество потоков только для OpenMP
    for (int omp_threads = 10; omp_threads <= 1000; omp_threads += 50) {
        // Вычисляем БПФ с OpenMP + OpenACC
        auto start = chrono::high_resolution_clock::now();
        my_compute_fft(u, f_my, a_my, time_step, omp_threads, 10);
        auto end = chrono::high_resolution_clock::now();
        cout << "Увеличение числа потоков OpenMP, потоки: " << omp_threads << ", время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;

        // Вычисляем БПФ с использованием FFTW
        start = chrono::high_resolution_clock::now();
        fftw_compute_fft(u, f_fftw, a_fftw, time_step);
        end = chrono::high_resolution_clock::now();
        cout << "FFTW, время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;
    }
}

// Тест 4: Увеличиваем размер выборки (500, 1000, 5000, 10000, 50000, 100000) с фиксированными потоками
void test_increase_sample_size(const vector<double>& u, double time_step) {
    cout << "Тест 4: Увеличение размера выборки с фиксированным количеством потоков для OpenACC + OpenMP и FFTW" << endl;

    // Массивы для хранения результатов
    vector<double> f_my, a_my, f_fftw, a_fftw;

    // Увеличиваем размер выборки
    vector<int> sample_sizes = {500, 1000, 5000, 10000, 50000, 100000};
    for (int size : sample_sizes) {
        vector<double> u_sample(u.begin(), u.begin() + size);  // Выбираем подмножество данных

        // Вычисляем БПФ с ЦПУ
        auto start = chrono::high_resolution_clock::now();
        serial_compute_fft(u_sample, f_my, a_my, time_step);
        auto end = chrono::high_resolution_clock::now();
        cout << "Размер выборки: " << size << ", ЦПУ, время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;

        // Вычисляем БПФ с OpenACC + OpenMP
        start = chrono::high_resolution_clock::now();
        my_compute_fft(u_sample, f_my, a_my, time_step, 10, 10);
        end = chrono::high_resolution_clock::now();
        cout << "Размер выборки: " << size << ", OpenACC + OpenMP, время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;

        // Вычисляем БПФ с использованием FFTW
        start = chrono::high_resolution_clock::now();
        fftw_compute_fft(u_sample, f_fftw, a_fftw, time_step);
        end = chrono::high_resolution_clock::now();
        cout << "Размер выборки: " << size << ", FFTW, время: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;
    }
}

//////////////////////////////////////////////////////////////////////////////////////
/// ОСНОВНОЙ КОД /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

int main() {
    // Тестирование на синтетических данных
    // cout << "Генерирую данные" << endl;
    // const int m = 1000;
    // sample_signal(my_signal, m, t, u);

    // Тестирование на данных из файла
    cout << "Загружаю данные из файла" << endl;
    read_csv("/home/golenischevms/CalculationFFT_Golenishchev_KE220/input_data/dark.csv", t, u, 10000);  // Используем 10 000 значений
    cout << "Данные загружены" << endl;

    double time_step = 4e-10;  // Шаг времени
    //
    cout << "---------------------------------------------------" << endl;
    cout << "Тестриуется собственная реализация ДПФ и БПФ в FFTW" << endl;
    cout << "---------------------------------------------------" << endl;
    // Проведение тестов
    test_openacc_omp_fftw_fixed_threads(u, time_step);
    test_openacc_omp_increase_openacc_threads(u, time_step);
    test_openacc_omp_increase_omp_threads(u, time_step);
    test_increase_sample_size(u, time_step);

    cout << "Готово." << endl;
    return 0;
}
