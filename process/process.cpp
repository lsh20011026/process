#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <chrono>

using namespace std;

mutex cout_mutex;

// 전역 변수로 실행 중인 프로세스 수를 관리
int running_processes = 0;

void print_prompt(const string& command) {
    lock_guard<mutex> lock(cout_mutex);
    cout << "prompt> " << command << endl;
}

void echo(const string& str, int period, int duration, int num) {
    // Running 메시지 출력
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Running: [" << ++running_processes << "B]" << endl;
        cout << "---------------------------" << endl;
    }

    vector<thread> threads;
    for (int i = 0; i < num; ++i) {
        threads.push_back(thread([str, period, duration]() {
            int iterations = duration / period;
            for (int t = 0; t < iterations; ++t) {
                {
                    lock_guard<mutex> lock(cout_mutex);
                    cout << str << endl;
                }
                this_thread::sleep_for(chrono::seconds(period));
            }
            }));
    }

    for (auto& t : threads) {
        t.join();
    }

    // 실행이 끝난 후 실행 중인 프로세스 수 감소
    {
        lock_guard<mutex> lock(cout_mutex);
        --running_processes;
    }
}

void dummy() {
    // 아무 일도 하지 않는 함수
}

int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

void gcd_command(int x, int y) {
    lock_guard<mutex> lock(cout_mutex);
    cout << "GCD of " << x << " and " << y << " is " << gcd(x, y) << endl;
}

vector<bool> sieve(int max) {
    vector<bool> is_prime(max + 1, true);
    is_prime[0] = is_prime[1] = false;
    for (int i = 2; i * i <= max; ++i) {
        if (is_prime[i]) {
            for (int j = i * i; j <= max; j += i) {
                is_prime[j] = false;
            }
        }
    }
    return is_prime;
}

void prime_command(int x) {
    vector<bool> is_prime = sieve(x);
    int count = count_if(is_prime.begin(), is_prime.end(), [](bool p) { return p; });
    lock_guard<mutex> lock(cout_mutex);
    cout << "Number of primes <= " << x << " is " << count << endl;
}

long long sum(int start, int end) {
    long long total = 0;
    for (int i = start; i <= end; ++i) {
        total += i;
    }
    return total;
}

void sum_command(int x, int m, int num) {
    for (int i = 0; i < num; ++i) {
        vector<thread> threads;
        vector<long long> results(m);
        int chunk_size = x / m;
        int remainder = x % m;

        for (int j = 0; j < m; ++j) {
            int start = j * chunk_size + 1;
            int end = (j + 1) * chunk_size;
            if (j == m - 1) {
                end += remainder;
            }
            threads.push_back(thread([start, end, &results, j]() {
                results[j] = sum(start, end);
                }));
        }

        for (auto& t : threads) {
            t.join();
        }

        long long total_sum = accumulate(results.begin(), results.end(), 0LL);
        lock_guard<mutex> lock(cout_mutex);
        cout << "Sum is " << total_sum << endl;
    }
}

void process_command(const string& command, bool is_background) {
    istringstream iss(command);
    string cmd;
    iss >> cmd;

    if (!is_background)
        print_prompt(command);

    if (is_background) {
        // Running 메시지 출력
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Running background command: [" << ++running_processes << "B]" << endl;
        }
    }

    if (cmd == "echo") {
        string str;
        int period = 1, duration = 100, num = 1;
        iss >> str;
        string option;
        while (iss >> option) {
            if (option == "-p") {
                iss >> period;
            }
            else if (option == "-d") {
                iss >> duration;
            }
            else if (option == "-n") {
                iss >> num;
            }
        }

        // 백그라운드 실행이면서 출력을 보장하기 위해 추가 처리
        if (is_background) {
            // 먼저 기본적인 정보를 출력
            cout << "echo " << str << " -p " << period << " -d " << duration << " -n " << num << " &" << endl;
        }

        // 명령을 실행
        echo(str, period, duration, num);

        // 실행이 끝난 후 실행 중인 프로세스 수 감소
        if (is_background) {
            lock_guard<mutex> lock(cout_mutex);
            --running_processes;
            cout << "---------------------------" << endl;
        }
    }
    else if (cmd == "dummy") {
        dummy();

        // 실행이 끝난 후 실행 중인 프로세스 수 감소
        if (is_background) {
            lock_guard<mutex> lock(cout_mutex);
            --running_processes;
            cout << "---------------------------" << endl;
        }
    }
    else if (cmd == "gcd") {
        int x, y;
        iss >> x >> y;
        gcd_command(x, y);

        // 실행이 끝난 후 실행 중인 프로세스 수 감소
        if (is_background) {
            lock_guard<mutex> lock(cout_mutex);
            --running_processes;
            cout << "---------------------------" << endl;
        }
    }
    else if (cmd == "prime") {
        int x;
        iss >> x;
        prime_command(x);

        // 실행이 끝난 후 실행 중인 프로세스 수 감소
        if (is_background) {
            lock_guard<mutex> lock(cout_mutex);
            --running_processes;
            cout << "---------------------------" << endl;
        }
    }
    else if (cmd == "sum") {
        int x, m = 1, num = 1;
        iss >> x;
        string option;
        while (iss >> option) {
            if (option == "-m") {
                iss >> m;
            }
            else if (option == "-n") {
                iss >> num;
            }
        }
        sum_command(x, m, num);

        // 실행이 끝난 후 실행 중인 프로세스 수 감소
        if (is_background) {
            lock_guard<mutex> lock(cout_mutex);
            --running_processes;
            cout << "---------------------------" << endl;
        }
    }
}

void process_line(const string& line) {
    bool background = false;
    string command = line;
    if (command.back() == '&') {
        background = true;
        command.pop_back(); // '&' 제거
    }

    if (background) {
        thread(process_command, command, true).detach();
    }
    else {
        process_command(command, false);
    }
}

int main() {
    ifstream file("commands.txt");
    string line;

    while (getline(file, line)) {
        process_line(line);
        this_thread::sleep_for(chrono::seconds(1)); // 각 명령어 사이에 1초 대기
    }
}
