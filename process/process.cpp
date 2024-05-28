#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <chrono>

using namespace std;

mutex cout_mutex;

void echo(const string& str, int period, int duration, int num) {
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

long long sum(int x) {
    long long total = 0;
    for (int i = 1; i <= x; ++i) {
        total = (total + i) % 1000000;
    }
    return total;
}

void sum_command(int x, int m, int num) {
    for (int i = 0; i < num; ++i) {
        vector<thread> threads;
        vector<long long> results(m);
        int chunk_size = x / m;

        for (int j = 0; j < m; ++j) {
            threads.push_back(thread([j, chunk_size, &results]() {
                long long local_sum = 0;
                for (int k = j * chunk_size + 1; k <= (j + 1) * chunk_size; ++k) {
                    local_sum = (local_sum + k) % 1000000;
                }
                results[j] = local_sum;
                }));
        }

        for (auto& t : threads) {
            t.join();
        }

        long long total_sum = accumulate(results.begin(), results.end(), 0LL) % 1000000;
        lock_guard<mutex> lock(cout_mutex);
        cout << "Sum is " << total_sum << endl;
    }
}

void process_command(const string& command) {
    istringstream iss(command);
    string cmd;
    iss >> cmd;

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
        echo(str, period, duration, num);
    }
    else if (cmd == "dummy") {
        dummy();
    }
    else if (cmd == "gcd") {
        int x, y;
        iss >> x >> y;
        gcd_command(x, y);
    }
    else if (cmd == "prime") {
        int x;
        iss >> x;
        prime_command(x);
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
        thread(process_command, command).detach();
    }
    else {
        process_command(command);
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
