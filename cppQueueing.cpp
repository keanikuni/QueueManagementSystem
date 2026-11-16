#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <io.h>
  #define IS_TTY() _isatty(_fileno(stdout))
#else
  #include <unistd.h>
  #define IS_TTY() isatty(STDOUT_FILENO)
#endif

using namespace std;

// ════════════════════════════════════════════════════════════════════════
//  QUEUE LOGIC
// ════════════════════════════════════════════════════════════════════════

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
    int getCounter() const { return counter; }
};

// ════════════════════════════════════════════════════════════════════════
//  TERMINAL STYLING
// ════════════════════════════════════════════════════════════════════════

const size_t WIDTH = 58;            // total width of the dashboard box
const size_t INNER = WIDTH - 4;     // usable text width inside the box

// Colors are switched off when output is piped or NO_COLOR is set.
bool useColor = false;

const string BOLD = "1", DIM = "2", RED = "31", GREEN = "32", YELLOW = "33", CYAN = "36";

string style(const string& text, const string& a = "", const string& b = "") {
    if (!useColor || a.empty()) return text;
    string codes = b.empty() ? a : a + ";" + b;
    return "\033[" + codes + "m" + text + "\033[0m";
}

// Length in visible characters (UTF-8 aware, so names like "Peña" pad correctly).
size_t vlen(const string& s) {
    size_t n = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) n++;
    return n;
}

string truncate(const string& s, size_t limit) {
    if (vlen(s) <= limit) return s;
    string out;
    size_t count = 0;
    for (size_t i = 0; i < s.size() && count < limit - 1; ) {
        unsigned char c = s[i];
        size_t len = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
        out += s.substr(i, len);
        i += len;
        count++;
    }
    return out + "…";
}

string repeat(const string& s, size_t n) {
    string out;
    for (size_t i = 0; i < n; i++) out += s;
    return out;
}

string padRight(const string& s, size_t width) {
    size_t len = vlen(s);
    return len >= width ? s : s + string(width - len, ' ');
}

string centered(const string& s, size_t width) {
    size_t len = vlen(s);
    if (len >= width) return s;
    size_t gap = width - len, left = gap / 2;
    return string(left, ' ') + s + string(gap - left, ' ');
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string ticketNo(int number) {
    ostringstream out;
    out << "#" << setfill('0') << setw(3) << number;
    return out.str();
}

string fmt(const Customer& c) {
    return ticketNo(c.number) + " - " + c.name;
}

string timeStr(time_t t) {
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t));
    return buf;
}

string humanDuration(double seconds) {
    long s = seconds < 0 ? 0 : (long)seconds;
    ostringstream out;
    if (s < 60)
        out << s << "s";
    else if (s < 3600)
        out << s / 60 << "m " << setfill('0') << setw(2) << s % 60 << "s";
    else
        out << s / 3600 << "h " << setfill('0') << setw(2) << (s % 3600) / 60 << "m";
    return out.str();
}

// ════════════════════════════════════════════════════════════════════════
//  BOX DRAWING
// ════════════════════════════════════════════════════════════════════════

void border(const string& left, const string& right) {
    cout << style(left + repeat("─", WIDTH - 2) + right, CYAN) << "\n";
}

// Print one line inside the box. `plain` is used to measure padding,
// `styled` (optional) is the colored version that is actually shown.
void row(string plain = "", string styled = "", bool center = false) {
    bool useStyled = !styled.empty();
    if (vlen(plain) > INNER) {
        plain = truncate(plain, INNER);
        useStyled = false;
    }
    size_t gap = INNER - vlen(plain);
    size_t left = center ? gap / 2 : 0;
    string bar = style("│", CYAN);
    cout << bar << " " << string(left, ' ') << (useStyled ? styled : plain)
         << string(gap - left, ' ') << " " << bar << "\n";
}

void statusRow(const string& label, const Customer* c, const string& color) {
    string value = c ? truncate(fmt(*c), INNER - 14) : "—";
    string plain = padRight(label, 14) + value;
    string styled = style(padRight(label, 14), DIM) +
                    (c ? style(value, BOLD, color) : style(value, DIM));
    row(plain, styled);
}

