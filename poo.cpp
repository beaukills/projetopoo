#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <ctime>
#include <cmath>
#include <cstdlib>

using namespace std;

struct Processo {
    int timestamp;
    int priority;
    int instructions;
    int memory_required;
    float io_rate;
    int remaining_instructions;

    Processo(int ts, int pri, int ins, int mem, double io)
        : timestamp(ts), priority(pri), instructions(ins), memory_required(mem), io_rate(io), remaining_instructions(ins) {}
};

struct Event {
    int timestamp;
    int type;
    Processo* processo;

    Event(int ts, int t, Processo* p)
        : timestamp(ts), type(t), processo(p) {}
};

struct CompareEvent {
    bool operator()(const Event* e1, const Event* e2) {
        return e1->timestamp > e2->timestamp;
    }
};

class SchedulerSimulator {
private:
    int quantum;
    int num_cpus;
    int cpu_speed;
    int ram_size;
    int swap_size;
    int global_time;
    priority_queue<Event*, vector<Event*>, CompareEvent> event_queue;
    vector<Processo*> ready_processos;
    vector<Processo*> waiting_processos;
    vector<double> cpu_usage_times;
    vector<double> cpu_idle_times;

public:
    SchedulerSimulator(string filename, int cpus, int speed, int ram, int q)
        : quantum(q), num_cpus(cpus), cpu_speed(speed), ram_size(ram), swap_size(ram / 2), cpu_usage_times(cpus, 0.0), cpu_idle_times(cpus, 0.0) {
        srand(time(NULL));

        loadProcessos(filename);

        simulate();
    }

    void loadProcessos(string filename) {
        ifstream file(filename);
        string aux;
        char lixo;
        if (file.is_open()) {
            int timestamp, priority, instructions, memory_required;
            float io_rate;
            cout << "file: " << filename << endl;
            while (getline(file, aux)) {
                sscanf(aux.c_str(), "%d%c%d%c%d%c%d%c%f", &timestamp, &lixo, &priority, &lixo, &instructions, &lixo, &memory_required, &lixo, &io_rate);
                Processo* processo = new Processo(timestamp, priority, instructions, memory_required, io_rate);

                cout << "Timestamp: " << timestamp << endl;

                ready_processos.push_back(processo);
                event_queue.push(new Event(timestamp, 0, processo));
            }
            file.close();
        } else {
            cerr << "Erro ao abrir o arquivo " << filename << endl;
            exit(1);
        }
    }

    void simulate() {
        global_time = 0;
        while (!event_queue.empty()) {
            Event* current_event = event_queue.top();
            event_queue.pop();

            global_time = current_event->timestamp;

            if (current_event->type == 0) {
                launchProcesso(current_event->processo);
            } else {
                completeIO(current_event->processo);
            }

            delete current_event;
        }

        displayStatistics();
    }

    void launchProcesso(Processo* processo) {
        if (ready_processos.empty()) {
            waiting_processos.push_back(processo);
        } else {
            Processo* running_processo = ready_processos.back();
            ready_processos.pop_back();

            executeProcesso(processo);

            ready_processos.push_back(running_processo);
        }
    }

    void completeIO(Processo* processo) {
        ready_processos.push_back(processo);
    }

    void executeProcesso(Processo* processo) {
        int cpu_index = rand() % num_cpus;

        double execution_time = ceil(static_cast<double>(processo->remaining_instructions) / cpu_speed);

        cpu_usage_times[cpu_index] += execution_time;

        event_queue.push(new Event(global_time + execution_time, 1, processo));

        double io_chance = static_cast<double>(rand()) / RAND_MAX;
        if (io_chance < processo->io_rate) {
            event_queue.push(new Event(global_time + 2 * quantum, 1, processo));
        }

        processo->remaining_instructions = 0;
    }

    void displayStatistics() {
        cout << "Estatísticas de uso das CPUs:" << endl;
        for (int i = 0; i < num_cpus; ++i) {
            double total_time = cpu_usage_times[i] + cpu_idle_times[i];
            cout << "CPU " << i + 1 << ": Uso: " << cpu_usage_times[i] << " segundos, Ociosidade: " << cpu_idle_times[i] << " segundos, Total: " << total_time << " segundos" << endl;
        }
    }
};

int main() {
    string filename = "processos.csv";
    int num_cpus = 4;
    int cpu_speed = 3000;
    int ram_size = 16;
    int quantum = 100;

    SchedulerSimulator simulator(filename, num_cpus, cpu_speed, ram_size, quantum);

    return 0;
}
