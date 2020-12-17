# Quick script to make a board for testing

loc = input("Where to save the board: ")
height = int(input("How many rows: "))
width = int(input("How many columns: "))

with open(loc, "w") as f:
    for i in range(height):
        f.write("."*width + "\n")
        