void statsRow(size_t waiting, size_t served, size_t issued) {
    vector<pair<string, size_t>> items = {
        {"Waiting", waiting}, {"Served", served}, {"Issued", issued}};
    string plain, styled;
    for (size_t i = 0; i < items.size(); i++) {
        if (i) { plain += "     "; styled += "     "; }
        string num = to_string(items[i].second);
        plain += items[i].first + " " + num;
        styled += style(items[i].first, DIM) + " " + style(num, BOLD);
    }
    row(plain, styled, true);
}

pair<string, string> menuCell(const string& key, const string& text, size_t width) {
    string plain = "[" + key + "] " + text;
    string styled = style("[" + key + "]", BOLD, YELLOW) + " " + text;
    return {padRight(plain, width), styled + string(width - plain.size(), ' ')};
}

void menuRow(const string& k1, const string& t1, const string& k2 = "", const string& t2 = "") {
    const size_t width = 27;
    auto left = menuCell(k1, t1, width);
    if (k2.empty()) {
        row(left.first, left.second);
    } else {
        auto right = menuCell(k2, t2, width);
        row(left.first + right.first, left.second + right.second);
    }
}

void drawDashboard(const QueueSystem& qs) {
    const Customer* current = qs.getServed().empty() ? nullptr : &qs.getServed().back();
    const Customer* upcoming = qs.getWaiting().empty() ? nullptr : &qs.getWaiting().front();

    border("╭", "╮");
    string title = "QUEUE MANAGEMENT SYSTEM";
    row(title, style(title, BOLD, CYAN), true);
    border("├", "┤");
    statusRow("NOW SERVING", current, GREEN);
    statusRow("NEXT IN LINE", upcoming, YELLOW);
    border("├", "┤");
    statsRow(qs.getWaiting().size(), qs.getServed().size(), qs.getCounter());
    border("├", "┤");
    menuRow("1", "Get a queue number", "4", "See who is next");
    menuRow("2", "Serve next customer", "5", "Cancel a queue number");
    menuRow("3", "View waiting line", "6", "View served history");
    menuRow("0", "Exit");
    border("╰", "╯");
}

// ════════════════════════════════════════════════════════════════════════
//  MESSAGES, TABLES, PROMPTS
// ════════════════════════════════════════════════════════════════════════

struct InputEnded {};   // thrown when stdin closes (Ctrl-D / end of file)

void ok(const string& message) {
    cout << "\n  " << style("✔", BOLD, GREEN) << " " << message << "\n";
}

void fail(const string& message) {
    cout << "\n  " << style("✖", BOLD, RED) << " " << message << "\n";
}

void printTable(const string& title, const vector<string>& headers,
                const vector<vector<string>>& rows, const vector<size_t>& widths) {
    cout << "\n  " << style(title, BOLD, CYAN) << "\n\n";
    string header;
    for (size_t i = 0; i < headers.size(); i++) {
        if (i) header += "  ";
        header += padRight(headers[i], widths[i]);
    }
    cout << "  " << style(header, DIM) << "\n";
    cout << "  " << style(repeat("─", vlen(header)), DIM) << "\n";
    for (const auto& r : rows) {
        string line;
        for (size_t i = 0; i < r.size(); i++) {
            if (i) line += "  ";
            line += padRight(truncate(r[i], widths[i]), widths[i]);
        }
        cout << "  " << line << "\n";
    }
}

void printWaiting(const QueueSystem& qs) {
    time_t now = time(nullptr);
    vector<vector<string>> rows;
    int pos = 1;
    for (const auto& c : qs.getWaiting())
        rows.push_back({to_string(pos++), ticketNo(c.number), c.name,
                        humanDuration(difftime(now, c.timeJoined))});
    printTable("WAITING LINE (" + to_string(rows.size()) + ")",
               {"Pos", "Ticket", "Name", "Waiting"}, rows, {3, 6, 24, 9});
}

