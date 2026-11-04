#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <limits>

using namespace std;

struct Customer {
    int number;
    string name;
    time_t timeJoined;
    time_t timeServed;
};

class QueueSystem {
private:
    deque<Customer> waiting;   // FIFO queue of waiting customers
    vector<Customer> served;   // history of served customers
    int counter = 0;           // last queue number issued

public:
    // Add a customer to the back of the queue and give them a number.
    Customer generateNumber(string name) {
        // trim spaces at both ends
        size_t start = name.find_first_not_of(" \t");
        size_t end = name.find_last_not_of(" \t");
        if (start == string::npos)
            throw invalid_argument("Name cannot be empty.");
        name = name.substr(start, end - start + 1);

        counter++;
        Customer c{counter, name, time(nullptr), 0};
        waiting.push_back(c);
        return c;
    }

    // Serve the customer at the front of the queue (FIFO).
    Customer serveNext() {
        if (waiting.empty())
            throw out_of_range("The queue is empty.");
        Customer c = waiting.front();
        waiting.pop_front();
        c.timeServed = time(nullptr);
        served.push_back(c);
        return c;
    }

    // Remove a specific queue number from the waiting line.
    Customer cancel(int number) {
        for (auto it = waiting.begin(); it != waiting.end(); ++it) {
            if (it->number == number) {
                Customer c = *it;
                waiting.erase(it);
                return c;
            }
        }
        throw out_of_range("Queue number " + to_string(number) + " was not found.");
    }

    // Look at who is next without removing them.
    Customer peek() const {
        if (waiting.empty())
            throw out_of_range("The queue is empty.");
        return waiting.front();
    }

    const deque<Customer>& getWaiting() const { return waiting; }
    const vector<Customer>& getServed() const { return served; }
};

string fmt(const Customer& c) {
    ostringstream out;
    out << "#" << setfill('0') << setw(3) << c.number << " - " << c.name;
    return out.str();
}

string timeStr(time_t t) {
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t));
    return buf;
}

void printMenu() {
    cout << "\n===== QUEUEING SYSTEM =====\n"
         << "1. Get a queue number\n"
         << "2. Serve next customer\n"
         << "3. View waiting line\n"
         << "4. See who is next\n"
         << "5. Cancel a queue number\n"
         << "6. View served history\n"
         << "0. Exit\n";
}

int main() {
    QueueSystem system;
    string choice;

    while (true) {
        printMenu();
        cout << "Choose an option: ";
        if (!getline(cin, choice)) break;  // end of input

        try {
            if (choice == "1") {
                cout << "Enter name: ";
                string name;
                getline(cin, name);
                Customer c = system.generateNumber(name);
                cout << "Your queue number is " << setfill('0') << setw(3) << c.number
                     << setfill(' ') << ". People ahead of you: "
                     << system.getWaiting().size() - 1 << "\n";

            } else if (choice == "2") {
                Customer c = system.serveNext();
                cout << "Now serving: " << fmt(c) << "\n";

            } else if (choice == "3") {
                const auto& line = system.getWaiting();
                if (line.empty()) {
                    cout << "No one is waiting.\n";
                } else {
                    cout << "Waiting (" << line.size() << "):\n";
                    int pos = 1;
                    for (const auto& c : line)
                        cout << "  " << pos++ << ". " << fmt(c) << "\n";
                }

            } else if (choice == "4") {
                Customer c = system.peek();
                cout << "Next in line: " << fmt(c) << "\n";

            } else if (choice == "5") {
                cout << "Enter queue number to cancel: ";
                string input;
                getline(cin, input);
                int number;
                try {
                    number = stoi(input);
                } catch (...) {
                    throw invalid_argument("Please enter a valid number.");
                }
                Customer c = system.cancel(number);
                cout << "Cancelled: " << fmt(c) << "\n";

            } else if (choice == "6") {
                const auto& history = system.getServed();
                if (history.empty()) {
                    cout << "No one has been served yet.\n";
                } else {
                    for (const auto& c : history)
                        cout << "  " << fmt(c) << " (served " << timeStr(c.timeServed) << ")\n";
                }

            } else if (choice == "0") {
                cout << "Goodbye!\n";
                break;

            } else {
                cout << "Invalid option. Please choose 0-6.\n";
            }

        } catch (const invalid_argument& e) {
            cout << "Invalid input: " << e.what() << "\n";
        } catch (const out_of_range& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}