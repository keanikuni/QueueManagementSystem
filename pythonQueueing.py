import os
import sys
from collections import deque
from datetime import datetime


# ════════════════════════════════════════════════════════════════════════
#  QUEUE LOGIC
# ════════════════════════════════════════════════════════════════════════

class QueueSystem:
    def __init__(self):
        self.waiting = deque()      # FIFO queue for waiting customers
        self.served = []            # history ng served customers
        self.counter = 0            # last queue number issued

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

    def serve_next(self):
        """Serve the customer at the front of the queue (FIFO)."""
        if not self.waiting:
            raise IndexError("The queue is empty.")
        customer = self.waiting.popleft()
        customer["time_served"] = datetime.now()
        self.served.append(customer)
        return customer

    def cancel(self, number):
        """Remove a specific queue number from the waiting line."""
        for customer in self.waiting:
            if customer["number"] == number:
                self.waiting.remove(customer)
                return customer
        raise LookupError(f"Queue number {number} was not found.")

    def peek(self):
        """Look at who is next without removing them."""
        if not self.waiting:
            raise IndexError("The queue is empty.")
        return self.waiting[0]

    def show_waiting(self):
        return list(self.waiting)


# ════════════════════════════════════════════════════════════════════════
#  TERMINAL STYLING
# ════════════════════════════════════════════════════════════════════════

WIDTH = 58                  # total width of the dashboard box
INNER = WIDTH - 4           # usable text width inside the box

# Colors are switched off when output is piped or NO_COLOR is set.
USE_COLOR = sys.stdout.isatty() and not os.environ.get("NO_COLOR")

if os.name == "nt":
    os.system("")           # makes Windows terminals understand ANSI codes

BOLD, DIM = "1", "2"
RED, GREEN, YELLOW, CYAN = "31", "32", "33", "36"


def style(text, *codes):
    if not USE_COLOR or not codes:
        return text
    return f"\033[{';'.join(codes)}m{text}\033[0m"


def truncate(text, limit):
    return text if len(text) <= limit else text[: limit - 1] + "…"


def human_duration(delta):
    seconds = int(delta.total_seconds())
    if seconds < 60:
        return f"{seconds}s"
    minutes, seconds = divmod(seconds, 60)
    if minutes < 60:
        return f"{minutes}m {seconds:02d}s"
    hours, minutes = divmod(minutes, 60)
    return f"{hours}h {minutes:02d}m"


def fmt(customer):
    return f"#{customer['number']:03d} - {customer['name']}"


# ════════════════════════════════════════════════════════════════════════
#  BOX DRAWING
# ════════════════════════════════════════════════════════════════════════

def border(left, right):
    print(style(left + "─" * (WIDTH - 2) + right, CYAN))


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


def status_row(label, customer, color):
    value = fmt(customer) if customer else "—"
    value = truncate(value, INNER - 14)
    plain = f"{label:<14}{value}"
    styled = style(f"{label:<14}", DIM) + style(value, BOLD, color if customer else DIM)
    row(plain, styled)


def stats_row(waiting, served, issued):
    items = [("Waiting", waiting), ("Served", served), ("Issued", issued)]
    plain = "     ".join(f"{k} {v}" for k, v in items)
    styled = "     ".join(f"{style(k, DIM)} {style(str(v), BOLD)}" for k, v in items)
    row(plain, styled, center=True)


def menu_cell(key, text, width):
    plain = f"[{key}] {text}"
    styled = style(f"[{key}]", BOLD, YELLOW) + f" {text}"
    return plain.ljust(width), styled + " " * (width - len(plain))


def menu_row(left, right=None):
    width = 27
    lp, ls = menu_cell(*left, width)
    if right:
        rp, rs = menu_cell(*right, width)
        row(lp + rp, ls + rs)
    else:
        row(lp, ls)


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


# ════════════════════════════════════════════════════════════════════════
#  MESSAGES, TABLES, PROMPTS
# ════════════════════════════════════════════════════════════════════════

def ok(message):
    print(f"\n  {style('✔', BOLD, GREEN)} {message}")


def fail(message):
    print(f"\n  {style('✖', BOLD, RED)} {message}")


def print_table(title, headers, rows, widths):
    print(f"\n  {style(title, BOLD, CYAN)}\n")
    header = "  ".join(h.ljust(w) for h, w in zip(headers, widths))
    print("  " + style(header, DIM))
    print("  " + style("─" * len(header), DIM))
    for r in rows:
        print("  " + "  ".join(truncate(str(v), w).ljust(w) for v, w in zip(r, widths)))


def print_waiting(system):
    now = datetime.now()
    rows = [
        (pos, f"#{c['number']:03d}", c["name"], human_duration(now - c["time_joined"]))
        for pos, c in enumerate(system.show_waiting(), start=1)
    ]
    print_table(f"WAITING LINE ({len(rows)})",
                ["Pos", "Ticket", "Name", "Waiting"], rows, [3, 6, 24, 9])


def print_served(system):
    rows = [
        (f"#{c['number']:03d}", c["name"], f"{c['time_joined']:%H:%M:%S}",
         f"{c['time_served']:%H:%M:%S}", human_duration(c["time_served"] - c["time_joined"]))
        for c in system.served
    ]
    print_table(f"SERVED HISTORY ({len(rows)})",
                ["Ticket", "Name", "Joined", "Served", "Waited"], rows, [6, 22, 8, 8, 8])


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


def ask(prompt):
    return input(f"\n  {style('›', BOLD, CYAN)} {prompt}: ").strip()


def pause():
    input(style("\n  Press Enter to return to the menu...", DIM))


def clear_screen():
    if sys.stdout.isatty():
        os.system("cls" if os.name == "nt" else "clear")
    else:
        print()


# ════════════════════════════════════════════════════════════════════════
#  MAIN LOOP
# ════════════════════════════════════════════════════════════════════════

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


if __name__ == "__main__":
    main()