void printServed(const QueueSystem& qs) {
    vector<vector<string>> rows;
    for (const auto& c : qs.getServed())
        rows.push_back({ticketNo(c.number), c.name, timeStr(c.timeJoined),
                        timeStr(c.timeServed),
                        humanDuration(difftime(c.timeServed, c.timeJoined))});
    printTable("SERVED HISTORY (" + to_string(rows.size()) + ")",
               {"Ticket", "Name", "Joined", "Served", "Waited"}, rows, {6, 22, 8, 8, 8});
}

void printTicket(const Customer& c, size_t ahead) {
    string position;
    if (ahead == 0)      position = "You're next!";
    else if (ahead == 1) position = "1 person ahead of you";
    else                 position = to_string(ahead) + " people ahead of you";

    vector<string> lines = {"YOUR QUEUE NUMBER", ticketNo(c.number),
                            truncate(c.name, 22), position};
    const size_t width = 26;
    cout << "\n  " << style("┌" + repeat("─", width) + "┐", GREEN) << "\n";
    for (size_t i = 0; i < lines.size(); i++) {
        string text = centered(lines[i], width);
        if (i == 1) text = style(text, BOLD, GREEN);
        cout << "  " << style("│", GREEN) << text << style("│", GREEN) << "\n";
    }
    cout << "  " << style("└" + repeat("─", width) + "┘", GREEN) << "\n";
}

string ask(const string& prompt) {
    cout << "\n  " << style("›", BOLD, CYAN) << " " << prompt << ": " << flush;
    string input;
    if (!getline(cin, input)) throw InputEnded();
    return trim(input);
}

void waitForEnter() {
    cout << style("\n  Press Enter to return to the menu...", DIM) << flush;
    string ignored;
    if (!getline(cin, ignored)) throw InputEnded();
}

void clearScreen() {
    if (IS_TTY())
        system(
#ifdef _WIN32
            "cls"
#else
            "clear"
#endif
        );
    else
        cout << "\n";
}

// ════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ════════════════════════════════════════════════════════════════════════

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);    // so box-drawing characters display correctly
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(console, &mode))
        SetConsoleMode(console, mode | 0x0004);   // enable ANSI colors
#endif
    useColor = IS_TTY() && !getenv("NO_COLOR");

    QueueSystem qs;

    try {
        while (true) {
            clearScreen();
            drawDashboard(qs);
            string choice = ask("Choose an option");

            try {
                if (choice == "1") {
                    string name = ask("Enter name");
                    Customer c = qs.generateNumber(name);
                    printTicket(c, qs.getWaiting().size() - 1);

                } else if (choice == "2") {
                    Customer c = qs.serveNext();
                    ok("Now serving: " + style(fmt(c), BOLD));

                } else if (choice == "3") {
                    if (qs.getWaiting().empty()) ok("No one is waiting.");
                    else printWaiting(qs);

                } else if (choice == "4") {
                    ok("Next in line: " + style(fmt(qs.peek()), BOLD));

                } else if (choice == "5") {
                    if (qs.getWaiting().empty())
                        throw out_of_range("The queue is empty.");
                    printWaiting(qs);
                    string input = ask("Enter queue number to cancel");
                    int number;
                    try {
                        size_t used = 0;
                        number = stoi(input, &used);
                        if (used != input.size()) throw invalid_argument("trailing text");
                    } catch (...) {
                        throw invalid_argument("Please enter a valid number.");
                    }
                    Customer c = qs.cancel(number);
                    ok("Cancelled: " + style(fmt(c), BOLD));

                } else if (choice == "6") {
                    if (qs.getServed().empty()) ok("No one has been served yet.");
                    else printServed(qs);

                } else if (choice == "0") {
                    break;

                } else {
                    fail("Invalid option. Please choose 0-6.");
                }

            } catch (const invalid_argument& e) {
                fail(string("Invalid input: ") + e.what());
            } catch (const out_of_range& e) {
                fail(string("Error: ") + e.what());
            }

            waitForEnter();
        }
    } catch (const InputEnded&) {
        cout << "\n";   // end of input: leave cleanly
    }

    cout << "\n  " << style("Goodbye!", BOLD, CYAN) << " " << qs.getServed().size()
         << " served, " << qs.getWaiting().size() << " still waiting.\n\n";
    return 0;
}
