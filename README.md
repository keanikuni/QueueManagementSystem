# QueueManagementSystem
Final project in lieu of CSPL 313 - Programming Languages

A simple **queue management system** for the terminal, written twice: once in **Python** and once in **C++**.
Think of the number-ticket machine at a bank or clinic: people take a number, wait, and get served in order.

This README is split into **two parts**. Each part shows the real code, block by block, with a plain-language explanation under every block.

| Part | File | Jump to |
|---|---|---|
| **Part 1** | `pythonQueueing.py` | [Python code blocks](#part-1--python-pythonqueueingpy) |
| **Part 2** | `cppQueueing.cpp` | [C++ code blocks](#part-2--c-cppqueueingcpp) |

---

## Before you read the code

### What the program does

| Option | What it does |
|---|---|
| `1` | **Get a queue number**: type a name and receive the next ticket number |
| `2` | **Serve next customer**: calls the person at the front of the line |
| `3` | **View waiting line**: shows everyone waiting, in order |
| `4` | **See who is next**: peeks at the front of the line without removing anyone |
| `5` | **Cancel a queue number**: removes someone from the line (they left) |
| `6` | **View served history**: shows everyone already served |
| `0` | **Exit** |

### How to run it

**Python** (needs Python 3, nothing else to install):
```bash
python pythonQueueing.py        # Windows
python3 pythonQueueing.py       # macOS / Linux
```

**C++** (needs a C++ compiler such as g++):
```bash
g++ -std=c++17 cppQueueing.cpp -o queue
./queue                         # macOS / Linux
queue.exe                       # Windows
```
With Visual Studio's *Developer Command Prompt*: `cl /EHsc /std:c++17 /utf-8 cppQueueing.cpp`, then run `cppQueueing.exe`.

> Run it in a real terminal (Windows Terminal, PowerShell, cmd, macOS Terminal, VS Code terminal) so colors and screen clearing work.

### The big idea: a FIFO queue

**FIFO** means **F**irst **I**n, **F**irst **O**ut: the first person to join is the first person served, just like a real line.

```text
  front (served first)                          back (newest)
        [ #001 Ana ] [ #002 Ben ] [ #003 Cara ]
             ^                                      ^
        serve_next() takes from here        generate_number() adds here
```

Both programs keep **two collections** and **one number**:

| Name | What it is | Purpose |
|---|---|---|
| `waiting` | a **deque** (double-ended queue) | people still waiting, in order. A deque is fast at both ends: adding at the back and removing from the front are both instant. |
| `served` | a list / vector | history of people already served |
| `counter` | a number | the last ticket number handed out. Each new customer gets `counter + 1`, so numbers never repeat, even after a cancel. |

Both programs are built in the same layers, each using the ones above it:

```text
 1. Queue logic         the "brain": the QueueSystem class (no printing at all)
 2. Styling helpers     colors and text formatting
 3. Box drawing         draws the dashboard
 4. Messages & prompts  tables, ticket card, asking for input
 5. Main loop           ties everything together
```

---

# Part 1 : Python (`pythonQueueing.py`)

## 1.1 Imports

```python
import os
import sys
from collections import deque
from datetime import datetime
```

- `os` lets the program ask about the operating system and clear the screen.
- `sys` lets it check whether output is going to a real terminal.
- `deque` is the double-ended queue used for the waiting line.
- `datetime` gives the current time, used to record when someone joined or was served.

## 1.2 Queue logic: `class QueueSystem`

This class holds all the data and rules. It **never prints anything**, which keeps the "brain" simple and separate from the display. Every block below is a method inside `class QueueSystem:`.

### `__init__`: start with an empty system

```python
def __init__(self):
    self.waiting = deque()      # FIFO queue for waiting customers
    self.served = []            # history ng served customers
    self.counter = 0            # last queue number issued
```

- Runs once, when `QueueSystem()` is created.
- Creates an empty `waiting` line, an empty `served` history, and sets `counter` to 0 (no tickets issued yet).

### `generate_number`: a customer joins the line

```python
def generate_number(self, name):
    """Add a customer to the back of the queue and give them a number."""
    name = name.strip()
    if not name:
        raise ValueError("Name cannot be empty.")
    self.counter += 1
    customer = {
        "number": self.counter,
        "name": name,
        "time_joined": datetime.now(),
    }
    self.waiting.append(customer)
    return customer
```

1. `name.strip()` removes spaces at both ends (`"  Ana "` becomes `"Ana"`).
2. If nothing is left, it raises `ValueError`, which stops the function and reports the problem.
3. `self.counter += 1` makes the next ticket number.
4. It builds a **dictionary** (a labeled bundle of values) holding the `number`, `name`, and `time_joined`.
5. `append` puts the customer at the **back** of the line.
6. It returns the customer so the caller can show the ticket.

### `serve_next`: call the next person

```python
def serve_next(self):
    """Serve the customer at the front of the queue (FIFO)."""
    if not self.waiting:
        raise IndexError("The queue is empty.")
    customer = self.waiting.popleft()
    customer["time_served"] = datetime.now()
    self.served.append(customer)
    return customer
```

1. If the line is empty, raise `IndexError`.
2. `popleft()` removes and returns the person at the **front**. This single line is the FIFO rule.
3. Record when they were served, add them to the `served` history, and return them.

### `cancel`: remove a specific ticket

```python
def cancel(self, number):
    """Remove a specific queue number from the waiting line."""
    for customer in self.waiting:
        if customer["number"] == number:
            self.waiting.remove(customer)
            return customer
    raise LookupError(f"Queue number {number} was not found.")
```

1. Walk through the line one customer at a time.
2. When a number matches, remove that customer and return them (the `return` ends the loop right away).
3. If the loop finishes with no match, raise `LookupError` ("not found").

### `peek`: look without removing

```python
def peek(self):
    """Look at who is next without removing them."""
    if not self.waiting:
        raise IndexError("The queue is empty.")
    return self.waiting[0]
```

- `self.waiting[0]` is the front of the line. Unlike `serve_next`, nobody is removed.
- Raises `IndexError` if the line is empty.

### `show_waiting`: get a copy of the line

```python
def show_waiting(self):
    return list(self.waiting)
```

- `list(self.waiting)` makes a plain list **copy**, so display code can read the line without being able to change the real queue.

## 1.3 Styling helpers

### Constants and color switch

```python
WIDTH = 58                  # total width of the dashboard box
INNER = WIDTH - 4           # usable text width inside the box

# Colors are switched off when output is piped or NO_COLOR is set.
USE_COLOR = sys.stdout.isatty() and not os.environ.get("NO_COLOR")

if os.name == "nt":
    os.system("")           # makes Windows terminals understand ANSI codes

BOLD, DIM = "1", "2"
RED, GREEN, YELLOW, CYAN = "31", "32", "33", "36"
```

- `WIDTH` is the total width of the dashboard box. `INNER` is the space inside the borders (2 border characters + 2 spaces of padding are subtracted).
- `USE_COLOR` is `True` only in a real terminal and when the `NO_COLOR` setting is not set. This stops stray color codes from appearing when output is saved to a file.
- `os.system("")` is a small trick that makes the Windows console understand color codes.
- `BOLD`, `DIM`, `RED`, etc. are the numbers of the ANSI color codes.

### `style`: add color

```python
def style(text, *codes):
    if not USE_COLOR or not codes:
        return text
    return f"\033[{';'.join(codes)}m{text}\033[0m"
```

- Wraps text in an **ANSI escape code** (for example bold green), then resets with `\033[0m`.
- If colors are off, it returns the text unchanged.

### `truncate`: shorten long text

```python
def truncate(text, limit):
    return text if len(text) <= limit else text[: limit - 1] + "…"
```

- If the text fits, return it as is. Otherwise keep the first `limit - 1` characters and add `…`, so long names never break the box.

### `human_duration`: friendly time spans

```python
def human_duration(delta):
    seconds = int(delta.total_seconds())
    if seconds < 60:
        return f"{seconds}s"
    minutes, seconds = divmod(seconds, 60)
    if minutes < 60:
        return f"{minutes}m {seconds:02d}s"
    hours, minutes = divmod(minutes, 60)
    return f"{hours}h {minutes:02d}m"
```

- Converts a time difference into total seconds, then returns `45s`, `3m 05s`, or `1h 02m`.
- `divmod(a, b)` returns the whole-number result and the remainder in one step (for example `divmod(125, 60)` is `(2, 5)`).

### `fmt`: format a customer

```python
def fmt(customer):
    return f"#{customer['number']:03d} - {customer['name']}"
```

- Produces text like `#001 - Ana`. The `:03d` pads the number with zeros to 3 digits.

## 1.4 Box drawing (the dashboard)

### `border`: horizontal lines

```python
def border(left, right):
    print(style(left + "─" * (WIDTH - 2) + right, CYAN))
```

- Prints a line such as `╭────────╮` or `├────────┤`. The corner characters are passed in, and `"─" * (WIDTH - 2)` repeats the dash to fill the width.

### `row`: one line inside the box

```python
def row(plain="", styled=None, center=False):
    """Print one line inside the box. `plain` is used to measure padding,
    `styled` (optional) is the colored version that is actually shown."""
    if len(plain) > INNER:
        plain, styled = truncate(plain, INNER), None
    gap = INNER - len(plain)
    left = gap // 2 if center else 0
    body = plain if styled is None else styled
    bar = style("│", CYAN)
    print(f"{bar} {' ' * left}{body}{' ' * (gap - left)} {bar}")
```

- `plain` is the text without colors; `styled` is the colored version actually shown.
- Color codes are invisible but still count as characters, so the padding is calculated from `plain` and the output uses `styled`. That keeps the right-hand `│` perfectly aligned.
- `center=True` splits the spare space between left and right.

### `status_row`: "NOW SERVING" and "NEXT IN LINE" lines

```python
def status_row(label, customer, color):
    value = fmt(customer) if customer else "—"
    value = truncate(value, INNER - 14)
    plain = f"{label:<14}{value}"
    styled = style(f"{label:<14}", DIM) + style(value, BOLD, color if customer else DIM)
    row(plain, styled)
```

- Shows the label (padded to 14 characters) and the customer, or `—` when nobody is there.

### `stats_row`: the counters line

```python
def stats_row(waiting, served, issued):
    items = [("Waiting", waiting), ("Served", served), ("Issued", issued)]
    plain = "     ".join(f"{k} {v}" for k, v in items)
    styled = "     ".join(f"{style(k, DIM)} {style(str(v), BOLD)}" for k, v in items)
    row(plain, styled, center=True)
```

- Builds `Waiting 2     Served 1     Issued 3` and centers it. Labels are dimmed, numbers are bold.

### `menu_cell` and `menu_row`: the menu

```python
def menu_cell(key, text, width):
    plain = f"[{key}] {text}"
    styled = style(f"[{key}]", BOLD, YELLOW) + f" {text}"
    return plain.ljust(width), styled + " " * (width - len(plain))
```

- Builds one entry like `[1] Get a queue number`, highlights the `[1]`, and pads it to a fixed width.

```python
def menu_row(left, right=None):
    width = 27
    lp, ls = menu_cell(*left, width)
    if right:
        rp, rs = menu_cell(*right, width)
        row(lp + rp, ls + rs)
    else:
        row(lp, ls)
```

- Places two entries side by side on one line (or one entry alone, like `[0] Exit`).

### `draw_dashboard`: puts it all together

```python
def draw_dashboard(system):
    current = system.served[-1] if system.served else None
    upcoming = system.waiting[0] if system.waiting else None

    border("╭", "╮")
    title = "QUEUE MANAGEMENT SYSTEM"
    row(title, style(title, BOLD, CYAN), center=True)
    border("├", "┤")
    status_row("NOW SERVING", current, GREEN)
    status_row("NEXT IN LINE", upcoming, YELLOW)
    border("├", "┤")
    stats_row(len(system.waiting), len(system.served), system.counter)
    border("├", "┤")
    menu_row(("1", "Get a queue number"), ("4", "See who is next"))
    menu_row(("2", "Serve next customer"), ("5", "Cancel a queue number"))
    menu_row(("3", "View waiting line"), ("6", "View served history"))
    menu_row(("0", "Exit"))
    border("╰", "╯")
```

- `current` is the **last** person served; `upcoming` is the **front** of the waiting line.
- Then it prints, top to bottom: top border, title, status rows, counters, menu, bottom border.

## 1.5 Messages, tables and prompts

### `ok` and `fail`

```python
def ok(message):
    print(f"\n  {style('✔', BOLD, GREEN)} {message}")
```
```python
def fail(message):
    print(f"\n  {style('✖', BOLD, RED)} {message}")
```

- Print a green ✔ success line or a red ✖ error line.

### `print_table`: a reusable table printer

```python
def print_table(title, headers, rows, widths):
    print(f"\n  {style(title, BOLD, CYAN)}\n")
    header = "  ".join(h.ljust(w) for h, w in zip(headers, widths))
    print("  " + style(header, DIM))
    print("  " + style("─" * len(header), DIM))
    for r in rows:
        print("  " + "  ".join(truncate(str(v), w).ljust(w) for v, w in zip(r, widths)))
```

- Prints a title, a header row, a divider, and then each row.
- Every cell is shortened with `truncate` and padded with `ljust` to its column width, so columns line up.
- `zip(a, b)` walks through two lists together (a header with its width, a value with its width).

### `print_waiting` and `print_served`

```python
def print_waiting(system):
    now = datetime.now()
    rows = [
        (pos, f"#{c['number']:03d}", c["name"], human_duration(now - c["time_joined"]))
        for pos, c in enumerate(system.show_waiting(), start=1)
    ]
    print_table(f"WAITING LINE ({len(rows)})",
                ["Pos", "Ticket", "Name", "Waiting"], rows, [3, 6, 24, 9])
```

- Builds one row per waiting customer: position, ticket, name, and how long they have waited (*now minus time_joined*).
- `enumerate(..., start=1)` numbers the rows from 1.

```python
def print_served(system):
    rows = [
        (f"#{c['number']:03d}", c["name"], f"{c['time_joined']:%H:%M:%S}",
         f"{c['time_served']:%H:%M:%S}", human_duration(c["time_served"] - c["time_joined"]))
        for c in system.served
    ]
    print_table(f"SERVED HISTORY ({len(rows)})",
                ["Ticket", "Name", "Joined", "Served", "Waited"], rows, [6, 22, 8, 8, 8])
```

- Same idea for the history: ticket, name, joined time, served time, and how long they waited.
- `:%H:%M:%S` formats a time as hours:minutes:seconds.

### `print_ticket`: the ticket card

```python
def print_ticket(customer, ahead):
    if ahead == 0:
        position = "You're next!"
    elif ahead == 1:
        position = "1 person ahead of you"
    else:
        position = f"{ahead} people ahead of you"

    lines = ["YOUR QUEUE NUMBER", f"#{customer['number']:03d}",
             truncate(customer["name"], 22), position]
    width = 26
    print()
    print("  " + style("┌" + "─" * width + "┐", GREEN))
    for i, text in enumerate(lines):
        text = text.center(width)
        text = style(text, BOLD, GREEN) if i == 1 else text
        print("  " + style("│", GREEN) + text + style("│", GREEN))
    print("  " + style("└" + "─" * width + "┘", GREEN))
```

- Chooses the message from `ahead` (how many people are in front): `You're next!`, `1 person ahead of you`, or `N people ahead of you`.
- Draws a green box and centers each line with `.center(width)`; the ticket number (line index 1) is bold.

### `ask`, `pause`, `clear_screen`

```python
def ask(prompt):
    return input(f"\n  {style('›', BOLD, CYAN)} {prompt}: ").strip()
```
```python
def pause():
    input(style("\n  Press Enter to return to the menu...", DIM))
```
```python
def clear_screen():
    if sys.stdout.isatty():
        os.system("cls" if os.name == "nt" else "clear")
    else:
        print()
```

- `ask` shows a `›` prompt, reads what you type, and strips extra spaces.
- `pause` waits for Enter so you can read the result before the screen clears.
- `clear_screen` runs `cls` (Windows) or `clear` (others), but only in a real terminal.

## 1.6 The main loop: `main()`

```python
def main():
    system = QueueSystem()

    try:
        while True:
            clear_screen()
            draw_dashboard(system)
            choice = ask("Choose an option")

            try:
                if choice == "1":
                    name = ask("Enter name")
                    c = system.generate_number(name)
                    print_ticket(c, len(system.waiting) - 1)

                elif choice == "2":
                    c = system.serve_next()
                    ok(f"Now serving: {style(fmt(c), BOLD)}")

                elif choice == "3":
                    if not system.waiting:
                        ok("No one is waiting.")
                    else:
                        print_waiting(system)

                elif choice == "4":
                    ok(f"Next in line: {style(fmt(system.peek()), BOLD)}")

                elif choice == "5":
                    if not system.waiting:
                        raise IndexError("The queue is empty.")
                    print_waiting(system)
                    try:
                        number = int(ask("Enter queue number to cancel"))
                    except ValueError:
                        raise ValueError("Please enter a valid number.")
                    c = system.cancel(number)
                    ok(f"Cancelled: {style(fmt(c), BOLD)}")

                elif choice == "6":
                    if not system.served:
                        ok("No one has been served yet.")
                    else:
                        print_served(system)

                elif choice == "0":
                    break

                else:
                    fail("Invalid option. Please choose 0-6.")

            except ValueError as e:
                fail(f"Invalid input: {e}")
            except (IndexError, LookupError) as e:
                fail(f"Error: {e}")

            pause()

    except (KeyboardInterrupt, EOFError):
        print()                     # Ctrl-C / end of input: leave cleanly

    print(f"\n  {style('Goodbye!', BOLD, CYAN)} "
          f"{len(system.served)} served, {len(system.waiting)} still waiting.\n")
```

In plain English:

```text
create an empty QueueSystem
repeat forever:
    clear the screen and draw the dashboard
    ask which option you want
    try to do that option:
        1 -> ask for a name, add to queue, show ticket
        2 -> serve the next customer
        3 -> show the waiting table (or "No one is waiting.")
        4 -> show who is next
        5 -> show the waiting table, ask for a number, cancel it
        6 -> show the served table (or "No one has been served yet.")
        0 -> stop the loop
        anything else -> "Invalid option"
    if something went wrong, show the error message
    wait for Enter
say goodbye
```

- The inner `try / except` catches **expected** problems: `ValueError` (empty name, non-numeric cancel number) and `IndexError` / `LookupError` (empty queue, unknown ticket). They are shown as friendly messages instead of crashing.
- The outer `try / except` catches Ctrl-C (`KeyboardInterrupt`) and end of input (`EOFError`), so the program exits politely.
- `break` leaves the loop when you choose `0`.

And the very last lines of the file:

```python
if __name__ == "__main__":
    main()
```

- This means "only start the program when this file is run directly, not when it is imported by another file."

---

# Part 2 : C++ (`cppQueueing.cpp`)

The C++ program does **exactly the same thing**, with the same structure and the same screen output. Only the language features differ.

### How Python ideas map to C++

| Python | C++ | Notes |
|---|---|---|
| customer `dict` | `struct Customer` | A struct is a bundle of related variables. |
| `datetime.now()` | `time(nullptr)` (type `time_t`) | Seconds since 1970; formatted with `strftime`. |
| `deque()` | `std::deque<Customer>` | Same FIFO idea. |
| `list` | `std::vector<Customer>` | Growable array for the history. |
| `ValueError` | `invalid_argument` | Bad input. |
| `IndexError`, `LookupError` | `out_of_range` | Empty queue / ticket not found. |
| `EOFError` | `struct InputEnded {}` | Our own tiny exception for "input ended". |
| `input()` | `getline(cin, text)` | Reads a whole line. |
| `str.ljust`, `str.center` | `padRight()`, `centered()` | C++ has no built-in versions, so they are written by hand. |
| `len(text)` | `vlen(text)` | Counts visible characters correctly for accented letters (UTF-8). |

## 2.1 Includes and platform setup

```cpp
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
```

- Each `#include` pulls in a library: `<iostream>` for input/output, `<string>`, `<deque>` and `<vector>` for the data, `<ctime>` for time, `<iomanip>` and `<sstream>` for formatting numbers, `<stdexcept>` for error types, `<cstdlib>` for `system` and `getenv`.
- The `#ifdef _WIN32 ... #else ... #endif` block is **compile-time choice**: on Windows it includes Windows headers, otherwise it includes Linux/macOS ones.
- `IS_TTY()` is a small macro answering "is output going to a real terminal?" on either system.

```cpp
using namespace std;
```

- Lets the code write `string` instead of `std::string`, `cout` instead of `std::cout`, and so on.

## 2.2 Queue logic

### `struct Customer`: one person in the system

```cpp
struct Customer {
    int number;
    string name;
    time_t timeJoined;
    time_t timeServed;
};
```

- Bundles a customer's `number`, `name`, and two timestamps. This is the C++ version of Python's dictionary.

### `class QueueSystem`: the brain

```cpp
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
```

The `private:` part hides `waiting`, `served` and `counter`, so outside code cannot change them directly. Only the `public:` methods can. How each method works:

| Method | How it works |
|---|---|
| `generateNumber(name)` | Finds the first and last non-space characters with `find_first_not_of` / `find_last_not_of` to trim the name. Throws `invalid_argument` if it is empty. Increments `counter`, builds a `Customer`, `push_back`s it, and returns it. |
| `serveNext()` | Throws `out_of_range` if the line is empty. Copies `front()`, removes it with `pop_front()` (the FIFO rule), sets `timeServed`, adds to `served`. |
| `cancel(number)` | Walks through the deque with an **iterator** (`it`, a pointer-like marker). On a match it copies the customer, `erase`s them, and returns. Throws `out_of_range` if not found. |
| `peek()` | Returns `waiting.front()` without removing. It is marked `const` because it does not change anything. |
| `getWaiting()`, `getServed()`, `getCounter()` | **Getters**: let the display code read the data safely. The `const&` return means "read-only, no copy". |

## 2.3 Styling helpers

### Constants and color switch

```cpp
const size_t WIDTH = 58;            // total width of the dashboard box
const size_t INNER = WIDTH - 4;     // usable text width inside the box

// Colors are switched off when output is piped or NO_COLOR is set.
bool useColor = false;

const string BOLD = "1", DIM = "2", RED = "31", GREEN = "32", YELLOW = "33", CYAN = "36";
```

- `WIDTH` is the total box width and `INNER` the space inside the borders.
- `useColor` is set in `main()`: colors are on only in a real terminal.
- `BOLD`, `DIM`, `RED`, etc. are ANSI color code numbers.

### `style`

```cpp
string style(const string& text, const string& a = "", const string& b = "") {
    if (!useColor || a.empty()) return text;
    string codes = b.empty() ? a : a + ";" + b;
    return "\033[" + codes + "m" + text + "\033[0m";
}
```

- Wraps text in an ANSI color code (joining two codes with `;` when both are given). If colors are off, it returns the text unchanged.

### Text helpers: `vlen`, `truncate`, `repeat`, `padRight`, `centered`, `trim`

```cpp
// Length in visible characters (UTF-8 aware, so names like "Peña" pad correctly).
size_t vlen(const string& s) {
    size_t n = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) n++;
    return n;
}
```

- Counts **visible characters**. In UTF-8, an accented letter like `ñ` uses 2 bytes, and the extra bytes all start with the bits `10`. Skipping those gives the true character count, so names like "Peña" do not break the box alignment.

```cpp
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
```

- Cuts text that is too long and adds `…`. It copies whole characters (1 to 4 bytes each) so it never slices a letter in half.

```cpp
string repeat(const string& s, size_t n) {
    string out;
    for (size_t i = 0; i < n; i++) out += s;
    return out;
}
```

- Repeats a piece of text `n` times (used to draw lines of `─`).

```cpp
string padRight(const string& s, size_t width) {
    size_t len = vlen(s);
    return len >= width ? s : s + string(width - len, ' ');
}
```

- Adds spaces on the right until the text reaches a given width (C++'s version of Python's `ljust`).

```cpp
string centered(const string& s, size_t width) {
    size_t len = vlen(s);
    if (len >= width) return s;
    size_t gap = width - len, left = gap / 2;
    return string(left, ' ') + s + string(gap - left, ' ');
}
```

- Adds spaces on both sides to center the text (C++'s version of Python's `center`).

```cpp
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}
```

- Removes spaces, tabs and line breaks from both ends of the text.

### Formatting helpers: `ticketNo`, `fmt`, `timeStr`, `humanDuration`

```cpp
string ticketNo(int number) {
    ostringstream out;
    out << "#" << setfill('0') << setw(3) << number;
    return out.str();
}
```

- Formats `7` as `#007`. `setw(3)` sets the width to 3 and `setfill('0')` fills the gap with zeros.

```cpp
string fmt(const Customer& c) {
    return ticketNo(c.number) + " - " + c.name;
}
```

- Formats a customer as `#007 - Ana`.

```cpp
string timeStr(time_t t) {
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t));
    return buf;
}
```

- Turns a `time_t` into `HH:MM:SS` text using `strftime`.

```cpp
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
```

- Turns seconds into `45s`, `3m 05s`, or `1h 02m` using division (`/`) for the whole part and remainder (`%`) for what is left over.

## 2.4 Box drawing (the dashboard)

```cpp
void border(const string& left, const string& right) {
    cout << style(left + repeat("─", WIDTH - 2) + right, CYAN) << "\n";
}
```

- Prints a horizontal line like `╭────╮`, with the corner characters passed in.

```cpp
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
```

- Prints one line inside the box. It measures the **plain** text for padding and prints the **styled** text, so invisible color codes never throw off the alignment. If the text is too long it is shortened and the styled version is dropped.

```cpp
void statusRow(const string& label, const Customer* c, const string& color) {
    string value = c ? truncate(fmt(*c), INNER - 14) : "—";
    string plain = padRight(label, 14) + value;
    string styled = style(padRight(label, 14), DIM) +
                    (c ? style(value, BOLD, color) : style(value, DIM));
    row(plain, styled);
}
```

- Prints rows like `NOW SERVING   #001 - Ana`. The parameter `const Customer* c` is a **pointer**: a null pointer means "nobody", which shows `—`. The `c ? a : b` form means "if `c` points to someone use `a`, otherwise use `b`".

```cpp
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
```

- Builds and centers the `Waiting / Served / Issued` counters line.

```cpp
pair<string, string> menuCell(const string& key, const string& text, size_t width) {
    string plain = "[" + key + "] " + text;
    string styled = style("[" + key + "]", BOLD, YELLOW) + " " + text;
    return {padRight(plain, width), styled + string(width - plain.size(), ' ')};
}
```

- Builds one menu entry and returns **two** strings as a `pair`: the plain padded version (for measuring) and the styled version (for printing).

```cpp
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
```

- Puts two menu entries side by side (or one alone when `k2` is empty).

```cpp
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
```

- Finds the `current` customer (last in `served`) and the `upcoming` one (front of `waiting`), using pointers, or `nullptr` when there is nobody. Then it prints the whole box top to bottom.

## 2.5 Messages, tables and prompts

```cpp
struct InputEnded {};   // thrown when stdin closes (Ctrl-D / end of file)
```

- An empty struct used as a **signal**. It is thrown when the input stream ends, and caught in `main()` to exit cleanly.

```cpp
void ok(const string& message) {
    cout << "\n  " << style("✔", BOLD, GREEN) << " " << message << "\n";
}
```
```cpp
void fail(const string& message) {
    cout << "\n  " << style("✖", BOLD, RED) << " " << message << "\n";
}
```

- Print a green ✔ success line or a red ✖ error line.

```cpp
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
```

- A reusable table printer: prints a title, a header, a divider, and then each row, with every cell shortened and padded to its column width.
- `vector<vector<string>>` is a list of rows, where each row is a list of text cells.

```cpp
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
```

- Builds one row per waiting customer (position, ticket, name, time waited) and hands the rows to `printTable`. `difftime` gives the number of seconds between two times.

```cpp
void printServed(const QueueSystem& qs) {
    vector<vector<string>> rows;
    for (const auto& c : qs.getServed())
        rows.push_back({ticketNo(c.number), c.name, timeStr(c.timeJoined),
                        timeStr(c.timeServed),
                        humanDuration(difftime(c.timeServed, c.timeJoined))});
    printTable("SERVED HISTORY (" + to_string(rows.size()) + ")",
               {"Ticket", "Name", "Joined", "Served", "Waited"}, rows, {6, 22, 8, 8, 8});
}
```

- Same idea for the history: ticket, name, joined time, served time, time waited.

```cpp
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
```

- Chooses the message from `ahead`, then draws the green ticket card with each line centered.

```cpp
string ask(const string& prompt) {
    cout << "\n  " << style("›", BOLD, CYAN) << " " << prompt << ": " << flush;
    string input;
    if (!getline(cin, input)) throw InputEnded();
    return trim(input);
}
```

- Shows the `›` prompt and reads a line with `getline`. If reading fails (input ended), it throws `InputEnded`. Otherwise it returns the trimmed text.

```cpp
void waitForEnter() {
    cout << style("\n  Press Enter to return to the menu...", DIM) << flush;
    string ignored;
    if (!getline(cin, ignored)) throw InputEnded();
}
```

- The "Press Enter" pause. (It is not called `pause` because that name already exists in a system header on Linux and macOS.)

```cpp
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
```

- Runs `cls` (Windows) or `clear` (others) in a real terminal, chosen at compile time with `#ifdef`. When output is piped, it prints a blank line instead.

## 2.6 The main loop: `main()`

```cpp
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
```

- The first block (`#ifdef _WIN32`) only compiles on Windows. It switches the console to UTF-8 so the `╭ │ ✔` characters display correctly, and turns on color-code support.
- `useColor` is set to true only in a real terminal without `NO_COLOR`.
- `QueueSystem qs;` creates the queue.
- The loop is the same as in Python: clear, draw the dashboard, ask, handle the choice inside `try`, show errors in `catch`, wait for Enter.
- `catch (const invalid_argument& e)` handles bad input; `catch (const out_of_range& e)` handles an empty queue or unknown ticket. `e.what()` is the error text.
- Option 5 has one extra safety check: `stoi` converts the text to a number, and `used != input.size()` rejects things like `12abc` (which `stoi` alone would accept as `12`).
- The outer `catch (const InputEnded&)` exits quietly when input ends.

---

## Example session

```text
╭────────────────────────────────────────────────────────╮
│                QUEUE MANAGEMENT SYSTEM                 │
├────────────────────────────────────────────────────────┤
│ NOW SERVING   #001 - Ana Reyes                         │
│ NEXT IN LINE  #002 - Ben Cruz                          │
├────────────────────────────────────────────────────────┤
│          Waiting 2     Served 1     Issued 3           │
├────────────────────────────────────────────────────────┤
│ [1] Get a queue number     [4] See who is next         │
│ [2] Serve next customer    [5] Cancel a queue number   │
│ [3] View waiting line      [6] View served history     │
│ [0] Exit                                               │
╰────────────────────────────────────────────────────────╯
```

Choosing `1` and entering a name shows a ticket:

```text
  ┌──────────────────────────┐
  │    YOUR QUEUE NUMBER     │
  │           #003           │
  │           Cara           │
  │  2 people ahead of you   │
  └──────────────────────────┘
```

Choosing `3` shows the waiting line:

```text
  Pos  Ticket  Name                      Waiting
  ────────────────────────────────────────────────
  1    #002    Ben Cruz                  0s
  2    #003    Cara                      0s
```

## Errors the program handles

| What you do | What happens |
|---|---|
| Enter an empty name | `Invalid input: Name cannot be empty.` |
| Serve, peek or cancel when no one is waiting | `Error: The queue is empty.` |
| Cancel a ticket number that is not in the line | `Error: Queue number N was not found.` |
| Type letters instead of a number when cancelling | `Invalid input: Please enter a valid number.` |
| Choose a menu option that does not exist | `Invalid option. Please choose 0-6.` |
| Press Ctrl-C or the input ends | The program says goodbye and exits instead of crashing. |

## Programming concepts used

- **Queue / FIFO data structure** (`deque`)
- **Classes and encapsulation**: data and rules grouped in `QueueSystem`; in C++ the data is `private`
- **Separation of concerns**: logic (`QueueSystem`) is kept apart from display (the UI functions)
- **Exceptions**: `try / except` (Python) and `try / catch` (C++) for error handling
- **Structs / dictionaries** to represent a customer
- **Pointers, references and iterators** (C++): `const Customer*`, `const deque<Customer>&`, walking the queue with `it`
- **Cross-platform code**: `#ifdef _WIN32`, `os.name == "nt"`
- **The same program in two languages**: a good way to compare how Python and C++ solve the same problem
