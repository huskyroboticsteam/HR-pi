import re


with open("Makefile") as f:
    contents = f.read()

lis = re.findall(r"\n(\w+)", contents)
for elem in lis:
    print(elem, end=" ")
print()