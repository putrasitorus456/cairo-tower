#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <cmath>
#include <random>
#include <numeric>
#include <iomanip>
#include <chrono>

using namespace std;

double expovariate(double mean, mt19937 &gen, uniform_real_distribution<> &dist) {
    return -mean * log(1.0 - dist(gen));
}

tuple<int, double, double, double> simulate_day(double a, double s, mt19937 &gen, uniform_real_distribution<> &dist) {
    const int open_time = 10 * 60;
    const int close_time = 16 * 60;

    vector<double> arrivals;
    double current_time = open_time;

    while (current_time < close_time) {
        double inter_arrival_time = expovariate(a, gen, dist);
        current_time += inter_arrival_time;
        if (current_time < close_time) {
            arrivals.push_back(current_time);
        }
    }

    queue<double> privilege_queue;
    queue<double> regular_queue;
    int privilege_ticket_profit = 0;
    vector<double> waiting_times;
    vector<double> waiting_times_privilege;
    vector<double> waiting_times_regular;
    current_time = open_time;

    for (double arrival : arrivals) {
        while (current_time <= arrival && (!privilege_queue.empty() || !regular_queue.empty())) {
            if (!privilege_queue.empty()) {
                double person = privilege_queue.front();
                privilege_queue.pop();
                double wait_time = max(0.0, current_time - person);
                waiting_times_privilege.push_back(wait_time);
            } else if (!regular_queue.empty()) {
                double person = regular_queue.front();
                regular_queue.pop();
                double wait_time = max(0.0, current_time - person);
                waiting_times_regular.push_back(wait_time);
            }
            current_time += expovariate(s, gen, dist);
        }

        if (!regular_queue.empty() || !privilege_queue.empty()) {
            if (dist(gen) < 0.5) {
                privilege_queue.push(arrival);
                privilege_ticket_profit += 30;
                continue;
            }
        }

        regular_queue.push(arrival);
    }

    while (!privilege_queue.empty() || !regular_queue.empty()) {
        if (!privilege_queue.empty()) {
            double person = privilege_queue.front();
            privilege_queue.pop();
            double wait_time = max(0.0, current_time - person);
            waiting_times_privilege.push_back(wait_time);
        } else if (!regular_queue.empty()) {
            double person = regular_queue.front();
            regular_queue.pop();
            double wait_time = max(0.0, current_time - person);
            waiting_times_regular.push_back(wait_time);
        }
        current_time += expovariate(s, gen, dist);
    }

    waiting_times.insert(waiting_times.end(), waiting_times_privilege.begin(), waiting_times_privilege.end());
    waiting_times.insert(waiting_times.end(), waiting_times_regular.begin(), waiting_times_regular.end());

    double avg_waiting_all = waiting_times.empty() ? 0 : accumulate(waiting_times.begin(), waiting_times.end(), 0.0) / waiting_times.size();
    double avg_waiting_privilege = waiting_times_privilege.empty() ? 0 : accumulate(waiting_times_privilege.begin(), waiting_times_privilege.end(), 0.0) / waiting_times_privilege.size();
    double avg_waiting_regular = waiting_times_regular.empty() ? 0 : accumulate(waiting_times_regular.begin(), waiting_times_regular.end(), 0.0) / waiting_times_regular.size();

    return make_tuple(privilege_ticket_profit, avg_waiting_all, avg_waiting_privilege, avg_waiting_regular);
}

void simulate(int n, double a, double s) {
    double total_profit = 0;
    double total_waiting_all = 0;
    double total_waiting_privilege = 0;
    double total_waiting_regular = 0;

    random_device rd;
    mt19937 gen(static_cast<unsigned>(chrono::system_clock::now().time_since_epoch().count()));
    uniform_real_distribution<> dist(0.0, 1.0);

    for (int i = 0; i < n; ++i) {
        auto result = simulate_day(a, s, gen, dist);
        int profit = get<0>(result);
        double avg_all = get<1>(result);
        double avg_privilege = get<2>(result);
        double avg_regular = get<3>(result);

        total_profit += profit;
        total_waiting_all += avg_all;
        total_waiting_privilege += avg_privilege;
        total_waiting_regular += avg_regular;
    }

    cout << round(total_waiting_all / n / 10) * 10 << endl;
    cout << round(total_waiting_privilege / n / 10) * 10 << endl;
    cout << round(total_waiting_regular / n / 10) * 10 << endl;
}

int main() {
    int n;
    double a, s;

    cin >> n >> s >> a;

    if (n > 0 && s > 0 && a > 0) {
        simulate(n, a, s);
    } else {
        cout << "Invalid input. Please enter positive values for n, s, and a.\n";
    }

    return 0;
}