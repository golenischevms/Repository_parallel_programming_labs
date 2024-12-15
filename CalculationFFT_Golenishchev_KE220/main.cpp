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

// Вычисление БПФ с OpenACC + OpenMP
void my_compute_fft(const vector<double> &u, vector<double> &frequencies, vector<double> &amplitudes) {
    int N = u.size();
    vector<complex<double>> fft_result(N);

#pragma acc data copyin(u[0:N]) copyout(fft_result[0:N])
#pragma acc parallel loop
    for (int k = 0; k < N; ++k) {
        complex<double> sum(0.0, 0.0);

// Параллельная обработка
#pragma omp parallel
        {
            complex<double> local_sum(0.0, 0.0); // Локальная переменная для суммы

// Каждый поток выполняет вычисления для определённой части данных
#pragma omp for
            for (int n = 0; n < N; ++n) {
                double angle = -2.0 * M_PI * k * n / N;
                local_sum += u[n] * complex<double>(cos(angle), sin(angle));
            }

// Суммируем локальные суммы в общую
#pragma omp critical
            sum += local_sum;
        }
        fft_result[k] = sum;
    }

    // Рассчитываем частоты и амплитуды
    double freq_step = 1.0 / (N * (1.0 / u.size()));
    for (int k = 0; k < N; ++k) {
        frequencies.push_back(k * freq_step);
        amplitudes.push_back(abs(fft_result[k]) / N);
    }
}

// БПФ без ускорителей (последовательный)
void serial_compute_fft(const vector<double> &u, vector<double> &frequencies, vector<double> &amplitudes) {
    int N = u.size();
    vector<complex<double>> fft_result(N);

    for (int k = 0; k < N; ++k) {
        complex<double> sum(0.0, 0.0);
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            sum += u[n] * complex<double>(cos(angle), sin(angle));
        }
        fft_result[k] = sum;
    }

    double freq_step = 1.0 / (N * (1.0 / u.size()));
    for (int k = 0; k < N; ++k) {
        frequencies.push_back(k * freq_step);
        amplitudes.push_back(abs(fft_result[k]) / N);
    }
}

// БПФ с использованием FFTW
void fftw_compute_fft(const vector<double> &u, vector<double> &frequencies, vector<double> &amplitudes) {
    int N = u.size();
    vector<complex<double>> fft_result(N);
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < N; ++i) {
        in[i][0] = u[i];
        in[i][1] = 0.0;
    }

    fftw_execute(plan);

    double freq_step = 1.0 / (N * (1.0 / u.size()));
    for (int k = 0; k < N; ++k) {
        frequencies.push_back(k * freq_step);
        amplitudes.push_back(sqrt(out[k][0] * out[k][0] + out[k][1] * out[k][1]) / N);
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}


int main() {
    // Тестирование на синтетических данных
    // cout << "Генерирую данные" << endl;
    // const int m = 1000;
    // sample_signal(my_signal, m, t, u);

    // Тестирование на данных из файла
    cout << "Загружаю данные из файла" << endl;
    read_csv("/home/golenischevms/CalculationFFT_Golenishchev_KE220/input_data/dark.csv", t, u, 500000);
    cout << "Данные загрузил" << endl;

    vector<double> f_serial, a_serial;
    vector<double> f_my, a_my;
    vector<double> f_fftw, a_fftw;

    cout << "Размер вектора данных (количество значений напряжения и времени): " << u.size() << endl;

    cout << "Вычисляю последовательно БПФ" << endl;
    auto start = chrono::high_resolution_clock::now();
    serial_compute_fft(u, f_serial, a_serial);
    auto end = chrono::high_resolution_clock::now();
    cout << "Время serial_compute_fft: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;
    write_csv("/home/golenischevms/CalculationFFT_Golenishchev_KE220/output_data/serial_results.csv", t, u, f_serial, a_serial);

    cout << "Вычисляю БПФ с помощью OpenMP и OpenACC" << endl;
    start = chrono::high_resolution_clock::now();
    my_compute_fft(u, f_my, a_my);
    end = chrono::high_resolution_clock::now();
    cout << "Время my_compute_fft: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;
    write_csv("/home/golenischevms/CalculationFFT_Golenishchev_KE220/output_data/my_results.csv", t, u, f_my, a_my);

    cout << "Вычисляю БПФ с помощью средств библиотеки FFTW" << endl;
    start = chrono::high_resolution_clock::now();
    fftw_compute_fft(u, f_fftw, a_fftw);
    end = chrono::high_resolution_clock::now();
    cout << "Время fftw_compute_fft: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " мкс" << endl;
    write_csv("/home/golenischevms/CalculationFFT_Golenishchev_KE220/output_data/fftw_results.csv", t, u, f_fftw, a_fftw);

    cout << "Готово." << endl;
    return 0;
}
