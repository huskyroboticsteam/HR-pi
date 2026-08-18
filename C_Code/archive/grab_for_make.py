import re


with open("Makefile", "r") as f:
    contents = f.read()
string = ""
lis = re.findall(r"\n(\w+)", contents)
for elem in lis:
    string+=elem + " "
    # print(elem, end=" ")
# print(string)
with open("Makefile", "r") as f:
    contents = f.readlines()
contents[0] = "all: " + string
with open("Makefile", "w") as f:
    f.writelines(contents)