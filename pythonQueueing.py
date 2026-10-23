from collections import deque
from datetime import datetime


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


def fmt(customer):
    return f"#{customer['number']:03d} - {customer['name']}"


def print_menu():
    print("\n===== QUEUEING SYSTEM =====")
    print("1. Get a queue number")
    print("2. Serve next customer")
    print("3. View waiting line")
    print("4. See who is next")
    print("5. Cancel a queue number")
    print("6. View served history")
    print("0. Exit")


def main():
    system = QueueSystem()

    while True:
        print_menu()
        choice = input("Choose an option: ").strip()

        try:
            if choice == "1":
                name = input("Enter name: ")
                c = system.generate_number(name)
                print(f"Your queue number is {c['number']:03d}. "
                      f"People ahead of you: {len(system.waiting) - 1}")

            elif choice == "2":
                c = system.serve_next()
                print(f"Now serving: {fmt(c)}")

            elif choice == "3":
                line = system.show_waiting()
                if not line:
                    print("No one is waiting.")
                else:
                    print(f"Waiting ({len(line)}):")
                    for position, c in enumerate(line, start=1):
                        print(f"  {position}. {fmt(c)}")

            elif choice == "4":
                print(f"Next in line: {fmt(system.peek())}")

            elif choice == "5":
                number = int(input("Enter queue number to cancel: "))
                c = system.cancel(number)
                print(f"Cancelled: {fmt(c)}")

            elif choice == "6":
                if not system.served:
                    print("No one has been served yet.")
                else:
                    for c in system.served:
                        print(f"  {fmt(c)} (served {c['time_served']:%H:%M:%S})")

            elif choice == "0":
                print("Goodbye!")
                break

            else:
                print("Invalid option. Please choose 0-6.")

        except ValueError as e:
            # int() failures and empty names both land here
            print(f"Invalid input: {e}")
        except (IndexError, LookupError) as e:
            print(f"Error: {e}")


if __name__ == "__main__":
    main()
