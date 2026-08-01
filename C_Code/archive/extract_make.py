import re

pattern = r"(\w+/?\w*\.c)\s+(?!\w+)"

with open("Makefile", "r") as f:
    contents = f.read()
out = re.findall(pattern, contents)

for item in out:
    if (not item=="functions.c"):
        print(item)
# print(out)